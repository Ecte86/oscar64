// crt.c
#include <crt.h>

#define	tmpy		__tmpy
#define	tmp			__tmp
	
#define	ip			__ip
#define	accu		__accu
#define	addr		__addr
#define	sp			__sp
#define	fp			__fp
	
#define	sregs		__sregs
#define	regs		__regs


void StackStart, StackEnd, BSSStart, BSSEnd, CodeStart, CodeEnd;

#pragma section(code, 0x0000, CodeStart, CodeEnd)
#pragma section(stack, 0x0000, StackStart, StackEnd)
#pragma section(bss, 0x0000, BSSStart, BSSEnd)

char spentry = 0;

int main(void);


__asm startup
{
st0:
#ifdef OSCAR_TARGET_CRT16
		byt 0x09
		byt 0x80
		byt 0xbc
		byt 0xfe		
		byt 0xc3
		byt 0xc2
		byt 0xcd
		byt 0x38
		byt 0x30

// Copy cartridge rom to ram

		ldx	#0
lc0:	lda $8000, x
		sta st0, x
		inx	
		bne	lc0

// Set up address for decompress

		lda #$00
		sta ip
		lda #$81
		sta ip + 1

		lda #<CodeStart
		sta sp
		lda #>CodeStart		
		sta sp + 1		

		ldy	#0
		lda (ip),y
lx0:
		sta accu
		bmi wx1

		clc
		lda ip + 0
		adc #1
		sta addr + 0
		lda ip + 1
		adc #0
		sta addr + 1

		clc
		lda addr + 0
		adc accu
		sta ip + 0
		lda addr + 1
		adc #0
		sta ip + 1
lx2:
		ldy #0
lx1:	lda (addr), y
		sta (sp), y
		iny
		cpy accu
		bne lx1

		clc
		lda sp
		adc accu
		sta sp
		lda sp + 1
		adc #0
		sta sp + 1

		ldy #0
		lda (ip), y
		bne lx0
		jmp w0
wx1:
		lda sp + 0
		sec
		ldy #1
		sbc (ip), y
		sta addr
		lda sp + 1
		sbc #0
		sta addr + 1

		clc
		lda ip + 0
		adc #2
		sta ip + 0
		lda ip + 1
		adc #0
		sta ip + 1

		lda accu
		and #$7f
		sta accu
		jmp lx2

w0:
		lda #$3f
		sta $00
		lda #$36
		sta $01

#elif defined(OSCAR_TARGET_BIN)

#else		
		byt	0x0b
		byt 0x08
		byt	0x0a
		byt	0x00
		byt	0x9e
		byt	[OSCAR_BASIC_START + 12] / 1000 % 10 + 0x30
		byt	[OSCAR_BASIC_START + 12] / 100 % 10 + 0x30
		byt	[OSCAR_BASIC_START + 12] / 10 % 10 + 0x30
		byt	[OSCAR_BASIC_START + 12] % 10 + 0x30
		byt	0x00
		byt	0x00
		byt	0x00

		tsx
		stx spentry

#endif
// Clear BSS Segment

		lda #<BSSStart
		sta ip
		lda #>BSSStart
		sta ip + 1

		sec
		lda #>BSSEnd
		sbc #>BSSStart
		beq w1
		tax
		lda #0
		ldy #0
l1:		sta (ip), y
		iny
		bne l1
		inc ip + 1
		dex
		bne l1
w1:
		sec
		lda #<BSSEnd
		sbc #<BSSStart
		beq w2
		tay
		lda #0
l2:		dey
		sta (ip), y
		bne l2
w2:

		lda	#<StackEnd - 2
		sta	sp
		lda	#>StackEnd - 2
		sta	sp + 1

#ifdef OSCAR_NATIVE_ALL

// All native code
		jsr	main
pexec:
tyexec:
yexec:
zexec:
exec:

#else

// Init byte code

		lda	#<bcode
		sta	ip
		lda	#>bcode
		sta	ip + 1
		
pexec:
		ldy	#0
		beq exec
tyexec:
		ldy	tmpy
yexec:
		iny
exec:
		lda	(ip), y
		sta	execjmp + 1
		iny		
execjmp:
		jmp 	(0x0900)
zexec:
		tya	
		ldy #0
		clc		
		adc	ip	
		sta	ip	
		bcc	exec	
		inc	ip + 1		
		bne	exec
bcode:		
		byt	BC_CALL_ABS * 2
		byt	<main
		byt	>main
		byt	BC_EXIT * 2
#endif

spexit:
		lda	#$4c
		sta	$54
		lda #0
		sta $13
		lda #$19
		sta $16
		rts
}

#pragma startup(startup)

__asm bcexec
{
#ifdef OSCAR_NATIVE_ALL
		jmp (accu)
#else	
		lda ip
		pha
		lda ip + 1	
		pha
		lda accu
		sta ip
		lda accu + 1
		sta ip + 1

		ldy	#0
		lda	#<bdone
		sta	(sp), y
		iny
		lda	#>bdone
		sta	(sp), y
		jmp	startup.pexec
bdone:	nop
		pla
		pla
		pla
		sta ip + 1
		pla
		sta ip
		rts		
#endif
}

__asm jmpaddr
{
		jmp	(addr)
}

#pragma runtime(bcexec, bcexec)
#pragma runtime(jmpaddr, jmpaddr)

__asm negaccu
{
		sec
		lda	#0
		sbc	accu
		sta	accu
		lda	#0
		sbc	accu + 1
		sta	accu + 1
		rts
}

__asm negtmp
{
		sec
		lda	#0
		sbc	tmp
		sta	tmp
		lda	#0
		sbc	tmp + 1
		sta	tmp + 1
		rts
}

__asm negaccu32
{
		sec
		lda	#0
		sbc	accu
		sta	accu
		lda	#0
		sbc	accu + 1
		sta	accu + 1
		lda	#0
		sbc	accu + 2
		sta	accu + 2
		lda	#0
		sbc	accu + 3
		sta	accu + 3
		rts
}

__asm negtmp32
{
		sec
		lda	#0
		sbc	tmp
		sta	tmp
		lda	#0
		sbc	tmp + 1
		sta	tmp + 1
		lda	#0
		sbc	tmp + 2
		sta	tmp + 2
		lda	#0
		sbc	tmp + 3
		sta	tmp + 3
		rts
}


// divide accu by tmp result in accu, remainder in tmp + 2

__asm divmod
{
		lda accu + 1
		bne WB
		lda	tmp + 1
		bne BW

// byte / byte
BB:
		// accu is zero at this point
		sta	tmp + 3
		ldx	#4
		asl	accu
LBB1:	rol
		cmp	tmp
		bcc	WBB1
		sbc	tmp
WBB1:	rol	accu
		rol
		cmp	tmp
		bcc	WBB2
		sbc	tmp
WBB2:	rol	accu
		dex
		bne	LBB1
		sta	tmp + 2
		rts	
	
// byte / word -> 0
BW: 
		lda	accu
		sta tmp + 2
		lda accu + 1
		sta tmp + 3
		lda #0
		sta accu
		sta accu + 1
		rts

WB:				
		lda tmp + 1
		bne WW
		lda tmp
		bmi WW

// word / byte

		lda #0
		sta	tmp + 3
		ldx	#16
		asl	accu
		rol	accu + 1		
LWB1:	rol
		cmp	tmp
		bcc	WWB1
		sbc	tmp
WWB1:	rol	accu
		rol	accu + 1
		dex
		bne	LWB1
		sta tmp + 2
		rts	

// word / word
WW:
		lda	#0
		sta	tmp + 2
		sta	tmp + 3

		sty	tmpy
		ldy	#16
		clc
L1:		rol	accu
		rol	accu + 1
		rol	tmp + 2
		rol	tmp + 3
		sec
		lda	tmp + 2
		sbc	tmp
		tax
		lda	tmp + 3
		sbc	tmp + 1
		bcc	W1
		stx	tmp + 2
		sta	tmp + 3
W1:		dey
		bne	L1
		rol	accu
		rol	accu + 1
		ldy	tmpy
		rts	
}

// divide accu by tmp result in accu, remainder in tmp + 4

__asm divmod32
{
		sty	tmpy
		ldy	#32

		lda	#0
		sta	tmp + 4
		sta	tmp + 5
		sta	tmp + 6
		sta	tmp + 7

		lda tmp + 2
		ora tmp + 3
		bne W32

// divide 32 by 16 bit

		clc
LS1:	rol	accu
		rol	accu + 1
		rol	accu + 2
		rol	accu + 3
		rol	tmp + 4
		rol	tmp + 5
		bcc LS1a
		
		lda	tmp + 4
		sbc	tmp
		tax
		lda	tmp + 5
		sbc	tmp + 1
		sec
		bcs LS1b
LS1a:
		sec
		lda	tmp + 4
		sbc	tmp
		tax
		lda	tmp + 5
		sbc	tmp + 1
		bcc	WS1
LS1b:
		stx	tmp + 4
		sta	tmp + 5
WS1:	dey
		bne	LS1
		rol	accu
		rol	accu + 1
		rol	accu + 2
		rol	accu + 3
		ldy	tmpy
		rts	

W32:
		clc
L1:		rol	accu
		rol	accu + 1
		rol	accu + 2
		rol	accu + 3
		rol	tmp + 4
		rol	tmp + 5
		rol	tmp + 6
		rol	tmp + 7
		
		lda	tmp + 4
		cmp	tmp + 0
		lda	tmp + 5
		sbc	tmp + 1
		lda	tmp + 6
		sbc	tmp + 2
		lda	tmp + 7
		sbc	tmp + 3
		bcc	W1
		lda	tmp + 4
		sbc	tmp + 0
		sta tmp + 4
		lda	tmp + 5
		sbc	tmp + 1
		sta tmp + 5
		lda	tmp + 6
		sbc	tmp + 2
		sta tmp + 6
		lda	tmp + 7
		sbc	tmp + 3
		sta tmp + 7
W1:		dey
		bne	L1
		rol	accu
		rol	accu + 1
		rol	accu + 2
		rol	accu + 3
		ldy	tmpy
		rts	
}

// Multiply accu by tmp result in tmp + 2

