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

void
print_tpsf (tpsf* f) {
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
		//printf("%d->%d\n", le32toh(f->b_start[i]), le32toh(f->b_end[i]));
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

		fclose(fp);
	} else {
		printf("Error: can't open file.\n");
		return NULL;
	}
	return f;
}

typedef struct tps_page {
	uint16_t p_code;
	uint16_t p_encode;
	uint16_t p_encode_;
	uint16_t p_l_count;
	uint8_t p_status;
        uint8_t p_zip;
} tpsp;

typedef struct tps_mpage {
	uint32_t p_seek;
	uint16_t p_size1;
	uint16_t p_size2;
	uint16_t p_size3;
	uint16_t p_list;
	uint16_t p_state;
	uint32_t* p_page;
	uint8_t p_zip;
} tpsmp;

void
print_tpsp (tpsp* p) {
	printf("|%10d|%10d|%10d|%10d|%10d|%10d|\n", p->p_code, p->p_encode, p->p_encode_, p->p_l_count, p->p_status, p->p_zip);
	/*
	printf("coded lenght: %d\n", p->p_code);
	printf("encoded lenght: %d\n", p->p_encode);
	printf("encoded lenght: %d\n", p->p_encode_);
	printf("line count: %d\n", p->p_l_count);
	printf("page status: %d\n", p->p_status);
	printf("page zip: %d\n", p->p_zip);
	*/
}

void
free_tpsp (tpsp* p) {
	free(p);
}

tpsp*
read_tps_page (tpsf* f, unsigned int page) {
	if ( (page > f->b_count)
		|| (f->b_start[page]==f->f_magic && 
			f->b_end[page]==f->f_magic) ) {
		//printf("empty block %d\n", page);
		return NULL;
	} else {
		FILE* fp;
		if ((fp = fopen(f->f_name, "rb"))!=NULL) {
			tpsp* p = malloc(sizeof(tpsp));
			//printf("****************\n");
			//printf("PAGE %d:\n", page);
			fseek(fp, f->h_size, SEEK_SET);
			fseek(fp, f->b_start[page], SEEK_CUR);
			//fseek(fp, (f->b_start[page]-f->h_size)/256, SEEK_SET);
			fseek(fp, sizeof(uint32_t), SEEK_CUR);
			fread(&p->p_code, sizeof(uint16_t), 1, fp);
			fread(&p->p_encode, sizeof(uint16_t), 1, fp);
			fread(&p->p_encode_, sizeof(uint16_t), 1, fp);
			fread(&p->p_l_count, sizeof(uint16_t), 1, fp);
			fread(&p->p_status, sizeof(uint8_t), 1, fp);
			fread(&p->p_zip, sizeof(uint8_t), 1, fp);


			fclose(fp);
			return p;
		} else {
			printf("Error: can't open file.\n");
			return NULL;
		}
	}
}

void
free_tpsf(tpsf* f) {
	free(f->b_start);
	free(f->b_end);
	free(f);
}

void
main (void) {
	tpsf* f = read_tps ("ARE.TPS");
	print_tpsf(f);
	int i;
	tpsp* p;
	printf("___________________________________________________________________\n");
	printf("| size1    | size2    | size3    |list count|  status  |    zip   |\n");
	for (i=1; i<f->b_count; i++) {
		p = read_tps_page(f, i);
		if(p) {
			//print_tpsp(p);
			free_tpsp(p);
		}
	}

	//return;

	f->b_start[0]=0;
	p=read_tps_page(f, 0);
	print_tpsp(p);
	free_tpsp(p);

	free_tpsf(f);


	//return ;
	uint8_t byte, pbyte;
	FILE* fp = fopen("ARE.TPS", "rb");
	fseek(fp, 0, SEEK_SET);
	int count = 0;
	for (i=0; i<f->f_size; ++i) {
		pbyte = byte;
		fread(&byte, sizeof(uint8_t), 1, fp);
		if(byte==0) count++; else count=0;
		if(count==4) printf(">> %d\n", i-3);
		if(pbyte!=0xb0 &&byte==0xb0) printf(">>>%d", i);
		if(pbyte==0xb0 &&byte!=0xb0) printf("<<<%d\n", i);
		//printf("%d\n", byte);
	}
	fclose(fp);
	
}

