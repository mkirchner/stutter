all:
	gcc -I../../include -o vm location.c bytecode.c bytecode_index.c chunk.c mem.c main.c ../log.c value.c vm.c

test:
	gcc -I../../include -o test_bytecode_index test_bytecode_index.c bytecode_index.c mem.c ../log.c
	gcc -I../../include -o test_vm test_vm.c value.c bytecode.c bytecode_index.c mem.c ../log.c chunk.c location.c