__asm mul16
{
		ldy	#0
		sty	tmp + 3

		lda	tmp
		ldx	tmp + 1
		beq W1

		sec
		ror
		bcc	L2
L1:
		tax
		clc
		tya
		adc	accu
		tay
		lda	tmp + 3
		adc	accu + 1
		sta	tmp + 3
		txa
L2:	
		asl	accu + 0
		rol	accu + 1
		lsr
		bcc	L2
		bne	L1

		lda tmp + 1
W1:		
		lsr
		bcc	L4
L3:
		tax
		clc
		tya
		adc	accu
		tay
		lda	tmp + 3
		adc	accu + 1
		sta	tmp + 3
		txa
L4:	
		asl	accu + 0
		rol	accu + 1
		lsr
		bcs	L3
		bne	L4

		sty tmp + 2
}

__asm mul32
{
		lda	#0
		sta	tmp + 4
		sta	tmp + 5
		sta	tmp + 6
		sta	tmp + 7

		ldx	#32
L1:		lsr	tmp + 3
		ror	tmp + 2
		ror	tmp + 1
		ror	tmp + 0
		bcc	W1
		clc
		lda	tmp + 4
		adc	accu
		sta	tmp + 4
		lda	tmp + 5
		adc	accu + 1
		sta	tmp + 5
		lda	tmp + 6
		adc	accu + 2
		sta	tmp + 6
		lda	tmp + 7
		adc	accu + 3
		sta	tmp + 7
W1:		asl	accu
		rol	accu + 1
		rol	accu + 2
		rol	accu + 3
		dex
		bne	L1
		rts
}

__asm mul16by8
{
		ldy	#0
		sty	tmp + 3

		lsr
		bcc	L2
L1:
		tax
		clc
		tya
		adc	accu
		tay
		lda	tmp + 3
		adc	accu + 1
		sta	tmp + 3
		txa
L2:	
		asl	accu + 0
		rol	accu + 1
		lsr
		bcs	L1
		bne	L2

		sty tmp + 2
		rts
}

__asm divs16
{
		bit	accu + 1
		bpl	L1
		jsr	negaccu
		bit	tmp + 1
		bpl	L2
		jsr	negtmp
L3:		jmp	divmod
L1:		bit	tmp + 1
		bpl	L3
		jsr	negtmp
L2:		jsr	divmod
		jmp	negaccu
}

__asm mods16
{
		bit	accu + 1
		bpl	L1
		jsr	negaccu
		bit	tmp + 1
		bpl	L2
		jsr	negtmp
L3:		jmp	divmod
L1:		bit	tmp + 1
		bpl	L3
		jsr	negtmp
L2:		jsr	divmod
		sec
		lda	#0
		sbc	tmp + 2
		sta	tmp + 2
		lda	#0
		sbc	tmp + 3
		sta	tmp + 3
		rts
}

__asm divs32
{
		bit	accu + 3
		bpl	L1
		jsr	negaccu32
		bit	tmp + 3
		bpl	L2
		jsr	negtmp32
L3:		jmp	divmod32
L1:		bit	tmp + 3
		bpl	L3
		jsr	negtmp32
L2:		jsr	divmod32
		jmp	negaccu32
}

__asm mods32
{
		bit	accu + 3
		bpl	L1
		jsr	negaccu32
		bit	tmp + 3
		bpl	L2
		jsr	negtmp32
L3:		jmp	divmod32
L1:		bit	tmp + 3
		bpl	L3
		jsr	negtmp32
L2:		jsr	divmod32
		sec
		lda	#0
		sbc	tmp + 4
		sta	tmp + 4
		lda	#0
		sbc	tmp + 5
		sta	tmp + 5
		lda	#0
		sbc	tmp + 6
		sta	tmp + 6
		lda	#0
		sbc	tmp + 7
		sta	tmp + 7
		rts
}

#pragma runtime(mul16, mul16);
#pragma runtime(mul16by8, mul16by8);
#pragma runtime(divu16, divmod);
#pragma runtime(modu16, divmod);
#pragma runtime(divs16, divs16);
#pragma runtime(mods16, mods16);
		
#pragma runtime(mul32, mul32);
#pragma runtime(divu32, divmod32);
#pragma runtime(modu32, divmod32);
#pragma runtime(divs32, divs32);
#pragma runtime(mods32, mods32);

	

__asm inp_nop
{
		jmp	startup.zexec
}

#pragma	bytecode(BC_NOP, inp_nop)

#pragma	bytecode(BC_EXIT, startup.spexit)

__asm inp_jsr
{
		lda	(ip), y
		sta	P1 + 1
		iny
		lda	(ip), y
		sta	P1 + 2
		tya
		sec
		adc	ip
		sta	ip
		bcc	P1
		inc	ip + 1
P1:
		jsr	$0000
		jmp	startup.pexec
}		
#pragma	bytecode(BC_JSR, inp_jsr)

__asm inp_native
{
		tya
		clc
		adc	ip
		sta	P1 + 1
		lda	ip + 1
		adc	#0
		sta P1 + 2
P1:
		jsr	$0000

		ldy	#0
		lda	(sp), y
		sta	ip
		iny
		lda	(sp), y
		sta	ip + 1
		dey
		jmp	startup.exec
}		

#pragma	bytecode(BC_NATIVE, inp_native)

__asm inp_const_8
{
		lda	(ip), y
		tax
		iny
		lda	(ip), y
		sta	$00, x
		jmp	startup.yexec
}

#pragma	bytecode(BC_CONST_8, inp_const_8)
	
__asm inp_const_p8
{
		lda	(ip), y
		tax
		iny
		lda	(ip), y
		sta	$00, x
		lda	#0
		sta	$01, x
		jmp	startup.yexec
}

#pragma	bytecode(BC_CONST_P8, inp_const_p8)

__asm inp_const_n8			
{
		lda	(ip), y
		tax
		iny
		lda	(ip), y
		sta	$00, x
		lda	#$ff
		sta	$01, x
		jmp	startup.yexec
}

#pragma	bytecode(BC_CONST_N8, inp_const_n8)

__asm inp_const_16
{
		lda	(ip), y
		tax
		iny
		lda	(ip), y
		sta	$00, x
		iny
		lda	(ip), y
		sta	$01, x
		jmp	startup.yexec
}

#pragma	bytecode(BC_CONST_16, inp_const_16)

__asm inp_const_32
{
		lda	(ip), y
		tax
		iny
		lda	(ip), y
		sta	$00, x
		iny
		lda	(ip), y
		sta	$01, x
		iny
		lda	(ip), y
		sta	$02, x
		iny
		lda	(ip), y
		sta	$03, x
		jmp	startup.yexec
}

#pragma	bytecode(BC_CONST_32, inp_const_32)

__asm inp_load_reg_8
{
		lda	(ip), y
		tax
		lda	$00, x
		sta	accu
		lda	#0
		sta	accu + 1
		jmp	startup.yexec
}
		
#pragma	bytecode(BC_LOAD_REG_8, inp_load_reg_8)

__asm inp_store_reg_8
{
		lda	(ip), y
		tax
		lda	accu
		sta	$00, x
		jmp	startup.yexec
}

#pragma	bytecode(BC_STORE_REG_8, inp_store_reg_8)
		
__asm inp_load_reg_16
{
		lda	(ip), y
		tax
		lda	$00, x
		sta	accu
		lda	$01, x
		sta	accu + 1
		jmp	startup.yexec
}
		
#pragma	bytecode(BC_LOAD_REG_16, inp_load_reg_16)

__asm inp_store_reg_16
{
		lda	(ip), y
		tax
		lda	accu
		sta	$00, x
		lda	accu + 1
		sta	$01, x
		jmp	startup.yexec
}

#pragma	bytecode(BC_STORE_REG_16, inp_store_reg_16)

__asm inp_load_reg_32
{
		lda	(ip), y
		tax
		lda	$00, x
		sta	accu
		lda	$01, x
		sta	accu + 1
		lda	$02, x
		sta	accu + 2
		lda	$03, x
		sta	accu + 3
		jmp	startup.yexec
}
		
#pragma	bytecode(BC_LOAD_REG_32, inp_load_reg_32)

__asm inp_store_reg_32
{
		lda	(ip), y
		tax
		lda	accu
		sta	$00, x
		lda	accu + 1
		sta	$01, x
		lda	accu + 2
		sta	$02, x
		lda	accu + 3
		sta	$03, x
		jmp	startup.yexec
}

#pragma	bytecode(BC_STORE_REG_32, inp_store_reg_32)

__asm inp_conv_s8_s16
{
		lda	(ip), y
		tax
		lda #$80
		and	$00, x
		bpl	W1
		lda	#$ff
W1:		sta $01, x
		jmp	startup.yexec
}

#pragma	bytecode(BC_CONV_I8_I16, inp_conv_s8_s16)

__asm inp_addr_reg
{
		lda	(ip), y
		tax
		lda	$00, x
		sta	addr
		lda	$01, x
		sta	addr + 1
		jmp	startup.yexec
}

#pragma	bytecode(BC_ADDR_REG, inp_addr_reg)

__asm inp_load_abs_8
{
		lda	(ip), y
		sta	addr
		iny
		lda	(ip), y
		sta	addr + 1
		iny
		lda	(ip), y
		tax
		sty	tmpy
		ldy	#0
L0:
		lda	(addr), y
		sta	$00, x
		jmp	startup.tyexec
inp_load_addr_8:
		lda	(ip), y
		tax
		iny		
		lda	(ip), y
		sty	tmpy
		tay
		jmp	L0
}
		
#pragma	bytecode(BC_LOAD_ABS_8, inp_load_abs_8)
#pragma	bytecode(BC_LOAD_ADDR_8, inp_load_abs_8.inp_load_addr_8)

__asm inp_load_abs_u8
{
		lda	(ip), y
		sta	addr
		iny
		lda	(ip), y
		sta	addr + 1
		iny
		lda	(ip), y
		tax
		sty	tmpy
		ldy	#0
L0:
		lda	(addr), y
		sta	$00, x
		lda #0
		sta $01, x
		jmp	startup.tyexec
inp_load_addr_u8:
		lda	(ip), y
		tax
		iny		
		lda	(ip), y
		sty	tmpy
		tay
		jmp	L0
}
		
#pragma	bytecode(BC_LOAD_ABS_U8, inp_load_abs_u8)
#pragma	bytecode(BC_LOAD_ADDR_U8, inp_load_abs_u8.inp_load_addr_u8)
	
