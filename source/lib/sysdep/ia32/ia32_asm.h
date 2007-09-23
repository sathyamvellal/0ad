/**
 * =========================================================================
 * File        : ia32_asm.h
 * Project     : 0 A.D.
 * Description : interface to various IA-32 functions (written in asm)
 * =========================================================================
 */

// license: GPL; see lib/license.txt

#ifndef INCLUDED_IA32_ASM
#define INCLUDED_IA32_ASM

#ifdef __cplusplus
extern "C" {
#endif

/**
 * order in which ia32_asm_cpuid stores register values
 **/
enum IA32Regs
{
	EAX,
	EBX,
	ECX,
	EDX
};

/**
 * try to call the specified CPUID sub-function.
 * (note: ECX is set to 0 beforehand as required by sub-function 4)
 * fills register array according to IA32Regs.
 * @return true on success or false if the sub-function isn't supported.
 **/
extern bool ia32_asm_cpuid(u32 func, u32* regs);

/**
 * for all 1-bits in mask, update the corresponding FPU control word bits
 * with the bit values in new_val.
 * @return 0 to indicate success.
 **/
extern uint ia32_asm_control87(uint new_val, uint mask);

/**
 * @return the current value of the TimeStampCounter in edx:eax
 * (interpretable as a u64 when using the standard Win32 calling convention)
 **/
extern u64 ia32_asm_rdtsc_edx_eax(void);

/**
 * write the current execution state (e.g. all register values) into
 * (Win32::CONTEXT*)pcontext (defined as void* to avoid dependency).
 **/
extern void ia32_asm_GetCurrentContext(void* pcontext);


// implementations of POSIX/SUS functions

/// see fpclassify
extern uint ia32_asm_fpclassifyd(double d);
extern uint ia32_asm_fpclassifyf(float f);

/// see rintf
extern float ia32_asm_rintf(float);
extern double ia32_asm_rint(double);
extern float ia32_asm_fminf(float, float);
extern float ia32_asm_fmaxf(float, float);


// misc

/**
 * @return the (integral) base 2 logarithm, or -1 if the number
 * is not a power-of-two.
 **/
extern int ia32_asm_log2_of_pow2(uint n);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef INCLUDED_IA32_ASM
