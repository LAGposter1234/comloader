#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#define _GNU_SOURCE
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

void *alloc_executable(const void *code, size_t code_size, size_t *out_size) {
    if (code == NULL || code_size == 0 || out_size == NULL)
        return NULL;
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0)
        return NULL;
    size_t page = (size_t)page_size;
    if (code_size > SIZE_MAX - (page - 1))
        return NULL;
    size_t alloc_size = ((code_size + page - 1) / page) * page;
    void *mem = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED)
        return NULL;
    memcpy(mem, code, code_size);
    if (mprotect(mem, alloc_size, PROT_READ | PROT_EXEC) != 0) {
        munmap(mem, alloc_size);
        return NULL;
    }
    *out_size = alloc_size;
    return mem;
}

void free_executable(void *mem, size_t alloc_size) {
    if (mem != NULL && alloc_size != 0) munmap(mem, alloc_size);
}


int main(int argc, char** argv) {
	if (argc < 2) return 1;
	FILE *file = fopen(argv[1], "rb");
	fseek(file, 0, SEEK_END);
	long filesize = ftell(file);
	fseek(file, 0, SEEK_SET);

	uint8_t* contents = malloc(filesize + 1);
	fread(contents, filesize, 1, file);
	size_t alloc_size;
	void *exe = alloc_executable(contents, filesize, &alloc_size);
	fclose(file);

	
	typedef int (*exe_func_t)(int, char **);
	exe_func_t exe_func = (exe_func_t)((char *)exe);

	return exe_func(argc--, argv++); 
}