__asm inp_load_abs_16
{
		lda	(ip), y
		sta	addr
		iny
		lda	(ip), y
		sta	addr + 1
		iny
		lda	(ip), y
		tax
		sty	tmpy
		ldy	#0
L0:
		lda	(addr), y
		sta	$00, x
		iny
		lda	(addr), y
		sta	$01, x
		jmp	startup.tyexec

inp_load_addr_16:
		lda	(ip), y
		tax
		iny		
		lda	(ip), y
		sty	tmpy
		tay
		jmp L0
}

#pragma	bytecode(BC_LOAD_ABS_16, inp_load_abs_16)
#pragma	bytecode(BC_LOAD_ADDR_16, inp_load_abs_16.inp_load_addr_16)

__asm inp_load_abs_addr
{
		lda	(ip), y
		sta	addr
		iny
		lda	(ip), y
		sta	addr + 1
		sty	tmpy

		ldy #0
		lda	(addr), y
		tax
		iny
		lda	(addr), y
		sta addr + 1
		stx addr
		jmp	startup.tyexec
}

#pragma	bytecode(BC_LOAD_ABS_ADDR, inp_load_abs_addr)


__asm inp_load_abs_32
{
		lda	(ip), y
		sta	addr
		iny
		lda	(ip), y
		sta	addr + 1
		iny
		lda	(ip), y
		tax
		sty	tmpy
		ldy	#0
L0:

		lda	(addr), y
		sta	$00, x
		iny
		lda	(addr), y
		sta	$01, x
		iny
		lda	(addr), y
		sta	$02, x
		iny
		lda	(addr), y
		sta	$03, x
		jmp	startup.tyexec

inp_load_addr_32:
		lda	(ip), y
		tax
		iny		
		lda	(ip), y
		sty	tmpy
		tay
		jmp L0
}
		
#pragma	bytecode(BC_LOAD_ABS_32, inp_load_abs_32)
#pragma	bytecode(BC_LOAD_ADDR_32, inp_load_abs_32.inp_load_addr_32)

__asm inp_store_abs_8
{
		lda	(ip), y
		sta	addr
		iny
		lda	(ip), y
		sta	addr + 1
		iny
		lda	(ip), y
		tax
		sty	tmpy
		ldy	#0
L0:

		lda	$00, x
		sta	(addr), y
		jmp	startup.tyexec

inp_store_addr_8:
		lda	(ip), y
		tax
		iny		
		lda	(ip), y
		sty	tmpy
		tay
		jmp L0
}

#pragma	bytecode(BC_STORE_ABS_8, inp_store_abs_8)
#pragma	bytecode(BC_STORE_ADDR_8, inp_store_abs_8.inp_store_addr_8)

__asm inp_store_abs_16
{
		lda	(ip), y
		sta	addr
		iny
		lda	(ip), y
		sta	addr + 1
		iny
		lda	(ip), y
		tax
		sty	tmpy
		ldy	#0
L0:
		lda	$00, x
		sta	(addr), y
		iny
		lda	$01, x
		sta	(addr), y
		jmp	startup.tyexec

inp_store_addr_16:
		lda	(ip), y
		tax
		iny		
		lda	(ip), y
		sty	tmpy
		tay
		jmp	L0
}

#pragma	bytecode(BC_STORE_ABS_16, inp_store_abs_16)
#pragma	bytecode(BC_STORE_ADDR_16, inp_store_abs_16.inp_store_addr_16)
		
__asm inp_store_abs_32
{
		lda	(ip), y
		sta	addr
		iny
		lda	(ip), y
		sta	addr + 1
		iny
		lda	(ip), y
		tax
		sty	tmpy
		ldy	#0
L0:
		lda	$00, x
		sta	(addr), y
		iny
		lda	$01, x
		sta	(addr), y
		iny
		lda	$02, x
		sta	(addr), y
		iny
		lda	$03, x
		sta	(addr), y
		jmp	startup.tyexec

inp_store_addr_32:
		lda	(ip), y
		tax
		iny		
		lda	(ip), y
		sty	tmpy
		tay
		jmp	L0
}

#pragma	bytecode(BC_STORE_ABS_32, inp_store_abs_32)
#pragma	bytecode(BC_STORE_ADDR_32, inp_store_abs_32.inp_store_addr_32)

__asm inp_lea_abs
{
		lda	(ip), y
		tax
		iny
		lda	(ip), y
		sta	$00, x
		iny
		lda	(ip), y
		sta	$01, x
		jmp	startup.yexec
}

#pragma	bytecode(BC_LEA_ABS, inp_lea_abs)
				
__asm inp_lea_abs_index
{
		lda	(ip), y
		tax
		iny
		clc
		lda $00, x
		adc	(ip), y
		sta	addr
		iny
		lda $01, x
		adc	(ip), y
		sta	addr + 1
		jmp	startup.yexec
}

#pragma	bytecode(BC_LEA_ABS_INDEX, inp_lea_abs_index)
				
__asm inp_lea_abs_index_u8
{
		lda	(ip), y
		tax
		iny
		clc
		lda $00, x
		adc	(ip), y
		sta	addr
		iny
		lda #$00
		adc	(ip), y
		sta	addr + 1
		jmp	startup.yexec
}

#pragma	bytecode(BC_LEA_ABS_INDEX_U8, inp_lea_abs_index_u8)

__asm inp_lea_accu_index
{
		lda	(ip), y
		tax
		clc
		lda $00, x
		adc	accu
		sta	addr
		lda $01, x
		adc accu + 1
		sta	addr + 1
		jmp	startup.yexec
}

#pragma	bytecode(BC_LEA_ACCU_INDEX, inp_lea_accu_index)
				
__asm inp_load_local_16
{
		lda	(ip), y
		tax
		iny
		lda	(ip), y
		sty	tmpy
		tay
		lda	(fp), y
		sta	$00, x
		iny
		lda	(fp), y
		sta	$01, x
		jmp	startup.tyexec
}

#pragma	bytecode(BC_LOAD_LOCAL_16, inp_load_local_16)
		
__asm inp_load_local_32
{
		lda	(ip), y
		tax
		iny
		lda	(ip), y
		sty	tmpy
		tay
		lda	(fp), y
		sta	$00, x
		iny
		lda	(fp), y
		sta	$01, x
		iny
		lda	(fp), y
		sta	$02, x
		iny
		lda	(fp), y
		sta	$03, x
		jmp	startup.tyexec
}

#pragma	bytecode(BC_LOAD_LOCAL_32, inp_load_local_32)

__asm inp_load_local_8
{
		lda	(ip), y
		tax
		iny
		lda	(ip), y
		sty	tmpy
		tay
		lda	(fp), y
		sta	$00, x
		jmp	startup.tyexec
}
		
#pragma	bytecode(BC_LOAD_LOCAL_8, inp_load_local_8)

__asm inp_load_local_u8
{
		lda	(ip), y
		tax
		iny
		lda	(ip), y
		sty	tmpy
		tay
		lda	(fp), y
		sta	$00, x
		lda #0
		sta $01, x
		jmp	startup.tyexec
}
		
#pragma	bytecode(BC_LOAD_LOCAL_U8, inp_load_local_u8)
		
__asm inp_store_local_8
{
		lda	(ip), y
		tax
		iny
		lda	(ip), y
		sty	tmpy
		tay
		lda	$00, x
		sta	(fp), y
		jmp	startup.tyexec
}

#pragma	bytecode(BC_STORE_LOCAL_8, inp_store_local_8)

__asm inp_store_local_16
{
		lda	(ip), y
		tax
		iny
		lda	(ip), y
		sty	tmpy
		tay
		lda	$00, x
		sta	(fp), y
		iny
		lda	$01, x
		sta	(fp), y
		jmp	startup.tyexec
}

#pragma	bytecode(BC_STORE_LOCAL_16, inp_store_local_16)
		
__asm inp_store_local_32
{
		lda	(ip), y
		tax
		iny
		lda	(ip), y
		sty	tmpy
		tay
		lda	$00, x
		sta	(fp), y
		iny
		lda	$01, x
		sta	(fp), y
		iny
		lda	$02, x
		sta	(fp), y
		iny
		lda	$03, x
		sta	(fp), y
		jmp	startup.tyexec
}

#pragma	bytecode(BC_STORE_LOCAL_32, inp_store_local_32)

__asm inp_lea_local
{
		lda	(ip), y
		tax
		iny
		clc
		lda	(ip), y
		adc	fp
		sta	$00, x
		iny
		lda	(ip), y
		adc	fp + 1
		sta	$01, x
		jmp	startup.yexec
}
		
#pragma	bytecode(BC_LEA_LOCAL, inp_lea_local)

__asm inp_store_frame_8
{
		lda	(ip), y
		tax
		iny
		lda	(ip), y
		sty	tmpy
		tay
		lda	$00, x
		sta	(sp), y
		jmp	startup.tyexec
}

#pragma	bytecode(BC_STORE_FRAME_8, inp_store_frame_8)

__asm inp_store_frame_16
{
		lda	(ip), y
		tax
		iny
		lda	(ip), y
		sty	tmpy
		tay
		lda	$00, x
		sta	(sp), y
		iny
		lda	$01, x
		sta	(sp), y
		jmp	startup.tyexec
}

#pragma	bytecode(BC_STORE_FRAME_16, inp_store_frame_16)

__asm inp_store_frame_32
{
		lda	(ip), y
		tax
		iny
		lda	(ip), y
		sty	tmpy
		tay
		lda	$00, x
		sta	(sp), y
		iny
		lda	$01, x
		sta	(sp), y
		iny
		lda	$02, x
		sta	(sp), y
		iny
		lda	$03, x
		sta	(sp), y
		jmp	startup.tyexec
}

#pragma	bytecode(BC_STORE_FRAME_32, inp_store_frame_32)

__asm inp_lea_frame
{
		lda	(ip), y
		tax
		iny
		clc
		lda	(ip), y
		adc	sp
		sta	$00, x
		iny
		lda	(ip), y
		adc	sp + 1
		sta	$01, x
		jmp	startup.yexec
}
		
#pragma	bytecode(BC_LEA_FRAME, inp_lea_frame)

__asm inp_op_negate_16
{
		sec
		lda	#0
		sbc	accu
		sta	accu
		lda	#0
		sbc	accu + 1
		sta	accu + 1
		jmp	startup.exec
}

#pragma	bytecode(BC_OP_NEGATE_16, inp_op_negate_16)

__asm inp_op_invert_16
{
		lda	accu
		eor	#$ff
		sta	accu
		lda	accu + 1
		eor	#$ff
		sta	accu + 1
		jmp	startup.exec
}

