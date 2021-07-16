// pio.c

#define IO_BANK0_BASE 0x40014000
#define GPIO_20_CTRL  (IO_BANK0_BASE+0xA4)
#define GPIO_21_CTRL  (IO_BANK0_BASE+0xAC)
#define GPIO_22_CTRL  (IO_BANK0_BASE+0xB4)

#define PIO_0_BASE          0x50200000
#define PIO_0_CTRL          PIO_0_BASE
#define PIO_0_FIFOSTATUS    (PIO_0_BASE+0x04)
#define PIO_0_FIFOLVL       (PIO_0_BASE+0x0C)
#define PIO_0_SM0_TX        (PIO_0_BASE+0x10)
#define PIO_0_INSTR_MEM     (PIO_0_BASE+0x48)
#define PIO_0_SM0_CLK       (PIO_0_BASE+0xC8)
#define PIO_0_SM0_EXEC      (PIO_0_BASE+0xCC)
#define PIO_0_SM0_SHIFTCRTL (PIO_0_BASE+0xD0)
#define PIO_0_SM0_INSTRADDR (PIO_0_BASE+0xD4)
#define PIO_0_SM0_INSTR     (PIO_0_BASE+0xD8)
#define PIO_0_SM0_PINCTRL   (PIO_0_BASE+0xDC)

u32 pioInstructions[] =  {
// bitloop1:
	/* 00 */0x7001, // out pins, 1       side 0b10
	/* 01 */0x1840, // jmp x-- bitloop1  side 0b11
    /* 02 */0x6001, // out pins, 1       side 0b00
    /* 03 */0xE82E, // set x, 14         side 0b01
// bitloop0:
    /* 04 */0x6001, // out pins, 1       side 0b00
    /* 05 */0x0844, // jmp x-- bitloop0  side 0b01
    /* 06 */0x7001, // out pins, 1       side 0b10
// public entry_point:
    /* 07 */0xF82E, // set x, 14         side 0b11
};


void
configPIO(void)
{
	volatile u32 *deviceCtrl;
	// set each of the following GPIO's to be under the control of PIO_0
	deviceCtrl = (u32*)GPIO_20_CTRL;
	*deviceCtrl = 6;
	deviceCtrl = (u32*)GPIO_21_CTRL;
	*deviceCtrl = 6;
	deviceCtrl = (u32*)GPIO_22_CTRL;
	*deviceCtrl = 6;
	deviceCtrl = (u32*)PIO_0_SM0_CLK;
	*deviceCtrl = (20480<<16);
	// set wrap top and bottom
	deviceCtrl = (u32*)PIO_0_SM0_EXEC;
	*deviceCtrl = (7<<12)|(0<<7);
	// merge RX into TX FIFO
	// enable auto pull
	deviceCtrl = (u32*)PIO_0_SM0_SHIFTCRTL;
	*deviceCtrl = (1<<30)|(1<<17);
	// set up side set bits
	// set up number of pins set by OUT
	// set up pin for base of sideset
	// set up pin for base of OUT
	deviceCtrl = (u32*)PIO_0_SM0_PINCTRL;
	*deviceCtrl = (2<<29)|(3<<26)|(1<<20)|(21<<10)|(20<<5)|(20<<0);
	// program instructions
	deviceCtrl = (u32*)PIO_0_INSTR_MEM;
	for (u32 i = 0; i < ARRAY_SIZE(pioInstructions); i++)
	{
		deviceCtrl[i] = pioInstructions[i];
	}
	
	// start up state machine
	deviceCtrl = (u32*)PIO_0_CTRL;
	*deviceCtrl = (1<<0)|(1<<4)|(1<<8); // reset and enable state machine 0
	deviceCtrl = (u32*)PIO_0_SM0_INSTR;
	*deviceCtrl = 0xE087, // 0: set pindirs, 3
	prints(" delay print so maybe the next instruction will work!\n");
	//~ *deviceCtrl = 0x0007; // jump to entry point
	deviceCtrl = (u32*)PIO_0_SM0_INSTRADDR;
	printWord(*deviceCtrl);
	prints(" instruction addr!\n");
}


u32 soundData[] =  {
	0x00000000,
	0x11111111,
	0x22222222,
	0x33333333,
	0x44444444,
	0x55555555,
	0x66666666,
	0x77777777,
	0x88888888,
	0x99999999,
	0xAAAAAAAA,
	0xBBBBBBBB,
	0xCCCCCCCC,
	0xDDDDDDDD,
	0xEEEEEEEE,
	0xFFFFFFFF,
};

u32 squareData[] =  {
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
	0xFFFFFFFF,
};

void
writeSound(u32 sound)
{
	volatile u32 *outputFifo;
	outputFifo = (u32*)PIO_0_SM0_TX;
	*outputFifo = sound;
}

u32
readFifoStatus(void)
{
	volatile u32 *fifoStatus;
	fifoStatus = (u32*)PIO_0_FIFOLVL;
	return *fifoStatus;
}

void
outputSound(void)
{
	volatile u32 *outputFifo;
	volatile u32 *deviceCtrl;
	u32 count = 10000;
	outputFifo = (u32*)PIO_0_SM0_TX;
	//~ deviceCtrl = (u32*)PIO_0_SM0_INSTRADDR;
	deviceCtrl = (u32*)PIO_0_SM0_INSTR;
	*deviceCtrl = 0x0007, // entry point
	deviceCtrl = (u32*)PIO_0_FIFOSTATUS;
	printWord(*deviceCtrl);
	printWord(readFifoStatus());
	prints("\n");
	//~ *outputFifo = soundData[0];
	writeSound(soundData[0]);
	printWord(readFifoStatus());
	prints("\n");
	//~ *outputFifo = soundData[1];
	writeSound(soundData[1]);
	printWord(readFifoStatus());
	prints("\n");
	//~ *outputFifo = soundData[2];
	writeSound(soundData[2]);
	printWord(readFifoStatus());
	prints("\n");
	//~ *outputFifo = soundData[3];
	writeSound(soundData[3]);
	printWord(readFifoStatus());
	prints("\n");
	//~ *outputFifo = soundData[4];
	writeSound(soundData[4]);
	printWord(readFifoStatus());
	prints("\n");
	//~ *outputFifo = soundData[5];
	writeSound(soundData[5]);
	//~ writeSound(soundData[6]);
	//~ writeSound(soundData[7]);
	//~ writeSound(soundData[8]);
	//~ writeSound(soundData[9]);
	//~ writeSound(soundData[10]);
	//~ writeSound(soundData[5]);
	//~ writeSound(soundData[6]);
	//~ writeSound(soundData[7]);
	//~ writeSound(soundData[8]);
	//~ writeSound(soundData[9]);
	//~ writeSound(soundData[10]);
	//~ printWord(*deviceCtrl);
	printWord(readFifoStatus());
	prints("\n");
	
	//~ volatile u32 *fifoStatus;
	//~ fifoStatus = (u32*)PIO_0_FIFOLVL;
	deviceCtrl = (u32*)PIO_0_SM0_INSTRADDR;
	printWord(*deviceCtrl);
		prints("\n");
	while (count!=0)
	{
		while ( (readFifoStatus()&0xF) > 7) { }
		*outputFifo = squareData[count&0xF];
		//~ *outputFifo = 0xFFFFFFFF;
		count--;
	}
	printWord(*deviceCtrl);
		prints("\n");
	prints("sound has been sent!\n");
}
