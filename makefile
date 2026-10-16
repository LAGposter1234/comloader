CC = gcc
AS = nasm

all: setup comloader test

setup:
	mkdir -p build/

comloader: main.c build/
	$(CC) main.c -o build/comloader

test: test.asm build/
	$(AS) test.asm -f bin -o build/test.com
