	.syntax unified
	.cpu cortex-m0
	.fpu softvfp
	.eabi_attribute 23, 1
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 1
	.eabi_attribute 30, 6
	.eabi_attribute 34, 0
	.eabi_attribute 18, 4
	.thumb
	.syntax unified
	.file	"div6.c"
	.global	__aeabi_uidiv
	.section	.text.foo0,"ax",%progbits
	.align	2
	.global	foo0
	.code	16
	.thumb_func
	.type	foo0, %function
foo0:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	push	{r7, lr}
	sub	sp, sp, #8
	add	r7, sp, #0
	movs	r2, r0
	adds	r3, r7, #6
	strh	r2, [r3]
	adds	r3, r7, #6
	ldrh	r3, [r3]
	movs	r1, #3
	movs	r0, r3
	bl	__aeabi_uidiv
	movs	r3, r0
	uxth	r3, r3
	movs	r0, r3
	mov	sp, r7
	add	sp, sp, #8
	@ sp needed
	pop	{r7, pc}
	.size	foo0, .-foo0
	.section	.text.foo3,"ax",%progbits
	.align	2
	.global	foo3
	.code	16
	.thumb_func
	.type	foo3, %function
foo3:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, lr}
	movs	r1, #3
	bl	__aeabi_uidiv
	@ sp needed
	uxth	r0, r0
	pop	{r4, pc}
	.size	foo3, .-foo3
	.section	.text.mul3,"ax",%progbits
	.align	2
	.global	mul3
	.code	16
	.thumb_func
	.type	mul3, %function
mul3:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	ldr	r3, .L5
	@ sp needed
	muls	r0, r3
	lsrs	r0, r0, #13
	bx	lr
.L6:
	.align	2
.L5:
	.word	2731
	.size	mul3, .-mul3
	.ident	"GCC: (GNU Tools for ARM Embedded Processors) 5.4.1 20160919 (release) [ARM/embedded-5-branch revision 240496]"
