#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>

#define FILENAME_LIMIT 256
typedef struct superblock{
	int disk_descriptor; // Contains 30192 - some magic number that informs, that this is our file system
	int blocks_for_inode_table; /* Number of blocks, which contains inode_table */
	int inode_number;
	int block_number;
	int free_inode_number;
	int free_block_number;
} superblock;

typedef struct inode{
	char filename[FILENAME_LIMIT];
	int size_of_file;
} inode;