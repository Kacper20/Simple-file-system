all:
	cc -o create create.c tools.c -lm
	cc -o lsvd lsvd.c tools.c -lm
	cc -o cpvd cpvd.c tools.c -lm
	cc -o rmvd rmvd.c tools.c -lm
