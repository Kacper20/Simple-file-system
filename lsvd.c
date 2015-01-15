#include "tools.h"


int list_vd_files(char *vd_name){
	FILE *disk;
	char *inode_eff_table;
	superblock block;
	int files_to_list;
	unsigned char byte;
	unsigned char mask;
	disk = fopen(vd_name, "rb");
	if (disk == NULL){
		perror("Cannot open virtual disk");
		return -1;
	}
	read_and_check_superblock(&block, disk); /*Read superblock into memory */
	/* If we have only free inodes - 0 files are in disk right now */
	if (block.free_inode_number == block.inode_number){
		printf("No files actually!\n");
		return 0;
	}
	/* we have files in file system - list it */
	files_to_list = block.inode_number - block.free_inode_number; /* number of files in file-system */
	printf("%d files:\n", files_to_list);
	inode_eff_table = (char *)malloc(block.bytes_for_bitmap);
	kread(inode_eff_table, block.bytes_for_bitmap, disk);
	kseek(disk, 3 * BLOCK_SIZE, SEEK_SET);
	/* We are in the beginning of inode_table */
	int files_listed = 0;
	mask = 0x80;
	int temp = 0; /* temporary variable, used to properly moving file pointer across inode table */
	int counter = 0;
	for (int i = 0; i < block.inode_number; i++){
		byte = inode_eff_table[counter];
		byte = mask & byte;
		printf("Byte: %x", byte);
		if (byte > 0){
			/* We have found inode - we should list it.*/
			/* Firstly move pointer to this inode */
			kseek(disk, (i - temp) * block.inode_size, SEEK_CUR);
			inode i_node;
			kread(&i_node, sizeof(inode), disk);
			kseek(disk, block.inode_size - sizeof(inode), SEEK_CUR);
			printf("Name: %s, size: %d\n", i_node.filename, i_node.size_of_file);
			temp = i+1;
			files_listed++;
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
	fclose(disk);
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