// memory.h

#ifndef MEM5MEMORY_HEADER
#define MEM5MEMORY_HEADER

// returns a pointer to memory of the requested size rounded up to a power of 2
// the memory returned is set to all 0s
void *zalloc(int nByte)__attribute__((malloc));
// returns memory allocated by zalloc or realloc back to the system
void  free(void *pOld);
// returns a pointer to memory of the requested size rounded up to a power of 2.
// IFF pPrior is a pointer returned from previous call to zalloc or realloc and
// nBytes is greater than the current size of the allocation. All of the
// previous data is copied to the new allocation and the new part is set to 0s.
void *realloc(void *pPrior, int nBytes);

#endif
