#include "tools.h"
/* Function that removes file from virtual dile disk */
int remove_file_from_vd(char *filename, char *vd_name){
	FILE *disk;
	char *inode_bitmap_eff_table;
	char *data_bitmap_eff_table;
	double size_file_to_copy;
	short *pointers_to_blocks;
	superblock block;
	int temp;
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
	int inode_table_bytes =  ceil(block.inode_number / 8.0);/* inode_table actual bytes number (without any free space )*/
	inode_bitmap_eff_table = (char *)malloc(inode_table_bytes);
	if (fread(inode_bitmap_eff_table, inode_table_bytes, 1, disk) != 1){
		perror("Cannot read inode_table");
	}
	if (fseek(disk, BLOCK_SIZE - inode_table_bytes, SEEK_CUR) != 0){
		perror("Cannot move file pointer");
	}
	int data_bitmap_table_bytes =  ceil(block.block_number / 8.0);/* data bitmap actual bytes number (without any free space )*/
	data_bitmap_eff_table = (char *)malloc(data_bitmap_table_bytes);
	if (fread(data_bitmap_eff_table, data_bitmap_table_bytes, 1, disk) != 1){
		perror("Cannot read bitmap_table");
	}
	if (fseek(disk, BLOCK_SIZE - data_bitmap_table_bytes, SEEK_CUR) != 0){
		perror("Cannot move file pointer");
	}
	/* Now we're in inode table - let's found if we have file like this */
	int counter = 0;
	unsigned char mask = 0x80;
	for (int i = 0; i < block.inode_number; i++){
		unsigned char byte = inode_bitmap_eff_table[counter];
		unsigned char result = byte & mask; /* if it's > 0 - inode is in use, we could check inode table if file name is the same! */
		if (result > 0){
			inode i_node;
			if (fread(&i_node, sizeof(inode), 1, disk) != 1){
				perror("Cannot read bitmap_table");
			}
			if (strcmp(filename, i_node.filename) == 0){ /* We have file, that has to be deleted */
				block.free_inode_number ++;
				inode_bitmap_eff_table[counter] = inode_bitmap_eff_table[counter] ^ mask; /* remove it from inode bitmap */
				if (i_node.size_of_file != 0){ /* if file consist some data - delete it from data bitmap */
					if (fread(pointers_to_blocks, block.inode_size - sizeof(inode), 1, disk) != 1){
						perror("Cannot read bitmap_table");
					}
					/* Now - delete blocks from blocks bitmap */

					int blocks_count = ceil((double)i_node.size_of_file / BLOCK_SIZE); /* number of blocks that file consist */
					temp = 0;
					mask = 0x80;
					counter = 0;	
					for (int i = 0; i < block.block_number; i++){
						byte = data_bitmap_eff_table[counter];
						if (i == pointers_to_blocks[temp]){ // If we're in block to delete
							data_bitmap_eff_table[counter] = data_bitmap_eff_table[counter] ^ mask;
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
			else if (strcmp(filename, i_node.filename) != 0 || i_node.size_of_file != 0){
				if (fseek(disk, block.inode_size - sizeof(inode), SEEK_CUR) != 0){
					perror("Cannot move file pointer");
				}
			}
			
		}
		mask = mask >> 1;
		if (mask == 0){
			mask = 0x80;
			counter++;
		}	
	}
	if (fseek(disk, 0, SEEK_SET) != 0){
		perror("Cannot move file pointer");
	} 
	/* write superblock */
	if (fwrite(&block, sizeof(superblock), 1, disk) != 1){
		perror("Cannot read bitmap_table");
	}
	if (fseek(disk, BLOCK_SIZE - sizeof(superblock), SEEK_CUR) != 0){
		perror("Cannot move file pointer");
	} 
	if (fwrite(inode_bitmap_eff_table, inode_table_bytes, 1, disk) != 1){
		perror("Cannot read bitmap_table");
	}
	if (fseek(disk, BLOCK_SIZE - inode_table_bytes, SEEK_CUR) != 0){
		perror("Cannot move file pointer");
	} 
	if (fwrite(data_bitmap_eff_table, data_bitmap_table_bytes, 1, disk) != 1){
		perror("Cannot read bitmap_table");
	}
	if (fseek(disk, BLOCK_SIZE - data_bitmap_table_bytes, SEEK_CUR) != 0){
		perror("Cannot move file pointer");
	} 

	
	
	free(inode_bitmap_eff_table);
	free(data_bitmap_eff_table);
	free(pointers_to_blocks);
	
	fclose(disk);
	return 0;
	
}


int main(int argc, char **argv){
	remove_file_from_vd(argv[1], argv[2]);
	
}