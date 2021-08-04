
#include "localTypes.h"
#include "memory.h"
#include "avl.h"

enum {
	fithConstant0,
	fithConstant1,
	fithConstant2,
	fithConstant3,
	fithConstant4,
	fithConstant5,
	fithConstant6,
	fithConstant7,
	fithConstant8,
	fithConstant1byte,
	fithConstant2byte,
	fithConstant3byte,
	fithConstant4byte,
	fithExit,
	fithAdd,
	fithSub,
	fithMul,
	fithDiv,
	fithMod,
	fithReturn,
	fithCallFunc,
	fithDup,
	fithJump,
	fithGreaterThanJump,
	fithGreaterThanEqualJump,
	fithLessThanJump,
	fithLessThanEqualJump,
	fithEqualJump,
	fithNotEqualJump,
	fithAbs,
	fithDrop,
	
};

enum{
	BLOCK_FUNCTION,
	BLOCK_CJUMP,
	BLOCK_ELSE,
};

enum{
	WORD_FUNCTION,
	WORD_LOCAL,
	WORD_GLOBAL,
	WORD_CONST,
};

typedef struct blockInfo {
	u8  *savedCursor;
	s32  blockType;	
} blockInfo;

typedef struct fithLexState {
	u8        *outBufferStart;
	u8        *outBufferCursor;
	s32        globalsBufferSize;
	avlNode   *wordTreeRoot;
	u32        inBlockStateStack;
	s32        blockStackIndex;
	blockInfo *blockStack;
} fithLexState;

typedef struct fithRegisters {
	s32  tos;
	s32 *globals;
	s32 *returnStack;
	s32 *bottomOfExprStack;
	s32 *currentStackPtr;
} fithRegisters;

typedef struct resultCursor {
	u8  *cursor;
	s32  result;
} resultCursor;

void  uartTX(u8 *sting, s32 size);
void  prints(u8 *sting);
void  printWord(s32 word);
void  fithPrepareExecute(u8 *codeToExec);
void  REBOOT(void);

#define PRINT_STRING(string) uartTX(string, sizeof string - 1)
#define ARRAY_SIZE(a) ((sizeof a)/(sizeof a[0]))

extern u8  __bss_end__[4];
extern fithRegisters fithExecutionState;
static blockInfo blockStackMem[32];
static fithLexState  fls = {
	.outBufferStart    = __bss_end__,
	.outBufferCursor   = 0,
	.wordTreeRoot      = 0,
	.globalsBufferSize = 0,
	.inBlockStateStack = 0,
	.blockStackIndex   = 0,
	.blockStack        = blockStackMem,
};

void
fithRegistersInit(void)
{
	fithExecutionState.globals = zalloc(32);
	fithExecutionState.returnStack = zalloc(64);
	fithExecutionState.bottomOfExprStack = zalloc(64);
	fithExecutionState.currentStackPtr = fithExecutionState.bottomOfExprStack;
}

void
walkNodes(avlNode *root)
{
	if (root == 0)
	{
		return;
	}
	walkNodes(root->next[0]);
	uartTX(root->key, root->keyLen);
	PRINT_STRING("\n");
	walkNodes(root->next[1]);
}

static s32
s2i(u8 *b)
{
	s32 result     = 0;
	s32 isNegative = 1;
	if (*b == '-')
	{
		b++;
		isNegative = -1;
	}
	
	if (*b == '0' && *(b+1) == 'x')
	{
		// process hex numbers
		b += 2;
		tryAnotherByte:
		if ( (*b >= '0') && (*b <= '9') )
		{
			result = (result * 16) + (*b - '0');
			b++;
			goto tryAnotherByte;
		} else if ( (*b >= 'A') && (*b <= 'F') ) {
			result = (result * 16) + (*b - ('A' - 10));
			b++;
			goto tryAnotherByte;
		} else {
			goto end;
		}
	}
	
	while( (*b >= '0') && (*b <= '9') )
	{
		result = (result * 10) + (*b - '0');
		b++;
	}
	
	end:
	result = isNegative * result;
	
	return result;
}

