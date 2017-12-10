extern char lma_sdata;
extern char vma_sdata;
extern char vma_edata;

/* This routine is called before .data is moved to RAM, thus
   we only use the stack and/or const data */
void copy_data(void) {
	char *src = &lma_sdata;
	char *dst = &vma_sdata;

	while (dst != &vma_edata)
		*dst++ = *src++;

	return;
}
