#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <string.h>

typedef struct tps_page {
	uint32_t p_seek;
	uint16_t p_size1;
	uint16_t p_size2;
	uint16_t p_size3;
	uint16_t p_list;
	uint32_t* p_ppage;
	uint8_t p_state;
	uint8_t p_zip;
} tpsp;

typedef struct tps_file {
	char* f_name;
	uint16_t h_size;
	uint32_t f_size;
	uint32_t f_magic;
	char f_type[5];
	uint32_t l_num;
	uint32_t f_ch;
	uint32_t m_page;
	int b_count;
	uint32_t* b_start;
	uint32_t* b_end;
	int page_count;
	tpsp* page;
} tpsf;

void
print_tpsf(tpsf* f) {
	int i, x;
	printf("File: %s - %db.\n", f->f_name, f->f_size);
	for (i=0; i<f->page_count; ++i) {
		printf("Page %d(%d) (%d-%d-%d) list: %d\n", i, f->page[i].p_state, 
			f->page[i].p_size1, f->page[i].p_size2, f->page[i].p_size3,
			f->page[i].p_list);
		printf("Child page:");
		for(x=0; x<f->page[i].p_list; ++x) {
			printf(" %d ", f->page[i].p_ppage[x]);
		}
		printf("\n");
	}
}