static u8*
writeInteger(u8 *out, u32 val)
{
	u8  *start = out;
	u32  byteCount = 0;
	out++;
	
	while (val != 0)
	{
		*out++ = val;
		val = val >> 8;
		byteCount++;
	}
	*start = byteCount + 8;
	return out;
}

/*!re2c                              // start of re2c block
	
	scm = "\\" [^\x00]*;
	wsp = ([ \n\t\r] | scm)+; // removed \v
	// integer literals
	dec = [0-9]+;
	hex = "0x" [0-9A-F]+; // a-f removed
	string_lit = ['] ([^'\x00\x03] | ([\\] [']))* ['];
	//string_lit_chain = string_lit ([ \n\t\r]* string_lit)+;
	//string_lit_chain = ([^"\n] | ([\\] ["]))* ("\n" | ["]);
	string_lit_chain = ([^'\n] | ([\\] [']))* "\n";
	string_lit_end = ([^'\n] | ([\\] [']))* ['];
	mangled_string_lit = ['] ([^'\x00\x03] | ([\\] [']))* "\x00";
	char_lit = [`] ([^`\x03] | ([\\] [`]))* [`];
	integer = "-"? (dec | hex);
	wordOperators = [@#$^~;:!?];
	word = [a-zA-Z_] [a-zA-Z_0-9?!-]*;
	word_definition = word "{";
	word_increment = word "++";
	function_call_addr = "@" word ;
	function_definition = word ":";

	
*/                                   // end of re2c block

#include "parser.c"
#include "pio.c"
#include "helperCpu.c"

void
configPioAsm(void);

static u8*
builtInWords(u8 *out, u8 *YYCURSOR)
{
	u8  *YYMARKER;
	
	/*!re2c                          // start of re2c block **/
	re2c:define:YYCTYPE = "u8";      //   configuration that defines YYCTYPE
	re2c:yyfill:enable  = 0;         //   configuration that turns off YYFILL

	* {
		return 0;
	}
	
	"walk" {
		walkNodes(fls.wordTreeRoot);
		return out;
	}
	
	"dup" {
		*out++ = fithDup;
		return out;
	}
	
	"drop" {
		*out++ = fithDrop;
		return out;
	}
	
	"abs" {
		*out++ = fithAbs;
		return out;
	}
	
	"reboot" {
		REBOOT();
		return out;
	}
	
	"config-pio" {
		configPIO();
		configPioAsm();
		memoryTesting();
		return out;
	}
	
	"send-sound" {
		outputSound();
		return out;
	}
	
	"hello-core1" {
		helper_speak();
		
		
		
		return out;
	}
	
	*/                               // end of re2c block
}