#pragma	bytecode(BC_OP_INVERT_16, inp_op_invert_16)
		
__asm inp_binop_addr_16
{
		lda	(ip), y
		tax
		clc
		lda	accu
		adc	$00, x
		sta	accu
		lda	accu + 1
		adc	$01, x
		sta	accu + 1
		jmp	startup.yexec
}

#pragma	bytecode(BC_BINOP_ADDR_16, inp_binop_addr_16)

__asm inp_binop_subr_16
{
		lda	(ip), y
		tax
		sec
		lda	accu
		sbc	$00, x
		sta	accu
		lda	accu + 1
		sbc	$01, x
		sta	accu + 1
		jmp	startup.yexec
}

#pragma	bytecode(BC_BINOP_SUBR_16, inp_binop_subr_16)

__asm inp_binop_andr_16
{
		lda	(ip), y
		tax
		lda	accu
		and	$00, x
		sta	accu
		lda	accu + 1
		and	$01, x
		sta	accu + 1
		jmp	startup.yexec
}

#pragma	bytecode(BC_BINOP_ANDR_16, inp_binop_andr_16)

__asm inp_binop_orr_16
{
		lda	(ip), y
		tax
		lda	accu
		ora	$00, x
		sta	accu
		lda	accu + 1
		ora	$01, x
		sta	accu + 1
		jmp	startup.yexec
}

#pragma	bytecode(BC_BINOP_ORR_16, inp_binop_orr_16)

__asm inp_binop_xorr_16
{
		lda	(ip), y
		tax
		lda	accu
		eor	$00, x
		sta	accu
		lda	accu + 1
		eor	$01, x
		sta	accu + 1
		jmp	startup.yexec
}

#pragma	bytecode(BC_BINOP_XORR_16, inp_binop_xorr_16)

__asm inp_binop_mulr_16
{
		lda	(ip), y
		tax
		lda	#0
		sta	tmp + 2
		sta	tmp + 3

		lda	$00, x
		sta	tmp + 0
		lda	$01, x
		sta	tmp + 1
		ldx	#16
L1:		lsr	tmp + 1
		ror	tmp + 0
		bcc	W1
		clc
		lda	tmp + 2
		adc	accu
		sta	tmp + 2
		lda	tmp + 3
		adc	accu + 1
		sta	tmp + 3
W1:		asl	accu
		rol	accu + 1
		dex
		bne	L1
		lda	tmp + 2
		sta	accu
		lda	tmp + 3
		sta	accu + 1
		jmp	startup.yexec
}

#pragma	bytecode(BC_BINOP_MULR_16, inp_binop_mulr_16)

__asm inp_binop_muli8_16
{
		lda	(ip), y
		iny
		tax
		
		lda	#0
		sta	tmp + 2
		sta	tmp + 3
		
		lda	$00, x
		sta	tmp + 0
		lda	$01, x
		sta	tmp + 1
		
		lda	(ip), y

		lsr
		sta	tmp + 4
		bcc	L2
L1:
		clc
		lda	tmp + 2
		adc	tmp + 0
		sta	tmp + 2
		lda	tmp + 3
		adc	tmp + 1
		sta	tmp + 3
L2:	
		asl	tmp + 0
		rol	tmp + 1
		lsr	tmp + 4
		bcs	L1
		bne	L2

		lda	tmp + 2
		sta	$00, x
		lda	tmp + 3
		sta	$01, x
		
		jmp	startup.yexec
}
		
#pragma	bytecode(BC_BINOP_MULI8_16, inp_binop_muli8_16)

__asm inp_binop_divr_u16
{
		lda	(ip), y
		tax
		lda	$00, x
		sta	tmp + 0
		lda	$01, x
		sta	tmp + 1
		jsr	divmod
		jmp	startup.yexec
}

#pragma	bytecode(BC_BINOP_DIVR_U16, inp_binop_divr_u16)

__asm inp_binop_modr_u16
{
		lda	(ip), y
		tax
		lda	$00, x
		sta	tmp + 0
		lda	$01, x
		sta	tmp + 1
		jsr	divmod
		lda	tmp + 2
		sta	accu
		lda	tmp + 3
		sta	accu + 1
		jmp	startup.yexec
}

#pragma	bytecode(BC_BINOP_MODR_U16, inp_binop_modr_u16)

__asm inp_binop_divr_s16
{
		lda	(ip), y
		tax
		lda	$00, x
		sta	tmp + 0
		lda	$01, x
		sta	tmp + 1
		bit	accu + 1
		bpl	L1
		jsr	negaccu
		bit	tmp + 1
		bpl	L2
		jsr	negtmp
L3:		jsr	divmod
		jmp	startup.yexec		
L1:		bit	tmp + 1
		bpl	L3
		jsr	negtmp
L2:		jsr	divmod
		jsr	negaccu
		jmp	startup.yexec
}

#pragma	bytecode(BC_BINOP_DIVR_I16, inp_binop_divr_s16)

__asm inp_binop_modr_s16
{
		lda	(ip), y
		tax
		lda	$00, x
		sta	tmp + 0
		lda	$01, x
		sta	tmp + 1
		bit	accu + 1
		bpl	L1
		jsr	negaccu
		bit	tmp + 1
		bpl	L2
		jsr	negtmp
L3:		jsr	divmod
		lda	tmp + 2
		sta	accu
		lda	tmp + 3
		sta	accu + 1
		jmp	startup.yexec		
L1:		bit	tmp + 1
		bpl	L3
		jsr	negtmp
L2:		jsr	divmod
		lda	tmp + 2
		sta	accu
		lda	tmp + 3
		sta	accu + 1
		jsr	negaccu
		jmp	startup.yexec
}

#pragma	bytecode(BC_BINOP_MODR_I16, inp_binop_modr_s16)

__asm inp_binop_addi_16
{
		lda	(ip), y
		iny
		tax
		clc
		lda	$00, x
		adc	(ip), y
		iny
		sta	$00, x
		lda	$01, x
		adc	(ip), y
		sta	$01, x
		jmp	startup.yexec
}

#pragma	bytecode(BC_BINOP_ADDI_16, inp_binop_addi_16)

__asm inp_binop_addi_8
{
		lda	(ip), y
		iny
		tax
		clc
		lda	$00, x
		adc	(ip), y
		sta	$00, x
		jmp	startup.yexec
}

#pragma	bytecode(BC_BINOP_ADDI_8, inp_binop_addi_8)

__asm inp_binop_andi_8
{
		lda	(ip), y
		iny
		tax
		lda	$00, x
		and	(ip), y
		sta	$00, x
		jmp	startup.yexec
}

#pragma	bytecode(BC_BINOP_ANDI_8, inp_binop_andi_8)

__asm inp_binop_ori_8
{
		lda	(ip), y
		iny
		tax
		lda	$00, x
		ora (ip), y
		sta	$00, x
		jmp	startup.yexec
}

#pragma	bytecode(BC_BINOP_ORI_8, inp_binop_ori_8)

__asm inp_binop_subi_16
{
		lda	(ip), y
		iny
		tax
		sec
		lda	(ip), y
		iny
		sbc	$00, x
		sta	$00, x
		lda	(ip), y
		sbc	$01, x
		sta	$01, x
		jmp	startup.yexec
}

#pragma	bytecode(BC_BINOP_SUBI_16, inp_binop_subi_16)

__asm inp_binop_andi_16
{
		lda	(ip), y
		iny
		tax
		lda	$00, x
		and	(ip), y
		iny
		sta	$00, x
		lda	$01, x
		and	(ip), y
		sta	$01, x
		jmp	startup.yexec
}

#pragma	bytecode(BC_BINOP_ANDI_16, inp_binop_andi_16)

__asm inp_binop_ori_16
{
		lda	(ip), y
		iny
		tax
		lda	$00, x
		ora	(ip), y
		iny
		sta	$00, x
		lda	$01, x
		ora	(ip), y
		sta	$01, x
		jmp	startup.yexec
}

#pragma	bytecode(BC_BINOP_ORI_16, inp_binop_ori_16)

__asm inp_binop_shl_16
{
inp_binop_shlr_16:
		lda	(ip), y
		tax
		lda	$00, x
		byt $2c
inp_binop_shli_16:
		lda	(ip), y
		and	#$0f
		beq	W1
		tax
		lda	accu + 1
L1:		asl	accu
		rol
		dex		
		bne	L1
		sta	accu + 1
W1:		jmp	startup.yexec
}

#pragma	bytecode(BC_BINOP_SHLI_16, inp_binop_shl_16.inp_binop_shli_16)
#pragma	bytecode(BC_BINOP_SHLR_16, inp_binop_shl_16.inp_binop_shlr_16)

__asm inp_binop_shr_u16
{
inp_binop_shrr_u16:
		lda	(ip), y
		tax
		lda	$00, x
		byt $2c
inp_binop_shri_u16:
		lda	(ip), y
		and	#$0f
		beq	W1
		tax
		lda	accu + 1
L1:		lsr
		ror	accu
		dex		
		bne	L1
		sta	accu + 1
W1:		jmp	startup.yexec
}

#pragma	bytecode(BC_BINOP_SHRI_U16, inp_binop_shr_u16.inp_binop_shri_u16)
#pragma	bytecode(BC_BINOP_SHRR_U16, inp_binop_shr_u16.inp_binop_shrr_u16)

__asm inp_binop_shr_s16
{
inp_binop_shrr_s16:
		lda	(ip), y
		tax
		lda	$00, x
		byt $2c
inp_binop_shri_s16:
		lda	(ip), y
		and	#$0f
		beq	W1
		tax
		lda	accu + 1
L1:		cmp	#$80
		ror
		ror	accu
		dex		
		bne	L1
		sta	accu + 1
W1:		jmp	startup.yexec
}

#pragma	bytecode(BC_BINOP_SHRI_I16, inp_binop_shr_s16.inp_binop_shri_s16)
#pragma	bytecode(BC_BINOP_SHRR_I16, inp_binop_shr_s16.inp_binop_shrr_s16)

__asm inp_binop_adda_16
{
		lda	(ip), y
		tax
		clc
		lda	$00, x
		adc	accu
		sta $00, x
		lda $01, x
		adc accu + 1
		sta $01, x
		jmp	startup.yexec
}

#pragma	bytecode(BC_BINOP_ADDA_16, inp_binop_adda_16)

