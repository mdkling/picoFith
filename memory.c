/*
** 2007 October 14
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains the C functions that implement a memory
** allocation subsystem for use by SQLite. 
**
** This version of the memory allocation subsystem omits all
** use of malloc(). The application gives SQLite a block of memory
** before calling sqlite3_initialize() from which allocations
** are made and returned by the xMalloc() and xRealloc() 
** implementations. Once sqlite3_initialize() has been called,
** the amount of memory available to SQLite is fixed and cannot
** be changed.
**
** This version of the memory allocation subsystem is included
** in the build only if SQLITE_ENABLE_MEMSYS5 is defined.
**
** This memory allocator uses the following algorithm:
**
**   1.  All memory allocation sizes are rounded up to a power of 2.
**
**   2.  If two adjacent free blocks are the halves of a larger block,
**       then the two blocks are coalesced into the single larger block.
**
**   3.  New memory is allocated from the first available free block.
**
** This algorithm is described in: J. M. Robson. "Bounds for Some Functions
** Concerning Dynamic Storage Allocation". Journal of the Association for
** Computing Machinery, Volume 21, Number 8, July 1974, pages 491-499.
** 
** Let n be the size of the largest allocation divided by the minimum
** allocation size (after rounding all sizes up to a power of 2.)  Let M
** be the maximum amount of memory ever outstanding at one time.  Let
** N be the total amount of memory available for allocation.  Robson
** proved that this memory allocator will never breakdown due to 
** fragmentation as long as the following constraint holds:
**
**      N >=  M*(1 + log2(n)/2) - n + 1
**
** The sqlite3_status() logic tracks the maximum values of n and M so
** that an application can, at any time, verify this constraint.
*/

/*
** This version of the memory allocator is used only when 
** SQLITE_ENABLE_MEMSYS5 is defined.
*/

/*
** A minimum allocation is an instance of the following structure.
** Larger allocations are an array of these structures where the
** size of the array is a power of 2.
**
** The size of this object must be a power of two.  That fact is
** verified in memsys5Init().
*/

#include "localTypes.h"
#include "memory.h"

typedef struct MemDLL MemDLL;
struct MemDLL {
  MemDLL *next;       /* Index of next free chunk */
  MemDLL *prev;       /* Index of previous free chunk */
};

void
dmaWordForwardCopy(void *src, void *dst, s32 size);
void
setZero(void *dst, s32 size);
void disableZeroizeDMA(void);
void enableZeroizeDMA(void);
void prints(u8 *string);

/*
** Maximum size of any allocation is ((1<<LOGMAX)*mem.szAtom). Since
** mem.szAtom is always at least 8 and 32-bit integers are used,
** it is not actually possible to reach this limit.
*/
//~ #define LOGMAX 30
#define LOGMAX 15 /* pico's memory is much smaller */
#define ATOM_SIZE 32

/*
** Masks used for mem.aCtrl[] elements.
*/
#define CTRL_LOGSIZE  0x1f    /* Log2 Size of this block */
#define CTRL_FREE     0x20    /* True if not checked out */


#define assert(x) 
/*
** All of the static variables used by this module are collected
** into a single structure named "mem".  This is to keep the
** static variables organized and to reduce namespace pollution
** when this module is combined with other in the amalgamation.
*/

typedef struct MemGlobal
{
	MemDLL sentinalNode; /* Sentinal Node used in free lists */
	u32 nBlock;      /* Number of szAtom sized blocks in zPool */
	u8 *zPool;       /* Memory available to be allocated */
	/*
	** Space for tracking which blocks are checked out and the size
	** of each block.  One byte per block.
	*/
	u8 *aCtrl;
	//~ MemDLL sentinalNode; /* Sentinal Node used in free lists */
	/*
	** Lists of free blocks.  aiFreelist[0] is a list of free blocks of
	** size mem.szAtom.  aiFreelist[1] holds blocks of size szAtom*2.
	** aiFreelist[2] holds free blocks of size szAtom*4.  And so forth.
	*/
	MemDLL *aiFreelist[LOGMAX+1];
	void *cached[3];
} MemGlobal;

static MemGlobal mem;

/*
** Assuming mem.zPool is divided up into an array of memLink
** structures, return a pointer to the idx-th such link.
*/
#define GET_MEMDLL(idx) ((MemDLL *)(&mem.zPool[(idx)*ATOM_SIZE]))

/*
** Unlink the chunk at mem.aPool[i] from list it is currently
** on.  It should be found on mem.aiFreelist[iLogsize].
*/

static void
memsys5Unlink(MemDLL *l){
  MemDLL *next, *prev;
  next = l->next;
  prev = l->prev;
  prev->next = next;
  next->prev = prev;
}

