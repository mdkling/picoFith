// memory.h

#ifndef MEM5MEMORY_HEADER
#define MEM5MEMORY_HEADER

#define MEMORY_LOCK_NUMBER 0
// returns a pointer to memory of the requested size rounded up to a power of 2
// the memory returned is set to all 0s by the DMA. Any size above 128 bytes
// will not be all zeros upon return, but the DMA is working hard to finish.
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

void populateCache(u32 iLogsize);

u32 readSysTimerVal(u32 base);
void takeSpinLock(u32 lockNum);
void giveSpinLock(u32 lockNum);
void helper_sendMsg1(u32 data1);
void *fastAlloc(u32 iLogsize);

#endif
