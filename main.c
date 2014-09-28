#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <string.h>

#include "tps.h"

tpsf*
read_tps_head (tpsf* f, FILE* fp) {
	fseek(fp, sizeof(uint8_t)*4, SEEK_SET);
	// 4
	fread(&f->h_size, sizeof(uint8_t), 2, fp);
	fread(&f->f_size, sizeof(uint8_t), 4, fp);
	fread(&f->f_size, sizeof(uint8_t), 4, fp);
	// 14
	f->f_magic = (f->f_size-f->h_size)/256;
	fread(&f->f_type, sizeof(uint8_t), 4, fp);
	f->f_type[4] = '\0';
	// 18
	fseek(fp, sizeof(uint8_t)*2, SEEK_CUR);
	// 20
	fread(&f->l_num, sizeof(uint8_t), 4, fp);
	fread(&f->f_ch, sizeof(uint8_t), 4, fp);
	fread(&f->m_page, sizeof(uint8_t), 4, fp);
	// 32
	f->b_count = (f->h_size-32*sizeof(uint8_t)) / (2*sizeof(uint32_t));
	// 
	f->b_start = malloc(sizeof(uint32_t)*f->b_count);
	fread(f->b_start, sizeof(uint32_t), f->b_count, fp);

	f->b_end = malloc(sizeof(uint32_t)*f->b_count);
	fread(f->b_end, sizeof(uint32_t), f->b_count, fp);
	// 512
}

tpsp*
read_tps_page (tpsp* p, FILE* fp) {
	fread(&p->p_seek, sizeof(uint32_t), 1, fp);
	fread(&p->p_size1, sizeof(uint16_t), 1, fp);
	fread(&p->p_size2, sizeof(uint16_t), 1, fp);
	fread(&p->p_size3, sizeof(uint16_t), 1, fp);
	fread(&p->p_list, sizeof(uint16_t), 1, fp);
	fread(&p->p_state, sizeof(uint8_t), 1, fp);
	if (p->p_state == 0) {
		fread(&p->p_zip, sizeof(uint8_t), 1, fp);	
	} else {
		p->p_zip = 0;
		p->p_ppage = malloc(sizeof(uint32_t) * p->p_list);
		fread(p->p_ppage, sizeof(uint32_t), p->p_list, fp);
	}
}

tpsp*
read_tps_block (tpsf* f, FILE* fp) {
	int i;

	f->page_count = 1;
	f->page = malloc(sizeof(tpsp) * f->page_count);

	for (i=0; i<1; ++i) {
		read_tps_page(&f->page[i], fp);
	}
}

tpsf*
read_tps (char* filename) {
	FILE* fp;
	if ((fp = fopen(filename, "rb"))!=NULL) {
		tpsf *f = malloc(sizeof(tpsf));
		f->f_name = malloc(strlen(filename));
		sprintf(f->f_name, "%s", filename);

		read_tps_head(f, fp);
		read_tps_block (f, fp);

		fclose(fp);
		return f;
	} else {
		printf("File open error...\n");
		return NULL;
	}
}

void
main (void) {
	tpsf* f = read_tps ("ARE.TPS");
	print_tpsf(f);
}

