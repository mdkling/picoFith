
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
	fithReturn0,
	fithReturn1,
	fithReturn2,
	fithReturn3,
	fithReturn4,
	fithReturn5,
	fithReturn6,
	fithReturn7,
	fithReturn8,
	fithCallFunc0,
	fithCallFunc1,
	fithCallFunc2,
	fithCallFunc3,
	fithCallFunc4,
	fithCallFunc5,
	fithCallFunc6,
	fithCallFunc7,
	fithCallFunc8,
	fithDiv,
	fithMod,
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
	fithZalloc,
	fithFree,
	fithRealloc,
	fithBitwiseNot,
	fithNegate,
	fithBitwiseAnd,
	fithBitwiseOr,
	fithBitwiseXor,
	fithLshift,
	fithRshift,
	fithSwap,
	fithOver,
	fithLoadGlobal,
	fithStoreGlobal,
	fithLoadLocal0,
	fithLoadLocal1,
	fithLoadLocal2,
	fithLoadLocal3,
	fithLoadLocal4,
	fithLoadLocal5,
	fithLoadLocal6,
	fithLoadLocal7,
	fithStoreLocal0,
	fithStoreLocal1,
	fithStoreLocal2,
	fithStoreLocal3,
	fithStoreLocal4,
	fithStoreLocal5,
	fithStoreLocal6,
	fithStoreLocal7,
	fithRandom,
	
	
	
	
	
	
	
	
	
	
	
	
	
	
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

typedef struct TypeInfo {
	avlNode *members;
} TypeInfo;

typedef struct VarInfo {
	u32  value;
	TypeInfo *t;
} VarInfo;

typedef struct fithLexState {
	u8         insideFunction;
	u8         pad1;
	u8         pad2;
	u8         pad3;
	u8        *outBufferStart;
	u8        *outBufferCursor;
	u8        *currentFunctionOut;
	s32        globalsBufferSize;
	avlNode   *wordTreeRoot;
	avlNode   *localVarsRoot;
	s32        numLocalVars;
	s32        numGlobalVars;
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
	.globalsBufferSize = 32,
	.blockStackIndex   = 0,
	.blockStack        = blockStackMem,
};

void
fithRegistersInit(void)
{
	fithExecutionState.globals = zalloc(32);
	fithExecutionState.returnStack = zalloc(64);
	fithExecutionState.bottomOfExprStack = zalloc(64*4);
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

s32
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
	var_declaration = word "$";
	var_assign = word "=";
	var_decl_assign = word "$=";
	const_declaration = word ":=";
	word_increment = word "++";
	function_call_addr = "@" word ;
	function_definition = word ":";

	
*/                                   // end of re2c block

#include "parser.c"
#include "pio.c"
#include "helperCpu.c"
#include "random.c"

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
	
	"zalloc" {
		*out++ = fithZalloc;
		return out;
	}
	
	"free" {
		*out++ = fithFree;
		return out;
	}
	
	"realloc" {
		*out++ = fithRealloc;
		return out;
	}
	
	"neg" {
		*out++ = fithNegate;
		return out;
	}
	
	"bw-not" {
		*out++ = fithBitwiseNot;
		return out;
	}
	
	"bw-and" {
		*out++ = fithBitwiseAnd;
		return out;
	}
	
	"bw-or" {
		*out++ = fithBitwiseOr;
		return out;
	}
	
	"bw-xor" {
		*out++ = fithBitwiseXor;
		return out;
	}
	
	"l-shift" {
		*out++ = fithLshift;
		return out;
	}
	
	"r-shift" {
		*out++ = fithRshift;
		return out;
	}
	
	"swap" {
		*out++ = fithSwap;
		return out;
	}
	
	"over" {
		*out++ = fithOver;
		return out;
	}
	
	"random" {
		*out++ = fithRandom;
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
		helper_unlock();
		return out;
	}
	
	"sendMsg" {
		helper_send();
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
		s32 val = s2i(start);
		if (val > 8 || val < 0)
		{
			out = writeInteger(out, val);
		} else {
			*out++ = val;
		}
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
		out = parseCloseBlock(out);
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
	
	// This is how variables are declared, each var is a typeless 4 byte word.
	// we first search the immediate context to see if the variable already
	// exists. There are two contexts to keep things simple, global and function
	// and if we are defining a function we are in that context otherwise we
	// are in the global context.
	var_declaration {
		parseVarDecl(start, YYCURSOR);
		goto loop;
	}
	
	const_declaration {
		prints("Not sure if there will be constants.\n");
		goto loop;
	}
	
	var_assign {
		out = parseVarAssignment(start, YYCURSOR, out);
		goto loop;
	}
	
	var_decl_assign {
		parseVarDecl(start, YYCURSOR - 1);
		out = parseVarAssignment(start, YYCURSOR - 1, out);
		goto loop;
	}

	word_definition {
		parseWordDefinition(out, start, YYCURSOR);
		goto loop;
	}
	
	*/                               // end of re2c block
}



