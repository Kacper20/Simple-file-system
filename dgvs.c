#include "tools.h"
int diagnose(char *vdisk){
	FILE *disk;
	superblock block;
	char *inode_bitmap;
	char *data_bitmap;
	char buff[BLOCK_SIZE];
	unsigned char byte;
	disk = fopen(vdisk, "rb");
	
	if (disk == NULL){
		perror("Cannot open virtual disk");
		return -1;
	}
	inode temp_inode;
	printf("SUPERBLOCK!");
	read_and_check_superblock(&block, disk);
	printf("%d\n", block.disk_descriptor); // Contains 30192 - some magic number that informs, that this is our file system
	printf("%d\n", block.blocks_for_inode_table); /* Number of blocks, which contains inode_table */
	printf("%d\n", block.bytes_for_bitmap); // full number of bytes for bitmaps
	printf("%d\n", block.inode_number);
	printf("%d\n", block.block_number);
	printf("%d\n", block.free_inode_number);
	printf("%d\n", block.free_block_number);
	printf("%d\n", block.inode_size); /* inode size in bytes (including numbers of data blocks)*/
	inode_bitmap = (char *)malloc(block.bytes_for_bitmap);
	data_bitmap = (char *)malloc(block.bytes_for_bitmap);
	read_bitmap_blocks(block.bytes_for_bitmap, inode_bitmap, data_bitmap, disk);
	printf("BITMAP INODE:\n");
	for (int i = 0; i < block.bytes_for_bitmap; i++){
		byte = inode_bitmap[i];
		printf("%d bajt: %x\n",i, byte);
	}
	printf("BITMAP DATA:\n");
	for (int i = 0; i < block.bytes_for_bitmap; i++){
		byte = data_bitmap[i];
		printf("%d bajt: %x\n", i,  byte);
		
	}
	
	printf("INODE: \n");
	kseek(disk, 3 * BLOCK_SIZE, SEEK_SET);
	kread(&temp_inode, sizeof(inode), disk);
	printf("inode name %s\n", temp_inode.filename);
	printf("inode size: %d", temp_inode.size_of_file);
	kseek(disk, (3 + block.blocks_for_inode_table) * BLOCK_SIZE, SEEK_SET);
	kread(buff, BLOCK_SIZE, disk);
	printf("char[0] to: %c", buff[0]);
	return 0;
	
	
	
	
}

int main(int argc, char **argv){
	diagnose(argv[1]);
}