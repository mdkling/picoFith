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
