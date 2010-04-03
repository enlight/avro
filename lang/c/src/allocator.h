#ifndef AVRO_ALLOCATOR_H
#define AVRO_ALLOCATOR_H

#include <stddef.h>

typedef void * (*avro_malloc_def)(size_t size);
typedef void * (*avro_calloc_def)(size_t count, size_t size);
typedef void * (*avro_realloc_def)(void *ptr, size_t size);
typedef void   (*avro_free_def)(void *ptr);

struct avro_allocator_t_
{
    avro_malloc_def  malloc;
    avro_calloc_def  calloc;
    avro_realloc_def realloc;
    avro_free_def    free;
};

extern struct avro_allocator_t_ g_avro_allocator;

char* avro_strdup(const char* source);

#endif // AVRO_ALLOCATOR_H
