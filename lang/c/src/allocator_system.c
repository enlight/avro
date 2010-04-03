#include "allocator_system.h"
#include "allocator.h"

#include <stdlib.h>

//------------------------------------------------------------------------------
/**
*/
void
avro_allocator_system_initialize(void)
{
    g_avro_allocator.malloc  = avro_allocator_system_malloc;
    g_avro_allocator.calloc  = avro_allocator_system_calloc;
    g_avro_allocator.realloc = avro_allocator_system_realloc;
    g_avro_allocator.free    = avro_allocator_system_free;
}

//------------------------------------------------------------------------------
/**
*/
void*
avro_allocator_system_malloc(size_t size)
{
    return malloc(size);
}

//------------------------------------------------------------------------------
/**
*/
void*
avro_allocator_system_calloc(size_t count, size_t size)
{
    return calloc(count, size);
}

//------------------------------------------------------------------------------
/**
*/
void*
avro_allocator_system_realloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}

//------------------------------------------------------------------------------
/**
*/
void
avro_allocator_system_free(void *ptr)
{
    free(ptr);
}

