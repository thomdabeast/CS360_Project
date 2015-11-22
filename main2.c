//Braedon Graika
//11346399
//Will Hemenway
//11337255
//CS360 Lab7

//#include </home/braedongraika/type.h>
#include "type.h"

MINODE *root;
PROC   proc[NPROC], *running;
MOUNT  mounttab[5];

char names[64][128],*name[64];
int fd, dev, n;
int nblocks, imap, inode_table, iblock;
char pathname[256], parameter[256];
int DEBUG;

GD *ggp;
SUPER *ssp;
PROC P0;
PROC P1;
char *disk = "mydisk";


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

    gd();

    // get number of inodes
    ninodes = sp->s_inodes_count;

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
    printf("Got group desc\n");
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
    int i;
    char buf2[BLKSIZE];
    char buf3[BLKSIZE];
    DIR *ddp;
    char *ccp;
    char dirName[200];
    
    int ino, dev = running->cwd->dev;
    MINODE *mip = running->cwd;
    //check if a pathname is given and if it starts in root
    if(strcmp(pathname, "") && strcmp(pathname, "/"))
    {
      if(pathname[0] == '/')
      {
        dev = root->dev;
      }
      ino = getino(&dev, pathname);
      mip = iget(dev, ino);

      printf("ls:ino %d\n", mip->ino);
    }

    printf("mip->ino %d\n", mip->ino);

    //get inode
    int iblock = ((mip->ino - 1) / 8) + inode_table;
    int iposition = (mip->ino - 1) % 8;

    get_block(fd, iblock, buf3);
    INODE *iip = (INODE *)buf3 + iposition;
    
    printf("inode iblock[0]: %d\n", iip->i_block[0]);
    get_block(dev, iip->i_block[0], buf2);
    ccp = buf2;
    ddp = (DIR *)ccp;
    
    getchar();
    printf("\n");
    // prints off every file name associated with the given directory
    while(ccp < buf2 + BLKSIZE)
    {
        i = 0;
        memset(dirName, 0, 200); //set dirName values to 0
        memcpy(dirName, ddp->name, ddp->name_len);
        printf("%s\n", dirName);

        ccp += ddp->rec_len;         // advance cp by rlen in bytes
        ddp = (DIR *)ccp;       // pull dp to the next DIR entryt
        getchar();
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

int createDirEntry(MINODE *parent, int type, char *name)
{
    int need_len = 4 * ((8 + strlen(name) + 3) / 4), i = 0, ideal_len = 0;
    char buf[BLKSIZE];
    
    for (i = 0; i < 12; ++i)
    {
        if (parent->INODE.i_block[i] != 0)
        { // Check each dir entry
            get_block(parent->dev, parent->INODE.i_block[i], buf);
            DIR *d = (DIR *)buf;
            
            
        }
    }
}

int mymkdir(char *path)
{
    int pino;
    MINODE *mip, *pip;
    
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
    else return 0;
    
    char copy[128], *base, *dir;
    strcpy(copy, path);
    dir = dirname(path);
    base = basename(path);
    
    if (strcmp(base, "/") == 0)
    {
        if (search(&pip->INODE, dir) != -1)
        {
            printf("Directory already exists.\n");
        }
        else
        {
            //createDirEntry()
        }
    }
    
    pino = getino(&dev, base);
    printf("debug\n");
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
    if (search(&pip->INODE, dir, dev) != -1)
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
{(int(*)())mymkdir, ls, quit, menu, 0};
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



int main(int argc, char *argv[ ])
{
    char command[64];
    char path[128], input[256];
    
    if (argc > 1)
        disk = argv[1];
    init();
    mount_root();
    printf("Mounted root\n");
    do
    {
        // clear globals
        memset(command, 0, 64);
        memset(path, 0, 128);
        
        //get commands and path from user
        printf("Enter a command: ");
        gets(input);
        
        sscanf(input, "%s %s", command, path);
        
        // check for no input
        if (strcmp(command, "") == 0 || command == NULL)
        {
            continue;
        }
        
        fptr[findCommand(command)](path);
    } while(1);
}