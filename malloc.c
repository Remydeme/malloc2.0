#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <limits.h>
#include "malloc.h"
#define LIBFOO_DLL_EXPORTED  __attribute__((visibility("default")))

// var static

static struct page array[8]; 
// 16 // 32 // 64 // 128 // 512 //




long size_page_block (int index)
{
          static int size[8] = {8, 16, 32, 64, 128, 256, 512};
          if (0 < index && index < 8)
                    return size[index]; 
          else
                    return PAGE_SIZE; 
}

void *allocate_page (size_t size)
{
          static int first = 0; 
          if (!first)
                {
                    first = 1; 
                    array[7].page_h = mmap(NULL, size + HEAD_SIZE, PROT_WRITE | PROT_READ, MAP_ANON | MAP_PRIVATE, 0, 0);
                    array[7].page_h->head_page = (pt_block)array[7].page_h; 
                    array[7].page_h->full = 0; 
                    array[7].page_h->next_h = NULL; 
                    array[7].page_h->next = (pt_block)array[7].page_h->limit; 
                    array[7].page_h->next->free = 0; 
                    array[7].page_h->next->next = NULL; 
                    array[7].page_h->size = size + HEAD_SIZE; 
                    return array[7].page_h->next->limit; 
                }
          else
                {
                    pt_page_h tmp_s = array[7].page_h; 
                    while(tmp_s->next_h)
                              tmp_s = tmp_s->next_h; 
                    tmp_s->next_h =  mmap(NULL, size + HEAD_SIZE, PROT_WRITE 
                                          | PROT_READ, MAP_ANON | MAP_PRIVATE, 0, 0);
                    tmp_s = tmp_s->next_h; 
                     tmp_s->size = size + HEAD_SIZE; 
                    tmp_s->next_h = NULL; 
                    tmp_s->next = (pt_block)array[7].page_h->limit; 
                    tmp_s->next->free = 0; 
                    return tmp_s->next->limit; 
                }
}

void *malloc(size_t size)
{
          static int first = 0; 
          size = ALIGN_BSIZE((size)); 
          if (first)
                {
                    if (size > 512)
                              return allocate_page(size); 
                    else
                              return find_block(size);  
                }
          else
                {
                    first = 1; 
                    create_pages(); 
                    if ( size > 512)
                              return allocate_page(size); 
                    else
                              return find_block(size); 
                }
}



void *find_block(size_t size)
{
		// printf("size: %ld \n::::::",size); 
		int page = 0; 
		PAGE_NUMBER(size, page) 
				pt_page_h page_head = array[page].page_h; 
		while (page_head->full)
            page_head = page_head->next_h; 
		printf("::::::::::::::: MALLOC PAGEHEAD : %p :::::::::::::::::::::::: \n", page_head); 
		pt_block block =  page_head->next; 
		while (block->next && !block->free)
				block = block->next;
		if (!(block->next && block->free))
		{
				printf("||||||||||||||||||||||||||||   NEWPAGE   |||||||||||||||||||||||||||||||||| \n"); 
				page_head->full = 1; 
				return new_page(page_head, page);
				//     page_head->free = 0; 

		}
  page_head->size = BLOCK_SIZE + size; 
		block->free = 0;
		printf (":::::::::  MALLOC   PTR ::::::::::::::: %p ::::::::::: \n", block->limit); 
		return block->limit; 
}


		__attribute__((__visibility__("default")))
void free(void *ptr)
{
		if (ptr)
		{
				printf (":::::::::  FREE  PTR ::::::::::::::: %p ::::::::::: \n", ptr); 
				int tmp = (int)ptr; 
				tmp = (tmp<<21)>>21; 
				pt_page_h head = (pt_page_h) (ptr - tmp); 
				printf("::::::::::::::: FREE PAGEHEAD : %p :::::::::::::::::::::::: \n", head); 
				head->full = 0; 
				pt_block block = (pt_block)(ptr - 16); 
				block->free = 1; 
		}
}

void *new_page(pt_page_h head, int page_number)
{
		pt_page_h tmp_s = head; 
		while (tmp_s->next_h)
				tmp_s = tmp_s->next_h; 
		printf("::::::::::::::: MALLOC PAGEHEAD : %p :::::::::::::::::::::::: \n",tmp_s); 
		tmp_s->next_h = mmap(NULL, PAGE_SIZE, PROT_WRITE 
						| PROT_READ, MAP_PRIVATE | MAP_ANON, 0, 0); 
		tmp_s = tmp_s->next_h;
		tmp_s->full = 0; 
		tmp_s->head_page = (pt_block)tmp_s; 
		tmp_s->next = (pt_block)(tmp_s->limit + size_page_block(page_number));
		tmp_s->next_h = NULL; 
		//     tmp_s->free = 1; 
		segment_page(size_page_block(page_number), tmp_s->next); 
		tmp_s->next->free = 0; 
		return tmp_s->next; 
}


void create_pages(void)
{
		for (unsigned int i = 0; i < 7; i++)
		{
				array[i].page_h = mmap(NULL, PAGE_SIZE, PROT_WRITE 
								| PROT_READ, MAP_ANON | MAP_PRIVATE, 0, 0);
				array[i].size = size_page_block(i); 
				array[i].page_h->head_page = (pt_block)array[i].page_h; 
				array[i].page_h->full = 0; 
				// array[i].page_h->free = 1;
				array[i].page_h->next = (pt_block)(array[i].page_h->limit + array[i].size);
				array[i].page_h->next_h = NULL;
		}
		for (unsigned int i = 0; i < 7; i++)
				segment_page(array[i].size, (array[i].page_h->next));
}

void segment_page(size_t size, pt_block block_add)
{ 
		size_t  counter = 1;  
		pt_block block = block_add; 
		//printf(":::::::::::::::::::::::::::: %ld ::::::::::::::::::::::::::::::::::\n", size); 
		while (counter < NBR_BLOCK(size))
		{
				block->free = 1; 
				block->next = (pt_block)(block->limit + size); 
				block = block->next; 
				counter++; 
		}
		//printf("counter : ::::::::::::::::::::::::::%ld::::::::::::::::::::: \n",counter); 
		block->next = NULL; 
}

