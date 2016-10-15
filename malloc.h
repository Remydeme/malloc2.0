#ifndef MALLOC_H
#define MALLOC_H

          // library 
#include <stddef.h>


// Macro

#define ALIGN_BSIZE(size) ((((((size) - 1)>>2)<<2) + 4))
#define PAGE_SIZE  sysconf(_SC_PAGE_SIZE)
#define BLOCK_SIZE  16
#define NBR_BLOCK(size) (PAGE_SIZE / ((size)  + BLOCK_SIZE))

#define PAGE_NUMBER(size, page) \
if (0 < size && size <= 8) page= 0;    \
if (8 < size && size <= 16) page= 1;  \
if (16 < size && page <= 32) page = 2;   \
if (32 < size && size <= 64) page = 3;    \
if (64 <size && size <= 128) page = 4;   \
if (128 < size && size <= 256) page = 5;\
if (256 < size && size <= 512)  page = 6; \
if (size > 512) page = 7; 


#define INDEX_ARRAY(size, page)\



          // Macro 
#define NO_MORE_SPACE 3 
#define LAST_BLOCK 1
#define FREE_BLOCK 0 // block in the middle 
          // typedef 

typedef  struct block_mem* pt_block; 
typedef struct  head* pt_page_h;
 
          // structure
struct page
{
   int size; 
	 pt_page_h page_h;  
};

struct block_mem
{
    int free; 
    pt_block next;
    char limit[4]; 
}; 

struct head
{
    int full;// 
   pt_block head_page;// save the page head add// need able to free  
    pt_block next; 
    pt_page_h next_h; // save the address head next block 
    char limit[4]; 
}; 

void *malloc(size_t size); 
          // prototype
int index_array (size_t  size);  
void segment_page(size_t size, pt_block block_add); 
          //void *new_page(const int page_number, size_t size); 
int size_page_block (int index); 
void create_pages(void); 
void segment_block_ff(pt_block tmp_s); 
void *allocate_block(int page_number); 
void free_memory(void *ptr); 
int free_block(pt_block *last_b);
void free(void *ptr); 
void *new_page(pt_page_h head, int page_number); 
void *find_block(size_t size); 
#endif 