/*
** Link the chunk at mem.aPool[i] so that is on the iLogsize
** free list.
*/
static void
memsys5Link(MemDLL * restrict l, u32 iLogsize, MemDLL ** restrict freeList){

  l->next = freeList[iLogsize];
  l->prev = (MemDLL *)&freeList[iLogsize];
  freeList[iLogsize]->prev = l;
  freeList[iLogsize] = l;
}

/*
** Return the size of an outstanding allocation, in bytes.
** This only works for chunks that are currently checked out.
*/
static u32 memsys5Size(void *p){
  u32 iSize, i;
  i = ((u32)((u8 *)p-mem.zPool)/ATOM_SIZE);
  iSize = ATOM_SIZE * (1 << mem.aCtrl[i]);
  return iSize;
}

/*
** Return a block of memory of at least nBytes in size.
** Return NULL if unable.  Return NULL if nBytes==0.
*/
//~ void *zalloc_internal(u32 nByte)
static void *zalloc_internal(u32 iFullSz, u32 iLogsize)
{
	u32 i;           /* Index of a mem.aPool[] slot */
	u32 iBin;        /* Index into mem.aiFreelist[] */
	//~ u32 iFullSz;     /* Size of allocation rounded up to power of 2 */
	//~ u32 iLogsize;    /* Log2 of iFullSz/POW2_MIN */
	MemDLL *freeNode;

	// if nByte is 0 -> return 0 to be consistent with realloc
	//~ if (nByte==0) { return 0; }


	/* Round nByte up to the next valid power of two */
	//~ for(iFullSz=ATOM_SIZE,iLogsize=0; iFullSz<nByte; iFullSz*=2,iLogsize++){}

	/* Make sure mem.aiFreelist[iLogsize] contains at least one free
	** block.  If not, then split a block of the next larger power of
	** two in order to create a new free block of size iLogsize.
	*/
	for(iBin=iLogsize; mem.aiFreelist[iBin]==&mem.sentinalNode; iBin++){}
	if( iBin>=LOGMAX ){
		return 0;
	}
	// get free node from free list
	freeNode = mem.aiFreelist[iBin];
	// unlink the node
	memsys5Unlink(freeNode);

	// set memory to zero using DMA
	void *memory = freeNode;
	setZero(memory, iFullSz);
	i = ((u32)((u8 *)memory-mem.zPool)/ATOM_SIZE);
	mem.aCtrl[i] = iLogsize;

	while( iBin>iLogsize ){
		u32 newSize;
		iBin--;
		newSize = 1 << iBin;
		mem.aCtrl[i+newSize] = CTRL_FREE + iBin;
		memsys5Link(GET_MEMDLL(i+newSize), iBin, mem.aiFreelist);
	}

	/* Return a pointer to the allocated memory. */
	return memory;
}

/*
** Free an outstanding memory allocation.
*/
void free_internal(void *pOld)
{
	u32 iLogsize;
	u32 iBlock;
	u8 *ctrlMem = mem.aCtrl;

  /* Set iBlock to the index of the block pointed to by pOld in 
  ** the array of mem.szAtom byte blocks pointed to by mem.zPool.
  */
	//~ if (pOld==0) { return; }
	iBlock = ((u32)((u8 *)pOld-mem.zPool)/ATOM_SIZE);
	iLogsize = ctrlMem[iBlock];
	ctrlMem[iBlock] = CTRL_FREE + iLogsize;
	while(1) {
		u32 iBuddy;
		iBuddy = iBlock ^ (1<<iLogsize);
		if(iBuddy>=mem.nBlock) { break; }
		if(ctrlMem[iBuddy]!=(CTRL_FREE + iLogsize)) { break; }
		memsys5Unlink(GET_MEMDLL(iBuddy));
		iLogsize++;
		ctrlMem[iBlock&iBuddy] = CTRL_FREE + iLogsize;
		ctrlMem[iBlock|iBuddy] = 0;
		iBlock = iBlock&iBuddy;
	}
	memsys5Link(GET_MEMDLL(iBlock), iLogsize, mem.aiFreelist);
}

void free(void *pOld)
{
	if (pOld==0) { return; }
	helper_sendMsg1(1+((u32)pOld<<8));
}

