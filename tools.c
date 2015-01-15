#include "tools.h"
int read_and_check_superblock(superblock *block, FILE *disk){
	if (fread(block, sizeof(superblock), 1, disk) != 1){
		perror("Cannot read superblock");
		return -1;
	}
	if (block->disk_descriptor != FILE_DESCRIPTOR){
		perror("It's not supported file system\n");
		return -1;
	}
	/* we have to move file pointer to the begin of i-node table */
	if (fseek(disk, BLOCK_SIZE - sizeof(superblock), SEEK_CUR) != 0){
		perror("Cannot move file pointer");
		return -1;
	}
	return 0;
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

void read_bitmap_blocks(int eff_size, char *inode_bitmap, char *data_bitmap, FILE *disk){
	/* Move to the begin of inode bitmap */
	if (fseek(disk, BLOCK_SIZE, SEEK_SET) != 0){
		perror("Cannot move file pointer");
	}
	if (fread(inode_bitmap, eff_size, 1, disk) != 1){
		perror("Cannot read inode_table");
	}
	if (fseek(disk, BLOCK_SIZE - eff_size, SEEK_CUR) != 0){
		perror("Cannot move file pointer");
	}
	if (fread(data_bitmap, eff_size, 1, disk) != 1){
		perror("Cannot read inode_table");
	}
	
}
void kseek(FILE *file, int offset, int mode){
	if (fseek(file, offset, mode) != 0){
		perror("Cannot move file pointer -from kseek");
	}
	
}
void kread(void *buff, int size, FILE *file){
	if(fread(buff, size, 1, file) != 1){
		perror("Cannot read file - from kread");
	}
	
}
void kwrite(void *buff, int size, FILE *file){
	if(fwrite(buff, size, 1, file) != 1){
		perror("Cannot write - from kwrite");
	}
}

int remove_file_from_vd(char *filename, char *vd_name){
	FILE *disk;
	char *inode_bitmap;
	char *data_bitmap;
	double size_file_to_copy;
	short *pointers_to_blocks;
	superblock block;
	int temp;
	unsigned char byte;
	int counter = 0;
	disk = fopen(vd_name, "r+b");
	if (disk == NULL){
		perror("Cannot open virtual disk");
		return -1;
	}
	read_and_check_superblock(&block, disk);
	if (block.free_inode_number ==  block.inode_number){
		printf("No files on virtual disk");
		return -1;
	}
	pointers_to_blocks = (short *)malloc(sizeof(short) * block.block_number);
	inode_bitmap = (char *)malloc(block.bytes_for_bitmap);
	data_bitmap = (char *)malloc(block.bytes_for_bitmap);
	read_bitmap_blocks(block.bytes_for_bitmap, inode_bitmap, data_bitmap, disk);
	kseek(disk, 3  * BLOCK_SIZE, SEEK_SET); // Move file descriptor to the inode structure table
	/* Now we're in inode table - let's found if we have file like this */
	unsigned char mask = 0x80;
	for (int i = 0; i < block.inode_number; i++){
		byte = inode_bitmap[counter];
		printf("Byte: %x", byte);
		byte = byte & mask; /* if it's > 0 - inode is in use, we could check inode table if file name is the same! */
		if (byte > 0){
			inode temp_inode;
			kread(&temp_inode, sizeof(inode), disk);
			printf("inode name: %s", temp_inode.filename);
			if (strcmp(filename, temp_inode.filename) == 0){ /* We have file, that has to be deleted */
				block.free_inode_number ++;
				inode_bitmap[counter] = inode_bitmap[counter] ^ mask; /* remove it from inode bitmap */
				if (temp_inode.size_of_file != 0){ /* if file consist some data - delete it from data bitmap */
					if (fread(pointers_to_blocks, block.inode_size - sizeof(inode), 1, disk) != 1){
						perror("Cannot read bitmap_table");
					}
					/* Now - delete blocks from blocks bitmap */

					int blocks_count = ceil((double)temp_inode.size_of_file / BLOCK_SIZE); /* number of blocks that file consist */
					temp = 0;
					mask = 0x80;
					counter = 0;	
					for (int i = 0; i < block.block_number; i++){
						byte = data_bitmap[counter];
						if (i == pointers_to_blocks[temp]){ // If we're in block to delete
							data_bitmap[counter] = data_bitmap[counter] ^ mask;
							block.free_block_number ++;
							temp++; /* take to another pointer to block */
						}
						mask = mask >> 1;
						if (mask == 0){
							mask = 0x80;
							counter++;
						}
						if (temp == blocks_count){
							break;
						}	
					}
					break;
				}
			}
			else if (strcmp(filename, temp_inode.filename) != 0 || temp_inode.size_of_file != 0){
				kseek(disk, block.inode_size - sizeof(inode), SEEK_CUR);
			}
			
		}
		mask = mask >> 1;
		if (mask == 0){
			mask = 0x80;
			counter++;
		}	
	}
	kseek(disk, 0, SEEK_SET);
	kwrite(&block, sizeof(superblock),disk); /* write superblock */
	kseek(disk, BLOCK_SIZE - sizeof(superblock), SEEK_CUR); // align
	kwrite(inode_bitmap, block.bytes_for_bitmap, disk); /* write superblock */
	kseek(disk, BLOCK_SIZE - block.bytes_for_bitmap, SEEK_CUR);
	kwrite(data_bitmap, block.bytes_for_bitmap, disk); /* write superblock */
	free(inode_bitmap);
	free(data_bitmap);
	free(pointers_to_blocks);
	
	fclose(disk);
	return 0;
	
}