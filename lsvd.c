#include "tools.h"


int list_vd_files(char *vd_name){
	FILE *disk;
	char *inode_eff_table;
	superblock block;
	int files_to_list;
	disk = fopen(vd_name, "rb");
	if (disk == NULL){
		perror("Cannot open virtual disk");
		return -1;
	}
	/*Read superblock into memory */
	read_and_check_superblock(&block, disk);
	/* If we have only free inodes - 0 files are in disk right now */
	if (block.free_inode_number == block.inode_number){
		printf("No files actually!\n");
		return 0;
	}
	/* we have files in file system - list it */
	files_to_list = block.inode_number - block.free_inode_number; /* number of files in file-system */
	printf("inode number: %d a free: %d\n", block.inode_number, block.free_inode_number);
	printf("%d files:\n", files_to_list);
	int inode_table_bytes =  ceil(block.inode_number / 8.0);/* inode_table actual bytes number (without any free space )*/
	inode_eff_table = (char *)malloc(inode_table_bytes);
	if (fread(inode_eff_table, inode_table_bytes, 1, disk) != 1){
		perror("Cannot read inode_table");
		return -1;
	}
	/* align it to the begin of data-blocks-bitmap */
	int bytes_to_move = BLOCK_SIZE - inode_table_bytes + BLOCK_SIZE;
	if (fseek(disk, bytes_to_move, SEEK_CUR) != 0){
		perror("Cannot move file pointer");
		return -1;
	}
	/* We are in the beginning of inode_table */
	int files_listed = 0;
	unsigned char mask = 0x80;
	int temp = 0; /* temporary variable, used to properly moving file pointer across inode table */
	int counter = 0;
	for (int i = 0; i < block.inode_number; i++){
		unsigned char byte = inode_eff_table[counter];
		unsigned char result = mask & byte;
		printf("Byte: %x", byte);
		if (result > 0){
			/* We have found inode - we should list it.*/
			/* Firstly move pointer to this inode */
			if (fseek(disk, (i - temp) *block.inode_size, SEEK_CUR) != 0){
				perror("Cannot move file pointer");
			}
			inode i_node;
			if (fread(&i_node, sizeof(inode), 1, disk) != 1){
				perror("Cannot read inode_table");
			}
			if (fseek(disk, block.inode_size - sizeof(inode), SEEK_CUR) != 0){
				perror("Cannot move file pointer");
			}
			printf("Name: %s, size: %d\n", i_node.filename, i_node.size_of_file);
			temp = i+1;
			files_listed ++;
		}
		mask = mask >> 1;
		if (mask == 0){
			mask = 0x80;
			counter ++;
		}
		if (files_listed == files_to_list){
			break;
		}
	}
	
	free(inode_eff_table);
	return 0;
	
}

int main(int argc, char **argv){
	
	if (argc == 2){
		if (strcmp(argv[1], "--help") == 0){
			printf("Funkcja uzywana do listowania plikow znajdujacych sie na wirtualnym systemie plikow\n");
			printf("USAGE: ./lsvd NAME_OD_DISK\n");
			return 0;
		} 
		else{
			list_vd_files(argv[1]);
		}
	}
	
	return 0;
}