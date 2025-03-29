// This is a simple implementation of malloc.
// It uses a linked list to keep track of allocated and free memory blocks.
// It is not thread-safe and should not be used in production code.
// It is only for educational purposes.
#include <unistd.h> // For sbrk
#include <stdio.h> 
#include <string.h>
#include <pthread.h>

typedef char ALIGN[16]; // Declare a 16-byte block for header metadata

union header{    // This is the header for each block of memory
    struct{
        size_t size; // Size of the block
        unsigned is_free; // Is the block free?
        union header *next; // Pointer to the next block
    } s;
    ALIGN stub; 
};
typedef union header header_t; // Define header_t as a union of header 

// global variables
pthread_mutex_t global_malloc_lock = PTHREAD_MUTEX_INITIALIZER; // Mutex for thread safety
header_t *head = NULL, *tail = NULL; // Pointers to the head and tail of the linked list

header_t *get_free_block(size_t size) 
/*

This function returns a free block of memory

Parameters:
size: size of the memory block to be allocated

Returns:
A pointer to the free block of memory, or NULL if no free block is found

*/
{
    header_t *curr = head;
    while(curr){
        if(curr->s.is_free && curr->s.size >= size){
            return curr;
        }
        curr = curr->s.next;
    }
    return NULL;
}

void *malloc(size_t size) 
/*

This function allocates memory of the given size

Parameters:
size: size of memory to be allocated

Returns:
A pointer to the allocated memory block, or NULL if the allocation fails

*/ 
{
    size_t total_size;
    void *block;
    header_t *header;
    if(!size){
        return NULL;
    }
    pthread_mutex_lock(&global_malloc_lock);
    header = get_free_block(size);
    if(header){
        header->s.is_free = 0;
        pthread_mutex_unlock(&global_malloc_lock);
        return (void*)(header + 1);
    }
    total_size = size + sizeof(header_t);
    block = sbrk(total_size);
    if(block == (void*)-1){
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL;
    }
    header = block;
    header->s.size = size;
    header->s.is_free = 0;
    header->s.next = NULL;
    if(!head){
        head = header;
    } else {
        tail->s.next = header;
    }
    tail = header;
    pthread_mutex_unlock(&global_malloc_lock);
    return (void*)(header + 1);
}

void free(void *block) 
/*

This function frees the memory block

Parameters:
block: pointer to the memory block to be freed

Returns:
None

*/ 
{
    header_t *header, *tmp;
    void *programbreak;
    if(!block){
        return;
    }
    pthread_mutex_lock(&global_malloc_lock);
    header = (header_t*)block - 1;

    programbreak = sbrk(0);
    if((char*)block + header->s.size == programbreak){
        if(head == tail){
            head = NULL;
            tail = NULL;
        } else {
            tmp = head;
            while(tmp){
                if(tmp->s.next == tail){
                    tmp->s.next = NULL;
                    tail = tmp;
                }
                tmp = tmp->s.next;
            }
        }
        sbrk(0 - sizeof(header_t) - header->s.size);
        pthread_mutex_unlock(&global_malloc_lock);
        return;
    }
    header ->s.size = 1;
    pthread_mutex_unlock(&global_malloc_lock);
}

void *calloc(size_t num, size_t nsize) 
/*

This function allocates memory for an array of num elements of nsize bytes each

Parameters:
num: number of elements
nsize: size of each element

Returns:
A pointer to the allocated memory block, or NULL if the allocation fails

*/
{
    size_t size;
    void *block;
    if(!num || !nsize){
        return NULL;
    }
    size = num * nsize;
    if (nsize != size / num){
        return NULL;
    }
    block = malloc(size);
    if(!block){
        return NULL;
    }
    memset(block, 0, size);
    return block;
}

void *realloc(void *block, size_t size) 
/*

This function reallocates memory for the given block to the new size

Parameters:
block: pointer to the memory block to be reallocated
size: new size of the memory block

Returns:
A pointer to the reallocated memory block, or NULL if the allocation fails

*/ 
{
    header_t *header;
    void *ret;
    if (!block || !size){
        return malloc(size);
    }
    header = (header_t*)block - 1;
    if(header->s.size >= size){
        return block;
    }
    ret = malloc(size);
    if (ret){
        memcpy(ret, block, header->s.size);
        free(block);
    }
    return ret;
}