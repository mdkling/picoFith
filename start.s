
.cpu cortex-m0plus
.thumb
.syntax unified

;@ Section Constants
ATOMIC_XOR         = 0x1000
ATOMIC_SET         = 0x2000
ATOMIC_CLR         = 0x3000
PBB_BASE           = 0xE0000000
SYST_CSR           = PBB_BASE + 0xE010
SYST_RVR           = 0x04
SYST_CVR           = 0x08
SYST_CALIB         = 0x0C
EXTERNAL_INTS_E      = PBB_BASE + 0xE180
EXTERNAL_INTS      = PBB_BASE + 0xE280
ICSR_REG           = PBB_BASE + 0xED04
VTOR               = PBB_BASE + 0xED08
XOSC_BASE          = 0x40024000
XOSC_CTRL_RW       = XOSC_BASE + 0x00
XOSC_CTRL_SET      = XOSC_CTRL_RW + 0x2000
XOSC_STATUS        = XOSC_BASE + 0x04
XOSC_STARTUP       = XOSC_BASE + 0x0C
CLOCKS_BASE        = 0x40008000
CLK_REF_CTRL       = CLOCKS_BASE + 0x30
CLK_REF_SELECTED   = CLOCKS_BASE + 0x38
CLK_SYS_CTRL       = CLOCKS_BASE + 0x3C
CLK_SYS_SELECTED   = CLOCKS_BASE + 0x44
CLK_PERI_CTRL      = CLOCKS_BASE + 0x48
CLK_SYS_RESUS_CTRL = CLOCKS_BASE + 0x78
PSM_BASE           = 0x40010000
PSM_WDSEL          = PSM_BASE + 0x08
SYSCFG_BASE        = 0x40004000
PROC_0_NMI         = SYSCFG_BASE + 0x00

SIO_BASE               = 0xD0000000
SIO_GPIO_OUT_CLR       = SIO_BASE + 0x18
SIO_GPIO_OUT_XOR       = SIO_BASE + 0x1C 
SIO_GPIO_OE_SET        = SIO_BASE + 0x24
SIO_GPIO_OE_CLR        = SIO_BASE + 0x28
SIO_SIGNED_DIVIDEND    = 0x068
SIO_SIGNED_DIVISOR     = 0x06C
SIO_QUOTIENT           = 0x070
SIO_REMAINDER          = 0x074
SIO_DIV_CSR            = 0x078


IO_BANK0_BASE          = 0x40014000
IO_GPIO00_CTRL         = IO_BANK0_BASE + 0x004
IO_GPIO01_CTRL         = IO_BANK0_BASE + 0x00C
IO_GPIO12_CTRL         = IO_BANK0_BASE + 0x064
IO_GPIO13_CTRL         = IO_BANK0_BASE + 0x06C


IO_GPIO25_CTRL_RW      = IO_BANK0_BASE + 0x0CC

RESETS_RESET_CLR       = 0x4000C000 + 0x3000
RESETS_RESET_DONE_RW   = 0x4000C000 + 0x8  

WATCHDOG_BASE      = 0x40058000
WATCHDOG_CNTRL     = WATCHDOG_BASE

PLL_SYS_BASE       = 0x40028000
PLL_SYS_CTRL       = PLL_SYS_BASE + 0x00
PLL_SYS_POWER      = PLL_SYS_BASE + 0x04
PLL_SYS_FBDIV      = PLL_SYS_BASE + 0x08
PLL_SYS_PRIM       = PLL_SYS_BASE + 0x0C

UART0_BASE         = 0x40034000
UART0_DR           = 0x000
UART0_FR           = 0x018
UART0_IBRD         = 0x024
UART0_FBRD         = 0x028
UART0_LCR          = 0x02C
UART0_CR           = 0x030
UART0_DMA_CR       = 0x048

DMA_BASE           = 0x50000000
DMA0_READ          = 0x000
DMA0_WRITE         = 0x004
DMA0_TRAN_CNT      = 0x008
DMA0_CTRL          = 0x00C
DMA1_READ          = 0x054
DMA1_WRITE         = 0x058
DMA1_TRAN_CNT      = 0x05C
DMA1_CTRL          = 0x050

DMA_2_BASE         = 0x50000090
DMA2_CTRL          = 0x0
DMA2_READ          = 0x4
DMA2_WRITE         = 0x8
DMA2_TRAN_CNT      = 0xc
      
DELAY_COUNT = 0x00100000

END_OF_RAM = 0x20042000

;@ Section Macros
.macro put32 dst, val
	ldr r0,=dst
	ldr r1,=val
	str r1,[r0]
.endm

.macro PUSH_TOS
	stm  r4!, {r0}
.endm

.macro POP_TOS
	subs r4, 4
	ldr  r0, [r4]
.endm

.macro PUSH_IP
	stm  r6!, {r5}
.endm

.macro POP_IP
	subs r6, 4
	ldr  r5, [r6]
.endm

.macro POP_SCATCH1
	subs r4, 4
	ldr  r1, [r4]
.endm

.macro NEXT_INSTRUCTION
	ldrb r1, [r5]
	lsls r2, r1, 2
	ldr  r2, [r7, r2]
	bx   r2
.endm

;@ Section Vector Table
vector_table:
	b reset
	.balign 4
	.word reset ;@ has to be offset 4
	.word whoisme ;@purgatory  ;@ 2 NMI
	.word whoisme ;@purgatory  ;@ 3 HardFault

	.word REBOOT  ;@ 4 Reserved
	.word REBOOT  ;@ 5 Reserved
	.word REBOOT  ;@ 6 Reserved
	.word REBOOT  ;@ 7 Reserved
	
	.word REBOOT  ;@ 8 Reserved
	.word REBOOT  ;@ 9 Reserved
	.word REBOOT  ;@ 10 Reserved
	.word REBOOT  ;@ 11 SVCall
	
	.word REBOOT  ;@ 12 Reserved
	.word REBOOT  ;@ 13 Reserved
	.word REBOOT  ;@ 14 PendSV
	.word sysTickISR  ;@ 15 SysTick
	
	.word REBOOT   ;@ 16 external interrupt 0
	.word REBOOT
	.word REBOOT
	.word REBOOT
	
	.word REBOOT   ;@ 4
	.word REBOOT
	.word REBOOT
	.word REBOOT
	
	.word REBOOT   ;@ 8
	.word REBOOT
	.word REBOOT
	.word REBOOT
	
	.word REBOOT   ;@ 12
	.word REBOOT
	.word REBOOT
	.word REBOOT
	
	.word REBOOT   ;@ 16
	.word REBOOT
	.word REBOOT
	.word REBOOT
	
	.word REBOOT   ;@ 20
	.word REBOOT
	.word REBOOT
	.word REBOOT
	
	.word REBOOT   ;@ 24
	.word REBOOT
	.word REBOOT
	.word REBOOT
	
	.word REBOOT   ;@ 28
	.word REBOOT
	.word REBOOT
	.word REBOOT