void *zalloc(u32 nByte)
{
	u32 iFullSz;     /* Size of allocation rounded up to power of 2 */
	u32 iLogsize;    /* Log2 of iFullSz/POW2_MIN */
	// if nByte is 0 -> return 0 to be consistent with realloc
	if (nByte==0) { return 0; }
	/* Round nByte up to the next valid power of two */
	for(iFullSz=ATOM_SIZE,iLogsize=0; iFullSz<nByte; iFullSz*=2,iLogsize++){}
	if (iFullSz <= ATOM_SIZE*4)
	{
		void *cachedMem = mem.cached[iLogsize];
		if (cachedMem)
		{
			helper_sendMsg1(0+(iLogsize<<8));
			mem.cached[iLogsize] = 0;
			return cachedMem;
		}
	}
	takeSpinLock(MEMORY_LOCK_NUMBER);
	void *memory = zalloc_internal(iFullSz, iLogsize);
	giveSpinLock(MEMORY_LOCK_NUMBER);
	return memory;
}

void *fastAlloc(u32 iLogsize)
{
	u32 iFullSz=32<<iLogsize;/* Size of allocation rounded up to power of 2 */
	void *cachedMem = mem.cached[iLogsize];
	if (cachedMem)
	{
		helper_sendMsg1(0+(iLogsize<<8));
		mem.cached[iLogsize] = 0;
		return cachedMem;
	}
	takeSpinLock(MEMORY_LOCK_NUMBER);
	void *memory = zalloc_internal(iFullSz, iLogsize);
	giveSpinLock(MEMORY_LOCK_NUMBER);
	return memory;
}

void populateCache(u32 iLogsize)
{
	u32 iFullSz = ATOM_SIZE << iLogsize;
	takeSpinLock(MEMORY_LOCK_NUMBER);
	mem.cached[iLogsize] = zalloc_internal(iFullSz, iLogsize);
	giveSpinLock(MEMORY_LOCK_NUMBER);
}

/*
* Flexible memory allocation function.
* 1. If pPrior is 0 -> call zalloc.
* 2. If nBytes is 0 -> call free. 
* 3. If nBytes > oldSize we get a larger fresh allocation and copy data into it
* 4. If nBytes <= oldSize/2 we get a smaller allocation and copy into it.
* 5. Otherwise we return pPrior 
*/
void *realloc(void *pPrior, u32 nBytes){
	u32   oldSize, copyAmount;
	void *newMemory;
	if (pPrior == 0)
	{
		return zalloc(nBytes);
	}
	if (nBytes == 0)
	{
		free(pPrior);
		return 0;
	}
	oldSize = memsys5Size(pPrior);
	if (nBytes>oldSize)
	{
		copyAmount = oldSize;
	} else if (nBytes <= (oldSize/2)) {
		copyAmount = nBytes;
	} else {
		return pPrior;
	}
	// standard realloc logic
	newMemory = zalloc(nBytes);
	if (newMemory)
	{
		dmaWordForwardCopy(pPrior, newMemory, copyAmount);
		free(pPrior);
	}
	return newMemory;
}

/*
* Flexible memory allocation function.
* 1. If pPrior is 0 -> call zalloc.
* 2. If nBytes is 0 -> call free. 
* 3. If nBytes > oldSize we get a larger fresh allocation and copy data into it
* 4. If nBytes <= oldSize/2 we get a smaller allocation and copy into it.
* 5. Otherwise we return pPrior 
*/
void *realloc2(void *pPrior, u32 nBytes){
	u32   oldSize, copyAmount;
	void *newMemory;
	if (pPrior == 0)
	{
		return zalloc(nBytes);
	}
	if (nBytes == 0)
	{
		free(pPrior);
		return 0;
	}
	oldSize = memsys5Size(pPrior);
	if (nBytes>oldSize)
	{
		copyAmount = oldSize;
	} else if (nBytes <= (oldSize/2)) {
		copyAmount = nBytes;
	} else {
		return pPrior;
	}
	// What comes next is a special case of a power of two allocator for realloc
	// We are going to free the old pointer BEFORE new allocation!!
	// 1. Growing the current allocation: the current allocation could be a sub-
	// set of the next allocation. Because they are all a power of 2 any NEW
	// memory will untouched, meaning if it overlaps as all it will be in some
	// subsection (half, quarter,etc) therefore copying the data has no risk of
	// overwriting itself, we can use a forward copy.
	// 2. Shrinking the current allocation: again the same principle applies
	// a symetric subsection of the data would be needed and copying it will not
	// overwrite it mid copy.
	// Now that we know copying is ok we have another problem, zalloc will zero
	// the memory, in order to stop this we must disable the zeroize DMA, then
	// reprogram it for any new section of memory, then re-enable it.
	// What about the meta data that is used by free? This will overwrite the 
	// first 2 4 byte chunks (the size of pointers) so we will copy them out
	// first before calling free.
	// This opens the possibilty that the returned memory is the same pointer.
	// In this case we only need to replace the 2 pointers of data and return.
	{
		u32 zeroizeAmount, newSize, reallocFail = 0;
		void *data1, *data2;
		void **tmpPtr = pPrior;
		u8   *zeroizeTarget;
		data1 = tmpPtr[0];
		data2 = tmpPtr[1];
		takeSpinLock(MEMORY_LOCK_NUMBER);
		free_internal(pPrior);
		giveSpinLock(MEMORY_LOCK_NUMBER);
		disableZeroizeDMA();
		newMemory = zalloc(nBytes);
		if (newMemory == 0)
		{
			// we failed to get a new allocation, but we have gotten rid of our
			// previous allocation. We need to regenerate it and give it back
			newMemory = zalloc(oldSize);
			reallocFail = 1;
		}
		// we have a new piece of memory, copy old data out
		dmaWordForwardCopy(pPrior, newMemory, copyAmount);
		// replace memory used by allocator
		tmpPtr = newMemory;
		tmpPtr[0] = data1;
		tmpPtr[1] = data2;
		// we now have an allocation
		newSize = memsys5Size(newMemory);
		zeroizeTarget = newMemory;
		zeroizeTarget += oldSize;
		zeroizeAmount = 0;
		if (newSize > oldSize)
		{
			zeroizeAmount = newSize-oldSize;
		}
		
		//~ if (newMemory != pPrior)
		//~ {
			// we have a new piece of memory, copy rest out
			//~ u8 *copyDestintation = newMemory;
			//~ pPrior = ((u8*)pPrior) + (sizeof(void*)*2);
			//~ copyDestintation = copyDestintation + (sizeof(void*)*2);
			//~ copyAmount = copyAmount-(sizeof(void*)*2);
			//~ dmaWordForwardCopy(pPrior, copyDestintation, copyAmount);
		//~ } else {
			// if we got the same memory back we are done
			//~ prints("pointer reused on return!\n");
		//~ }
		enableZeroizeDMA();
		setZero(zeroizeTarget, zeroizeAmount);
		return ((u8*)newMemory) + reallocFail;
	}
}

