#include "tools.h"
/* Function copies file from vdisk to destination path */
int copy_on_disk(char *filename, char *vdisk_name, char *destination_path){
	FILE *destinationFile;
	FILE *disk;
	FILE *temp; //temporary pointer to read
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
		return -1;
	}
	if (block.inode_number == block.free_inode_number){
		printf("No file to copy!\n");
		return -1;
	}
	pointers_to_blocks = (short *)malloc(block.inode_size - sizeof(inode)); // Pointers in inode structure*/
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
		printf("wchodzac jestesmy pointerem pliku na: %lu", ftell(disk));
		temp_byte = inode_bitmap[counter];
		printf("temp byte: %x", temp_byte);
		printf("mask: %x", mask);
		temp_byte = temp_byte & mask;
		printf("temp byte po maskowaniu: %x", temp_byte);
		if (temp_byte > 0){ /* if inode is in use */
			printf("jest wiekszy od 0\n");
			kread(&temp_inode, sizeof(inode), disk); // Read inode into the memory, to check it's filename
			kread(pointers_to_blocks, block.inode_size - sizeof(inode), disk); // read pointers table
			printf("pointers[0]: %d, pointers[1]: %d", pointers_to_blocks[0], pointers_to_blocks[1]);
			printf("nazwa pliku przenoszonego to : %s\n", temp_inode.filename);
			if (strcmp(temp_inode.filename, filename) == 0){ // If it's this file 
			printf("nazwa jest taka sama\n");
			destinationFile = fopen(destination_path, "wb");/* opens file on disk */
			temp = fopen(vdisk_name, "rb");
				if (temp_inode.size_of_file > 0){
					printf("wiekszy od 0\n");
					counter = 0;
					int temp_size = temp_inode.size_of_file;
					kseek(temp, (3 + block.blocks_for_inode_table) * BLOCK_SIZE, SEEK_SET); // move to the begin of data structure table
					/* Now move to the place where first data block is !!!! */
					kseek(temp, pointers_to_blocks[counter] * BLOCK_SIZE, SEEK_CUR);
					while(temp_size > 0){ // we're always starting in the good place to read data - kseek does the work! :) 
						temp_size -= BLOCK_SIZE;
						counter ++;
						if (temp_size > 0){ // We should read all of the block
							printf("mniejszy");
							fflush(stdout);
							printf("czytajac caly blokjestem na! %lu\n", ftell(temp));
							kread(buffer, BLOCK_SIZE, temp);							
							kwrite(buffer, BLOCK_SIZE, destinationFile);
							kseek(temp,  (pointers_to_blocks[counter] - pointers_to_blocks[counter-1] -1)*BLOCK_SIZE,SEEK_CUR);
							
						}
						else{ // we should read temp_size + BLOCK_SIZE files :)
							printf("mniejszy");
							printf("czytajac %d blokjestem na! %lu\n",temp_size + BLOCK_SIZE, ftell(temp));
							fflush(stdout);
							kread(buffer, temp_size + BLOCK_SIZE, temp);
							kwrite(buffer, temp_size + BLOCK_SIZE, destinationFile);
						}
						
						
						
					}	
				}
				fclose(destinationFile);
				fclose(temp);
				break;
			}
		}
		else{
			kseek(disk, block.inode_size, SEEK_CUR);
			
		}
		printf("inold sajz to: %d", block.inode_size);
		/* with every iteration move the pointer to next inode-structure if it's wrong inode(non-active)*/
		printf("\na po przesunieciu jestesmy na %lu\n", ftell(disk));
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