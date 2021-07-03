
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
	
	
	
};


typedef struct fithLexState {
	u8  *outBufferStart;
	u8  *outBufferCursor;
	s32  globalsBufferSize;
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

#define PRINT_STRING(string) uartTX(string, sizeof string - 1)


u8  __bss_end__[4];
fithRegisters fithExecutionState;

static avlNode      *wordTreeRoot;
static fithLexState  fls = {
	__bss_end__, 0, 0 };

void
fithRegistersInit(void)
{
	fithExecutionState.globals = zalloc(16);
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
	
	scm = "\\" [^\n\x00]*;
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
	word = [a-zA-Z_] [a-zA-Z_0-9?!-]*;
	word_definition = word "{";
	function_call_addr = "@" word ;
	function_definition = word ":";

	
*/                                   // end of re2c block


void
fithLexLine(u8 *line)
{
	u8  *YYMARKER;
	u8  *start;
	u8  *out;
	u8  *YYCURSOR = line;
	
	if (fls.outBufferCursor != 0)
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
		if (fls.outBufferCursor == 0)
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
		// output return
		*out++ = fithReturn;
		// move start forward so we save the function we created
		fls.outBufferStart  = out;
		fls.outBufferCursor = 0;
		goto loop;
	}
	
	"walk" {
		walkNodes(wordTreeRoot);
		goto loop;
	}
	
	word {
		avlNode *retNode = avl_find(
			wordTreeRoot,     // pointer to tree
			start,      // pointer to string
			YYCURSOR - start);  // length of string (255 max)
		if (retNode) {
			s32 difference = ((u8*)retNode->value) - (out + 3);
			*out++ = fithCallFunc;
			*out++ = difference & 0xFF;
			*out++ = (difference>>8) & 0xFF;
		}
	}
	
	word_definition {
		avlNode *retNode = avl_insert(
			&wordTreeRoot,   // pointer memory holding address of tree
			start,     // pointer to string
			YYCURSOR - start - 1,  // length of string (255 max)
			out );   // value to be stored
		//~ printWord((s32)retNode);
		fls.outBufferCursor = out;
		//~ PRINT_STRING("\n");
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