/*
** Round up a request size to the next valid allocation size.  If
** the allocation is too large to be handled by this allocation system,
** return 0.
**
** All allocations must be a power of two and must be expressed by a
** 32-bit signed integer.  Hence the largest allocation is 0x40000000
** or 1073741824 bytes.
*/
//~ int memsys5Roundup(int n){
  //~ int iFullSz;
  //~ if( n > 0x40000000 ) return 0;
  //~ for(iFullSz=ATOM_SIZE; iFullSz<n; iFullSz *= 2);
  //~ return iFullSz;
//~ }

/*
** Return the ceiling of the logarithm base 2 of iValue.
**
** Examples:   memsys5Log(1) -> 0
**             memsys5Log(2) -> 1
**             memsys5Log(4) -> 2
**             memsys5Log(5) -> 3
**             memsys5Log(8) -> 3
**             memsys5Log(9) -> 4
*/
static inline int memsys5Log(int iValue){
  int iLog;
  for(iLog=0; (iLog<(int)((sizeof(int)*8)-1)) && (1<<iLog)<iValue; iLog++);
  return iLog;
}

/*
** Initialize the memory allocator.
**
** This routine is not threadsafe.  The caller must be holding a mutex
** to prevent multiple threads from entering at the same time.
*/
void memSysInit(void)
{
	s32 ii;            /* Loop counter */
	u32 iOffset;       /* An offset into mem.aCtrl[] */

	mem.nBlock = 6950; // heapsize / (ATOM+1)
	mem.zPool = (u8*)0x20008000; // start of memory
	mem.aCtrl = &mem.zPool[mem.nBlock*ATOM_SIZE]; // start of control

	// initialize free lists to sentinal
	for(ii=0; ii<LOGMAX; ii++){
	mem.aiFreelist[ii] = &mem.sentinalNode;
	}
	// set sentinal free list to a value other than sentinal node
	mem.aiFreelist[LOGMAX] = (MemDLL*)&mem.aiFreelist[LOGMAX];
	// fill free lists with largest possible blocks of powers of 2
	iOffset = 0;
	for(ii=LOGMAX; ii>=0; ii--)
	{
		int nAlloc = (1<<ii);
		if( (iOffset+nAlloc)<=mem.nBlock ){
			mem.aCtrl[iOffset] = ii + CTRL_FREE;
			memsys5Link(GET_MEMDLL(iOffset), ii, mem.aiFreelist);
			iOffset += nAlloc;
		}
	}
	// pre-populate cache
	mem.cached[0] = zalloc_internal(32, 0);
	mem.cached[1] = zalloc_internal(64, 1);
	mem.cached[2] = zalloc_internal(128, 2);
	return;
}
