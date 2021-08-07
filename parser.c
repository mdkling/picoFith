// parser.c

static u8*
parseWord(u8 *out, u8 *start, u8 *YYCURSOR)
{
	if (fls.insideFunction)
	{
		// we are inside a function, check local context
		avlNode *retNode = avl_find(
			fls.localVarsRoot,     // pointer to tree
			start,      // pointer to string
			YYCURSOR - start);  // length of string (255 max)
		if (retNode)
		{
			prints("local: ");
			uartTX(start, YYCURSOR - start);
			prints(" is being loaded.\n");
			// variable exists, emit load
			u32 rawVal = (u32)retNode->value;
			*out++ = fithLoadLocal0 +  rawVal;
			return out;
		}
	}
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
		if((u8*)rawVal==fls.currentFunctionOut){rawVal+=fls.numLocalVars<<28;}
		prints("num locals for call ");
		printWord((rawVal>>28));
		prints("\n");
		if (rawVal>>31 == 0) // WORD_FUNCTION
		{
			u8  *startOfFunc = (u8*)(((rawVal<<4)>>4)+0x20000000);
			s32 difference = startOfFunc - (out + 3);
			*out++ = fithCallFunc0 + (rawVal>>28);
			*out++ = difference & 0xFF;
			*out++ = (difference>>8) & 0xFF;
		} else if (rawVal>>31 == 1) { // WORD_GLOBAL_VAR
			*out++ = fithLoadGlobal;
			*out++ = (rawVal<<1)>>1;
		}
	}
	return out;
}

static void
parseWordDefinition(u8 *out, u8 *start, u8 *YYCURSOR)
{
	void *functionStart = (void*)(((u32)out<<4)>>4);
	avlNode *retNode = avl_insert(
		&fls.wordTreeRoot,   // pointer memory holding address of tree
		start,     // pointer to string
		YYCURSOR - start - 1,  // length of string (255 max)
		functionStart );   // value to be stored
	if (retNode) {
		prints("word already existed\n");
	}
	fls.insideFunction = 1; // set flag we are inside a function
	fls.currentFunctionOut = functionStart;
	// save off node pointer into the block stack
	fls.blockStack[fls.blockStackIndex].savedCursor = (u8*)
		avl_find(
			fls.wordTreeRoot,
			start,
			YYCURSOR - start - 1);
	// label block as function
	fls.blockStack[fls.blockStackIndex++].blockType = BLOCK_FUNCTION;
}

static u8*
parseVarAssignment(u8 *start, u8 *YYCURSOR, u8 *out)
{
	if (fls.insideFunction)
	{
		// we are inside a function, check local context
		avlNode *retNode = avl_find(
			fls.localVarsRoot,     // pointer to tree
			start,      // pointer to string
			YYCURSOR - start - 1);  // length of string (255 max)
		if (retNode)
		{
			// variable exists, emit store
			u32 rawVal = (u32)retNode->value;
			*out++ = fithStoreLocal0 + rawVal;
			return out;
		}
	}
	// we are not inside a function check global context
	avlNode *retNode = avl_find(
		fls.wordTreeRoot,     // pointer to tree
		start,      // pointer to string
		YYCURSOR - start - 1);  // length of string (255 max)
	if (retNode)
	{
		// variable exists, emit store
		u32 rawVal = (u32)retNode->value;
		*out++ = fithStoreGlobal;
		*out++ = (rawVal<<1)>>1;
	} else {
		// variable does not exist
		prints("variable ");
		uartTX(start, YYCURSOR - start - 1);
		prints(" does not exist.\n");
	}
	return out;
}

