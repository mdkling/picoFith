// avl.c

#include "localTypes.h"
#include "memory.h"
#include "avl.h"

#define AVL_MALLOC(string) zalloc(string)
#define AVL_FREE(string) free(string)

#define LEFT     0
#define RIGHT    1
#define NEITHER -1
#define Balanced(n) ((n)->longer < 0)

/*******************************************************************************
 * Section Local Prototypes
 ******************************************************************************/

static inline s32
keyCmp(u8 *key1, u8 *key2, s32 key1Len, s32 key2Len)
__attribute__((always_inline));

/*******************************************************************************
 * Section Helper Functions
*******************************************************************************/

static inline s32
keyCmp(u8 *key1, u8 *key2, s32 key1Len, s32 key2Len)
{
	s32 c1, c2, i = 0;
	
	while(1){
		c1=key1[i];
		c2=key2[i++];
		c1-=c2;
		if (c1 != 0) { return c1; }
		if (i >= key1Len) { return key1Len - key2Len; }
	}
}

static u32
nodeSize(u32 keyLen)
{
	return sizeof(avlNode) + ((keyLen + 3) / 4 * 4); // round up to 4 bytes
}

static avlNode*
makeNode(u8 *key, u32 keyLen, void *value)
{
	avlNode *tree;
	u32      i = 0;
	tree = AVL_MALLOC(nodeSize(keyLen));
	tree->next[0] = tree->next[1] = 0;
	tree->value   = value;
	tree->longer  = NEITHER;
	tree->taken   = NEITHER;
	tree->keyLen  = keyLen;
	do {tree->key[i] = key[i]; i++; } while(i < keyLen);
	tree->key[keyLen] = 0; // null terminate
	return tree;
}

static void
encodeInt(s32 key, u8 *keyBuffer)
{
	key = key ^ 0x80000000;
	keyBuffer[0] = key >> 24;
	keyBuffer[1] = key >> 16;
	keyBuffer[2] = key >> 8;
	keyBuffer[3] = key;
	return;
}

/*******************************************************************************
 * Section Find
*******************************************************************************/

avlNode * // returns address of node with same key if it exists or 0
avl_find(
	avlNode  *tree,     // pointer to tree
	u8  *key,      // pointer to string
	u32  keyLen)   // length of string (255 max)
{
	while (tree) {
		s32 result = keyCmp(key, tree->key, keyLen, tree->keyLen);
		if (result == 0) {
			break; // target found
		} else if (result > 0) {
			tree = tree->next[1];
		} else {
			tree = tree->next[0];
		}
	}
	return tree;
}

avlNode *
avl_findIntKey(avlNode *tree, s32 key)
{
	u8 keyBuffer[8];
	encodeInt(key, keyBuffer);
	return avl_find(tree, keyBuffer, 4);
}

/*******************************************************************************
 * Section Rotation
 ******************************************************************************/

static avlNode*
avl_rotate_2(avlNode* *path_top, s32 dir)
{
	avlNode* B, *C, *D, *E;
	s32 otherDir = 1^dir;
	B = *path_top;
	D = B->next[dir];
	C = D->next[otherDir];
	E = D->next[dir];

	*path_top = D;
	D->next[otherDir] = B;
	B->next[dir] = C;
	B->longer = NEITHER;
	D->longer = NEITHER;
	return E;
}

static avlNode*
avl_rotate_3(avlNode* *path_top, s32 dir, s32 third)
{
	avlNode *B, *F, *D, *C, *E;
	s32 otherDir = 1^dir;
	B = *path_top;
	F = B->next[dir];
	D = F->next[otherDir];
	/* note: C and E can be NULL */
	C = D->next[otherDir];
	E = D->next[dir];
	*path_top = D;
	D->next[otherDir] = B;
	D->next[dir] = F;
	B->next[dir] = C;
	F->next[otherDir] = E;
	D->longer = NEITHER;

	/* assume both trees are balanced */
	B->longer = F->longer = NEITHER;

	if (third == NEITHER){
		return 0;
	}
	else if (third == dir) {
		/* E holds the insertion so B is unbalanced */ 
		B->longer = otherDir;
		return E;
	} else {
		/* C holds the insertion so F is unbalanced */
		F->longer = dir;
		return C;
	}
}

/*******************************************************************************
 * Section Insertion
 ******************************************************************************/

static inline void
avl_rebalance_path(avlNode* path)
{
	/* Each avlNode* in path is currently balanced.
	 * Until we find target, mark each avlNode* as longer
	 * in the s32 of target because we know we have
	 * inserted target there
	 */
	if(path!=0){
		while (path->taken > NEITHER) {
			s32 next_step = path->taken;
			path->longer = next_step;
			path = path->next[next_step];
		}
	}
}

