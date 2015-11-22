#include "type.h"

int decFreeInodes(int dev)
{
    char buf[BLKSIZE];
    
    // dec free inodes count in SUPER and GD
    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_inodes_count--;
    put_block(dev, 1, buf);
    
    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_inodes_count--;
    put_block(dev, 2, buf);
}

int ialloc(int dev)
{
    int  i;
    char buf[BLKSIZE];
    
    // read inode_bitmap block
    get_block(dev, IBITMAP, buf);
    
    for (i=0; i < ninodes; i++){
        if (tst_bit(buf, i)==0){
            set_bit(buf,i);
            decFreeInodes(dev);
            
            put_block(dev, IBITMAP, buf);
            
            return i+1;
        }
    }
    printf("ialloc(): no more free inodes\n");
    return 0;
}

int balloc(int dev)
{
    char buffer[BLKSIZE];
    get_block(dev, BBITMAP, buffer);
    
    int  i;
    for (i = 0; i < ninodes; i++)
    {
        if (tst_bit(bmap, i) == 0)
        {
            set_bit(&bmap, i);
            put_block(dev, BBITMAP, buffer);
            return i;
        }
    }
    printf("balloc(): no more free blocks\n");
    return -1;
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