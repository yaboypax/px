#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#include "px_memory.h"

#ifndef PX_VECTOR_H
#define PX_VECTOR_H
/* -------------------------------------------------------------------------


    type generic vector


   -------------------------------------------------------------------------*/

typedef struct {
    size_t size;
    size_t capacity;
    void** data;
} px_vector;

static px_vector* px_vector_create();
static void px_vector_initialize(px_vector* vector);
static void px_vector_destroy(px_vector* vector);

static void px_vector_push(px_vector* vector, void* value);
static void px_vector_pop(px_vector* vector);

static void* px_vector_get(px_vector* vector, size_t index);
static void px_vector_remove(px_vector* vector, size_t index);

static void px_vector_copy(px_vector* dest_vector, px_vector* source_vector);
static void px_vector_resize(px_vector* vector, const size_t new_size);

// ----------------------------------------------------------------------------

static px_vector* px_vector_create()
{
    px_vector* vector = (px_vector*)px_malloc(sizeof(px_vector));
    px_vector_initialize(vector);
    return vector;
}

static void px_vector_initialize(px_vector* vector)
{
    assert(vector);

    vector->data = NULL;
    vector->size = 0;
    vector->capacity = 0;
}

static void px_vector_destroy(px_vector* vector)
{
    if (vector)
    {
        px_free(vector->data);
        px_free(vector);
    }
}

static void px_vector_push(px_vector* vector, void* value)
{
    assert(vector);

    if (vector->size == vector->capacity) 
    {
        size_t new_capacity = (vector->capacity == 0) ? 1 : vector->capacity * 2;
        void** new_data = (void**)realloc(vector->data, sizeof(void*) * new_capacity);
        if (new_data)
	{
            vector->data = new_data;
            vector->capacity = new_capacity;
        }
    }

    vector->data[vector->size] = value;
    vector->size++;
}

static void px_vector_pop(px_vector* vector)
{
    assert(vector);
    if (vector->size > 0)
    {
        vector->size--;
        vector->data[vector->size] = NULL;
    }
}

static void* px_vector_get(px_vector* vector, size_t index)
{
    assert(vector);
    void* ptr;
    
    if (index < vector->size)
    {
        ptr = vector->data[index];
    }
    else
    {
	printf("OUT OF RANGE");
        ptr = NULL;	
    }
    
    return ptr;
}

static void px_vector_remove(px_vector* vector , size_t index)
{
    assert(vector);
    if (index < vector->size)
    {
	vector->data[index] = NULL;
	for (size_t i = index; i < (vector->size - 1); ++i)
    	{
	    vector->data[i] = vector->data[i+1];
	}
	vector->size--;
    }
    else
    {
	printf("OUT OF RANGE");
    }    

}

static void px_vector_copy(px_vector* dest_vector, px_vector* source_vector)
{
    assert(dest_vector && source_vector);

    dest_vector->size = source_vector->size;
    dest_vector->capacity = source_vector->capacity;

    void** new_data = (void**)px_malloc(sizeof(void*) * dest_vector->capacity);
    if (new_data) 
    {
        memcpy(new_data, source_vector->data, sizeof(void*) * source_vector->size);
        dest_vector->data = new_data;
    }
}

static void px_vector_resize(px_vector* vector, const size_t new_size)
{
    if (!vector)
        return;
    if (new_size == vector->size)
        return;

    if (new_size > vector->capacity)
    {
        size_t new_capacity = new_size * 2;
        void** new_data = (void**)realloc(vector->data, sizeof(void*) * new_capacity);
        if (new_data)
	{
            vector->data = new_data;
            vector->capacity = new_capacity;
        }
    }

    vector->size = new_size;
}

#endif