void
fithLexLine(u8 *line)
{
	u8  *YYMARKER;
	u8  *start;
	u8  *out;
	u8  *YYCURSOR = line;
	
	if (fls.blockStackIndex != 0)
	{
		out = fls.outBufferCursor;
	} else {
		out = fls.outBufferStart;
	}
	

loop:
	start = YYCURSOR;

	/*!re2c                          // start of re2c block **/
	re2c:define:YYCTYPE = "u8";      //   configuration that defines YYCTYPE
	re2c:yyfill:enable  = 0;         //   configuration that turns off YYFILL

	* {
		PRINT_STRING("lex failure\n");
		uartTX(line, YYCURSOR - line);
		PRINT_STRING("<--\n");
		return;
	}
	
	[\x00] {
		//~ if (out != fls.outBufferStart)
		if (fls.blockStackIndex == 0)
		{
			// execute code
			// this is a function of tos, esp, rsp, globals, ip
			*out++ = fithExit;
			fithPrepareExecute(fls.outBufferStart);
		} else {
			fls.outBufferCursor = out;
		}
		return;
	}
	
	wsp {
		goto loop;
	}
	
	integer {
		//~ PRINT_STRING("integer detected\n");
		s32 val = s2i(start);
		if (val > 8 || val < 0)
		{
			out = writeInteger(out, val);
		} else {
			*out++ = val;
		}
		//~ printWord(val);
		//~ PRINT_STRING("\n");
		goto loop;
	}
	
	"+" {
		*out++ = fithAdd;
		goto loop;
	}
	
	"-" {
		*out++ = fithSub;
		goto loop;
	}
	
	"*" {
		*out++ = fithMul;
		goto loop;
	}
	
	"/" {
		*out++ = fithDiv;
		goto loop;
	}
	
	"%" {
		*out++ = fithMod;
		goto loop;
	}
	
	"}" {
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
			*out++ = fithReturn;
			// move start forward so we save the function we created
			prints("size of word:");
			printWord(out - fls.outBufferStart);
			prints("\n");
			fls.outBufferStart  = out;
		}
		goto loop;
	}
	
	"}else{" {
		s32 blockType = fls.blockStack[--fls.blockStackIndex].blockType;
		if (blockType == BLOCK_CJUMP)
		{
			*out = fithJump; // ouput unconditional jump
			out += 3;
			u8 *jout = fls.blockStack[fls.blockStackIndex].savedCursor;
			s32 difference = out - jout; // fill in conditional jump
			*(jout-2)= difference & 0xFF;
			*(jout-1) = (difference>>8) & 0xFF;
			// add block for else
			fls.blockStack[fls.blockStackIndex].savedCursor = out - 3;
			fls.blockStack[fls.blockStackIndex++].blockType = BLOCK_ELSE;
		} else {
			prints("}else{ is not matched with a conditional jump.\n");
		}
		goto loop;
	}
	
	">{" {
		// compare top two items, jump if result is less than or equal
		*out = fithGreaterThanJump;
		finishOutCJUMP:
		out += 3;
		fls.blockStack[fls.blockStackIndex].savedCursor = out;
		fls.blockStack[fls.blockStackIndex++].blockType = BLOCK_CJUMP;
		goto loop;
	}
	
	">={" {
		*out = fithGreaterThanEqualJump;
		goto finishOutCJUMP;
	}
	
	"<{" {
		*out = fithLessThanJump;
		goto finishOutCJUMP;
	}
	
	"<={" {
		*out = fithLessThanEqualJump;
		goto finishOutCJUMP;
	}
	
	"=={" {
		*out = fithEqualJump;
		goto finishOutCJUMP;
	}
	
	"!={" {
		*out = fithNotEqualJump;
		goto finishOutCJUMP;
	}
	
	// A word can be a function or a variable. I could add different syntax
	// for variables versus functions, but that would probably look bad?
	// On the other hand loading and storing could be used as postfix operators.
	// without the postfix operators we are left with more complex discrimiation
	// between if this a global or other etc.
	word {
		u8 *tmpOut = builtInWords(out, start);
		if (tmpOut)
		{
			out = tmpOut;
		} else {
			out = parseWord(out, start, YYCURSOR);
		}
		goto loop;
	}
	
	//~ word_increment {
		//~ *out++ = fithDup;
		//~ goto loop;
	//~ }

	word_definition {
		//~ u32 timerVal = readSysTimerVal(0);
		avlNode *retNode = avl_insert(
			&fls.wordTreeRoot,   // pointer memory holding address of tree
			start,     // pointer to string
			YYCURSOR - start - 1,  // length of string (255 max)
			out );   // value to be stored
		//~ printWord(-readSysTimerVal(timerVal));
		//~ prints("\n");
		fls.blockStack[fls.blockStackIndex++].blockType = BLOCK_FUNCTION;
		if (retNode) {
			PRINT_STRING("word already existed\n");
		} else {
			prints("saving start of word: ");
			uartTX(start, YYCURSOR - start);
			PRINT_STRING("\n");
		}
		goto loop;
	}
	
	*/                               // end of re2c block
}



