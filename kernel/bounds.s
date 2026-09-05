	.text
	.file	"bounds.c"
	.globl	main                            // -- Begin function main
	.p2align	2
	.type	main,@function
main:                                   // @main
// %bb.0:
	//APP
	
.ascii "->NR_PAGEFLAGS 24 __NR_PAGEFLAGS"
	//NO_APP
	//APP
	
.ascii "->MAX_NR_ZONES 4 __MAX_NR_ZONES"
	//NO_APP
	//APP
	
.ascii "->NR_CPUS_BITS 8 order_base_2(CONFIG_NR_CPUS)"
	//NO_APP
	//APP
	
.ascii "->SPINLOCK_SIZE 4 sizeof(spinlock_t)"
	//NO_APP
	mov	w0, wzr
	ret
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
                                        // -- End function
	.ident	"Proton clang version 13.0.0 (https://github.com/llvm/llvm-project b4fd512c36ca344a3ff69350219e8b0a67e9472a)"
	.section	".note.GNU-stack","",@progbits