__asm cmp16
{
inp_binop_cmpr_s16:
		lda	(ip), y
		tax

		sec
		lda	$01, x
		sbc accu + 1
		beq cmpeq
cmpnes:		
		bvc cmpsv
		eor #$80
cmpsv:	bmi cmp_lt
		bpl cmp_gt

inp_binop_cmpr_u16:
		lda	(ip), y
		tax

		lda	$01, x
		cmp	accu + 1
		bne	cmpne
cmpeq:
		lda	$00 , x
		cmp	accu
		bne	cmpne
		beq	cmp_eq

inp_binop_cmpi_u16:
		lda	(ip), y
		tax
		iny
		lda	(ip), y

		cmp	accu + 1
		bne	cmpne
		cpx	accu
		bne	cmpne
cmp_eq:
		lda	#0
		sta	accu
		sta	accu + 1
		jmp	startup.yexec
cmp_lt:		
		lda	#$ff
		sta	accu
		sta	accu +1 
		jmp	startup.yexec
cmpne:
		bcc	cmp_lt
cmp_gt:
		lda	#1
		sta	accu
		lda	#0
		sta	accu + 1
		jmp	startup.yexec

inp_binop_cmpi_s16:
		lda	(ip), y
		iny
		tax
		lda	(ip), y

		sec
		sbc	accu + 1
		bne	cmpnes
		cpx	accu
		bne	cmpne		
		beq	cmp_eq
}

#pragma	bytecode(BC_BINOP_CMPSR_16, cmp16.inp_binop_cmpr_s16)
#pragma	bytecode(BC_BINOP_CMPSI_16, cmp16.inp_binop_cmpi_s16)
#pragma	bytecode(BC_BINOP_CMPUR_16, cmp16.inp_binop_cmpr_u16)
#pragma	bytecode(BC_BINOP_CMPUI_16, cmp16.inp_binop_cmpi_u16)


__asm cmp8
{
inp_binop_cmpr_s8:
		lda	(ip), y
		tax

		sec
		lda	$00, x
		sbc accu
		beq cmp_eq
cmpnes:		
		bvc cmpsv
		eor #$80
cmpsv:	bmi cmp_lt
		bpl cmp_gt

inp_binop_cmpr_u8:
		lda	(ip), y
		tax

		lda	$00, x
		cmp	accu 
		bne	cmpne
		beq	cmp_eq

inp_binop_cmpi_u8:
		lda	(ip), y

		cmp	accu
cmp_check:		
		bne	cmpne
cmp_eq:
		lda	#0
		sta	accu
		sta	accu + 1
		jmp	startup.yexec
cmp_lt:		
		lda	#$ff
		sta	accu
		sta	accu +1 
		jmp	startup.yexec
cmpne:
		bcc	cmp_lt
cmp_gt:
		lda	#1
		sta	accu
		lda	#0
		sta	accu + 1
		jmp	startup.yexec

inp_binop_cmpi_s8:
		lda	(ip), y

		sec
		sbc	accu
		bne	cmpnes
		beq	cmp_eq
}


#pragma	bytecode(BC_BINOP_CMPSR_8, cmp8.inp_binop_cmpr_s8)
#pragma	bytecode(BC_BINOP_CMPSI_8, cmp8.inp_binop_cmpi_s8)
#pragma	bytecode(BC_BINOP_CMPUR_8, cmp8.inp_binop_cmpr_u8)
#pragma	bytecode(BC_BINOP_CMPUI_8, cmp8.inp_binop_cmpi_u8)

__asm loopu8
{
		lda	(ip),y
		tax
		inc $00, x
		iny
		lda (ip), y
		cmp $00, x
		jmp cmp8.cmp_check
}

#pragma	bytecode(BC_LOOP_U8, loopu8)

__asm bra
{
inp_jumps:
		lda	(ip), y
		bmi	W1
		sec
		adc	ip
		sta	ip
		bcc	W2
		inc	ip + 1
W2:		jmp	startup.zexec		
W1:		sec
		adc	ip
		sta	ip
		bcs	W3
		dec	ip + 1
W3:		jmp	startup.zexec		
	
inp_branchs_eq:
		lda	accu
		ora	accu + 1
		beq	inp_jumps
		jmp	startup.yexec
inp_branchs_ne:
		lda	accu
		ora	accu + 1
		bne	inp_jumps
		jmp	startup.yexec
inp_branchs_gt:
		lda	accu + 1
		bmi	W4
		ora	accu
		bne	inp_jumps
W4:		jmp	startup.yexec
inp_branchs_ge:
		lda	accu + 1
		bpl	inp_jumps
		jmp	startup.yexec
inp_branchs_lt:
		lda	accu + 1
		bmi	inp_jumps
		jmp	startup.yexec
inp_branchs_le:
		lda	accu + 1
		bmi	inp_jumps
		ora	accu
		beq	inp_jumps
		jmp	startup.yexec
}

#pragma	bytecode(BC_JUMPS, bra.inp_jumps)
#pragma	bytecode(BC_BRANCHS_EQ, bra.inp_branchs_eq)
#pragma	bytecode(BC_BRANCHS_NE, bra.inp_branchs_ne)
#pragma	bytecode(BC_BRANCHS_GT, bra.inp_branchs_gt)
#pragma	bytecode(BC_BRANCHS_GE, bra.inp_branchs_ge)
#pragma	bytecode(BC_BRANCHS_LT, bra.inp_branchs_lt)
#pragma	bytecode(BC_BRANCHS_LE, bra.inp_branchs_le)

__asm braf
{
inp_jumpf:
		sec
		lda	(ip), y
		adc	ip
		tax
		iny
		lda	(ip) , y
		adc	ip + 1
		sta	ip + 1
		stx	ip
		jmp	startup.zexec
	
inp_branchf_eq:
		lda	accu
		ora	accu + 1
		beq	inp_jumpf
		iny
		jmp	startup.yexec
inp_branchf_ne:
		lda	accu
		ora	accu + 1
		bne	inp_jumpf
		iny
		jmp	startup.yexec
inp_branchf_gt:
		lda	accu + 1
		bmi	W1
		ora	accu
		bne	inp_jumpf
W1:		iny
		jmp	startup.yexec
inp_branchf_ge:
		lda	accu + 1
		bpl	inp_jumpf
		iny
		jmp	startup.yexec		
inp_branchf_lt:
		lda	accu + 1
		bmi	inp_jumpf
		iny
		jmp	startup.yexec
inp_branchf_le:
		lda	accu + 1
		bmi	inp_jumpf
		ora	accu
		beq	inp_jumpf
		iny
		jmp	startup.yexec
}
	
#pragma	bytecode(BC_JUMPF, braf.inp_jumpf)
#pragma	bytecode(BC_BRANCHF_EQ, braf.inp_branchf_eq)
#pragma	bytecode(BC_BRANCHF_NE, braf.inp_branchf_ne)
#pragma	bytecode(BC_BRANCHF_GT, braf.inp_branchf_gt)
#pragma	bytecode(BC_BRANCHF_GE, braf.inp_branchf_ge)
#pragma	bytecode(BC_BRANCHF_LT, braf.inp_branchf_lt)
#pragma	bytecode(BC_BRANCHF_LE, braf.inp_branchf_le)

__asm inp_enter
{
		// allocate space on stack
		sec
		lda	sp
		sbc	(ip), y
		iny
		sta	sp
		lda	sp + 1
		sbc	(ip), y
		iny
		sta	sp + 1
		
		// number of registers to save
		lda	(ip), y	
		sty	tmpy

		// save frame pointer at end of list

		tay
		lda	fp
		sta	(sp), y
		iny
		lda	fp + 1
		sta	(sp), y
		
		// calculate new frame pointer

		tya
		sec
		adc	sp
		sta	fp
		lda	#0
		adc	sp + 1
		sta	fp + 1
		
		// copy registers
		dey
		beq	W1
		
L1:		lda	sregs - 1, y
		dey
		sta	(sp), y
		bne	L1
		
		// done
		
W1:		ldy	tmpy
		
		jmp	startup.yexec
}
	
#pragma	bytecode(BC_ENTER, inp_enter)

__asm inp_return
{
		// number of registers to restore

		lda	(ip), y
		iny
		sty	tmpy
		
		// restore frame pointer
		
		tay
		lda	(sp), y
		sta	fp
		iny
		lda	(sp), y
		sta	fp + 1

		// copy registers
		
		dey
		beq	W1
		dey
			
L1:		lda	(sp), y
		sta	sregs, y
		dey
		bpl	L1
W1:
		
		// adjust stack space
		
		ldy	tmpy
		clc
		lda	(ip), y
		iny
		adc	sp
		sta	sp		
		lda	(ip), y
		iny
		adc	sp + 1
		sta	sp + 1

		// reload ip from stack
		
		ldy	#1
		lda	(sp), y
		sta	ip + 1
		dey
		lda	(sp), y
		sta	ip
		jmp	startup.exec
}

#pragma	bytecode(BC_RETURN, inp_return)

__asm inp_push_frame
{
		sec
		lda	sp
		sbc	(ip), y
		iny
		sta	sp
		lda	sp + 1
		sbc	(ip), y
		sta	sp + 1

		jmp	startup.yexec
}

#pragma	bytecode(BC_PUSH_FRAME, inp_push_frame)

__asm inp_pop_frame
{
		clc
		lda	(ip), y
		iny
		adc	sp
		sta	sp		
		lda	(ip), y
		adc	sp + 1
		sta	sp + 1

		jmp	startup.yexec
}

#pragma	bytecode(BC_POP_FRAME, inp_pop_frame)

__asm inp_call
{		lda (ip), y
		sta addr
		iny
		lda (ip), y
		sta addr + 1
		iny
inp_call_addr:
		tya
		ldy	#0
		clc
		adc ip
		sta	(sp), y
		iny
		lda	ip + 1
		adc	#0
		sta	(sp), y
		lda	addr
		sta	ip
		lda	addr + 1
		sta	ip + 1
		jmp	startup.pexec
}

#pragma	bytecode(BC_CALL_ADDR, inp_call.inp_call_addr)
#pragma	bytecode(BC_CALL_ABS, inp_call)

__asm inp_copy
{
		lda	(ip), y
		sty	tmpy
		tay
		dey
		beq	W1
L1:		lda	(accu), y
		sta	(addr), y
		dey
		bne	L1
W1:
		lda	(accu), y
		sta	(addr), y
		ldy	tmpy
		jmp	startup.yexec
}

