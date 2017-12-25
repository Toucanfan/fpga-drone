#ifndef RVSIM_MEM_H
#define RVSIM_MEM_H 1

#include <stdint.h>

extern void mem_init(void);
extern void mem_rom_load_flatbin(uint32_t offset, char *filepath);

/* Memory access functions
   RETURN VALUE: 0 on success, negative value on error */
extern int mem_store_byte(uint32_t addr, uint8_t value);
extern int mem_load_byte(uint32_t addr, uint8_t *result);

#endif /* RVSIM_MEM_H */
