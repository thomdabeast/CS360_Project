/* Util Function Code */

#include "type.h"

extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;
extern MOUNT  mounttab[5];

char *token_path[128]; 
extern int fd, dev, n;
extern int nblocks, ninodes, bmap, imap, inode_start, iblock;
extern char pathname[256], parameter[256];
extern int DEBUG;

extern int get_block(int dev, int blk, char *buf);
extern int put_block(int dev, int blk, char *buf);

int get_block(int fd, int blk, char buf[ ])
{
  lseek(fd, (long)blk*BLKSIZE, 0);
  read(fd, buf, BLKSIZE);
}
int put_block(int fd, int blk, char buf[ ])
{
  lseek(fd, (long)blk*BLKSIZE, 0);
  write(fd, buf, BLKSIZE);
}
//given a pathname, this function splits pathname at the / characters
//returns result to global variable "token_path"
void tokenizePath(char *path)
{
  char copy[128], *tokens;
  int k = 0;

  //tokenize PATH	
  strncpy(copy, path, strlen(path));
  copy[strlen(path)] = 0;
  tokens = strtok(copy, "/");

  do
  {
    printf("token %s\n", tokens);

    // alloc string
    if(token_path[k] == NULL) token_path[k] = malloc(sizeof(char) * 128);

    // reset string
    memset(token_path[k], 0, 128);
    strcpy(token_path[k], tokens);

    printf("token_path[%d]: %s\n", k, token_path[k]);
    tokens = strtok(NULL, "/");
    getchar();
  }
  while(tokens != NULL);
}

int search(INODE *inodePtr, char *name, int fd2)
{
  //search for name[0] in data blocks
  char buf2[BLKSIZE];
  char *cp;
  char dirName[200];

  get_block(fd2, inodePtr->i_block[0], buf2);
  dp = (DIR *)buf2;
  cp = buf2;

  while(cp < buf2 + BLKSIZE)
  { // reset and copy
    memset(dirName, 0, 200);
    memcpy(dirName, dp->name, dp->name_len);
    printf("Current search: %s\n", dirName);
    //check if current filename is same as file searching for
    if (strcmp(dirName, name) == 0)
  	{
  	  return dp->inode;
  	}

    cp += dp->rec_len;         // advance cp by rlen in bytes
    dp = (DIR *)cp;       // pull dp to the next DIR entry
    getchar();
  }
  return 0;
}

void print_blocks(INODE *ptr)
{
  int i = 0;
  for (i; i < 12; ++i)
  {
    if (ptr->i_block[i] != 0)
    {
      printf("i_block[%d] = %d\n", i, ptr->i_block[i]);
    }
  }
}

MINODE *iget(int dev, int ino)
{
  MINODE *mp;
  char buf2[BLKSIZE];
  int freeIndex = 0;
  int iblock;
  int ipos;

  iblock = ((ino - 1) / 8) + gp->bg_inode_table;
  ipos = (ino - 1) % 8;
  get_block(dev, iblock, buf2);
  INODE *ip = (INODE *)buf2 + ipos;

  print_blocks(ip);

  for(int i = 0; i < NMINODE; i++) //search for minode in memory
  {

    if(minode[i].ino == ino)
    {

      printf("found minode in memory\n");
      minode[i].refCount++;
      return &minode[i];
    }

    if(minode[i].ino == 0)
    {
      freeIndex = i;
      break;
    }
  }

  //if the inode isn't found in minode[]
  //store the inode in minode[] and return the address
  mp = &minode[freeIndex];
  memcpy(&mp->INODE, ip, sizeof(INODE));
  mp->dev = dev;
  mp->dirty = 0;
  mp->ino = ino;
  mp->mounted = 0;
  mp->mountptr = NULL;
  mp->refCount = 1;
  return mp;
}

int getino(int *dev, char* path)
{
  char buf2[BLKSIZE];
  MINODE *mip = proc[0].cwd;
  char* dirName;
  int inumber = 0;
  int i = 0;
  int iblock;
  int iposition;

  if (path == NULL)
  {
    *dev = mip->dev;
    return mip->ino;
  }

  if (path[0] == '/')
  {
    mip = root;
  }


  dirName = malloc(sizeof(char) * 128);

  //split path into tokens
  tokenizePath(path);

  printf("getino dev: %d\n", *dev);
  getchar();
  //loop through every path name and find the file associated
  //with each name
  i = 0;
  int temp = 0;
  INODE *inodePtr = &mip->INODE;
  while(token_path[i])
  {
    printf("token %s\n", token_path[i]);
    inumber = search(inodePtr, token_path[i], *dev);

    if(inumber == 0)
  	{
  	  printf("Cant Find %s\n", token_path[i]);
  	  break;
  	}

    printf("getino: getting inode %d\n", inumber);

    iput(mip);
    mip = iget(*dev, inumber);
    inodePtr = &mip->INODE;
    i++;
    getchar();
  }
  return inumber;
}



int iput(MINODE *mip)
{
  char buf2[BLKSIZE];
  int block;
  int pos;

  mip->refCount--;

  if(mip->refCount > 0)
    {
      return;
    }

  if(mip->dirty == 0)
    {
      return;
    }

  if(mip->dirty == 1)
    {
      block = ((mip->ino - 1) / 8) + gp->bg_inode_table;
      pos = (mip->ino - 1) % 8;
      get_block(mip->dev, block, buf2);
      INODE *ip = (INODE *)buf2 + pos;
	
      memcpy(ip, &mip->INODE, sizeof(INODE));
      put_block(mip->dev, block, buf2);
    }
  return;
}
	
