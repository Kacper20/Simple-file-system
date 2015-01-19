#include "tools.h"
/* Function that removes file from virtual dile disk */


int main(int argc, char **argv){
	char *vmachine;
	
	if (argc  < 3){
		if (argc == 2 && strcmp(argv[2], "--help") == 0){
			printf("Uzycie:\n.%s PLIKI_DO_USUNIECIA[...] VIRTUAL FILE SYSTEM", argv[0]);
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
		if (remove_file_from_vd(*arg_pointer, vmachine) == -1){
			printf("Blad podczas usuwania\n");
		}
		arg_pointer++;
	}
	
	return 0;
}