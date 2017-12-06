extern void *lma_sdata;
extern void *vma_sdata;
extern void *vma_edata;

const int a = 2;
int b = 3;

/* This routine is called before .data is moved to RAM, thus
   we only use the stack and/or const data */
void copy_data(void) {
	char *src = lma_sdata;
	char *dst = vma_sdata;

	while (dst != vma_edata)
		*dst++ = *src++;

	return;
}
