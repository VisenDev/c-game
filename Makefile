CFLAGS=                                 \
	-Wall                               \
	-Wextra                             \
	-Wpedantic                          \
	-std=c89                            \
	-Wint-conversion                    \
	-Wimplicit-fallthrough              \
	-Wshadow                            \
	-Wcast-qual                         \
	-Wconversion                        \
	-Wdouble-promotion                  \
	-Wfloat-equal                       \
	-Wformat=2                          \
	-Wno-format-nonliteral 	            \
	-Winit-self                         \
	-Wstrict-prototypes                 \
	-Wswitch-default                    \
	-Wswitch-enum                       \
	-Wundef                             \
	-Wunused-macros                     \
	-Wwrite-strings                     \
	-Wcast-align                        \
	-Wunreachable-code                  \
	-Wpointer-arith                     \
	-Wnull-dereference                  \
	-g                                  \
	-fstack-protector-all               \
	-fstack-check                       \
	-fsanitize=address,undefined        \
	-fsanitize-address-use-after-scope  \
	-ftrapv	                            \
	-rdynamic

test: test.c generated.c
	cc $(CFLAGS) test.c -o test
	./test

generated.c: stage1 structs.sexpr
	./stage1 > generated.c

stage1: stage1.c core.h Makefile
	cc $(CFLAGS) stage1.c -o stage1 -g
