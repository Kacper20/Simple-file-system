#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "tools.h"

#define BLOCK_SIZE 4096 // SIZE OF THE BLOCKg
/*
 * Function task is to create virtual file system in current directory 
 * int size - size of the system in kB
 * const char *msg - name of the file system
 * returns: 1 - completed successfully
 * returns: -1 - error
 */
/*
First block - const size - always 4096 - superblock
Second block - const size - always 4096 - bitmap of inodes
Third block - const size - always 4096 - bitmap of data_blocks
Fourth - var size - space for data_blocks
*/

void fill_buffer(char *buff, int size, char number){
	int i;
	for (i = 0; i < size; i++){
		buff[i] = number;
	}
}
int create_vdisk(int size, const char *msg){
	FILE *disk;
	char buff[BLOCK_SIZE];
	int number_of_blocks = size / 4;
	int i = 0;
	superblock block;
	block.disk_descriptor = 30192;
	block.inode_number = number_of_blocks;
	block.block_number = number_of_blocks;
	block.free_inode_number = 0;
	block.free_block_number = 0;
	disk = fopen(msg, "wb");
	if (disk == NULL){
		perror("Cannot create virtual disk");
		return -1;
	}
	fill_buffer(buff, BLOCK_SIZE, 0);
	if (fwrite(&block, sizeof(superblock), 1, disk) != 1){
		perror("Cannot write bitmap on virtual disk");
	}
	/* Now we want to write another number of bytes to align it well */
	if (fwrite(&block, BLOCK_SIZE - sizeof(superblock), 1, disk) != 1){
		perror("Cannot write bitmap on virtual disk");
	}
	/*Write bitmap of inodes */
	if (fwrite(buff, BLOCK_SIZE, 1, disk) != 1){
		perror("Cannot write bitmap on virtual disk");
	}
	/* Write bitmap of data blocks */
	if (fwrite(buff, BLOCK_SIZE, 4, disk) != 1){
		perror("Cannot write bitmap on virtual disk");
	}
	/* Now we have to write i-nodes to disc.
	i-node size:
	sizeof(inode_struct) + (sizeof(short) * number_of_blocks)
	number of inodes = number of blocks on disk.
	*/
	const int sizeof_inode = sizeof(inode) + sizeof(short) * number_of_blocks;
	printf("rozmiar i-node to %d", sizeof_inode);
    
	return 0;
}
int main(int argc, char **argv){
	create_vdisk(4, "marko");
	return 0;
	
}
