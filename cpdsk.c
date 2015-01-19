#include "tools.h"
/* Function copies file from vdisk to destination path */
int copy_on_disk(char *filename, char *vdisk_name, char *destination_path){
	bool found = false;
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
		temp_byte = inode_bitmap[counter];
		temp_byte = temp_byte & mask;
		if (temp_byte > 0){ /* if inode is in use */
			printf("jest wiekszy od 0\n");
			kread(&temp_inode, sizeof(inode), disk); // Read inode into the memory, to check it's filename
			kread(pointers_to_blocks, block.inode_size - sizeof(inode), disk); // read pointers table
			if (strcmp(temp_inode.filename, filename) == 0){ // If it's this file 
			found = true;
			destinationFile = fopen(destination_path, "wb");/* opens file on disk */
			temp = fopen(vdisk_name, "rb");
				if (temp_inode.size_of_file > 0){
					counter = 0;
					int temp_size = temp_inode.size_of_file;
					kseek(temp, (3 + block.blocks_for_inode_table) * BLOCK_SIZE, SEEK_SET); // move to the begin of data structure table
					/* Now move to the place where first data block is !!!! */
					kseek(temp, pointers_to_blocks[counter] * BLOCK_SIZE, SEEK_CUR);
					while(temp_size > 0){ // we're always starting in the good place to read data - kseek does the work! :) 
						temp_size -= BLOCK_SIZE;
						counter ++;
						if (temp_size > 0){ // We should read all of the block
							kread(buffer, BLOCK_SIZE, temp);							
							kwrite(buffer, BLOCK_SIZE, destinationFile);
							kseek(temp,  (pointers_to_blocks[counter] - pointers_to_blocks[counter-1] -1)*BLOCK_SIZE,SEEK_CUR);
							
						}
						else{ // we should read temp_size + BLOCK_SIZE files :)
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
		/* with every iteration move the pointer to next inode-structure if it's wrong inode(non-active)*/
		printf("\na po przesunieciu jestesmy na %lu\n", ftell(disk));
		mask = mask >> 1; /* check next bit */
		if (mask == 0){/* end of byte, move to another */
			mask = 0x80;
			counter++;
		}
	}
	if (found == true){
		printf("Plik zostal skopiowany z systemu plikow\n");
	}
	else{
		printf("No file to copy\n");
	}
	fclose(disk);
	free(inode_bitmap);
	free(data_bitmap);
	free(pointers_to_blocks);
	return 0;
}

int main(int argc, char **argv){
	char *vmachine;
	
	if (argc  < 3){
		if (argc == 2 && strcmp(argv[2], "--help") == 0){
			printf("Uzycie:\n.%s PLIKI_DO_SKOPIOWANIE[...] VIRTUAL FILE SYSTEM", argv[0]);
			return 0;
		}
		else{
			printf("Nieprawidlowe wywolanie. wywolaj z opcja --help: %s --help\n", argv[0]);
			return -1;
		}
		
	}
	char **arg_pointer = argv;
	arg_pointer++;
	while (*arg_pointer){
		arg_pointer++;
	}
	arg_pointer--; // jestesmy na ostatnim wskazniku
	vmachine = *arg_pointer;
	printf("vmachine to : %s", vmachine);
	arg_pointer = argv;
	arg_pointer++;
	while (*arg_pointer != vmachine){
		printf("kopiujemy: %s\n", *arg_pointer);
		if (copy_on_disk(*arg_pointer, vmachine, *arg_pointer) == -1){
			printf("BLAD podczas kopiowania\n");
		}
		arg_pointer++;
	}
	
	return 0;
}