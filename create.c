#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

/* Function task is to create virtual disk of given size in current directory */

#define BLOCK_SIZE 4096 // SIZE OF THE BLOCKg

int create_vdisk(int size){
	FILE *disk;
	
	char buff[BLOCK_SIZE];
		
	disk = fopen("vdisk", "wb");
	if (disk == NULL){
		perror("Cannot create virtual disk");
		return -1;
	}
}