static inline void
avl_rebalance_insert(avlNode **path_top)
{
	avlNode *path = *path_top;
	s32 first, second, third;
	if (Balanced(path)) {
		;
	}
	else if (path->longer != (first = path->taken) ) {
		/* took the shorter path */
		path->longer = NEITHER;
		path = path->next[first];
	} else if (first == (second = path->next[first]->taken)) {
		/* just a two-point rotate */
		path = avl_rotate_2(path_top, first);
	} else {
		/* fine details of the 3 point rotate depend on the third step.
		 * However there may not be a third step, if the third point of the
		 * rotation is the newly inserted point.  In that case we record
		 * the third step as NEITHER
		 */
		path  = path->next[first]->next[second];
		third = path->taken;
		path  = avl_rotate_3(path_top, first, third);
	}
	avl_rebalance_path(path);
}

avlNode * // returns address of node with same key if it exists or 0
avl_insert(
	avlNode **treep,   // pointer memory holding address of tree
	u8  *key,     // pointer to string
	u32  keyLen,  // length of string (255 max)
	void     *value)   // value to be stored
{
	avlNode  *tree     = *treep;
	avlNode **path_top = treep;
	while (tree) {
		if (!Balanced(tree)) { path_top = treep; }
		s32 result = keyCmp(key, tree->key, keyLen, tree->keyLen);
		if (result == 0) { return tree; } // tree already exists
		tree->taken = result > 0;
		treep = &tree->next[result > 0];
		tree = *treep;
	}
	tree = makeNode(key, keyLen, value);
	*treep = tree;
	avl_rebalance_insert(path_top);
	return 0;
}

avlNode *
avl_insertInt(
	avlNode **treep,
	s32 key,
	void *value)
{
	u8 keyBuffer[8];
	encodeInt(key, keyBuffer);
	return avl_insert(treep, keyBuffer, 4, value);
}

/*******************************************************************************
 * Section Deletion
*******************************************************************************/

static inline void*
avl_swap_del(avlNode **targetp, avlNode **treep, s32 dir)
{
	avlNode *targetn = *targetp;
	avlNode *tree = *treep;
	void    *value = targetn->value;

	*targetp = tree;
	*treep = tree->next[1^dir];
	tree->next[LEFT] = targetn->next[LEFT];
	tree->next[RIGHT]= targetn->next[RIGHT];
	tree->longer = targetn->longer;

	AVL_FREE(targetn);
	return value;
}

static inline avlNode**
avl_rebalance_del(avlNode **treep, avlNode **targetp)
{
	/* each avlNode* from treep down towards target, but
	 * excluding the last, will have a subtree grow
	 * and need rebalancing
	 */
	avlNode *targetn = *targetp;
	avlNode *tree;
	s32 dir, otherDir;

	while (1) {
		tree = *treep;
		dir = tree->taken;
		otherDir = 1^dir;

		if (tree->next[dir]==0){
			break;
		}

		if (Balanced(tree)){
			tree->longer = otherDir;
		}
		else if (tree->longer == dir){
			tree->longer = NEITHER;
		}
		else {
			s32 second = tree->next[otherDir]->longer;
			if (second == dir) {
				avl_rotate_3(treep, otherDir, 
					     tree->next[otherDir]->next[dir]->longer);
			}
			else if (second == NEITHER) {
				avl_rotate_2(treep, otherDir);
				tree->longer = otherDir;
				(*treep)->longer = dir;
			} else {
				avl_rotate_2(treep, otherDir);
			}
			if (tree == targetn){
				targetp = &(*treep)->next[dir];
			}
		}

		treep = &tree->next[dir];
	}
	return targetp;
}


void * // returns value of node deleted or 0
avl_delete(
	avlNode **treep,    // pointer memory holding address of tree
	u8  *key,      // pointer to string
	u32  keyLen)   // length of string (255 max)
{
	/* delete the target from the tree, returning 1 on success or 0 if
	 * it wasn't found
	 */
	avlNode **path_top;
	avlNode **targetp;
	avlNode  *tree;
	s32 dir, otherDir, res;
	
	path_top =  treep;
	tree     = *treep;
	targetp  =  0;
	
	if (!tree) { return 0; }

	 while (1) {
		res = keyCmp(key, tree->key, keyLen, tree->keyLen);
		dir = tree->taken = res > 0;
		otherDir = 1^dir;
		if (res == 0){
			targetp = treep;
		}
		if (tree->next[dir] == 0){
			break;
		}
		if (Balanced(tree)
		    || (tree->longer == otherDir && Balanced(tree->next[otherDir]))
			) { path_top = treep; }
		treep = &tree->next[dir];
		tree  = *treep;
	}
	// no matching node found, return 0
	if (!targetp){
		return 0;
	}

	/* adjust balance, but don't lose 'targetp' */
	targetp = avl_rebalance_del(path_top, targetp);

	/* We have re-balanced everything, it remains only to 
	 * swap the end of the path (*treep) with the deleted item
	 * (*targetp)
	 */
	return avl_swap_del(targetp, treep, dir);
}

void * // returns value of node deleted or 0
avl_deleteIntKey(
	avlNode **treep,
	s32 key)
{
	u8 keyBuffer[8];
	encodeInt(key, keyBuffer);
	return avl_delete(treep, keyBuffer, 4);
}