#pragma	bytecode(BC_COPY, inp_copy)

__asm inp_strcpy
{
		sty tmpy
		ldy #$ff
L1:		iny
		lda	(accu), y
		sta	(addr), y
		bne	L1
		ldy	tmpy
		jmp	startup.exec
}

#pragma	bytecode(BC_STRCPY, inp_strcpy)

__asm inp_copyl
{
		lda	(ip), y
		sta tmp
		iny
		lda (ip), y
		tax
		sty tmpy

		beq	W1
		ldy	#0
Loop1:
		lda (accu), y
		sta (addr), y
		iny
		bne	Loop1
		inc accu + 1
		inc addr + 1
		dex
		bne	Loop1
W1:
		ldy	tmp
		beq	W2
		dey
		beq	W3
Loop2:
		lda (accu), y
		sta (addr), y
		dey
		bne Loop2
W3:
		lda (accu), y
		sta (addr), y
W2:
		ldy	tmpy
		jmp	startup.yexec
}

#pragma	bytecode(BC_COPY_LONG, inp_copyl)

__asm freg
{
split_exp:
		lda	(ip), y
		iny
		tax
split_xexp:
		lda	$00, x
		sta	tmp + 0
		lda	$01, x
		sta	tmp + 1
		lda	$02, x
		sta	tmp + 2
		lda	$03, x
		sta	tmp + 3
split_texp:
		lda	tmp + 2
		asl	
		lda	tmp + 3
		rol
		sta	tmp + 5
		beq	ZT
		lda	tmp + 2
		ora	#$80
		sta	tmp + 2		
ZT:

split_aexp:
		lda	accu + 2
		asl	
		lda	accu + 3
		rol
		sta	tmp + 4
		beq	ZA
		lda	accu + 2
		ora	#$80
		sta	accu + 2
ZA:
		rts
		
merge_aexp:
		asl	accu + 3
		lda	tmp + 4
		ror
		sta	accu + 3
		bcs	W1
		lda	accu + 2
		and	#$7f
		sta	accu + 2
W1:				
		rts
}
		
__asm faddsub
{
		lda #$ff
		cmp tmp + 4
		beq INF
		cmp tmp + 5
		bne nINF
INF:
		lda	accu + 3
		ora #$7f
		sta	accu + 3
		lda #$80
		sta accu + 2
		lda #$00
		sta accu + 0
		sta accu + 1
		rts		
nINF:
		sec
		lda	tmp + 4
		sbc	tmp + 5
		beq	fas_aligned		
		tax
		
		bcs	fas_align2nd
		
		// check if first operand is below rounding
		cpx	#-23
		bcs	W1
		
		lda	tmp + 5
		sta	tmp + 4
		lda	#0
		sta	accu
		sta	accu + 1
		sta	accu + 2
		beq	fas_aligned
W1:
		lda accu + 2
L1:		
		lsr
		ror	accu + 1
		ror	accu
		inx
		bne	L1
		sta	accu + 2
		lda	tmp + 5
		sta	tmp + 4
		jmp	fas_aligned

fas_align2nd:
		// check if second operand is below rounding
		cpx	#24	
		bcs	fas_done
		lda tmp + 2
L2:		lsr
		ror	tmp + 1
		ror	tmp
		dex
		bne	L2
		sta tmp + 2

fas_aligned:
		lda	accu + 3
		and	#$80
		sta	accu + 3
		eor	tmp + 3
		bmi	fas_sub
		
		clc
		lda	accu
		adc	tmp
		sta	accu
		lda	accu + 1
		adc	tmp + 1
		sta	accu + 1
		lda	accu + 2
		adc	tmp + 2
		sta	accu + 2
		bcc	fas_done
		ror	accu + 2
		ror	accu + 1
		ror	accu
		inc	tmp + 4
fas_done:
		lda	tmp + 4
		cmp	#$ff
		beq	INF
		lsr
		ora	accu + 3
		sta	accu + 3
		bcs	W2
		lda	accu + 2
		and	#$7f
		sta	accu + 2
W2:				
		rts

fas_sub:	
		sec
		lda	accu
		sbc	tmp
		sta	accu
		lda	accu + 1
		sbc	tmp + 1
		sta	accu + 1
		lda	accu + 2
		sbc	tmp + 2
		sta	accu + 2
		bcs	fas_pos
		sec
		lda	#0
		sbc	accu
		sta	accu
		lda	#0
		sbc	accu + 1
		sta	accu + 1
		lda	#0
		sbc	accu + 2
		sta	accu + 2
		lda	accu + 3
		eor	#$80
		sta	accu + 3
fas_pos:
		lda	accu + 2
		bmi	fas_done
		
		ora	accu + 1
		ora	accu + 0
		beq	fas_zero
L3:
		dec	tmp + 4
		beq	fas_zero		// underflow
		asl	accu
		rol	accu + 1
		rol	accu + 2
		bpl	L3
		jmp	fas_done
fas_zero:
		lda	#0
		sta	accu + 0
		sta	accu + 1
		sta	accu + 2
		sta	accu + 3
		rts

}	


__asm inp_binop_add_f32
{
		jsr	freg.split_exp
		jsr faddsub
		jmp	startup.exec
}

#pragma	bytecode(BC_BINOP_ADD_F32, inp_binop_add_f32)
		
__asm inp_binop_sub_f32
{
		jsr	freg.split_exp
		lda	tmp + 3
		eor	#$80
		sta	tmp + 3
		jsr	faddsub
		jmp	startup.exec
}

#pragma	bytecode(BC_BINOP_SUB_F32, inp_binop_sub_f32)
		
__asm fmul8
{
		sec
		ror
		bcc	L2
L1:		tax
		clc
		tya
		adc	tmp + 6
		sta	tmp + 6
		lda	tmp + 7
		adc	accu + 1
		sta	tmp + 7
		lda	tmp + 8
		adc	accu + 2
		ror
		sta	tmp + 8
		txa
		ror	tmp + 7
		ror	tmp + 6
		lsr
		beq	W1
		bcs	L1
L2:
		ror	tmp + 8
		ror	tmp + 7
		ror	tmp + 6
		lsr
		bcc	L2
		bne	L1
W1:
		rts
}

__asm fmul
{
		lda	accu
		ora	accu + 1
		ora	accu + 2
		bne	W1
		sta	accu + 3
		rts
W1:
		lda	tmp
		ora	tmp + 1
		ora	tmp + 2
		bne	W2
		sta	accu
		sta	accu + 1
		sta	accu + 2
		sta	accu + 3
		rts
W2:	
		lda	accu + 3
		eor	tmp + 3
		and	#$80
		sta	accu + 3

		lda #$ff
		cmp tmp + 4
		beq INF
		cmp tmp + 5
		beq INF

		lda	#0
		sta	tmp + 6
		sta	tmp + 7
		sta	tmp + 8
		
		ldy accu
		lda	tmp
		bne	W4
		lda tmp + 1
		beq W5
		bne W6
W4:
		jsr	fmul8
		lda	tmp + 1
W6:		
		jsr	fmul8
W5:		
		lda	tmp + 2
		jsr	fmul8
		
		sec
		lda	tmp + 8
		bmi	W3
		asl	tmp + 6
		rol	tmp + 7
		rol
		clc
W3:		and	#$7f
		sta	tmp + 8
		
		lda	tmp + 4
		adc	tmp + 5
		bcc W7

		sbc	#$7f
		bcs INF
		cmp #$ff
		bne W8
INF:
		lda	accu + 3
		ora #$7f
		sta	accu + 3
		lda #$80
		sta accu + 2
		lda #$00
		sta accu + 0
		sta accu + 1
		rts
W7:
		sbc	#$7e
		bcc ZERO
W8:
		lsr
		ora	accu + 3
		sta	accu + 3
		lda	#0
		ror
		ora	tmp + 8
		sta	accu + 2
		lda	tmp + 7
		sta	accu + 1
		lda	tmp + 6
		sta	accu
		rts
ZERO:
		lda #0
		sta accu
		sta accu + 1
		sta accu + 2
		sta accu + 3
		rts
}

__asm inp_binop_mul_f32
{
		jsr	freg.split_exp
		sty tmpy
		jsr fmul
		ldy tmpy
		jmp	startup.exec
}

__asm fdiv
{
		lda	accu
		ora	accu + 1
		ora	accu + 2
		bne	W1
		sta	accu + 3
		rts
W1:
		lda	accu + 3
		eor	tmp + 3
		and	#$80
		sta	accu + 3

		lda tmp + 5
		beq INF
		lda tmp + 4
		cmp #$ff
		beq INF

		lda	#0
		sta	tmp + 6
		sta	tmp + 7
		sta	tmp + 8
		ldx	#24
		
L1:
		lda	accu + 0
		cmp	tmp + 0
		lda	accu + 1
		sbc	tmp + 1
		lda	accu + 2
		sbc	tmp + 2
		bcc	W2
		
L2:
		lda	accu + 0
		sbc	tmp + 0
		sta	accu + 0
		lda	accu + 1
		sbc	tmp + 1
		sta	accu + 1
		lda	accu + 2
		sbc	tmp + 2
		sta	accu + 2
		sec
W2:
		rol	tmp + 6
		rol	tmp + 7
		rol	tmp + 8
		dex
		beq	W3

		asl	accu
		rol	accu + 1
		rol	accu + 2
		bcs	L2
		bcc	L1
W3:
		sec
		lda	tmp + 8
		bmi	W4
		asl	tmp + 6
		rol	tmp + 7
		rol
		clc
W4:
		and	#$7f
		sta	tmp + 8

		lda	tmp + 4
		sbc	tmp + 5
		bcc W5		

		clc
		adc	#$7f
		bcs INF
		cmp #$ff
		bne W6
INF:
		lda	accu + 3
		ora #$7f
		sta	accu + 3
		lda #$80
		sta accu + 2
		lda #$00
		sta accu + 1
		sta accu + 0
		rts
W5:
		adc	#$7f
		bcc ZERO
W6:
		lsr
		ora	accu + 3
		sta	accu + 3
		lda	#0
		ror
		ora	tmp + 8
		sta	accu + 2
		lda	tmp + 7
		sta	accu + 1
		lda	tmp + 6
		sta	accu
		rts

ZERO:
		lda #$00
		sta	accu + 3
		sta accu + 2
		sta accu + 1
		sta accu + 0
		rts


}

