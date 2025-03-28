// This is a simple implementation of malloc.
// It uses a linked list to keep track of allocated and free memory blocks.
// It is not thread-safe and does not handle fragmentation.
// It is meant for educational purposes only and should not be used in production.
#include <unistd.h> // For sbrk
#include <stdio.h> 
#include <string.h>
#include <pthread.h>

typedef char ALIGN[16];

union header{
    struct{
        size_t size;
        unsigned is_free;
        union header *next;
    } s;
    ALIGN stub;
};
typedef union header header_t;

pthread_mutex_t global_malloc_lock = PTHREAD_MUTEX_INITIALIZER;
header_t *head = NULL, *tail = NULL;

header_t *get_free_block(size_t size)
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


