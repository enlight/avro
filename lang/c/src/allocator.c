#include "allocator.h"
#include <string.h>

struct avro_allocator_t_ g_avro_allocator = {
    NULL,   // malloc
    NULL,   // calloc
    NULL,   // realloc
    NULL    // free
};

char* 
avro_strdup(const char* source)
{
    char* dest = (char*)g_avro_allocator.malloc(strlen(source) + 1);
    strcpy(dest, source);
    return dest;
}