#pragma	bytecode(BC_BINOP_MUL_F32, inp_binop_mul_f32)

__asm inp_binop_div_f32
{
		jsr	freg.split_exp
		jsr fdiv
		jmp	startup.exec
}

#pragma	bytecode(BC_BINOP_DIV_F32, inp_binop_div_f32)

__asm fcmp
{
		lda	accu + 3
		eor	tmp + 3
		bpl	W1
		
		// different sig, check zero case

		lda	accu + 3
		and	#$7f
		ora	accu + 2
		ora	accu + 1
		ora	accu
		bne	W2
		
		lda	tmp + 3
		
		and	#$7f
		ora	tmp + 2
		ora	tmp + 1
		ora	tmp + 0
		beq	fcmpeq
W2:		lda	accu + 3
		bmi	fcmpgt
		bpl	fcmplt		
		
W1:		
		// same sign
		lda	accu + 3
		cmp	tmp + 3
		bne	W3
		lda	accu + 2
		cmp	tmp + 2
		bne	W3
		lda	accu + 1
		cmp	tmp + 1
		bne	W3
		lda	accu
		cmp	tmp
		bne	W3

fcmpeq:
		lda	#0
		rts
		
W3:		bcs	W4

		bit	accu + 3
		bmi	fcmplt
		
fcmpgt:
		lda	#1
		rts

W4:		bit	accu + 3
		bmi	fcmpgt

fcmplt:
		lda	#$ff
		rts
}


__asm inp_binop_cmp_f32
{
		lda	(ip), y
		iny
		tax
		
		lda	accu + 3
		eor	$03, x
		bpl	W1
		
		// different sig, check zero case

		lda	accu + 3
		and	#$7f
		ora	accu + 2
		ora	accu + 1
		ora	accu
		bne	W2
		
		lda	$03, x
		
		and	#$7f
		ora	$02, x
		ora	$01, x
		ora	$00, x
		beq	ibcmpf32eq
W2:		lda	accu + 3
		bmi	ibcmpf32gt
		bpl	ibcmpf32lt		
		
W1:		
		// same sign
		lda	accu + 3
		cmp	$03, x
		bne	W3
		lda	accu + 2
		cmp	$02, x
		bne	W3
		lda	accu + 1
		cmp	$01, x
		bne	W3
		lda	accu
		cmp	$00, x
		bne	W3

ibcmpf32eq:
		lda	#0
		sta	accu
		sta	accu + 1
		jmp	startup.exec
		
W3:		bcs	W4

		bit	accu + 3
		bmi	ibcmpf32lt
		
ibcmpf32gt:
		lda	#0
		sta	accu + 1
		lda	#1
		sta	accu
		jmp	startup.exec

W4:		bit	accu + 3
		bmi	ibcmpf32gt

ibcmpf32lt:
		lda	#$ff
		sta	accu
		sta	accu + 1		
		jmp	startup.exec
}

#pragma	bytecode(BC_BINOP_CMP_F32, inp_binop_cmp_f32)
		
__asm inp_op_negate_f32
{
		lda	accu + 3
		eor	#$80
		sta	accu + 3
		jmp	startup.exec
}

#pragma	bytecode(BC_OP_NEGATE_F32, inp_op_negate_f32)

__asm uint16_to_float
{
		lda	accu
		ora	accu + 1
		bne	W1
		sta	accu + 2
		sta	accu + 3
		rts
W1:
		ldx	#$8e
		lda	accu + 1
		bmi	W2
L1:
		dex
		asl	accu
		rol
		bpl	L1
W2:
		and	#$7f
		sta	accu + 2
		lda	accu
		sta	accu + 1
		txa
		lsr
		sta	accu + 3
		lda	#0
		sta	accu
		ror
		ora	accu + 2
		sta	accu + 2
		rts
}

__asm sint16_to_float
{
		bit	accu + 1
		bmi	W1
		jmp	uint16_to_float
W1:		
		sec
		lda	#0
		sbc	accu
		sta	accu
		lda	#0
		sbc	accu + 1
		sta	accu + 1	
		jsr	uint16_to_float
		lda	accu + 3
		ora	#$80
		sta	accu + 3
		rts
}


__asm inp_conv_u16_f32	
{
		jsr	uint16_to_float
		jmp	startup.exec
}

#pragma	bytecode(BC_CONV_U16_F32, inp_conv_u16_f32)

__asm inp_conv_i16_f32		
{
		jsr	sint16_to_float
		jmp	startup.exec
}

#pragma	bytecode(BC_CONV_I16_F32, inp_conv_i16_f32)

__asm f32_to_i16
{
		jsr	freg.split_aexp
		lda	tmp + 4
		cmp	#$7f
		bcs	W1
		lda	#0
		sta	accu
		sta	accu + 1
		rts
W1:
		sec
		sbc	#$8e
		bcc	W2
		lda	#$ff
		sta	accu
		lda	#$7f
		sta	accu + 1
		bne	W3
W2:
		tax
L1:
		lsr	accu + 2
		ror	accu + 1
		inx
		bne	L1
W3:
		bit	accu + 3
		bpl	W4
		
		sec
		lda	#0
		sbc	accu + 1
		sta	accu
		lda	#0
		sbc	accu + 2
		sta	accu + 1
		rts
W4:
		lda	accu + 1
		sta	accu
		lda	accu + 2
		sta	accu + 1
		rts
}

__asm inp_conv_f32_i16
{
		jsr	f32_to_i16
		jmp	startup.exec
}

#pragma	bytecode(BC_CONV_F32_I16, inp_conv_f32_i16)

__asm f32_to_u16
{
		jsr	freg.split_aexp
		lda	tmp + 4
		cmp	#$7f
		bcs	W1
		lda	#0
		sta	accu
		sta	accu + 1
		rts
W1:
		sec
		sbc	#$8e
		beq	W2
		bcc	W3
		lda	#$ff
		sta	accu
		sta	accu + 1
		rts
W3:
		tax
L1:
		lsr	accu + 2
		ror	accu + 1
		inx
		bne	L1
W2:
		lda	accu + 1
		sta	accu
		lda	accu + 2
		sta	accu + 1

		rts
}

__asm inp_conv_f32_u16
{
		jsr f32_to_u16
		jmp	startup.exec
}

#pragma	bytecode(BC_CONV_F32_U16, inp_conv_f32_u16)

__asm inp_op_abs_f32	
{
		lda	accu + 3
		and	#$7f
		sta	accu + 3
		jmp	startup.exec
}

#pragma	bytecode(BC_OP_ABS_F32, inp_op_abs_f32)

unsigned char ubitmask[8] = {0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};

unsigned char bitshift[56] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#pragma runtime(bitshift, bitshift)

__asm fround {
ffloor:
		bit	accu + 3
		bpl	frdown
		bmi	frup

fceil:
		bit	accu + 3
		bmi	frdown
		bpl	frup

frdzero:
		lda	#0
		sta	accu
		sta	accu + 1
		sta	accu + 2
		sta	accu + 3
		rts
		
frdown:
		lda	tmp + 4
		cmp	#$7f
		bcc	frdzero
		cmp	#$87
		bcc	frd1
		cmp	#$8f
		bcc	frd2
		cmp	#$97
		bcs	frd3

		sec
		sbc	#$8f
		tax
		lda	accu
		and	ubitmask, x
		sta	accu
		
		jmp	frd3
frd1:
		sec
		sbc	#$7f
		tax
		lda	accu + 2
		and	ubitmask, x
		sta	accu + 2		
		lda	#0
		sta	accu
		sta	accu + 1
		
		jmp	frd3
frd2:
		sec
		sbc	#$87
		tax
		lda	accu + 1
		and	ubitmask, x
		sta	accu + 1		
		lda	#0
		sta	accu

		jmp	frd3

frd3:
		jmp	freg.merge_aexp

frone:
		lda	#$7f
		sta	tmp + 4
		lda	#0
		sta	accu + 0
		sta	accu + 1
		lda	#$80
		sta	accu + 2
		jmp	freg.merge_aexp

frup:
		lda	accu
		ora	accu + 1
		ora	accu + 2
		beq	frdzero		
		lda	tmp + 4
		cmp	#$7f
		bcc	frone
		cmp	#$87
		bcc	fru1
		cmp	#$8f
		bcc	fru2
		cmp	#$97
		bcs	fru3
		
		sec
		sbc	#$8f
		tax

		clc
		lda	ubitmask, x
		eor	#$ff				
		adc	accu
		sta	accu
		lda	#0
		adc	accu + 1
		sta	accu + 1
		lda	#0
		adc	accu + 2		
		bcc	W1
		ror
		ror	accu + 1
		ror	accu
		inc	tmp + 4
W1:		sta	accu + 2		
		jmp	frdown		
fru1:
		sec
		sbc	#$7f
		tax

		clc
		lda	#$ff
		adc	accu
		lda	#$ff
		adc	accu + 1
		lda	ubitmask, x
		eor	#$ff				
		adc	accu + 2
		bcc	W2
		ror
		ror	accu + 1
		ror	accu
		inc	tmp + 4
W2:		sta	accu + 2		
		jmp	frdown
fru2:
		sec
		sbc	#$87
		tax

		clc
		lda	#$ff
		adc	accu
		lda	ubitmask, x
		eor	#$ff				
		adc	accu + 1
		sta	accu + 1
		lda	#0
		adc	accu + 2		
		bcc	W3
		ror
		ror	accu + 1
		ror	accu
		inc	tmp + 4
W3:		sta	accu + 2		
		jmp	frdown		
fru3:
		jmp	freg.merge_aexp
}

#pragma runtime(fsplita, freg.split_aexp)
#pragma runtime(fsplitt, freg.split_texp)
#pragma runtime(fsplitx, freg.split_xexp)
#pragma runtime(fmergea, freg.merge_aexp)
#pragma runtime(faddsub, faddsub)
#pragma runtime(fmul, fmul)
#pragma runtime(fdiv, fdiv)
#pragma runtime(fcmp, fcmp)
#pragma runtime(ffromi, sint16_to_float)
#pragma runtime(ffromu, uint16_to_float)
#pragma runtime(ftoi, f32_to_i16)
#pragma runtime(ftou, f32_to_u16)
#pragma runtime(ffloor, fround.ffloor)
#pragma runtime(fceil, fround.fceil)

