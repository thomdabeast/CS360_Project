//Braedon Graika
//11346399
//Will Hemenway
//11337255
//CS360 Lab7

#include "type.h"

MINODE minode[NMINODE];
MINODE *root;
PROC proc[NPROC], *running;
MOUNT mounttab[5];
GD *ggp;
SUPER *ssp;
char names[64][128],*name[64];
int fd, dev, n, DEBUG, inode_table;
int nblocks, ninodes, bmap, imap, iblock;
char pathname[256], parameter[256];
char *disk = "mydisk";

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

MINODE *mialloc()
{
	MINODE *temp;
	for(int i =0; i < 100; i++)
	{
		if(minode[i].refCount == 0)
		{
			temp = &minode[i];
			return temp;
		}
	}

	return temp;
}

int midealloc(MINODE *mip)
{
	mip->refCount = 0;
}

char* dirName(char *path)
{
	char *token = strtok(path, "/"),
				*temp;
	while(token)
	{
		temp = token;
		token = strtok(NULL, "/");
	}

	return temp;
}

char* baseName(char *path)
{
	char base[128];
	memset(base, path, strlen(path) - strlen(dirname));
	return base;
}

//given a pathname, this function splits pathname at the / characters
//returns a char** that contains all strings of pathname
char** tokenizePath(char *path)
{
	char* tokens2;
	char** pathTokens;
	pathTokens = malloc(sizeof *pathTokens * 128);
	int k = 1;
	tokens2 = malloc(sizeof tokens2 * 100);
	//tokenize PATH
	strcpy(tokens2, path);
	tokens2 = strtok(tokens2, "/");
	pathTokens[0] = malloc(sizeof *pathTokens[0] * 64);
	strcpy(pathTokens[0], tokens2);
	printf("Path Token[0]: %s\n", pathTokens[0]);
	while((tokens2 = strtok(NULL, "/")) != NULL)
	{
		pathTokens[k] = malloc(sizeof *pathTokens[k] * 64);
		strcpy(pathTokens[k], tokens2);
		printf("Path Token[%d]: %s\n", k, pathTokens[k]);
		k++;
	}
	return pathTokens;
}
int search2(INODE *inodePtr, char *name)
{
	//search for name[0] in data blocks
	char buf[1024];
	DIR *ddp;
	char *ccp;
	int i = 0;
	char dirName[200];
	get_block(fd, inodePtr->i_block[0], buf);
	ddp = (DIR *)buf;
	ccp = buf;
	while(ccp < buf + BLKSIZE)
	{
		i = 0;
		memset(dirName, 0, 200); //set dirName values to 0
		//get the name of the current file
		while(i < ddp->name_len)
		{
			dirName[i] = ddp->name[i];
			i++;
		}
		//check if current filename is same as file searching for
		if (strcmp(dirName, name) == 0)
		{
			return ddp->inode;
		}
		ccp += ddp->rec_len; // advance cp by rlen in bytes
		ddp = (DIR *)ccp; // pull dp to the next DIR entry
	}
	return -1;
}

int getino2(int *dev, char* path)
{
	char buf5[BLKSIZE];
	INODE *ip;
	char** name;
	char* dirName;
	int inumber = 0;
	int i = 0;
	int iblock;
	int iposition;
	dirName = malloc(sizeof *dirName * 128);
	//split path into tokens
	name = tokenizePath(path);
	// read gd block
	get_block(fd, 2, buf5);
	ggp = (GD *)buf5;
	int InodesBeginBlock = ggp->bg_inode_table;
	printf("inode begin: %d\n", InodesBeginBlock);
	// read SUPER block
	get_block(fd, 1, buf5);
	ssp = (SUPER *)buf5;
	// check for EXT2 magic number:
	printf("s_magic = %x\n", ssp->s_magic);
	if (ssp->s_magic != 0xEF53){
	printf("NOT an EXT2 FS\n");
	exit(1);
	}
	printf("EXT2 FS OK\n");
	//get starting inode location
	get_block(fd, InodesBeginBlock, buf5);
	ip = (INODE *)buf5 +1;
	//loop through every path name and find the file associated
	//with each name
	i = 0;
	int temp = 0;
	while(name[i] != 0)
	{
		printf("----------------------------------------------\n");
		printf("name[%d]: %s\n", i, name[i]);
		printf("----------------------------------------------\n");
		printf("Press Enter To Continue\n");
		getchar();
		inumber = search2(ip, name[i]);
		printf("inumber = %d\n", inumber);
		if(inumber == 0)
		{
			printf("Cant Find %s\n", name[i]);
			break;
		}
		iblock = ((inumber - 1) / 8) + InodesBeginBlock;
		iposition = (inumber - 1) % 8;
		get_block(fd, iblock, buf5);
		ip = (INODE *)buf5 + iposition;
		i++;
	}
	return inumber;
}
//function that initializes P0,P1,minode[],and root;
void init()
{
	proc[0].uid = 0;
	proc[0].cwd = 0;
	proc[1].uid = 1;
	proc[1].cwd = 0;
	//set every minode's refCount to 0
	for(int i = 0; i < NMINODE; i++)
	{
		minode[i].refCount = 0;
	}
	root = 0;
	running = &proc[0];
}