.balign 4
.code 16
.thumb_func
.type sysTickISR, %function
sysTickISR:
	;@~ ldr r0,=DMA_BASE ;@ base reg
	;@~ ldr r1, =255 ;@ reset tranaction count on DMA
	;@~ str r1,[r0, #DMA0_TRAN_CNT + 0x14]
	adr  r0, gpioToggleCount
	ldr  r1, [r0]
	adds r1, 1
	str  r1, [r0]
	cmp  r1, 121
	bne  1f
	movs r2, 1
	str  r2, [r0]
	lsls r3,r2,25
	;@ldr  r0, =SIO_GPIO_OUT_XOR
	str  r3, [r0]
1:
	bx lr

.balign 4
.code 16
.thumb_func
.type whoisme, %function
whoisme:
	mrs r0, IPSR
	bl printWord
	b purgatory

.code 16
.thumb_func
.type purgatory, %function
purgatory:
	b purgatory

.balign 4
gpioToggleCount:
.word 0

.balign 4
.global fithExecutionState
fithExecutionState:
.word 0
.word 0
.word 0
.word 0
.word 0

.balign 256
DMA_READ_BUFFER:
.word 0,0,0,0,0,0,0,0
.word 0,0,0,0,0,0,0,0
.word 0,0,0,0,0,0,0,0
.word 0,0,0,0,0,0,0,0
.word 0,0,0,0,0,0,0,0
.word 0,0,0,0,0,0,0,0
.word 0,0,0,0,0,0,0,0
.word 0,0,0,0,0,0,0,0

;@ Section ISRs


.global reset
.balign 4
.code 16
.thumb_func
.type reset, %function
reset:
	;@ configure vector table
	ldr r1,=VTOR
	ldr r0,=vector_table
	str r0,[r1]
	;@ set stack
	ldr r0,= END_OF_RAM - 8
	msr MSP, r0
	;@ set heap
	ldr r0, =__bss_end__
	mov r8, r0
	movs r2,1
	bl setup
	;@~ clear the BSS using the DMA
	ldr r0, =__bss_start__
	ldr r1, =__bss_end__
	subs r1, r0
	bl setZero
	bl mainLoop
	;@~ bl notmain
	b REBOOT

.balign 4
.code 16
.thumb_func
.global REBOOT
.type REBOOT, %function
REBOOT:
	;@~ ldr r0,=PSM_WDSEL ;@ ENABLE MASS RESET
	;@~ ldr r1,=0x1FFFC
	;@~ str r1,[r0]
	ldr r0,=WATCHDOG_CNTRL ;@ reboot
	ldr r1,=1<<31
	str r1,[r0]
	b purgatory

.balign 4
.ltorg

.balign 4
.code 16
.thumb_func
.type setup, %function
setup:
	push {lr}
	bl resetIOBank
	;@~ bl blinkLED
	bl configClock
	bl configUART
	bl configSysTick
	;@~ bl blinkLED
	;@~ bl blinkLED
	;@ set up terminal
	movs r0, 0x0D
	bl print1Byte
	movs r0, 0x0A
	bl print1Byte
	movs r0, 0x3E
	bl print1Byte
	pop  {pc}

.balign 4
.code 16
.thumb_func
.type resetIOBank, %function
resetIOBank:
	lsls r3,r2,#5
	lsls r1,r2,#10
	adds r3, r1
	lsls r1,r2,#11
	adds r3, r1
	ldr  r0, =RESETS_RESET_CLR
	str  r3, [r0]
	ldr  r0, =RESETS_RESET_DONE_RW
	1:
	ldr  r1,[r0]
	tst  r1, r3
	beq  1b
	lsls r3,r2,25
	ldr  r0, =SIO_GPIO_OE_CLR
	str  r3, [r0]
	ldr  r0, =SIO_GPIO_OUT_CLR
	str  r3, [r0]
	
	;@ movs r1, 5 GPIO control
	movs r1, 7 ;@ PIO 1 control
	ldr  r0, =IO_GPIO25_CTRL_RW
	str  r1, [r0]
	;@ ldr  r0, =SIO_GPIO_OE_SET
	;@ lsls r1,r2,20
	;@ adds r3, r1
	;@ lsls r1,r2,21
	;@ adds r3, r1
	;@ lsls r1,r2,22
	;@ adds r3, r1
	;@ str  r3, [r0]
	bx   lr

;@~ .balign 4
;@~ .code 16
;@~ .thumb_func
;@~ .type blinkLED, %function
;@~ blinkLED:
	;@~ push {lr}
	;@~ movs r2, 1
	;@~ lsls r3,r2,25
	;@~ movs r7, 2
	;@~ ldr r0, =SIO_GPIO_OUT_XOR
;@~ blink:
	;@~ str r3, [r0]
	;@~ ldr r6, =DELAY_COUNT
	;@~ bl delay
	;@~ subs r7, 1
	;@~ bne blink
	;@~ pop  {pc}

;@~ .balign 4
;@~ .code 16
;@~ .thumb_func
;@~ .type delay, %function
;@~ delay:
	;@~ subs r6,#1
	;@~ bne delay
	;@~ bx lr 

.balign 4
.code 16
.thumb_func
.type configClock, %function
configClock:
	ldr r0,=CLK_SYS_RESUS_CTRL ;@ disable resus mis-feature
	ldr r1,=0
	str r1,[r0]
	ldr r0,=XOSC_STARTUP ;@ set timeout, from manual
	ldr r1,=47
	str r1,[r0]
	ldr r0,=XOSC_CTRL_RW ;@ set enable and speed range
	ldr r1,=0xFABAA0
	str r1,[r0]

	ldr r0,=XOSC_STATUS ;@ read status to see if it is ready
1:
	ldr r1,[r0]
	cmp r1, #0
	bge 1b

	ldr r0,=CLK_REF_CTRL ;@ move reference clock to crystal
	ldr r1,=2
	str r1,[r0]
	lsls r3,r2,#2
	ldr r0,=CLK_REF_SELECTED ;@ read status to see if it is ready
1:
	ldr r1,[r0]
	tst r1, r3
	beq 1b
	
	;@ bring up PLL
	lsls r3,r2,#12
	ldr r0, =RESETS_RESET_CLR
	str r3, [r0]
	ldr r0, =RESETS_RESET_DONE_RW ;@ read status to see if it is ready
1:
	ldr r1,[r0]
	tst r1, r3
	beq 1b
	
	ldr r4,=PLL_SYS_BASE ;@ base reg 
	ldr r1,=40
	str r1,[r4, #PLL_SYS_FBDIV - PLL_SYS_BASE] ;@ set up multiplier
	
	ldr r0,=PLL_SYS_POWER + ATOMIC_CLR ;@ turn on power
	ldr r1,=(1<<5)|(1<<0)
	str r1,[r0]
	
	lsls r3,r2,#31
1:
	ldr r1,[r4] ;@ wait for resonance cascade
	tst r1, r3
	beq 1b
	
	ldr r1,=(4<<16)|(1<<12)
	str r1,[r4, #PLL_SYS_PRIM - PLL_SYS_BASE] ;@ set post dividers
	
	;@ turn on post dividers
	ldr r1,=(1<<3)
	str r1,[r0]
	;@ PLL_SYS is now at 120 mhz, pico max is 133, this is 90% of that
	;@ so nice margin of safety
	
	ldr r0,=CLK_SYS_CTRL ;@ set sys clock to PLL_SYS
	ldr r1,=1
	str r1,[r0]
	movs r3,2
	ldr r0,=CLK_REF_SELECTED ;@ read status to see if it is ready
1:
	ldr r1,[r0]
	tst r1, r3
	bne 1b
	
	ldr r0,=CLK_PERI_CTRL ;@ set peripheral clock to the ref clock and enable
	ldr r1,=((1<<11)|(4<<5))
	str r1,[r0]
	;@ we are done
	bx lr
	
.balign 4
.code 16
.thumb_func
.type configClock, %function
configUART:
	;@ Section UART 
	;@bring up UART
	lsls r3,r2,#22
	ldr r0, =RESETS_RESET_CLR
	str r3, [r0]
	ldr r0, =RESETS_RESET_DONE_RW ;@ read status to see if it is ready
1:
	ldr r1,[r0]
	tst r1, r3
	beq 1b
	
	ldr r4,=UART0_BASE ;@ base reg
	ldr r1,=6
	str r1,[r4, #UART0_IBRD] ;@ set up baud
	
	ldr r1,=33
	str r1,[r4, #UART0_FBRD] ;@ set up fractional baud
	
	ldr r1,=0x70
	str r1,[r4, #UART0_LCR] ;@ set up fractional baud
	
	ldr r1,=(1<<9)|(1<<8)|(1<<0)
	str r1,[r4, #UART0_CR] ;@ enable UART, TX and RX
	
	ldr r1,=1<<0
	str r1,[r4, #UART0_DMA_CR] ;@ enable DMA on RX
	
	;@~ ldr r0,=IO_GPIO00_CTRL ;@ set gpio 0 to UART 0 TX
	;@~ ldr r1,=2
	;@~ str r1,[r0]
	
	;@~ ldr r0,=IO_GPIO01_CTRL ;@set gpio 1 to UART 0 RX
	;@~ ldr r1,=2
	;@~ str r1,[r0]
	
	ldr r0,=IO_GPIO12_CTRL ;@ set gpio 0 to UART 0 TX
	ldr r1,=2
	str r1,[r0]
	
	ldr r0,=IO_GPIO13_CTRL ;@set gpio 1 to UART 0 RX
	ldr r1,=2
	str r1,[r0]
	
	;@ get DMA out of reset
	lsls r3,r2,#2
	ldr r0, =RESETS_RESET_CLR
	str r3, [r0]
	ldr r0, =RESETS_RESET_DONE_RW ;@ read status to see if it is ready
1:
	ldr r1,[r0]
	tst r1, r3
	beq 1b
	
	movs r3, r4
	
	ldr r4,=DMA_BASE ;@ base reg
	str r3,[r4, #DMA0_READ] 
	ldr r1, =DMA_READ_BUFFER
	str r1,[r4, #DMA0_WRITE]
	ldr r1, =0xFFFFFFFF ;@ don't over write the size of our buffer!!
	str r1,[r4, #DMA0_TRAN_CNT]
	ldr r1, =(21<<15)|(1<<10)|(8<<6)|(1<<5)|(1<<0) ;@(1<<21)|
	str r1,[r4, #DMA0_CTRL]
	
	ldr r1, =(0x3F<<15)|(1<<21)|(1<<11)|(1<<5)|(1<<4)|(2<<2)|(1<<0)
	str r1,[r4, #DMA1_CTRL]
	
	ldr r4,=DMA_2_BASE ;@ base reg
	ldr r1, =(0x3F<<15)|(1<<21)|(2<<11)|(1<<5)|(2<<2)|(1<<0)
	str r1,[r4, #DMA2_CTRL]
	adr r1, ZERO_CONSTANT_LOCATION
	str r1,[r4, #DMA2_READ]
	
	bx lr

.balign 4
ZERO_CONSTANT_LOCATION:
.word 0

.balign 4
.code 16
.thumb_func
.global dmaWordForwardCopy
.type dmaWordForwardCopy, %function
dmaWordForwardCopy: ;@ r0 = src r1 = dst r2 = size
	ldr  r3,=DMA_BASE ;@ base reg
	str  r0,[r3, #DMA1_READ]
	str  r1,[r3, #DMA1_WRITE]
	lsrs r2, 2
	str  r2,[r3, #DMA1_TRAN_CNT]
	bx lr

.balign 4
.code 16
.thumb_func
.global setZero
.type setZero, %function
setZero: ;@ r0 = dst r1 = size
	ldr  r2,=DMA_2_BASE ;@ base reg
	str  r0,[r2, #DMA2_WRITE]
	lsrs r1, 2
	str  r1,[r2, #DMA2_TRAN_CNT]
	bx lr

.balign 4
.code 16
.thumb_func
.type configSysTick, %function
configSysTick:
	;@ Section SysTick 
	;@~ set up sys tick
	ldr r0, =SYST_CSR
	ldr r1, =1000000 - 1 ;@ 120mhz clock, this puts us at 120 hz
	str r1, [r0, #SYST_RVR]
	movs r1, 0
	str r1, [r0, #SYST_CVR]
	ldr r1, =7
	str r1, [r0]
	
	;@ enable on interrupt, doesnt seem to need it?
	
	bx lr

.balign 4
.ltorg

;@ design doc for fith language
;@ register allocation
;@  r0: TOS
;@  r1: SP
;@  r2: SCRATCH for expressions
;@  r3: 
;@  r4: 
;@  r5: 
;@  r6: 
;@  r7: 
;@ ==================== Restricted Registers ===================================
;@  r8: Cursor in memory for generating functions
;@  r9: Cursor in memory for providing memory
;@ r10: 
;@ r11: 
;@ r12: 
;@ r13: MSP
;@ r14: LR
;@ r15: PC

;@ How do you access globals that are not constant? 
;@ Load address as constant then load value
;@ How do you load large constants? 
;@ PC Relative load with constants appended to end of the function
;@ How do you manage memory?
;@ getNode returns a 32 byte piece of memory set to all zeros
;@ freeNode returns node to pool
;@ getBlock returns a memory block of flat memory, no freeing, 32 byte sizes

.balign 4
.code 16
.thumb_func
.global fithPrepareExecute
.type fithPrepareExecute, %function
fithPrepareExecute: ;@ r0 = address of start of bytecode
	push {r4, r5, r6, r7, lr}
	;@ set up data for fithVM
	ldr  r1, =fithExecutionState
	adr  r7, fithVMjumpTable
	movs r5, r0        ;@ set up bytecode ip
	ldr  r0, [r1]      ;@ load TOS
	ldr  r4, [r1, #16]  ;@ load ESP
	ldr  r6, [r1, #8]  ;@ load RSP
	ldr  r2, [r1, #4] ;@ load GLB 
	mov  r8, r2        ;@ move to high register
	bl fithVM
	ldr  r1, =fithExecutionState
	str  r0, [r1]      ;@ store TOS
	str  r4, [r1, #16]  ;@ store ESP
	movs r6, r0
	ldr  r5, [r1, #12]  ;@ load ESP_BOTTOM
	cmp  r5, r4
	beq  3f
	;@ print stack
	b    2f
1:
	ldr  r0, [r5]
	;@~ movs r0, r6
	bl   printWord
	movs r0, ' '
	bl print1Byte
2:
	adds r5, 4
	cmp  r5, r4
	bne  1b
	movs r0, r6
	bl   printWord
	movs r0, '\n'
	bl print1Byte
3:
	pop {r4, r5, r6, r7, pc}

.balign 4
.pool

fithVMjumpTable:
.word fithConstant0
.word fithConstant1
.word fithConstant2
.word fithConstant3
.word fithConstant4
.word fithConstant5
.word fithConstant6
.word fithConstant7
.word fithConstant8
.word fithConstant1byte
.word fithConstant2byte
.word fithConstant3byte
.word fithConstant4byte
.word fithExit
.word fithAdd
.word fithSub
.word fithMul
.word fithDiv
.word fithMod
.word fithReturn
.word fithCallFunc
.word fithDup
.word fithJump
.word fithGreaterThanJump
.word fithGreaterThanEqualJump
.word fithLessThanJump
.word fithLessThanEqualJump
.word fithEqualJump
.word fithNotEqualJump


.balign 4
.code 16
.thumb_func
.type fithVM, %function
fithVM:
	push {lr}
	;@ load first instruction
	ldrb r1, [r5]
	lsls r1, 2
	ldr  r1, [r7, r1]
	bx   r1

.thumb_func
fithConstant0:	
	PUSH_TOS
	movs r0, 0
	adds r5, 1
	NEXT_INSTRUCTION

.thumb_func
fithConstant1:
	PUSH_TOS
	movs r0, 1
	adds r5, 1
	NEXT_INSTRUCTION

.thumb_func
fithConstant2:
	PUSH_TOS
	movs r0, 2
	adds r5, 1
	NEXT_INSTRUCTION

.thumb_func
fithConstant3:
	PUSH_TOS
	movs r0, 3
	adds r5, 1
	NEXT_INSTRUCTION

.thumb_func
fithConstant4:
	PUSH_TOS
	movs r0, 4
	adds r5, 1
	NEXT_INSTRUCTION

.thumb_func
fithConstant5:
	PUSH_TOS
	movs r0, 5
	adds r5, 1
	NEXT_INSTRUCTION

.thumb_func
fithConstant6:
	PUSH_TOS
	movs r0, 6
	adds r5, 1
	NEXT_INSTRUCTION

.thumb_func
fithConstant7:
	PUSH_TOS
	movs r0, 7
	adds r5, 1
	NEXT_INSTRUCTION

.thumb_func
fithConstant8:
	PUSH_TOS
	movs r0, 8
	adds r5, 1
	NEXT_INSTRUCTION

.thumb_func
fithConstant1byte:
	PUSH_TOS
	ldrb r0, [r5, #1]
	adds r5, 2
	NEXT_INSTRUCTION

.thumb_func
fithConstant2byte:
	PUSH_TOS
	ldrb r0, [r5, #1]
	ldrb r1, [r5, #2]
	lsls r1, 8
	adds r0, r1
	adds r5, 3
	NEXT_INSTRUCTION

.thumb_func
fithConstant3byte:
	PUSH_TOS
	ldrb r0, [r5, #1]
	ldrb r1, [r5, #2]
	lsls r1, 8
	adds r0, r1
	ldrb r1, [r5, #3]
	lsls r1, 16
	adds r0, r1
	adds r5, 4
	NEXT_INSTRUCTION

.thumb_func
fithConstant4byte:
	PUSH_TOS
	ldrb r0, [r5, #1]
	ldrb r1, [r5, #2]
	lsls r1, 8
	adds r0, r1
	ldrb r1, [r5, #3]
	lsls r1, 16
	adds r0, r1
	ldrb r1, [r5, #4]
	lsls r1, 24
	adds r0, r1
	adds r5, 5
	NEXT_INSTRUCTION

.thumb_func
fithExit:
	pop {pc}

.thumb_func
fithAdd:
	POP_SCATCH1
	adds r0, r1
	adds r5, 1
	NEXT_INSTRUCTION

.thumb_func
fithSub:
	POP_SCATCH1
	subs r0, r1, r0
	adds r5, 1
	NEXT_INSTRUCTION

.thumb_func
fithMul:
	POP_SCATCH1
	muls r0, r1
	adds r5, 1
	NEXT_INSTRUCTION

.thumb_func
fithDiv:
	POP_SCATCH1
	ldr  r2, = SIO_BASE
	str  r1, [r2, #SIO_SIGNED_DIVIDEND]
	str  r0, [r2, #SIO_SIGNED_DIVISOR] ;@ now takes 8 cycles to finish
	movs r0, r2			;@ 1
	adds r5, 1			;@ 2
	ldrb r1, [r5]		;@ 4
	lsls r2, r1, 2		;@ 5
	ldr  r2, [r7, r2]	;@ 7
	movs r0, r0			;@ 8
	ldr  r0, [r0, #SIO_QUOTIENT]
	bx   r2

.thumb_func
fithMod:
	POP_SCATCH1
	ldr  r2, = SIO_BASE
	str  r1, [r2, #SIO_SIGNED_DIVIDEND]
	str  r0, [r2, #SIO_SIGNED_DIVISOR] ;@ now takes 8 cycles to finish
	movs r0, r2			;@ 1
	adds r5, 1			;@ 2
	ldrb r1, [r5]		;@ 4
	lsls r2, r1, 2		;@ 5
	ldr  r2, [r7, r2]	;@ 7
	movs r0, r0			;@ 8
	ldr  r0, [r0, #SIO_REMAINDER]
	bx   r2

.thumb_func
fithReturn:
	POP_IP
	NEXT_INSTRUCTION

.thumb_func
fithCallFunc:
	ldrb r2, [r5, #1]
	ldrb r1, [r5, #2]
	lsls r1, 8
	adds r2, r1
	sxth r2, r2
	adds r5, 3
	PUSH_IP
	adds r5, r2
	NEXT_INSTRUCTION

.thumb_func
fithDup:
	PUSH_TOS
	adds r5, 1
	NEXT_INSTRUCTION

.thumb_func
fithJump: ;@ unconditional jump
	ldrb r2, [r5, #1]
	ldrb r1, [r5, #2]
	lsls r1, 8
	adds r2, r1
	sxth r2, r2
	adds r5, r2
	NEXT_INSTRUCTION

.thumb_func
fithGreaterThanJump:
	ldrb r2, [r5, #1]
	ldrb r1, [r5, #2]
	lsls r1, 8
	adds r2, r1
	sxth r2, r2
	subs r4, 8
	ldr  r1, [r4, #4]
	cmp  r1, r0
	bgt  1f
	adds r5, r2
1:  ldr  r0, [r4]
	adds r5, 3
	NEXT_INSTRUCTION

.thumb_func
fithGreaterThanEqualJump:
	ldrb r2, [r5, #1]
	ldrb r1, [r5, #2]
	lsls r1, 8
	adds r2, r1
	sxth r2, r2
	subs r4, 8
	ldr  r1, [r4, #4]
	cmp  r1, r0
	bge  1f
	adds r5, r2
1:  ldr  r0, [r4]
	adds r5, 3
	NEXT_INSTRUCTION

.thumb_func
fithLessThanJump:
	ldrb r2, [r5, #1]
	ldrb r1, [r5, #2]
	lsls r1, 8
	adds r2, r1
	sxth r2, r2
	subs r4, 8
	ldr  r1, [r4, #4]
	cmp  r1, r0
	blt  1f
	adds r5, r2
1:  ldr  r0, [r4]
	adds r5, 3
	NEXT_INSTRUCTION

.thumb_func
fithLessThanEqualJump:
	ldrb r2, [r5, #1]
	ldrb r1, [r5, #2]
	lsls r1, 8
	adds r2, r1
	sxth r2, r2
	subs r4, 8
	ldr  r1, [r4, #4]
	cmp  r1, r0
	ble  1f
	adds r5, r2
1:  ldr  r0, [r4]
	adds r5, 3
	NEXT_INSTRUCTION

.thumb_func
fithEqualJump:
	ldrb r2, [r5, #1]
	ldrb r1, [r5, #2]
	lsls r1, 8
	adds r2, r1
	sxth r2, r2
	subs r4, 8
	ldr  r1, [r4, #4]
	cmp  r1, r0
	beq  1f
	adds r5, r2
1:  ldr  r0, [r4]
	adds r5, 3
	NEXT_INSTRUCTION

.thumb_func
fithNotEqualJump:
	ldrb r2, [r5, #1]
	ldrb r1, [r5, #2]
	lsls r1, 8
	adds r2, r1
	sxth r2, r2
	subs r4, 8
	ldr  r1, [r4, #4]
	cmp  r1, r0
	bne  1f
	adds r5, r2
1:  ldr  r0, [r4]
	adds r5, 3
	NEXT_INSTRUCTION

.balign 4
.code 16
.thumb_func
.type mainLoop, %function
mainLoop:
	push {lr}
	;@ software init
	bl memsys5Init
	bl fithRegistersInit
	;@ end software init
	;@~ ldr  r7, =120*60
1:
	bl   getCharacter
	cmp  r1, 0
	beq  2f ;@ nothing in DMA, goto end
	bl   processChar
	cmp  r5, 0
	beq  1b ;@ no line to process check for another character
	bl   processLine
	movs r0, 0x3E ;@ carrot
	bl   print1Byte
	b    1b
2:
	wfi
	b   1b
	;@~ subs r7, 1
	;@~ bne 1b
	pop  {pc}


.balign 4
.code 16
.thumb_func
.type getCharacter, %function
getCharacter:
	;@ need r0 and r1 for return values
	adr  r3, dmaReadCursor ;@ load address of current cursor
	ldr  r2,[r3] ;@ load current cursor, lock r2 as cursor
	ldr  r1, =DMA_READ_BUFFER ;@ base reg
	adds r0, r1, r2 ;@ compute read cursor addr
	ldr  r1,=DMA_BASE ;@ DMA base reg
	ldr  r1, [r1, #DMA0_WRITE] ;@ load dma write pointer
	subs r1, r0, r1 ;@ r1 needs to be the result of the subtraction
	beq  1f ;@ if equal return 0 in r1
	ldrb r0,[r0] ;@ not equal, load one byte into r0
	adds r2, 1 ;@ increment cursor
	movs r1, 0xFF ;@ load mask
	ands r2, r1   ;@ mask off to important bits
	str  r2,[r3] ;@ store current cursor
1:
	bx lr

.balign 4
.code 16
.thumb_func
.type moveCursorForward, %function
moveCursorForward: ;@ r4 = struct
	push {lr}
	ldr  r0, [r4, 0x04]
	ldr  r1, [r4, 0x08]
	cmp  r0, r1
	bge  1f
	adds r0, 1
	str  r0, [r4, 0x04]
	;@ echo right arrow
	movs r0, 0x1B
	bl print1Byte
	movs r0, 0x5B
	bl print1Byte
	movs r0, 0x43
	bl print1Byte
1:
	pop  {pc}

.balign 4
.code 16
.thumb_func
.type moveCursorBackward, %function
moveCursorBackward: ;@ r4 = struct
	push {lr}
	ldr  r0, [r4, 0x04]
	cmp  r0, 0
	beq  1f
	subs r0, 1
	str  r0, [r4, 0x04]
	movs r0, 0x08 ;@ move cursor back one
	bl print1Byte
1:
	pop  {pc}

.balign 4
.code 16
.thumb_func
.type insertChar, %function
insertChar: ;@ r4 = struct
	push {r5, lr}
	movs r5, r0
	ldr  r1, [r4, 0x04]
	adds r1, 1
	str  r1, [r4, 0x04] ;@ move cursor forward
	ldr  r2, [r4, 0x08]
	adds r2, 1
	str  r2, [r4, 0x08] ;@ increase size by 1
	subs r2, r1 ;@ size of items to be moved
	adds r1, 11 ;@ move cursor to data buffer section
	adds r0, r1, r4 ;@ calculate address
	adds r1, r0, 1  ;@ next address
	bl   copyBackward
	strb r5, [r0] ;@ write char
	pop  {r5, pc}

.balign 4
.code 16
.thumb_func
.type deleteChar, %function
deleteChar: ;@ r4 = struct
	push {lr}
	ldr  r0, [r4, 0x04]
	cmp r0, 0
	beq 1f
	subs r0, 1
	str  r0, [r4, 0x04] ;@ move cursor backward
	ldr  r2, [r4, 0x08]
	subs r2, 1
	str  r2, [r4, 0x08] ;@ decrease size by 1
	subs r2, r0 ;@ size of items to be moved
	adds r0, 13 ;@ move cursor to data buffer section
	adds r0, r4 ;@ calculate address
	subs r1, r0, 1  ;@ previous address
	bl   copyForward
1:
	pop  {pc}

.balign 4
.code 16
.thumb_func
.type printLineam, %function
printLine: ;@ r4 = struct
	push {lr}
	movs r0, 0x0D ;@ restart line
	bl print1Byte
	movs r0, 0x3E ;@ carrot
	bl print1Byte
	ldr  r1, [r4, 0x08]
	movs r0, r4
	adds r0, 12
	bl uartTX
	movs r0, 0x20 ;@ space
	bl print1Byte
	movs r0, 0x08 ;@ backspace
	bl print1Byte ;@ this will correctly clear a line that is getting shorter
	ldr  r1, [r4, 0x08]
	ldr  r2, [r4, 0x04]
	subs r3, r1, r2
	b 2f
1:
	movs r0, 0x08 ;@ move cursor back one
	bl print1Byte
	subs r3, 1
2:
	bne 1b
	;@~ movs r0, 0x0A ;@ space
	;@~ bl print1Byte
	;@~ ldr  r0, [r4, 0x04]
	;@~ bl printWord
	;@~ ldr  r0, [r4, 0x08]
	;@~ bl printWord
	;@~ movs r0, 0x0A ;@ space
	;@~ bl print1Byte
	pop  {pc}

.balign 4
.code 16
.thumb_func
.type copyForward, %function
copyForward: ;@ r0 = src r1 = dst r2 = size
	adds r0, r2
	adds r1, r2
	rsbs r3, r2, 0
	cmp r3, 0
	b 2f
1:
	ldrb r2,[r0, r3] 
	strb r2,[r1, r3] 
	adds r3, 1
2:
	bne 1b
	bx lr

.balign 4
.code 16
.thumb_func
.type copyBackward, %function
copyBackward: ;@ r0 = src r1 = dst r2 = size
	b 2f
1:
	ldrb r3,[r0, r2] 
	strb r3,[r1, r2]
2:
	subs r2, 1
	bge 1b
	bx lr

.global uartTX
.balign 4
.code 16
.thumb_func
.type uartTX, %function
uartTX: ;@ r0 = pointer to data start r1 = size of transmission
	ldr r2,=UART0_BASE ;@ get address of UART
	cmp r1, #0
	b 2f
1:
	ldr r3,[r2, #UART0_FR]
	lsls r3, 26 
	cmp r3, #0
	blt 1b
	ldrb r3,[r0] 
	strb r3,[r2] ;@ write data out the serial port
	adds r0, 1
	subs r1, 1
2:
	bne 1b
	bx lr

.global prints
.balign 4
.code 16
.thumb_func
.type prints, %function
prints: ;@ r0 = pointer to null terminated string
	ldr r1,=UART0_BASE ;@ get address of UART
	b 2f
1:
	ldr  r3, [r1, #UART0_FR]
	lsls r3, 26 
	cmp  r3, #0
	blt  1b
	strb r2, [r1] ;@ write data out the serial port
	adds r0, 1
2:
	ldrb r2, [r0]
	cmp  r2, #0
	bne 1b
	bx lr

.balign 4
.code 16
.thumb_func
.global printWord
.type printWord, %function
printWord: ;@ r0 = data to print
	push {r4,r5,r6,lr}
	ldr r2,=0x0F
	ldr r3,=UART0_BASE ;@ get address of UART
	movs r4, r0
	movs r5, 28
	movs r6, #(1<<5)
1:
	ldr   r0,[r3, #UART0_FR]
	tst   r0, r6
	bne   1b
	movs  r0, r4
	lsrs  r0, r5
	ands  r0, r0, r2
	adds  r0, 48
	cmp   r0, 57
	ble   2f
	adds  r0, 7
2:
	strb  r0,[r3, #UART0_DR] ;@ write data out the serial port
	subs  r5, 4
	bge   1b
	pop {r4,r5,r6,pc}

.balign 4
.code 16
.thumb_func
.type print1Byte, %function
print1Byte: ;@ r0 = data to print
	ldr r1,=UART0_BASE ;@ get address of UART
1:
	ldr r2,[r1, #UART0_FR]
	lsls r2, 26 
	cmp r2, #0
	blt 1b
	strb r0,[r1] ;@ write data out the serial port
	bx lr

.balign 4
.code 16
.thumb_func
.type clearLine, %function
clearLine:
	push {lr}
	movs r0, 0x1B
	bl print1Byte
	movs r0, '[' 
	bl print1Byte
	movs r0, '2' 
	bl print1Byte
	movs r0, 'K' 
	bl print1Byte
	pop {pc}


.balign 4
.code 16
.thumb_func
.type processChar, %function
processChar:
	;@ r0 contains character to process
	;@ returns a completed line to process or nothing?
	;@ depending on the character we may simply echo it
	;@ or we may want re-transmit the whole line
	;@ or we may be ending the line, therefore can push it on for more processing
	;@ DEL = 7f, arrows = 1b, 5b, then left 44 down 42 right 43 up 41
	push {r4, lr}
	movs r5, 0
	adr  r4, currentInputBuff
	ldr  r1,[r4] ;@ load state number
	cmp  r1, 1
	bne  1f
	;@ in state 1, look for second arrow escape
	cmp  r0, 0x5B
	bne  resetTerminalState
	movs r1, 2
	str  r1, [r4]
	pop  {r4, pc}
1:
	cmp  r1, 2
	bne  state0
	;@ in state 2, look for up arrow
	cmp  r0, 0x41
	bne  2f
	;@ process up arrow
	bl clearLine
	adr  r2, inputBuffIndex ;@ get index address
	ldr  r1,[r2, #4] ;@ load index
	subs r1, 1
	movs r3, 0x0F ;@ load mask
	ands r1, r3   ;@ mask off to important bits
	str  r1,[r2, #4] ;@ store index
	movs r2, 128 ;@ load mask
	muls r1, r2
	adr  r0, INPUT_BUFFERS
	adds r0, r1 ;@ load mask
	movs r1, r4
	bl dmaWordForwardCopy
	movs r1, 0
	str  r1, [r4]
	b  outputConsolLine
2:
	;@ in state 2, look for down arrow
	cmp  r0, 0x42
	bne  3f
	bl clearLine
	adr  r2, inputBuffIndex ;@ get index address
	ldr  r1,[r2, #4] ;@ load index
	adds r1, 1
	movs r3, 0x0F ;@ load mask
	ands r1, r3   ;@ mask off to important bits
	str  r1,[r2, #4] ;@ store index
	movs r2, 128 ;@ load mask
	muls r1, r2
	adr  r0, INPUT_BUFFERS
	adds r0, r1 ;@ load mask
	movs r1, r4
	bl dmaWordForwardCopy
	movs r1, 0
	str  r1, [r4]
	b  outputConsolLine
3:
	;@ in state 2, look for right arrow
	cmp  r0, 0x43
	bne  4f
	;@ move cursor one forward
	bl moveCursorForward
	b  resetTerminalState
4:
	;@ in state 2, look for left arrow
	cmp  r0, 0x44
	bne  resetTerminalState
	bl moveCursorBackward
resetTerminalState:
	movs r1, 0
	str  r1, [r4]
	pop  {r4, pc}
state0:
	cmp  r0, 0x1B ;@ start of escape sequence
	bne  1f
	movs r1, 1
	str  r1, [r4]
	pop  {r4, pc}
1:
	cmp  r0, 0x0D ;@ the line may be complete
	bne  2f
	movs r0, 0x0A ;@ restart line
	bl print1Byte
	ldr  r1, [r4, 0x08]
	cmp  r1, 0
	bne  1f
	movs r0, 0x3E ;@ carrot
	bl print1Byte
	pop  {r4, pc}
1:
	adr  r2, inputBuffIndex ;@ get index address
	ldr  r1,[r2] ;@ load index
	movs r0, r1
	adds r0, 1
	movs r3, 0x0F ;@ load mask
	ands r0, r3   ;@ mask off to important bits
	str  r0, [r2] ;@ store incremented address
	str  r0, [r2, #4] ;@ store incremented address
	movs r0, 128
	muls r1, r0 ;@ multiply by buffer size
	adr  r0, INPUT_BUFFERS
	adds r1, r0 ;@ calculate address for buffer to save to
	movs r0, r4
	movs r2, 128
	bl dmaWordForwardCopy
	movs r0, 0
	str r0, [r4, #4]
	str r0, [r4, #8] ;@ zeroise cursor and size
	movs r5, r1
	pop  {r4, pc}
2:
	cmp  r0, 0x7F ;@ backspace input
	bne  normalChar
	;@~ movs r0, 0x0A ;@ restart line
	;@~ bl print1Byte
	bl deleteChar
	b  outputConsolLine
normalChar:	
	bl print1Byte
	bl insertChar
	
outputConsolLine:
	bl printLine
	pop  {r4, pc}

.balign 4
.code 16
.thumb_func
.type processLine, %function
processLine: ;@ r5 contains line data 
	;@ Section Process Line
	;@ we take in a line of text at a time and output a compiled program
	;@ or nothing
	push {r7, lr}
	
	;@~ mov  r7, r8 ;@ starting pointer is in r8
	;@ null terminate input
	ldr  r0, [r5, #8]
	adds r0, 12
	movs r1, 0
	strb r1, [r5, r0]
	;@ move r5 to first character
	adds r5, 12
	
	;@ call c function
	movs r0, r5
	bl fithLexLine
	
	;@~ ;@ load character and increment pointer
;@~ 1:
	;@~ ldrb r0, [r5]
	;@~ adds r5, 1
	;@~ ;@ load charcter class to begin processing
	;@~ adr  r1, charClasses
	;@~ ldrb r2, [r1, r0]
	;@~ cmp  r2, 0
	;@~ beq  1b
	;@~ cmp  r2, 1
	;@~ bne  2f
	;@~ movs r4, 0
	;@~ movs r3, 10
	;@~ b 3
;@~ 2:
	;@~ adds r5, 1
;@~ 3:
	;@~ subs r0, '0'
	;@~ muls r4, r3
	;@~ adds r4, r0
	;@~ ldrb r0, [r5]
	;@~ ldrb r2, [r1, r0]
	;@~ cmp  r2, 1
	;@~ beq  2b
	;@~ ;@ r4 contains integer to be produced, increment r5
	;@~ adds r5, 1
	;@~ bl compileInteger
;@~ 4:
	
	;@~ cmp  r2, 99
	;@~ bne 99f
	;@~ pop  {r7, pc}
;@~ 99:
	;@~ movs r0, 'b'
	;@~ bl print1Byte
	;@~ movs r0, 'a'
	;@~ bl print1Byte
	;@~ movs r0, 'd'
	;@~ bl print1Byte
	;@~ movs r0, '\n'
	;@~ bl print1Byte
	pop  {r7, pc}


.balign 4
.ltorg

dmaReadCursor:
.word 0

;@ data struct
;@ int stateNum
;@ int cursor
;@ int sizeOfLine
inputBuffIndex:
.word 0
.word 0

currentInputBuff:
.word 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

INPUT_BUFFERS:
.word 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
.word 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
.word 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
.word 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
.word 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
.word 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
.word 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
.word 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
.word 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
.word 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
.word 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
.word 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
.word 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
.word 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
.word 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
.word 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0


;@ constants
PIO_1_BASE          = 0x50300000
PIO_1_CTRL          = 0x00
PIO_1_FIFOSTATUS    = 0x04
PIO_1_FIFOLVL       = 0x0C
PIO_1_SM0_TX        = 0x10
PIO_1_INSTR_MEM     = 0x48
PIO_1_SM0_BASE      = PIO_1_BASE + 0xC8
PIO_1_SM0_CLK       = 0x00
PIO_1_SM0_EXEC      = 0x04
PIO_1_SM0_SHIFTCRTL = 0x08
PIO_1_SM0_INSTRADDR = 0x0C
PIO_1_SM0_INSTR     = 0x10
PIO_1_SM0_PINCTRL   = 0x14

.balign 4
.code 16
.thumb_func
.global configPioAsm
.type configPioAsm, %function
configPioAsm: ;@ no arguments
	;@push {lr}
	;@ configure state machine 0 on PIO 1
	ldr  r3, =PIO_1_SM0_BASE
	;@ldr  r0, =((20480*3)<<16)
	ldr  r0, =((337)<<16)
	str  r0, [r3, #PIO_1_SM0_CLK]
	ldr  r0, =(0x1f<<12)|(0<<7) ;@ (1<<17)|
	str  r0, [r3, #PIO_1_SM0_EXEC]
	;@ ldr  r0, =(1<<30)|(1<<17)
	;@ str  r0, [r3, #PIO_1_SM0_SHIFTCRTL]
	ldr  r0, =(1<<26)|(25<<5)
	str  r0, [r3, #PIO_1_SM0_PINCTRL]
	
	;@ program instructions
	subs r3, #(32*4)
	adr  r2, pioInstructions
	movs r1, 0
1:
	ldr  r0, [r2, r1]
	str  r0, [r3, r1]
	adds r1, 4
	cmp  r1, 128
	bne  1b
	
	;@ enable state machine
	subs r3, 0x48
	movs r0, 1
	str  r0, [r3, #0]
	
	;@pop  {pc}
	bx lr


.balign 4
.ltorg

pioInstructions:
;@ .word 0xE081 ;@ 00 set pindirs, 1
;@ .word 0xFF01 ;@ 00 set pindirs, 1
;@ .word 0x0001 ;@ 31 jmp  goto 01 [31]
.word 0xE081 ;@ 00 set pindirs, 1
.word 0xE05F ;@ 01 set y, 31 START_OF_SET_LOOP
.word 0xFF3F ;@ 02 set x, 31
.word 0xFF01 ;@ 03 set pins, 1 [31]
.word 0xFF01 ;@ 04 set pins, 1 [31]
.word 0xFF01 ;@ 05 set pins, 1 [31]
.word 0xFF01 ;@ 06 set pins, 1 [31]
.word 0xFF01 ;@ 07 set pins, 1 [31]
.word 0xFF01 ;@ 08 set pins, 1 [31]
.word 0xFF01 ;@ 09 set pins, 1 [31]
.word 0xFF01 ;@ 10 set pins, 1 [31]
.word 0xFF01 ;@ 11 set pins, 1 [31]
.word 0xFF01 ;@ 12 set pins, 1 [31]
.word 0xFF01 ;@ 13 set pins, 1 [31]
.word 0x1F43 ;@ 14 jmp x-- goto 03 [31]
.word 0x1F82 ;@ 15 jmp y-- goto 02 [31]
.word 0xE05F ;@ 16 set y, 31 START_OF_UNSET_LOOP
.word 0xFF3F ;@ 17 set x, 31
.word 0xFF00 ;@ 18 set pins, 0 [31]
.word 0xFF00 ;@ 19 set pins, 0 [31]
.word 0xFF00 ;@ 20 set pins, 0 [31]
.word 0xFF00 ;@ 21 set pins, 0 [31]
.word 0xFF00 ;@ 22 set pins, 0 [31]
.word 0xFF00 ;@ 23 set pins, 0 [31]
.word 0xFF00 ;@ 24 set pins, 0 [31]
.word 0xFF00 ;@ 25 set pins, 0 [31]
.word 0xFF00 ;@ 26 set pins, 0 [31]
.word 0xFF00 ;@ 27 set pins, 0 [31]
.word 0xFF00 ;@ 28 set pins, 0 [31]
.word 0x1F52 ;@ 29 jmp x-- goto 18 [31]
.word 0x1F91 ;@ 30 jmp y-- goto 17 [31]
.word 0x0001 ;@ 31 jmp  goto 01 [31]

;@ node struct definition
nodeNext0    = 0
nodeNext1    = 4
nodeValue    = 8
nodeLevel    = 12
nodeKeyLen   = 13
nodeKeyStart = 14

.balign 4
.code 16
.thumb_func
.global aa_Insert
.type aa_Insert, %function
aa_Insert: ;@ r0: pointer to tree r1: pointer to key r2: keyLen r3: value
	mov  r8, r5
	adr  r5, nilNode
	cmp  r0, r5
	mov  r5, r8
	beq  makeNode
	cmp  r0, 0
	beq  makeNode
	push {r4, r5, r6, r7, lr}
	movs r4, r0
	adds r4, nodeKeyStart
	movs r7, 0
1:
	ldrb r5, [r1, r7]
	ldrb r6, [r4, r7]
	subs r5, r6 ;@ return value is r5 if not equal to zero
	bne  1f
	adds r7, 1
	cmp  r7, r2
	blt  1b
	ldrb r6, [r0, #nodeKeyLen]
	subs r5, r2, r6 ;@ return value is r2 - r7->keyLen
	bne  1f
	str  r3, [r0, #nodeValue]
	pop  {r4, r5, r6, r7, pc}
1:
	lsrs r5, 31
	lsls r5, 2
	adds r4, r0, r5
	ldr  r0, [r4]
	bl   aa_Insert
	str  r0, [r4]
	subs r0, r4, r5
	bl   skew
	bl   split
	pop  {r4, r5, r6, r7, pc}

.balign 4
.code 16
.thumb_func
.type makeNode, %function
makeNode: ;@ r0: pointer to tree r1: pointer to key r2: keyLen r3: value
	mov  r12, lr
	movs r0, r2 ;@ r0 is 0, and not needed
	adds r0, 18
	push {r1, r2, r3}
	bl   zalloc ;@ r0 is now new address
	pop  {r1, r2, r3}
	str  r3, [r0, #nodeValue]
	adr  r3, nilNode
	str  r3, [r0, #nodeNext0]
	str  r3, [r0, #nodeNext1]
	strb r2, [r0, #nodeKeyLen]
	movs r3, 1
	strb r3, [r0, #nodeLevel]
	adds r0, nodeKeyStart
	movs r3, 0
	strb r3, [r0, r2] ;@ null terminate
1: ;@ copy loop
	subs r2, 1
	ldrb r3, [r1, r2]
	strb r3, [r0, r2]
	bne  1b
	subs r0, nodeKeyStart
	bx   r12

.balign 4
.code 16
.thumb_func
.global walkAA
.type walkAA, %function
walkAA: ;@ r0: pointer to tree
	push {r4, lr}
	adr  r1, nilNode
	cmp  r0, r1
	beq  1f
	movs r4, r0
	movs r1, nodeKeyStart
	adds r0, r4, r1
	bl   prints
	ldr  r0, [r4, #0]
	bl   walkAA
	ldr  r0, [r4, #4]
	bl   walkAA
1:
	pop  {r4, pc}

.balign 4
.ltorg

nilNode:
.word nilNode
.word nilNode
.word 0
.word 0


;@~ charClasses:
;@~ ;@ 00   0   1   2   3   4   5   6   7   8  \t  \n  11  12  \r  14  15
;@~ .byte  99,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
;@~ ;@ 10  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31
;@~ .byte   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
;@~ ;@ 20 SPC   !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /
;@~ .byte   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
;@~ ;@ 30   0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?
;@~ .byte   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0
;@~ ;@ 40   @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O
;@~ .byte   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
;@~ ;@ 50   P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _
;@~ .byte   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
;@~ ;@ 60   `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o
;@~ .byte   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
;@~ ;@ 70   p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~ DEL
;@~ .byte   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0

;@~ .global __gnu_thumb1_case_uqi
;@~ .thumb_func
;@~ __gnu_thumb1_case_uqi:
	;@~ mov     r12, r1
	;@~ mov     r1, lr
	;@~ lsrs    r1, r1, #1
	;@~ lsls    r1, r1, #1
	;@~ ldrb    r1, [r1, r0]
	;@~ lsls    r1, r1, #1
	;@~ add     lr, lr, r1
	;@~ mov     r1, r12
	;@~ bx      lr

;@~ .global __gnu_thumb1_case_sqi
;@~ .thumb_func
;@~ __gnu_thumb1_case_sqi:
	;@~ mov     r12, r1
	;@~ mov     r1, lr
	;@~ lsrs    r1, r1, #1
	;@~ lsls    r1, r1, #1
	;@~ ldrsb   r1, [r1, r0]
	;@~ lsls    r1, r1, #1
	;@~ add     lr, lr, r1
	;@~ mov     r1, r12
	;@~ bx      lr

;@~ .global __gnu_thumb1_case_uhi
;@~ .thumb_func
;@~ __gnu_thumb1_case_uhi:
	;@~ push    {r0, r1}
	;@~ mov     r1, lr
	;@~ lsrs    r1, r1, #1
	;@~ lsls    r0, r0, #1
	;@~ lsls    r1, r1, #1
	;@~ ldrh    r1, [r1, r0]
	;@~ lsls    r1, r1, #1
	;@~ add     lr, lr, r1
	;@~ pop     {r0, r1}
	;@~ bx      lr

;@~ .global __gnu_thumb1_case_shi
;@~ .thumb_func
;@~ __gnu_thumb1_case_shi:
	;@~ push    {r0, r1}
	;@~ mov     r1, lr
	;@~ lsrs    r1, r1, #1
	;@~ lsls    r0, r0, #1
	;@~ lsls    r1, r1, #1
	;@~ ldrsh   r1, [r1, r0]
	;@~ lsls    r1, r1, #1
	;@~ add     lr, lr, r1
	;@~ pop     {r0, r1}
	;@~ bx      lr

;@~ .global __gnu_thumb1_case_si
;@~ .thumb_func
;@~ __gnu_thumb1_case_si:
	;@~ push	{r0, r1}
	;@~ mov	r1, lr
	;@~ adds.n	r1, r1, #2
	;@~ lsrs	r1, r1, #2
	;@~ lsls	r0, r0, #2
	;@~ lsls	r1, r1, #2
	;@~ ldr	r0, [r1, r0]
	;@~ adds	r0, r0, r1
	;@~ mov	lr, r0
	;@~ pop	{r0, r1}
	;@~ mov	pc, lr


