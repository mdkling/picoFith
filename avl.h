// avl.h

#ifndef AVLTREE_HEADER
#define AVLTREE_HEADER

typedef struct avlNode avlNode;
typedef struct avlNode {
	avlNode *next[2];
	void    *value;
	s8       longer;
	s8       taken;
	u8       keyLen;
	u8       key[1];
} avlNode;

avlNode * // returns address of node with same key if it exists or 0
avl_find(
	avlNode  *tree,     // pointer to tree
	u8       *key,      // pointer to string
	u32       keyLen);  // length of string (255 max)

avlNode *
avl_findIntKey(avlNode *tree, s32 key);

avlNode * // returns address of node with same key if it exists or 0
avl_insert(
	avlNode **treep,   // pointer memory holding address of tree
	u8       *key,     // pointer to string
	u32       keyLen,  // length of string (255 max)
	void     *value);  // value to be stored

avlNode *
avl_insertInt(
	avlNode **treep,
	s32 key,
	void *value);

void * // returns value of node deleted or 0
avl_delete(
	avlNode **treep,    // pointer memory holding address of tree
	u8       *key,      // pointer to string
	u32       keyLen);  // length of string (255 max)

void * // returns value of node deleted or 0
avl_deleteIntKey(
	avlNode **treep,
	s32       key);

void
avl_freeAll(avlNode *root);

#endif
