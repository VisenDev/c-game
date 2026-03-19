#generated.c: stage1 structs.sexpr
#	./stage1 > generated.c

stage1: stage1.c core.h Makefile
	cc stage1.c -o stage1 -g
	./stage1
