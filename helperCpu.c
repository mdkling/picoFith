// helperCpu.c

#define SIO_BASE  0xD0000000
#define SIO_FIFO_ST (SIO_BASE+0x50)

#define PPB_BASE              0xE0000000
#define PPB_INTERRUPT_ENABLE  (PPB_BASE+0xE100)
#define PPB_INTERRUPT_DISABLE (PPB_BASE+0xE180)
#define PPB_INTERRUPT_PEND    (PPB_BASE+0xE280)
#define PPB_SYS_CNTRL         (PPB_BASE+0xED10)

#define FIFO_BUFFER      0x20040000
#define FIFO_WRITER_ADDR 0x20040100
#define FIFO_READER_ADDR 0x20040104

extern u32 vector_table[];

typedef struct ProcessorFifo {
	volatile u32 status;
	volatile u32 write;
	volatile u32 read;
} ProcessorFifo;

typedef struct FifoPtrs {
	volatile u32 *writer;
	         //~ u32 reader;
} FifoPtrs;

void
helper_hello(void)
{
	// wait for an event
	//~ asm("wfe");
	// wait for an another event
	//~ asm("wfe");
	prints("Helper says Hello World!\n");
	while(1){ asm("wfe"); }
}

void
helper_init(void)
{
	FifoPtrs *fifoPtrs = (void*)FIFO_WRITER_ADDR;
	u32      *reader   = (void*)FIFO_BUFFER;
	{
		// clear error flags in FIFO
		ProcessorFifo *fifo = (void*)SIO_FIFO_ST;
		fifo->status = 0x0C;
		// disable all interrupts
		u32 *intDisable = (void*)PPB_INTERRUPT_DISABLE;
		*intDisable = 0xFFFFFFFF;
		// clear all pending interrupts
		u32 *intPend = (void*)PPB_INTERRUPT_PEND;
		*intPend = 0xFFFFFFFF;
		// initialize fifoPtrs
		fifoPtrs->writer = FIFO_BUFFER;
		//~ fifoPtrs->reader = FIFO_BUFFER;
		// enable SIO interrupt
		u32 *intEnable = (void*)PPB_INTERRUPT_ENABLE;
		*intEnable = (1<<16);
		// disable deep sleep and disable "wake up on every possible event"
		u32 *sysCtrl = (void*)PPB_SYS_CNTRL;
		*sysCtrl = 0;
	}
	// enter helper loop
while (1)
{
	asm("wfi");
	// we have had an interrupt check for data
	while (reader != fifoPtrs->writer)
	{
		u32 data = *reader++;
		if (reader == (u32*)FIFO_WRITER_ADDR)
		{
			reader = (void*)FIFO_BUFFER;
		}
		//~ printWord(data);
		//~ prints("\n");
		if ((data & 0xFF) == 0)
		{
			// message is to allocate memory into the cache
			populateCache(data>>8);
		} else if ((data & 0xFF) == 1) {
			// message is to free memory
			takeSpinLock(MEMORY_LOCK_NUMBER);
			free_internal(0x20000000 + (data>>8));
			giveSpinLock(MEMORY_LOCK_NUMBER);
		}
	}
}
}

void
helper_send(void)
{
	ProcessorFifo *fifo = (void*)SIO_FIFO_ST;
	//~ printWord(fifo->status);
	//~ prints("\n");
	fifo->write = 1;
	fifo->write = 2;
	fifo->write = 3;
	fifo->write = 4;
	// send an event
	//~ asm("sev");
}

void
helper_sendMsg1(u32 data1)
{
	ProcessorFifo *fifo = (void*)SIO_FIFO_ST;
	fifo->write = data1;
}

void
helper_sendMsg2(u32 data1, u32 data2)
{
	ProcessorFifo *fifo = (void*)SIO_FIFO_ST;
	fifo->write = data1;
	fifo->write = data2;
}

void
helper_sendMsg3(u32 data1, u32 data2, u32 data3)
{
	ProcessorFifo *fifo = (void*)SIO_FIFO_ST;
	fifo->write = data1;
	fifo->write = data2;
	fifo->write = data3;
}

void
helper_sendMsg4(u32 data1, u32 data2, u32 data3, u32 data4)
{
	ProcessorFifo *fifo = (void*)SIO_FIFO_ST;
	fifo->write = data1;
	fifo->write = data2;
	fifo->write = data3;
	fifo->write = data4;
}

void
helper_unlock(void)
{
	u32 data;
	ProcessorFifo *fifo = (void*)SIO_FIFO_ST;
	// drain input FIFO
	// consume event from system
	//~ asm("wfe");
	while (fifo->status&1)
	{
		data = fifo->read;
		//~ prints("draining fifo ");
		//~ printWord(data);
		//~ prints("\n");
	}
	// send an event
	//~ asm("sev");
	// send a zero
	fifo->write = 0;
	// send an event
	asm("sev");
	// wait for an event
	//~ asm("wfe");
	// read fifo
	while ((fifo->status&1) == 0) { }
	data = fifo->read;
	//~ prints("response to 0 ");
	//~ printWord(fifo->read);
	//~ prints("\n");
	//==========================================================================
	// send a 1
	fifo->write = 1;
	// send an event
	asm("sev");
	// wait for an event
	//~ asm("wfe");
	// read fifo
	while ((fifo->status&1) == 0) { }
	data = fifo->read;
	//~ prints("response to 1 ");
	//~ printWord(fifo->read);
	//~ prints("\n");
	//==========================================================================
	// send a vector table
	fifo->write = vector_table;
	// send an event
	asm("sev");
	// wait for an event
	//~ asm("wfe");
	// read fifo
	while ((fifo->status&1) == 0) { }
	data = fifo->read;
	//~ prints("response to Vector Table ");
	//~ printWord(fifo->read);
	//~ prints("\n");
	//==========================================================================
	// send a stack pointer
	fifo->write = 0x20042000 - 0x1800;
	// send an event
	asm("sev");
	// wait for an event
	//~ asm("wfe");
	// read fifo
	while ((fifo->status&1) == 0) { }
	data = fifo->read;
	//~ prints("response to stack pointer ");
	//~ printWord(fifo->read);
	//~ prints("\n");
	//==========================================================================
	// send an entry point
	fifo->write = helper_init;
	// send an event
	asm("sev");
	// wait for an event
	//~ asm("wfe");
	// read fifo
	while ((fifo->status&1) == 0) { }
	data = fifo->read;
	//~ prints("response to entry point ");
	//~ printWord(fifo->read);
	//~ prints("\n");
	// send an event
	//~ prints("end\n");
	//~ asm("sev");
}




