// memory.h

#ifndef MEM5MEMORY_HEADER
#define MEM5MEMORY_HEADER

// returns a pointer to memory of the requested size rounded up to a power of 2
// the memory returned is set to all 0s
void *zalloc(u32 nByte)__attribute__((malloc));
// returns memory allocated by zalloc or realloc back to the system
void  free(void *pOld);
// returns a pointer to memory of the requested size rounded up to a power of 2.
// IFF pPrior is a pointer returned from previous call to zalloc or realloc and
// nBytes is greater than the current size of the allocation. All of the
// previous data is copied to the new allocation and the new part is set to 0s.
void *realloc(void *pPrior, u32 nBytes);
// This will set bit 0 to 1 on failure. It also will free the pointer currently
// being used before grabbing a new pointer. This helps on memory constrained
// situtions where the current block could make up the larger block needed.
void *realloc2(void *pPrior, u32 nBytes);

#endif
