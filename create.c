

#include "tools.h"
#include "errno.h"
/*
 * Function task is to create virtual file system in current directory 
 * int size - size of the system in kB
 * const char *msg - name of the file system
 * returns: 0 - completed successfully
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
int create_vdisk(int size, const char *name){
	
	
	FILE *disk;
	char buff[BLOCK_SIZE];
	int number_of_blocks = ceil(size / 4.0);
	const int sizeof_inode = sizeof(inode) + sizeof(short) * number_of_blocks;
	const int sizeof_inode_table = sizeof_inode * number_of_blocks; /* sizeof inode_table in bytes */
	int inode_blocks = ceil((double)sizeof_inode_table / BLOCK_SIZE);
	int bitmap_bytes = ceil(number_of_blocks / 8.0);/* inode_table actual bytes number (without any free space )*/
	printf("Liczba bloków: %d\n", number_of_blocks);
	printf("Liczba blokow na i-node: %d\n", inode_blocks);
	superblock block;
	block.disk_descriptor = FILE_DESCRIPTOR;
	block.blocks_for_inode_table = inode_blocks;
	block.inode_number = number_of_blocks;
	block.block_number = number_of_blocks;
	block.free_inode_number = number_of_blocks;
	block.free_block_number = number_of_blocks;
	block.inode_size = sizeof_inode;
	block.bytes_for_bitmap = bitmap_bytes;
	
	disk = fopen(name, "wb");
	if (disk == NULL){
		perror("Cannot create virtual disk");
		return -1;
	}
	fill_buffer(buff, BLOCK_SIZE , 0);
	kwrite(&block, sizeof(superblock), disk);
	/* Now we want to write another number of bytes to align it well */

	kwrite(&block, BLOCK_SIZE - sizeof(superblock), disk);
	/*Write bitmap of inodes */
	kwrite(buff, BLOCK_SIZE, disk);
	/* Write bitmap of data blocks */
	kwrite(buff, BLOCK_SIZE, disk);
	/* Now we have to write i-nodes to disc.
	i-node size:
	sizeof(inode_struct) + (sizeof(short) * number_of_blocks)
	number of inodes = number of blocks on disk.
	*/
	char *buffer2 = (char *)malloc(sizeof(char) * sizeof_inode);
	fill_buffer(buffer2, sizeof_inode, 0);
	int i;
	for (int i = 0; i < number_of_blocks; i++){
		kwrite(buffer2, sizeof_inode, disk);
	}
	free(buffer2);
	/* Align data well! */
	int empty_bytes_to_add = BLOCK_SIZE - sizeof_inode_table % BLOCK_SIZE;
	kwrite(buff, empty_bytes_to_add, disk);
	/* Now it's time to write user-data blocks.*/
	for (i = 0; i < number_of_blocks; i ++){
		kwrite(buff, BLOCK_SIZE, disk);
	}
	fclose(disk);
	return 0;
}
int main(int argc, char **argv){
	long size;
	char *end;
	char *name;
	if (argc == 2){
		if (strcmp(argv[1], "--help") != 0){
			printf("Nieprawidlowe wywolanie. Wiecej informacji: create --help\n");
			return -1;
		} 
		else{
			printf("Funkcja pozwalajaca stworzyc wirtualny dysk.\n");
			printf("Uzycie: create ROZMIAR_DYSKU[KB] NAZWA\n");
			return 0;
		}
	}
	else if (argc != 3){
		printf("Nieprawidlowe wywolanie. Wiecej informacji : create --help\n");
		return -1;
	}
	size = strtol(argv[1], &end, 10);
	if (end == argv[1] || *end != '\0' || errno == ERANGE){
		printf("nieprawidlowe wywolanie. Wiecej informacji: create --help\n");
		return -1;
	}
	name = argv[2];
	
	
	if (create_vdisk(size, name) == 0){
		printf("Disk has been created successfully\n");
	}
	else{
		printf("Disk was not created!\n");
	}
	
	return 0;
	
}
