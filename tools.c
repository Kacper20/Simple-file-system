#include "tools.h"
void read_and_check_superblock(superblock *block, FILE *disk){
	if (fread(block, sizeof(superblock), 1, disk) != 1){
		perror("Cannot read superblock");
	}
	if (block->disk_descriptor != FILE_DESCRIPTOR){
		perror("It's not supported file system\n");
	}
	/* we have to move file pointer to the begin of i-node table */
	if (fseek(disk, BLOCK_SIZE - sizeof(superblock), SEEK_CUR) != 0){
		perror("Cannot move file pointer");
	}
}
void write_string_to_array(char *str, char arr[]){
	int i = 0;
	char temp = str[i];
	while (temp != '\0'){
		arr[i] = temp;
		i++;
		temp = str[i];
	}
	arr[i] = '\0';
}