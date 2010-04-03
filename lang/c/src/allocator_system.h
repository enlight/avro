#ifndef AVRO_ALLOCATOR_SYSTEM_H
#define AVRO_ALLOCATOR_SYSTEM_H

#include <stddef.h>

void  avro_allocator_system_initialize(void);
void* avro_allocator_system_malloc(size_t size);
void* avro_allocator_system_calloc(size_t count, size_t size);
void* avro_allocator_system_realloc(void *ptr, size_t size);
void  avro_allocator_system_free(void *ptr);

#endif // AVRO_ALLOCATOR_SYSTEM_H
