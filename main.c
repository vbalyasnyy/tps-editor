#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <string.h>

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
} tpsf;

typedef struct tps_block {
	int16_t b_code;
	int16_t b_encode;
} tpsb;

void
print_tps (tpsf* f) {
	int i;
	printf("FILE: %s\n", f->f_name);
	printf("HEADER:\n");
	printf("header size %d\n", f->h_size);
	printf("file size   %d\n", f->f_size);
	printf("file type   %s\n", f->f_type);
	printf("file magic  %d\n", f->f_magic);
	printf("last number %d\n", f->l_num);
	printf("file change %d\n", f->f_ch);
	printf("main page   %d\n", f->m_page);
	printf("block count %d\n", f->b_count);
	for (i=0; i<f->b_count; i++) {
		printf("%d->%d\n", f->b_start[i], f->b_end[i]);
		if (f->b_start[i] == f->f_magic) break;
	}
}

tpsf*
read_tps (char* filename) {
	tpsf *f = malloc(sizeof(tpsf));
	FILE* fp;
	if ((fp = fopen(filename, "rb"))!=NULL) {
		f->f_name = malloc(strlen(filename));
		sprintf(f->f_name, "%s", filename);
		fseek(fp, sizeof(uint8_t)*4, SEEK_SET);
		fread(&f->h_size, sizeof(uint8_t), 2, fp);
		fread(&f->f_size, sizeof(uint8_t), 4, fp);
		fread(&f->f_size, sizeof(uint8_t), 4, fp);
		f->f_magic = (f->f_size-f->h_size)/256;
		fread(&f->f_type, sizeof(uint8_t), 4, fp);
		f->f_type[4] = '\0';
		uint16_t ttt;
		fseek(fp, sizeof(uint8_t)*2, SEEK_CUR);
		fread(&f->l_num, sizeof(uint32_t), 1, fp);
		fread(&f->f_ch, sizeof(uint32_t), 1, fp);
		fread(&f->m_page, sizeof(uint32_t), 1, fp);
		int i;
		uint32_t f_block;
		f->b_count = (f->h_size-28*sizeof(uint8_t))/2/sizeof(uint32_t);
		f->b_start = malloc(sizeof(uint32_t)*f->b_count);
		//for (i=0; i< f->b_count/2; i+=sizeof(uint32_t)) {
			fread(f->b_start, sizeof(uint32_t), f->b_count, fp);
		//}
		f->b_end = malloc(sizeof(uint32_t)*60);
		//for (i=0; i< f->b_count/2; i+=sizeof(uint32_t)) {
			fread(f->b_end, sizeof(uint32_t), f->b_count, fp);
		//}
	} else {
		printf("Error: can't open file.\n");
		return NULL;
	}
	return f;
}

tpsb*
read_tps_block (tpsf* f, unsigned int block) {
	if ( (block > f->b_count)
		|| (f->b_start[block]==f->f_magic && 
			f->b_end[block]==f->f_magic) ) {
		printf("read bad block %d\n", block);
		return NULL;
	} else {
		tpsb* b = malloc(sizeof(tpsb));
		FILE* fp;
		if ((fp = fopen(f->f_name, "rb"))!=NULL) {
			fseek(fp, (f->b_start[block]-f->h_size)/256, SEEK_SET);
			fseek(fp, sizeof(uint32_t), SEEK_CUR);
			fread(&b->b_code, sizeof(uint16_t), 1, fp);
			fread(&b->b_encode, sizeof(uint16_t), 1, fp);
			printf(">> %d %d\n", b->b_code, b->b_encode);
		} else {
			printf("Error: can't open file.\n");
			return NULL;
		}
	}
}

void
free_tps(tpsf* f) {
	free(f->b_start);
	free(f->b_end);
	free(f);
}

void
main (void) {
	tpsf* f = read_tps ("IMA1.TPS");
	//print_tps(f);
	int i;
	for (i=0; i<60; i++)
		read_tps_block(f, i);
	free_tps(f);
}

