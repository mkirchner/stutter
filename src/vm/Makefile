all:
	gcc -g -I../../include \
		-I../../lib/gc/src \
		-o vm \
		../../lib/gc/src/gc.c \
		../ast.c \
		../eval_utils.c \
		../exc.c \
		../ir.c \
		../lexer.c \
		../list.c \
		../log.c \
		../primes.c \
		../reader.c \
		../reader_stack.c \
		../value.c \
		bytecode.c \
		bytecode_index.c \
		chunk.c \
		compiler.c \
		fnv1a.c \
		location.c \
		main.c \
		mem.c \
		object.c \
		vm.c \
		vmvalue.c \
		vmvalue_map.c \
		-ledit

test:
	gcc -g -I../../include -o test_bytecode_index test_bytecode_index.c bytecode_index.c mem.c ../log.c
	gcc -g -I../../include -o test_vm test_vm.c vmvalue.c bytecode.c bytecode_index.c mem.c ../log.c chunk.c location.c object.c fnv1a.c
	gcc -g -I ../../include -o test_vmvalue_map test_vmvalue_map.c vmvalue.c mem.c ../log.c ../primes.c fnv1a.c object.c	

