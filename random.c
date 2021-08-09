// random.c

//~ u32 randomState[] = {
//~ 0x4979B167,
//~ 0x53AD8C65,
//~ 0x226A5E44,
//~ 0x3E7D7F52,
//~ 0x29530A58,
//~ 0xCBB6F3D2,
//~ 0xCEF2B61E,
//~ 0x1A58C0C3,
//~ 0x60E11ACC,
//~ 0x406EBFE3,
//~ 0x5631C685,
//~ 0x24BDB6F5,
//~ 0x84B8C9D6,
//~ 0x1B220806,
//~ 0xC36B2401,
//~ 0x1EBFFF9A,
//~ 0x77F80685,
//~ 0x99C79C32,
//~ 0xF08927D4,
//~ 0x9AADA2BB,
//~ 0xC13CD6D9,
//~ 0x56ED5D81,
//~ 0x6212D962,
//~ 0x4565EF11,
//~ 0x7258B344,
//~ 0x6931C8FE,
//~ 0x65CA7752,
//~ 0xF08B36CB,
//~ 0xF75A8370,
//~ 0x0F48E5A2,
//~ 0x40EF48F3,
//~ 0xEB710748,
//~ 0xEF49F5CE,
//~ 0x2835ABFC,
//~ 0x399B2556,
//~ 0x35663E28,
//~ 0xBAF61AC5,
//~ 0xC255C655,
//~ 0xBC1B8B9F,
//~ 0x7C5BDE53,
//~ 0x76E4EE58,
//~ 0x32C5B943,
//~ 0xCF9B61A5,
//~ 0xB85D4DA5,
//~ 0x89BC1A57,
//~ 0xA23E2D4F,
//~ 0xE5087BB9,
//~ 0xC08CDCA3,
//~ 0x78D01A5A,
//~ 0xC92922AE,
//~ 0x172A4C0B,
//~ 0x6F95E6E1,
//~ 0x4C71CF5B,
//~ 0xFDEC976D,
//~ 0x425A7A74,
//~ 0x9DC91B2B,
//~ 0xC0297599,
//~ 0x92084E86,
//~ 0x2984E2F6,
//~ 0x70782E8B,
//~ 0xD70CED90,
//~ 0x22AAC790,
//~ 0x03145F9A,
//~ 0xA245EEAD,
//~ };

typedef struct PrngT{
  u32 produce_count;          /* True if initialized */
  u8 i, j;            /* State variables */
  u32 state[64];          /* State variables */
} PrngT;

static PrngT Prng = {
	0,65,85,{
0x4979B167,
0x53AD8C65,
0x226A5E44,
0x3E7D7F52,
0x29530A58,
0xCBB6F3D2,
0xCEF2B61E,
0x1A58C0C3,
0x60E11ACC,
0x406EBFE3,
0x5631C685,
0x24BDB6F5,
0x84B8C9D6,
0x1B220806,
0xC36B2401,
0x1EBFFF9A,
0x77F80685,
0x99C79C32,
0xF08927D4,
0x9AADA2BB,
0xC13CD6D9,
0x56ED5D81,
0x6212D962,
0x4565EF11,
0x7258B344,
0x6931C8FE,
0x65CA7752,
0xF08B36CB,
0xF75A8370,
0x0F48E5A2,
0x40EF48F3,
0xEB710748,
0xEF49F5CE,
0x2835ABFC,
0x399B2556,
0x35663E28,
0xBAF61AC5,
0xC255C655,
0xBC1B8B9F,
0x7C5BDE53,
0x76E4EE58,
0x32C5B943,
0xCF9B61A5,
0xB85D4DA5,
0x89BC1A57,
0xA23E2D4F,
0xE5087BB9,
0xC08CDCA3,
0x78D01A5A,
0xC92922AE,
0x172A4C0B,
0x6F95E6E1,
0x4C71CF5B,
0xFDEC976D,
0x425A7A74,
0x9DC91B2B,
0xC0297599,
0x92084E86,
0x2984E2F6,
0x70782E8B,
0xD70CED90,
0x22AAC790,
0x03145F9A,
0xA245EEAD,
} };

//~ void 
//~ fithRandomnessInit(void)
//~ {
	//~ u32 i;
	//~ u8 t;
	//~ Prng.j = 0;
	//~ Prng.i = 0;
	//~ for(i=0; i<256; i++){
		//~ Prng.s[i] = (u8)i;
	//~ }
	//~ for(i=0; i<256; i++){
		//~ Prng.j += Prng.s[i] + ((u8*)randomSeedData)[i];
		//~ t = Prng.s[Prng.j];
		//~ Prng.s[Prng.j] = Prng.s[i];
		//~ Prng.s[i] = t;
	//~ }
	//~ // throw away first chunk of data
	//~ for(i=0; i<256; i++){
		//~ Prng.i++;
		//~ t = Prng.s[Prng.i];
		//~ Prng.j += t;
		//~ Prng.s[Prng.i] = Prng.s[Prng.j];
		//~ Prng.s[Prng.j] = t;
		//~ t += Prng.s[Prng.i];
	//~ }
//~ }

/*
** Return N random bytes.
*/
void 
fithRandomness(u32 N, void *pBuf)
{
	u8 *zBuf = pBuf;
	u8 t;
	u8 *s = (u8*)Prng.state;

	//~ if( (Prng.produce_count&0x40000000)==0 )
	//~ {
		//~ Prng.produce_count = 0x40000000;
		//~ fithRandomnessInit();
	//~ }
	Prng.produce_count+=N;

	do{
		Prng.i++;
		t = s[Prng.i];
		Prng.j += t;
		s[Prng.i] = s[Prng.j];
		s[Prng.j] = t;
		t += s[Prng.i];
		*(zBuf++) = s[t];
	}while( --N );
}

u32
random32(void)
{
	u32 random;
	fithRandomness(4, &random);
	return random;
}


