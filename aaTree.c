// aaTree.c

#include "localTypes.h"
#include "memory.h"
// 24 byte node
typedef struct AAnode AAnode;
typedef struct AAnode {
	AAnode *child[2];
	void   *val;
	u8      level;
	u8      keyLen;
	u8      key[2];
} AAnode;

static AAnode nil = {
	.val = 0,
	.child[0] = &nil,
	.child[1] = &nil,
	.level = 0,
	.key = {0}
};

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

AAnode * makeNode(void *val, u8 level, u8 *key, u32 keyLen)
{
	AAnode *new = zalloc(sizeof(AAnode));
	
	new->val=val;
	new->child[0]=&nil;
	new->child[1]=&nil;
	new->keyLen = keyLen;
	new->level=level;
	s32 i = 0;
	do {new->key[i] = key[i]; i++; } while(i < keyLen);
	new->key[keyLen] = 0; // null terminate
	
	return new;
}

AAnode * skew(AAnode *root)
{
	AAnode *child = root->child[0];
	if(root->level == child->level)
	{
		AAnode *save = child;
		root->child[0] = save->child[1];
		save->child[1] = root;
		root=save;
	}
	return root;
}

AAnode * split(AAnode *root)
{
	AAnode *child, *childOfChild;
	
	child = root->child[1];
	childOfChild = child->child[1];
	
	if(root->level == childOfChild->level)
	{
		AAnode *save = child;
		root->child[1] = save->child[0];
		save->child[0] = root;
		root=save;
		root->level+=1;
	}
	return root;
}

AAnode * AAinsert(AAnode *root, u8 *key, u32 keyLen, void *val)
{
	if (root == 0)
	{
		return makeNode(val, 1, key, keyLen);
	}
	s32 dir = keyCmp(key, root->key, keyLen, root->keyLen);
	if (dir == 0) { root->val = val; return root; }
	dir = ((u32)dir>>31);
	root->child[dir] = AAinsert(root->child[dir], key, keyLen, val);
	//printf("insert2\n");
	root = skew(root);
	//printf("insert3\n");
	root = split(root);
	//printf("insert4\n");
	return root;
}

void AAinsert2(AAnode **rootp, u8 *key, u32 keyLen, void *val)
{
	AAnode *root = *rootp;
	if (root == 0)
	{
		*rootp = makeNode(val, 1, key, keyLen);
		return;
	}
	s32 dir = keyCmp(key, root->key, keyLen, root->keyLen);
	dir = ((u32)dir>>31);
	AAinsert2(&root->child[dir], key, keyLen, val);
	//printf("insert2\n");
	root = skew(root);
	//printf("insert3\n");
	root = split(root);
	//printf("insert4\n");
	*rootp = root;
}

//~ void AAinsert2(AAnode **rootp, u8 *key, u32 keyLen, void *val)
//~ {
	//~ AAinsert_r2(rootp, key, keyLen, val);
//~ }