static void
parseVarDecl(u8 *start, u8 *YYCURSOR)
{
	if (fls.insideFunction)
	{
		// we are inside a function, check local context
		avlNode *retNode = avl_find(
			fls.localVarsRoot,     // pointer to tree
			start,      // pointer to string
			YYCURSOR - start - 1);  // length of string (255 max)
		if (retNode)
		{
			// local already exists
			prints("local variable ");
			uartTX(start, YYCURSOR - start - 1);
			prints(" already exists.\n");
		} else {
			prints("local variable ");
			uartTX(start, YYCURSOR - start - 1);
			prints(" is being created.\n");
			// local does not exist
			s32 localIndex = fls.numLocalVars++;
			// check if we are over the limit of locals
			if (fls.numLocalVars > 8)
			{
				prints("Error: Cannot make more than 8 locals.\n");
				return;
			}
			// save off local index
			(void)avl_insert(
				&fls.localVarsRoot,   // pointer memory holding address of tree
				start,     // pointer to string
				YYCURSOR - start - 1,  // length of string (255 max)
				(void*)localIndex );   // value to be stored
		}
	} else {
		// we are not inside a function check global context
		avlNode *retNode = avl_find(
			fls.wordTreeRoot,     // pointer to tree
			start,      // pointer to string
			YYCURSOR - start - 1);  // length of string (255 max)
		if (retNode)
		{
			// global already exists
			prints("global variable ");
			uartTX(start, YYCURSOR - start - 1);
			prints(" already exists.\n");
		} else {
			// global does not exist
			s32 globalIndex = fls.numGlobalVars++;
			// check if we need to expand global array
			if (fls.numGlobalVars > fls.globalsBufferSize)
			{
				fls.globalsBufferSize*=2;
				fithExecutionState.globals = realloc(fithExecutionState.globals, fls.globalsBufferSize);
			}
			// save off global index
			(void)avl_insert(
				&fls.wordTreeRoot,   // pointer memory holding address of tree
				start,     // pointer to string
				YYCURSOR - start - 1,  // length of string (255 max)
				(void*)(globalIndex+(1<<31)) );   // value to be stored
		}
	}
}

static u8*
parseCloseBlock(u8 *out)
{
	s32 blockType = fls.blockStack[--fls.blockStackIndex].blockType;
	if (blockType == BLOCK_CJUMP)
	{
		u8 *jout = fls.blockStack[fls.blockStackIndex].savedCursor;
		s32 difference = out - jout;
		*(jout-2)= difference & 0xFF;
		*(jout-1) = (difference>>8) & 0xFF;
	} else if (blockType == BLOCK_ELSE) {
		u8 *jout = fls.blockStack[fls.blockStackIndex].savedCursor;
		s32 difference = out - jout;
		*(jout+1)= difference & 0xFF;
		*(jout+2) = (difference>>8) & 0xFF;
		// wrap up else chain if there
		while (fls.blockStack[fls.blockStackIndex-1].blockType==BLOCK_ELSE)
		{
			jout = fls.blockStack[--fls.blockStackIndex].savedCursor;
			difference = out - jout;
			*(jout+1)= difference & 0xFF;
			*(jout+2) = (difference>>8) & 0xFF;
		}
	} else if (blockType == BLOCK_FUNCTION) {
		// output return
		*out++ = fithReturn0+fls.numLocalVars;
		prints("num locals for return ");
		printWord(fls.numLocalVars);
		prints("\n");
		// pack number of locals into address
		avlNode *retNode=(void*)fls.blockStack[fls.blockStackIndex].savedCursor;
		u32 rawVal = (u32)retNode->value;
		rawVal = (rawVal<<4)>>4;
		rawVal += fls.numLocalVars << 28;
		retNode->value = (void*)rawVal;
		// set we are not in a function anymore
		fls.insideFunction = 0;
		fls.currentFunctionOut = 0;
		// reset num locals to zero
		fls.numLocalVars = 0;
		// clear out the locals tree
		avl_freeAll(fls.localVarsRoot);
		fls.localVarsRoot = 0;
		if (fls.blockStackIndex != 0)
		{
			prints("WARNING: word defined within a block. Probably wrong.");
		}
		prints("size of word:");
		printWord(out - fls.outBufferStart);
		prints("\n");
		// move start forward so we save the function we created
		fls.outBufferStart  = out;
	}
	return out;
}

#define MEM_TEST_VAL 64

