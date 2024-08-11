#include "stdio.h"
#include <stdint.h>
#include <stdlib.h>

typedef struct px_memory_entry {
    void* pointer;
    char* file;
    uint32_t line;
};

static px_memory_entry px_memory_list[];

px_memory_entry *px_malloc(size_t size, char* file, uint32_t line)
{
    void* pointer = (void*) malloc(size);
    

    px_memory_entry entry;
    entry.file = file;
    entry.line = line;
    entry.pointer = pointer;

    if (pointer)
        return &entry;    

}

void px_free(void* pointer, char* file, uint32_t line)
{
    if (pointer)
        free(pointer);
    return;
}