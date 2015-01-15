#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>
#define FILE_DESCRIPTOR 30192
#define FILENAME_LIMIT 256
#define BLOCK_SIZE 4096 // SIZE OF THE BLOCKg

typedef struct superblock{
	int disk_descriptor; // Contains 30192 - some magic number that informs, that this is our file system
	int blocks_for_inode_table; /* Number of blocks, which contains inode_table */
	int bytes_for_bitmap; // full number of bytes for bitmaps
	int inode_number;
	int block_number;
	int free_inode_number;
	int free_block_number;
	int inode_size; /* inode size in bytes (including numbers of data blocks)*/
} superblock;

typedef struct inode{
	char filename[FILENAME_LIMIT];
	int size_of_file;
} inode;

void write_string_to_array(char *str, char arr[]);
int read_and_check_superblock(superblock *block, FILE *disk);
void read_bitmap_blocks(int eff_size, char *inode_bitmap, char *data_bitmap, FILE *disk);
void kseek(FILE *file, int offset, int mode);
void kread(void *buff, int size, FILE *file);
void kwrite(void *buff, int size, FILE *file);
int remove_file_from_vd(char *filename, char *vd_name);