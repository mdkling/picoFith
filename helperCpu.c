// helperCpu.c

#define SIO_BASE  0xD0000000
#define SIO_FIFO_ST (SIO_BASE+0x50)

extern u32 vector_table[];

typedef struct ProcessorFifo {
	volatile u32 status;
	volatile u32 write;
	volatile u32 read;
} ProcessorFifo;

void
helper_hello(void)
{
	// wait for an event
	//~ asm("wfe");
	// wait for an another event
	asm("wfe");
	prints("Helper says Hello World!\n");
	while(1){ asm("wfe"); }
}

void
helper_speak(void)
{
	u32 data;
	ProcessorFifo *fifo = (void*)SIO_FIFO_ST;
	// drain input FIFO
	// consume event from system
	asm("wfe");
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
	asm("wfe");
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
	asm("wfe");
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
	asm("wfe");
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
	asm("wfe");
	// read fifo
	while ((fifo->status&1) == 0) { }
	data = fifo->read;
	//~ prints("response to stack pointer ");
	//~ printWord(fifo->read);
	//~ prints("\n");
	//==========================================================================
	// send an entry point
	fifo->write = helper_hello;
	// send an event
	asm("sev");
	// wait for an event
	asm("wfe");
	// read fifo
	while ((fifo->status&1) == 0) { }
	data = fifo->read;
	//~ prints("response to entry point ");
	//~ printWord(fifo->read);
	//~ prints("\n");
	// send an event
	//~ prints("end\n");
	asm("sev");
}




