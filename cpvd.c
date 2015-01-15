
#include "tools.h"

/* in result, file pointer is pointing to the begin of i-node bitmap */
int copy_file_to_vd(char *filename, char *vd_name){
	FILE *file_to_copy;
	FILE *disk;
	char *inode_eff_table;
	char *data_eff_table;
	char buffer[BLOCK_SIZE];
	struct stat st;
	double size_file_to_copy;
	superblock block;
	int temp;
	file_to_copy = fopen(filename, "rb");
	if (file_to_copy == NULL){
		perror("Cannot open file to copy disk");
		return -1;
	}
	stat(filename, &st);
	size_file_to_copy = st.st_size;
	disk = fopen(vd_name, "r+b");
	if (disk == NULL){
		perror("Cannot open virtual disk");
		return -1;
	}
	read_and_check_superblock(&block, disk);
	int blocks_needed_to_copy_file = ceil(size_file_to_copy / BLOCK_SIZE);
	
	if (block.free_block_number < blocks_needed_to_copy_file){
		printf("Too few space on virtual disk!");
		return -1;
	}
	if (block.free_inode_number == 0){
		printf("Too many files on virtual disk\n");
	}
	int inode_table_bytes =  ceil(block.inode_number / 8.0);/* inode_table actual bytes number (without any free space )*/
	inode_eff_table = (char *)malloc(inode_table_bytes);
	data_eff_table = (char *)malloc(inode_table_bytes);
	if (fread(inode_eff_table, inode_table_bytes, 1, disk) != 1){
		perror("Cannot read inode_table");
	}
	if (fseek(disk, BLOCK_SIZE - inode_table_bytes, SEEK_CUR) != 0){
		perror("Cannot move file pointer");
	}
	if (fread(data_eff_table, inode_table_bytes, 1, disk) != 1){
		perror("Cannot read inode_table");
	}
	if (fseek(disk, BLOCK_SIZE - inode_table_bytes, SEEK_CUR) != 0){
		perror("Cannot move file pointer");
	}
	/* First, we have to check if we have file, which name is the same as file to copy*/
	inode temp_inode;
	temp = 0;
	for (int i = 0; i < block.inode_number; i++){
		if (fread(&temp_inode, sizeof(inode), 1, disk) != 1){
			perror("Cannot read");
		}
		if (strcmp(filename, temp_inode.filename) == 0){ /* We have file like that on disk - */
			/*we have to count, have many blocks this file consisted */
			int blocks_for_old_file = ceil((double)temp_inode.filename / BLOCK_SIZE);
			if (blocks_needed_to_copy_file <= blocks_for_old_file + block.free_block_number){
				remove_file_from_vd(filename, vd_name); /* Delete it */
					/*File is removed, it's time to write new file*/
			}
			else{
				printf("New file is too big!");
				return -1;
			}
		}
		if (fseek(disk, block.inode_size - sizeof(inode), SEEK_CUR) != 0){
			perror("Cannot move file pointer");
		}
		temp++;
	}
	
	
	if (fseek(disk, 3 * BLOCK_SIZE, SEEK_SET) != 0){
		perror("Cannot move file pointer");
	}
	/* File pointer is at the beginning of inode structure table */
	unsigned char mask = 0x80;
	short *pointers_to_blocks = (short *)malloc(sizeof(short) * block.block_number);
	temp = 0; /* temporary variable, used to properly moving file pointer across inode table */
	int counter = 0;
	for (int i = 0; i < block.inode_number; i++){
		unsigned char byte = inode_eff_table[counter];
		unsigned char result = mask & byte;
		if (result == 0){
			/* We have found unused i-node */
			/* Firstly move pointer to this inode in inode-table */
			if (fseek(disk, i *block.inode_size, SEEK_CUR) != 0){
				perror("Cannot move file pointer");
			}
			inode i_node;
			write_string_to_array(filename, i_node.filename);
			i_node.size_of_file = size_file_to_copy;
			/*Now write i-node description to i_node table */
			if (fwrite(&i_node, sizeof(inode), 1, disk) != 1){
				perror("Cannot write ");
			}
			/* Now read pointers, update them, and write to the */
			if (fread(pointers_to_blocks, sizeof(short) * block.block_number, 1, disk) != 1){
				perror("Cannot read pointers to blocks ");
			}
			mask = 0x80;
			counter = 0;
			int pointer_counter = 0;
			FILE *secondDescriptor = fopen(vd_name, "r+b"); 
			temp = ceil(double(block.inode_size) * block.block_number / BLOCK_SIZE); /*Block number for inode_structure_table */
			if (fseek(secondDescriptor, 3 * BLOCK_SIZE + temp, SEEK_CUR) != 0){
				perror("Cannot move file pointer");
			}
			for(int i = 0; i < block.block_number; i++){
				byte = data_eff_table[counter];
				result = mask & byte;
				if (result == 0){/*We have found data block  - bit is 0!*/
					temp = size_file_to_copy > BLOCK_SIZE ? BLOCK_SIZE : size_file_to_copy;
					size_file_to_copy -= BLOCK_SIZE;
					if (fread(buffer, temp, 1, file_to_copy) != 1){
						perror("Cannot read pointers to blocks ");
					}
					if (fwrite(buffer, temp, 1, secondDescriptor) != 1){
						perror("Cannot read pointers to blocks ");
					}
					if (size_file_to_copy < 0) 
						break;
				}
				if (fseek(secondDescriptor, BLOCK_SIZE, SEEK_CUR) != 0){ /* Move to another block of data! */
					perror("Cannot move file pointer");
				}
				
				pointers_to_blocks[pointer_counter] = i;
				
				mask = mask >> 1;
				data_eff_table = data_eff_table[counter] | mask;
				if (mask == 0){
					mask = 0x80;
					counter ++;
				}	
			}
			if (fseek(disk, -sizeof(short) * block.block_number, SEEK_CUR) != 0){
				perror("Cannot move file pointer");
			}
			if (fwrite(pointers_to_blocks, sizeof(short) * block.block_number, 1, disk) != 1){
				perror("Cannot read pointers to blocks ");
			}
			
			/* I-node is now used by the file system - wrote information about it */
			inode_eff_table[counter] = inode_eff_table[counter] | mask;
			block.free_inode_number--;
			fclose(secondDescriptor);
			break;
		}
		mask = mask >> 1;
		if (mask == 0){
			mask = 0x80;
			counter ++;
		}
	}
	/* set file pointer at the beginning */
	if (fseek(disk, 0, SEEK_SET) != 0){
		perror("Cannot move file pointer");
	}
	if (fwrite(&block, sizeof(superblock), 1, disk) != 1){
		perror("Cannot write ");
	}
	if (fseek(disk, BLOCK_SIZE - sizeof(superblock), SEEK_CUR) != 0){
		perror("Cannot move file pointer");
	}
	if (fwrite(inode_eff_table, inode_table_bytes, 1, disk) != 1){
		perror("Cannot write ");
	}
	
	
	
	free(inode_eff_table);
	free(pointers_to_blocks);
	free(data_eff_table);
	fclose(file_to_copy);
	fclose(disk);
	return 0;
		
}
int main(int argc, char **argv){
	
	copy_file_to_vd(argv[1], argv[2]);
	
	return 0;
}