void mount_root()
{
	char buf[BLKSIZE];
	printf("disk: %s\n", disk);
	fd = open(disk, O_RDWR);
	if (fd < 0)
	{
		printf("open %s failed\n", disk);
		exit(1);
	}
	// read SUPER block
	get_block(fd, 1, buf);
	sp = (SUPER *)buf;
	// check for EXT2 magic number:
	printf("s_magic = %x\n", sp->s_magic);
	if (sp->s_magic != 0xEF53)
	{
		printf("NOT an EXT2 FS\n");
		exit(1);
	}
	printf("EXT2 FS OK\n");
	root = iget(fd, 2);
	proc[0].cwd = iget(fd, 2);
	proc[1].cwd = iget(fd, 2);
}

void gd()
{
	char buf[1024];
	// read gd block
	get_block(fd, 2, buf);
	gp = (GD *)buf;
	inode_table = gp->bg_inode_table;
}

void print_inode(INODE *ino)
{
	printf("inode->i_mode:\t%d\n", ino->i_mode);
	printf("inode->i_uid:\t%d\n", ino->i_uid);
	printf("inode->i_size:\t%d\n", ino->i_size);
	printf("inode->i_atime:\t%d\n", ino->i_atime);
	printf("inode->i_ctime:\t%d\n", ino->i_ctime);
	printf("inode->i_mtime:\t%d\n", ino->i_mtime);
	printf("inode->i_dtime:\t%d\n", ino->i_dtime);
	printf("inode->i_gid:\t%d\n", ino->i_gid);
	printf("inode->i_links_count:\t%d\n", ino->i_links_count);
}

void ls(char* pathname)
{
	printf("OK\n");
	int i;
	char buf2[1024];
	char buf3[1024];
	DIR *ddp;
	char *ccp;
	char dirName[200];
	int ino;
	dev = running->cwd->dev;
	MINODE *mip = running->cwd;
	//check if a pathname is given and if it starts in root
	if(pathname)
	{
		if(pathname[0] == '/')
		{
			dev = root->dev;
		}
		ino = getino2(&dev, pathname);
		mip = iget(dev, ino);
	}
	printf("ino: %d\n", ino);
	
	//get inode
	int iblock = ((ino - 1) / 8) + inode_table;
	int iposition = (ino - 1) % 8;

	get_block(fd, iblock, buf3);
	INODE *iip = (INODE *)buf3 + iposition;

	if (iip->i_mode == FILE_MODE)
	{
			print_inode(iip);
			return 0;
	}

	get_block(fd, iip->i_block[0], buf2);
	ddp = (DIR *)buf2;
	ccp = buf2;
	printf("\n");
	// prints off every file name associated with the given directory
	while(ccp < buf2 + BLKSIZE)
	{
		i = 0;
		memset(dirName, 0, 200); //set dirName values to 0
		//get the name of the current file
		while(i < ddp->name_len)
		{
			dirName[i] = ddp->name[i];
			i++;
		}
		printf("%s\n", dirName);
		ccp += ddp->rec_len; // advance cp by rlen in bytes
		ddp = (DIR *)ccp; // pull dp to the next DIR entry
	}
}



void cd(char *path)
{
	if (path)
	{
		
	}
	else
	{

	}
}

int mymkdir(MINODE *pip, char *name)
{

}

int createDirEntry(MINODE *parent, int ino, int type, char *name)
{

}

int mkdir(char *path)
{
	int pino;
	MINODE *mip, *pip;
	char *parent, *child;
	
	if (path)
	{
 		if (path[0] == '/')
 		{
 			mip = root;
 			dev = root->dev;
 		}
 		else
 		{
 			mip = running->cwd;
 			dev = running->cwd->dev;
 		}
 	} 
 
	child = dirName();
	parent = baseName();

	pino = getino2(&dev, parent);
  if(pino == -1)
	{
		printf("Could not find parent.\n");
		return -1;
	}
	pip = iget(dev, pino);

	// is base a dir?
	if (pip->INODE.i_mode != DIR_MODE)
	{
		printf("Parent isn't a directory.\n");
	}

	// Check if dir already exists
	if (search2(pip, child) != -1)
	{
		printf("Directory already exists.\n");
	}


}

int quit()
{
	exit(1);
}

int menu()
{
  printf("commands:\n");
  printf("ls\t[pathname]\n");
  printf("mkir\tpathname\n");
  printf("quit\nmenu\n");
}

// Function pointer for clean code
int (*fptr[])(char*) = 
	{(int(*)())mkdir, ls, quit, menu, 0};
char *cmd[] = 
	{"mkdir", "ls", "quit", "menu", 0};

int findCommand(char *command)
{
  int i = 0;
  while(cmd[i])
  {
    if(!strcmp(command, cmd[i])) return i;
    i++;
  }

  printf("\n%s isn't a command.\n", command);
  findCommand("menu");
}



main(int argc, char *argv[ ])
{
	char command[64];
	char path[128], input[256];

	if (argc > 1)
		disk = argv[1];
	init();
	mount_root();
	gd();

	do
	{
		//get commands and path from user
		printf("Enter a command: ");
		gets(input);
		sscanf(input, "%s %s", command, path);
		printf("%s\n", command);
		
		fptr[findCommand(command)](path);
	} while(1);
}