__asm inp_op_floor_f32
{
		jsr	freg.split_aexp
		bit	accu + 3
		bpl	W1
		jsr	fround.frup
		jmp	startup.exec
W1:		jsr	fround.frdown
		jmp	startup.exec
}

#pragma	bytecode(BC_OP_FLOOR_F32, inp_op_floor_f32)


__asm inp_op_ceil_f32				
{
		jsr	freg.split_aexp
		bit	accu + 3
		bpl	W1
		jsr	fround.frdown
		jmp	startup.exec
W1:		jsr	fround.frup		
		jmp	startup.exec
}

#pragma	bytecode(BC_OP_CEIL_F32, inp_op_ceil_f32)

__asm inp_op_extrt
{
		lda	(ip), y
		iny
		sta	_c1 + 1
		lda	(ip), y
		iny
		sta	_c1 + 2
		lda	(ip), y
		iny
		tax
_c1:	jsr $0000
		jmp	startup.exec		
}

#pragma bytecode(BC_EXTRT, inp_op_extrt)


__asm inp_op_ext_u16
{
		lda	#0
		sta	accu + 2
		sta	accu + 3
}

#pragma bytecode(BC_CONV_U16_U32, inp_op_ext_u16)


__asm inp_op_ext_s16
{
		lda	accu + 1
		ora #$7f
		bmi w1
		lda #0
w1:
		sta	accu + 2
		sta	accu + 3
}

#pragma bytecode(BC_CONV_I16_I32, inp_op_ext_s16)

__asm inp_op_invert_32
{
		lda	accu + 0
		eor #$ff
		sta accu + 0
		lda	accu + 1
		eor #$ff
		sta accu + 1
		lda	accu + 2
		eor #$ff
		sta accu + 2
		lda	accu + 3
		eor #$ff
		sta accu + 3
}

#pragma bytecode(BC_OP_INVERT_32, inp_op_invert_32)


#pragma bytecode(BC_OP_NEGATE_32, negaccu32)


__asm inp_op_addr_32
{
		clc
		lda	accu + 0
		adc $00, x
		sta accu + 0
		lda	accu + 1
		adc $01, x
		sta accu + 1
		lda	accu + 2
		adc $02, x
		sta accu + 2
		lda	accu + 3
		adc $03, x
		sta accu + 3
}

#pragma bytecode(BC_BINOP_ADD_L32, inp_op_addr_32)

__asm inp_op_subr_32
{
		sec
		lda	accu + 0
		sbc $00, x
		sta accu + 0
		lda	accu + 1
		sbc $01, x
		sta accu + 1
		lda	accu + 2
		sbc $02, x
		sta accu + 2
		lda	accu + 3
		sbc $03, x
		sta accu + 3
}

#pragma bytecode(BC_BINOP_SUB_L32, inp_op_subr_32)

__asm inp_op_andr_32
{
		lda	accu + 0
		and $00, x
		sta accu + 0
		lda	accu + 1
		and $01, x
		sta accu + 1
		lda	accu + 2
		and $02, x
		sta accu + 2
		lda	accu + 3
		and $03, x
		sta accu + 3
}

#pragma bytecode(BC_BINOP_AND_L32, inp_op_andr_32)

__asm inp_op_orr_32
{
		lda	accu + 0
		ora $00, x
		sta accu + 0
		lda	accu + 1
		ora $01, x
		sta accu + 1
		lda	accu + 2
		ora $02, x
		sta accu + 2
		lda	accu + 3
		ora $03, x
		sta accu + 3
}

#pragma bytecode(BC_BINOP_OR_L32, inp_op_orr_32)

__asm inp_op_xorr_32
{
		lda	accu + 0
		eor $00, x
		sta accu + 0
		lda	accu + 1
		eor $01, x
		sta accu + 1
		lda	accu + 2
		eor $02, x
		sta accu + 2
		lda	accu + 3
		eor $03, x
		sta accu + 3
}

#pragma bytecode(BC_BINOP_XOR_L32, inp_op_xorr_32)

__asm inp_op_mulr_32
{
		lda	#0
		sta	tmp + 4
		sta	tmp + 5
		sta	tmp + 6
		sta	tmp + 7

		lda	$00, x
		sta	tmp + 0
		lda	$01, x
		sta	tmp + 1
		lda	$02, x
		sta	tmp + 2
		lda	$03, x
		sta	tmp + 3
		ldx	#32
L1:		lsr	tmp + 3
		ror	tmp + 2
		ror	tmp + 1
		ror	tmp + 0
		bcc	W1
		clc
		lda	tmp + 4
		adc	accu
		sta	tmp + 4
		lda	tmp + 5
		adc	accu + 1
		sta	tmp + 5
		lda	tmp + 6
		adc	accu + 2
		sta	tmp + 6
		lda	tmp + 7
		adc	accu + 3
		sta	tmp + 7
W1:		asl	accu
		rol	accu + 1
		rol	accu + 2
		rol	accu + 3
		dex
		bne	L1
		lda	tmp + 4
		sta	accu
		lda	tmp + 5
		sta	accu + 1
		lda	tmp + 6
		sta	accu + 2
		lda	tmp + 7
		sta	accu + 3
}

#pragma bytecode(BC_BINOP_MUL_L32, inp_op_mulr_32)

__asm inp_binop_div_u32
{
		lda	$00, x
		sta	tmp + 0
		lda	$01, x
		sta	tmp + 1
		lda	$02, x
		sta	tmp + 2
		lda	$03, x
		sta	tmp + 3
		jsr	divmod32
}

#pragma	bytecode(BC_BINOP_DIV_U32, inp_binop_div_u32)

__asm inp_binop_mod_u32
{
		lda	$00, x
		sta	tmp + 0
		lda	$01, x
		sta	tmp + 1
		lda	$02, x
		sta	tmp + 2
		lda	$03, x
		sta	tmp + 3
		jsr	divmod32
		lda	tmp + 4
		sta	accu
		lda	tmp + 5
		sta	accu + 1
		lda	tmp + 6
		sta	accu + 2
		lda	tmp + 7
		sta	accu + 3
}

#pragma	bytecode(BC_BINOP_MOD_U32, inp_binop_mod_u32)

__asm inp_binop_div_s32
{
		lda	$00, x
		sta	tmp + 0
		lda	$01, x
		sta	tmp + 1
		lda	$02, x
		sta	tmp + 2
		lda	$03, x
		sta	tmp + 3
		bit	accu + 3
		bpl	L1
		jsr	negaccu32
		bit	tmp + 3
		bpl	L2
		jsr	negtmp32
L3:		jsr	divmod32
		rts
L1:		bit	tmp + 3
		bpl	L3
		jsr	negtmp32
L2:		jsr	divmod32
		jsr	negaccu32
}

#pragma	bytecode(BC_BINOP_DIV_I32, inp_binop_div_s32)

__asm inp_binop_mod_s32
{
		lda	$00, x
		sta	tmp + 0
		lda	$01, x
		sta	tmp + 1
		lda	$02, x
		sta	tmp + 2
		lda	$03, x
		sta	tmp + 3
		bit	accu + 3
		bpl	L1
		jsr	negaccu32
		bit	tmp + 3
		bpl	L2
		jsr	negtmp32
L3:		jsr	divmod32
		lda	tmp + 4
		sta	accu
		lda	tmp + 5
		sta	accu + 1
		lda	tmp + 6
		sta	accu + 2
		lda	tmp + 7
		sta	accu + 3
		rts
L1:		bit	tmp + 3
		bpl	L3
		jsr	negtmp32
L2:		jsr	divmod32
		lda	tmp + 4
		sta	accu
		lda	tmp + 5
		sta	accu + 1
		lda	tmp + 6
		sta	accu + 2
		lda	tmp + 7
		sta	accu + 3
		jsr	negaccu32
}

#pragma	bytecode(BC_BINOP_MOD_I32, inp_binop_mod_s32)

__asm inp_op_shl_l32
{
		lda $00, x
		and #31
		beq W1
		tax
		lda accu + 0
L1:		asl
		rol accu + 1
		rol accu + 2
		rol accu + 3
		dex
		bne L1
		sta accu + 0
W1:
}

#pragma bytecode(BC_BINOP_SHL_L32, inp_op_shl_l32)


__asm inp_op_shr_u32
{
		lda $00, x
		and #31
		beq W1
		tax
		lda accu + 3
L1:		lsr
		ror accu + 2
		ror accu + 1
		ror accu + 0
		dex
		bne L1
		sta accu + 3
W1:
}

#pragma bytecode(BC_BINOP_SHR_U32, inp_op_shr_u32)

__asm inp_op_shr_s32
{
		lda $00, x
		and #31
		beq W1
		tax
		lda accu + 3
L1:		cmp #$80
		ror
		ror accu + 2
		ror accu + 1
		ror accu + 0
		dex
		bne L1
		sta accu + 3
W1:
}

#pragma bytecode(BC_BINOP_SHR_I32, inp_op_shr_s32)

__asm inp_op_cmp_u32
{
		lda	$03, x
		cmp	accu + 3
		bne	cmpne
		lda	$02, x
		cmp	accu + 2
		bne	cmpne
		lda	$01, x
		cmp	accu + 1
		bne	cmpne
		lda	$00 , x
		cmp	accu
		bne	cmpne

		lda	#0
		sta	accu
		sta	accu + 1
		rts
cmp_lt:		
		lda	#$ff
		sta	accu
		sta	accu +1 
		rts
cmpne:
		bcc	cmp_lt
		lda	#1
		sta	accu
		lda	#0
		sta	accu + 1
		rts
}

#pragma bytecode(BC_BINOP_CMP_U32, inp_op_cmp_u32)

__asm inp_op_cmp_s32
{
		lda	accu + 3
		eor	#$80
		sta	accu + 3
		lda	$03, x
		eor #$80
		cmp	accu + 3
		bne	cmpne
		lda	$02, x
		cmp	accu + 2
		bne	cmpne
		lda	$01, x
		cmp	accu + 1
		bne	cmpne
		lda	$00 , x
		cmp	accu
		bne	cmpne

		lda	#0
		sta	accu
		sta	accu + 1
		rts
cmp_lt:		
		lda	#$ff
		sta	accu
		sta	accu +1 
		rts
cmpne:
		bcc	cmp_lt
		lda	#1
		sta	accu
		lda	#0
		sta	accu + 1
		rts
}

#pragma bytecode(BC_BINOP_CMP_S32, inp_op_cmp_s32)
