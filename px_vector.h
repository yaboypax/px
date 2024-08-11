#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
/* -------------------------------------------------------------------------


    float vector used in px_buffer



   -------------------------------------------------------------------------*/

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


