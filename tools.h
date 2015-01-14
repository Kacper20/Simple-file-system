#define FILENAME_LIMIT 255
typedef struct superblock{
	int disk_descriptor; // Contains 30192 - some magic number that informs, that this is our file system
	int inode_number;
	int block_number;
	int free_inode_number;
	int free_block_number;
} superblock;

typedef struct inode{
	char filename[FILENAME_LIMIT];
	int size_of_file;
} inode;