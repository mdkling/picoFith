// parser.c

static u8*
parseWord(u8 *out, u8 *start, u8 *YYCURSOR)
{
	avlNode *retNode = avl_find(
		fls.wordTreeRoot,     // pointer to tree
		start,      // pointer to string
		YYCURSOR - start);  // length of string (255 max)
	if (retNode == 0) {
		prints("Word: \"");
		uartTX(start, YYCURSOR - start);
		prints("\" is not defined.\n");
	} else {
		u32 rawVal = (u32)retNode->value;
		if (rawVal>>30 == 0) // WORD_FUNCTION
		{
			s32 difference = ((u8*)rawVal) - (out + 3);
			*out++ = fithCallFunc;
			*out++ = difference & 0xFF;
			*out++ = (difference>>8) & 0xFF;
		} else if (rawVal>>30 == 1) { // WORD_LOCAL
			
		} else if (rawVal>>30 == 2) { // WORD_GLOBAL
			
		} else {                    // WORD_CONST
			
		}
	}
	return out;
}

static void
memoryTesting(void)
{
	// test memory under stress
	u32 *memory1, *memory2, *memory3;
	// allocate 64K memory
	memory1 = zalloc(64*1024);
	if (memory1 == 0)
	{
		prints("memory1 allocation failed!\n");
		return;
	}
	// now allocate another 64k
	memory3 = zalloc(64*1024);
	memory2 = zalloc(64*1024);
	if (memory2 == 0)
	{
		prints("memory2 allocation failed!\n");
		free(memory1);
		return;
	}
	for(u32 x = 0; x < 64*1024/4; x++)
	{
		memory2[x] = 0xDEADBEEF;
	}
	// now allocate another 64k
	
	if (memory3 == 0)
	{
		prints("memory3 allocation failed!\n");
		free(memory1);
		return;
	}
	for(u32 x = 0; x < 64*1024/4; x++)
	{
		memory3[x] = 0x8BADF00D;
	}
	free(memory3);
	// now allocate another 64k
	memory3 = realloc2(memory2, 128*1024);
	if (memory3 == 0)
	{
		prints("memory2 realloc failed!\n");
		free(memory1);
		free(memory2);
		prints("after suspect free's\n");
		return;
	}
	printWord(memory3[0]);
	prints("\n");
	printWord(memory3[1]);
	prints("\n");
	printWord(memory3[64*1024/4]);
	prints("\n");
	printWord(memory3[64*1024/4+1]);
	prints("\n");
	
}
