// This is a simple implementation of malloc using sbrk.
// It is not thread-safe and should not be used in production code.
#include <unistd.h> // For sbrk
#include <stdio.h> 
#include <stdlib.h> 
#include <stddef.h>
void *malloc(size_t size)
{
    void *block;
    block = sbrk(size);
    if (block == (void *)-1)
    {
        return NULL; // Allocation failed//-
    }
    return block; // Return the pointer to the allocated memory//-
}