static void
memoryTesting(void)
{
	#if 0
	// test memory under stress
	u32 *memory1, *memory2, *memory3;
	// allocate 64K memory
	memory1 = zalloc(64*MEM_TEST_VAL);
	if (memory1 == 0)
	{
		prints("memory1 allocation failed!\n");
		return;
	}
	// now allocate another 64k
	memory3 = zalloc(64*MEM_TEST_VAL);
	memory2 = zalloc(64*MEM_TEST_VAL);
	if (memory2 == 0)
	{
		prints("memory2 allocation failed!\n");
		free(memory1);
		return;
	}
	for(u32 x = 0; x < 64*MEM_TEST_VAL/4; x++)
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
	for(u32 x = 0; x < 64*MEM_TEST_VAL/4; x++)
	{
		memory3[x] = 0x8BADF00D;
	}
	free(memory3);
	// now allocate another 64k
	memory3 = realloc2(memory2, 128*MEM_TEST_VAL);
	if (((u32)memory3&1) == 1)
	{
		prints("memory2 realloc failed!\n");
		free(memory1);
		free(memory2);
		prints("after suspect free's\n");
		return;
	}
	printWord(memory3[128*MEM_TEST_VAL/4-1]);
	memory3[128*MEM_TEST_VAL/4-1] = 0x8BADF00D;
	prints("\n");
	printWord(memory3[0]);
	prints("\n");
	printWord(memory3[1]);
	prints("\n");
	printWord(memory3[64*MEM_TEST_VAL/4]);
	prints("\n");
	printWord(memory3[64*MEM_TEST_VAL/4+1]);
	prints("\n");
	printWord(memory3[128*MEM_TEST_VAL/4-1]);
	prints("\n");
	free(memory1);
	free(memory3);
	
	//~ memory1 = zalloc(1*MEM_TEST_VAL);
	
	u32 timerVal;
	//~ memory1 = zalloc(1*MEM_TEST_VAL);
	//~ memory2 = zalloc(1*MEM_TEST_VAL);
	//~ memory3 = zalloc(1*MEM_TEST_VAL);
	timerVal = readSysTimerVal(0);
	//~ memory2 = zalloc(1*MEM_TEST_VAL);
	memory2 = fastAlloc(1);
	//~ free(memory1);
	printWord(-readSysTimerVal(timerVal));
	prints("\n");
	timerVal = readSysTimerVal(0);
	//~ memory2 = zalloc(1*MEM_TEST_VAL);
	memory2 = fastAlloc(1);
	//~ free(memory2);
	printWord(-readSysTimerVal(timerVal));
	prints("\n");
	//~ timerVal = readSysTimerVal(0);
	//~ memory2 = zalloc(1*MEM_TEST_VAL);
	//~ free(memory3);
	//~ printWord(-readSysTimerVal(timerVal));
	//~ prints("\n");
	timerVal = readSysTimerVal(0);
	//~ memory2 = zalloc(1*MEM_TEST_VAL);
	printWord(-readSysTimerVal(timerVal));
	#endif
	prints("\n");
	
	#if 0
	memory2 = zalloc(1*MEM_TEST_VAL);
	if (memory2 == 0)
	{
		prints("memory2 allocation failed!\n");
		return;
	}
	for(u32 x = 0; x < 1*MEM_TEST_VAL/4; x++)
	{
		memory2[x] = 0xDEADBEEF;
	}
	free(memory2);
	memory2 = zalloc(1*MEM_TEST_VAL);
	//~ memory2 = zalloc(1*MEM_TEST_VAL);
	//~ prints("memory2 ");
	//~ printWord(memory2);
	//~ prints("\n");
	printWord(memory2[1*MEM_TEST_VAL/4-1]);
	prints("\n");
	altMemInit();
	printWord(zalloc2(1*MEM_TEST_VAL));
	prints("\n");
	printWord(zalloc2(1*MEM_TEST_VAL));
	prints("\n");
	memory1 = zalloc2(1*MEM_TEST_VAL);
	prints("memory1 ");
	printWord(memory1);
	prints("\n");
	printWord(zalloc2(1*MEM_TEST_VAL));
	prints("\n");
	printWord(zalloc2(1*MEM_TEST_VAL));
	prints("\n");
	//~ free2(memory1);
	//~ printWord(memory2);
	//~ prints("\n");
	u32 timerVal = readSysTimerVal(0);
	//~ free(memory2);
	//~ free2(memory1);
	memory2 = zalloc(1*MEM_TEST_VAL);
	//~ memory2 = zalloc2(1*MEM_TEST_VAL);
	printWord(-readSysTimerVal(timerVal));
	prints("\n");
	//~ prints("memory2 ");
	//~ printWord(memory2);
	//~ prints("\n");
	#endif
}
