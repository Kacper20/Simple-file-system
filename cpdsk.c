#include "tools.h"
/* Function copies file from vdisk to destination path */
int copy_on_disk(char *filename, char *vdisk_name, char *destination_path){
	FILE *destinationFile;
	FILE *disk;
	superblock block;
	char *inode_bitmap;
	char *data_bitmap;
	short *pointers_to_blocks; 
	unsigned char mask;
	unsigned char temp_byte;
	int counter;
	inode temp_inode;
	char buffer [BLOCK_SIZE];
	/* open virtual file system  and check it's consistancy(and if it has any file, too)*/
	disk = fopen(vdisk_name, "rb");
	if (disk == NULL){
		perror("Cannot open file virtual machine");
		return -1;
	}
	if (read_and_check_superblock(&block, disk) != 0){
		
	}
	if (block.inode_number == block.free_inode_number){
		printf("No file to copy!\n");
		return -1;
	}
	pointers_to_blocks = (short *)malloc(sizeof(short) * block.block_number); // Pointers in inode structure*/
	inode_bitmap = (char *)malloc(block.bytes_for_bitmap);
	data_bitmap = (char *)malloc(block.bytes_for_bitmap);
	read_bitmap_blocks(block.bytes_for_bitmap, inode_bitmap, data_bitmap, disk);
	/* Move pointer to the beginning of inode-structure */
	kseek(disk, 3 * BLOCK_SIZE, SEEK_SET);
	/* Now we're in the beginning of inode data structure table */
	/* Read bitmap, if it's 1 on the bit - inode is in use, check name of the file */
	counter = 0;
	mask = 0x80;
	for (int i = 0; i < block.inode_number; i++){
		temp_byte = inode_bitmap[counter];
		temp_byte = temp_byte & mask;
		if (temp_byte > 0){ /* if inode is in use */
			kread(&temp_inode, sizeof(inode), disk); // Read inode into the memory, to check it's filename
			if (strcmp(temp_inode.filename, filename) == 0){ // If it's this file 
			destinationFile = fopen(destination_path, "wb");/* opens file on disk */
				if (temp_inode.size_of_file > 0){
					counter = 0;
					int temp_size = temp_inode.size_of_file;
					kread(pointers_to_blocks, block.inode_size - sizeof(inode), disk); // read pointers table
					kseek(disk, (3 + block.blocks_for_inode_table) * BLOCK_SIZE, SEEK_SET); // move to the begin of data structure table
					/* Now move to the place where first data block is !!!! */
					kseek(disk, pointers_to_blocks[counter], SEEK_CUR);
					counter ++; // move to another entry of pointers table
					while(temp_size < 0){ // we're always starting in the good place to read data - kseek does the work! :) 
						temp_size -= BLOCK_SIZE;
						if (temp_size > 0){ // We should read all of the block
							kread(buffer, BLOCK_SIZE, disk);
							kwrite(buffer, BLOCK_SIZE, destinationFile);
						}
						else{ // we should read temp_size + BLOCK_SIZE files :)
							kread(buffer, temp_size + BLOCK_SIZE, disk);
							kwrite(buffer, temp_size + BLOCK_SIZE, disk);
						}
						kseek(disk , (pointers_to_blocks[counter] - pointers_to_blocks[counter]-1) * BLOCK_SIZE, SEEK_CUR); // Move PROPERLY !!!!!
						counter ++;
					}	
				}
				fclose(destinationFile);
				break;
			}
		}
		
		/* with every iteration move the pointer to next inode-structure if it's wrong inode(non-active)*/
		kseek(disk, block.inode_size, SEEK_CUR);
		mask = mask >> 1; /* check next bit */
		if (mask == 0){/* end of byte, move to another */
			mask = 0x80;
			counter++;
		}
	}
	fclose(disk);
	free(inode_bitmap);
	free(data_bitmap);
	free(pointers_to_blocks);
	return 0;
}

int main(int argc, char **argv){
	copy_on_disk(argv[1], argv[2], argv[3]);
}