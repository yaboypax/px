#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>


/* -------------------------------------------------------------------------


    type generic vector



   -------------------------------------------------------------------------*/

typedef struct {
    size_t size;
    size_t capacity;
    void** data;
} px_vector;

static px_vector* px_vector_create();
static void px_vector_destroy(px_vector* vector);

static void px_vector_push(px_vector* vector, void* value);
static void px_vector_pop_back(px_vector* vector);

static void px_vector_copy(px_vector* dest_vector, px_vector* source_vector);
static void px_vector_resize(px_vector* vector, const size_t new_size);

// ----------------------------------------------------------------------------

static px_vector* px_vector_create()
{
    px_vector* vector = (px_vector*)malloc(sizeof(px_vector));
    assert(vector);

    vector->data = NULL;
    vector->size = 0;
    vector->capacity = 0;

    return vector;
}

static void px_vector_destroy(px_vector* vector)
{
    if (vector)
    {
        free(vector->data);
        free(vector);
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

static void px_vector_pop_back(px_vector* vector)
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
        ptr = NULL;	
    }
    
    return ptr;

}

static void px_vector_copy(px_vector* dest_vector, px_vector* source_vector)
{
    assert(dest_vector && source_vector);

    dest_vector->size = source_vector->size;
    dest_vector->capacity = source_vector->capacity;

    void** new_data = (void**)malloc(sizeof(void*) * dest_vector->capacity);
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




/* -------------------------------------------------------------------------


    float vector used in px_buffer


typedef struct 
{
    size_t size;
    size_t capacity;
    float* data;
}px_fvector;

static px_fvector* px_fvector_create();
static void px_fvector_destroy(px_fvector* vector);

static void px_fvector_push(px_fvector* vector, float value);
static void px_fvector_pop_back(px_fvector* vector);

static void px_fvector_copy(px_fvector* destVector, px_fvector* sourceVector);
static void px_fvector_resize(px_fvector* vector, const size_t newSize);





static px_fvector* px_fvector_create()
{
    px_fvector* vector = (px_fvector*)malloc(sizeof(px_fvector));
    assert(vector);

    vector->data = NULL;
    vector->size = 0;
    vector->capacity = 0;

    return vector;
}

static void px_fvector_destroy(px_fvector* vector)
{
    if (vector)
    {
        free(vector->data);
        free(vector);
    }
    
}


static void px_fvector_push(px_fvector* vector, float value)
{
    assert(vector);
    
    if (vector->size == vector->capacity)
    {
        size_t newCapacity = (vector->capacity == 0) ? 1 : vector->capacity * 2;  

        float* newData = (float*)realloc(vector->data, sizeof(float*) * newCapacity);
        if (newData)
        {
            vector->data = newData;
            vector->capacity = newCapacity;
        }
    }
    
    vector->data[vector->size] = value;
    vector->size++;

    return;
}

static void px_fvector_pop_back(px_fvector* vector)
{
    assert(vector);

    vector->data[vector->size] = 0.f;
    vector->size--;
    
    return;
    
}

static void px_fvector_copy(px_fvector* destVector, px_fvector* sourceVector)
{
    assert(destVector && sourceVector);

    destVector->size = sourceVector->size;
    destVector->capacity = sourceVector->capacity;

    float* newData = (float*)malloc(sizeof(float)*destVector->capacity);
    if (newData)
    {
        memcpy(newData, sourceVector->data, sizeof(float) * sourceVector->size);
        destVector->data = newData;
    }
    return;
}

static void px_fvector_resize(px_fvector* vector, const size_t newSize)
{
    if (!vector)
        return;
    if (newSize == vector->size)
        return;

    if (newSize == vector->capacity)
    {
        size_t newCapacity = newSize * 2;  

        float* newData = (float*)realloc(vector->data, sizeof(float*) * newCapacity);
        if (newData)
        {
            vector->data = newData;
            vector->capacity = newCapacity;
        }
    }
    vector->size = newSize;
    return;
}

   -------------------------------------------------------------------------*/

