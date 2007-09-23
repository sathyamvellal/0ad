/**
 * =========================================================================
 * File        : ia32.h
 * Project     : 0 A.D.
 * Description : C++ and inline asm implementations of IA-32 functions
 * =========================================================================
 */

// license: GPL; see lib/license.txt

#ifndef INCLUDED_IA32
#define INCLUDED_IA32

#if !CPU_IA32
#error "including ia32.h without CPU_IA32=1"
#endif

#include "ia32_asm.h"


/**
 * CPU vendor.
 * (this is exposed because some CPUID functions are vendor-specific.)
 * (an enum is easier to compare than the original string values.)
 **/
enum Ia32Vendor
{
	IA32_VENDOR_UNKNOWN,
	IA32_VENDOR_INTEL,
	IA32_VENDOR_AMD,
};

extern Ia32Vendor ia32_Vendor();


/**
 * @return the colloquial processor generation
 * (6 = Pentium II / K6, 7 = Pentium III / Athlon, 8 = Opteron)
 **/
extern uint ia32_Generation();


/**
 * bit indices of CPU capability flags (128 bits).
 * values are defined by IA-32 CPUID feature flags - do not change!
 **/
enum IA32Cap
{
	// standard (ecx) - currently only defined by Intel
	IA32_CAP_SSE3            = 0+0,	// Streaming SIMD Extensions 3
	IA32_CAP_EST             = 0+7,	// Enhanced Speedstep Technology

	// standard (edx)
	IA32_CAP_FPU             = 32+0,	// Floating Point Unit
	IA32_CAP_TSC             = 32+4,	// TimeStamp Counter
	IA32_CAP_CMOV            = 32+15,	// Conditional MOVe
	IA32_CAP_TM_SCC          = 32+22,	// Thermal Monitoring and Software Controlled Clock
	IA32_CAP_MMX             = 32+23,	// MultiMedia eXtensions
	IA32_CAP_SSE             = 32+25,	// Streaming SIMD Extensions
	IA32_CAP_SSE2            = 32+26,	// Streaming SIMD Extensions 2
	IA32_CAP_HT              = 32+28,	// HyperThreading

	// extended (ecx)
	IA32_CAP_AMD_CMP_LEGACY  = 64+1,	// N-core and IA32_CAP_HT is falsely set

	// extended (edx)
	IA32_CAP_AMD_MP          = 96+19,	// MultiProcessing capable; reserved on AMD64
	IA32_CAP_AMD_MMX_EXT     = 96+22,
	IA32_CAP_AMD_3DNOW_PRO   = 96+30,
	IA32_CAP_AMD_3DNOW       = 96+31
};

/**
 * @return whether the CPU supports the indicated IA32Cap / feature flag.
 **/
extern bool ia32_cap(IA32Cap cap);


//-----------------------------------------------------------------------------
// stateless

/**
 * @return APIC ID of the currently executing processor.
 *
 * the implementation uses CPUID.1 and only works on >= 8th generation CPUs;
 * (P4/Athlon XP); otherwise it returns 0. the alternative of accessing the
 * APIC mmio registers is not feasible - mahaf_MapPhysicalMemory only works
 * reliably on WinXP. also, the OS already has the APIC registers mapped and
 * in constant use, and we don't want to interfere.
 **/
extern uint ia32_ApicId();


/**
 * check if there is an IA-32 CALL instruction right before ret_addr.
 * @return INFO::OK if so and ERR::FAIL if not.
 *
 * also attempts to determine the call target. if that is possible
 * (directly addressed relative or indirect jumps), it is stored in
 * target, which is otherwise 0.
 *
 * this function is used for walking the call stack.
 **/
extern LibError ia32_GetCallTarget(void* ret_addr, void** target);


/// safe but slow inline-asm version
extern u64 ia32_rdtsc_safe(void);

/**
 * @return the current value of the TimeStampCounter (a counter of
 * CPU cycles since power-on, which is useful for high-resolution timing
 * but potentially differs between multiple CPUs)
 **/
extern u64 ia32_rdtsc();	// only for CppDoc's benefit
#if CONFIG_RETURN64_EDX_EAX
# define ia32_rdtsc ia32_asm_rdtsc_edx_eax
#else
# define ia32_rdtsc ia32_rdtsc_safe
#endif

/**
 * trigger a breakpoint inside this function when it is called.
 **/
extern void ia32_DebugBreak(void);



/// fpclassify return values
#define IA32_FP_NAN       0x0100
#define IA32_FP_NORMAL    0x0400
#define IA32_FP_INFINITE  (IA32_FP_NAN | IA32_FP_NORMAL)
#define IA32_FP_ZERO      0x4000
#define IA32_FP_SUBNORMAL (IA32_FP_NORMAL | IA32_FP_ZERO)

// FPU control word (for ia32_asm_control87)
// .. Precision Control:
#define IA32_MCW_PC 0x0300
#define IA32_PC_24  0x0000
// .. Rounding Control:
#define IA32_MCW_RC  0x0C00
#define IA32_RC_NEAR 0x0000
#define IA32_RC_DOWN 0x0400
#define IA32_RC_UP   0x0800
#define IA32_RC_CHOP 0x0C00
// .. Exception Mask:
#define IA32_MCW_EM 0x003f
#define IA32_EM_INVALID    BIT(0)
#define IA32_EM_DENORMAL   BIT(1)
#define IA32_EM_ZERODIVIDE BIT(2)
#define IA32_EM_OVERFLOW   BIT(3)
#define IA32_EM_UNDERFLOW  BIT(4)
#define IA32_EM_INEXACT    BIT(5)

#endif	// #ifndef INCLUDED_IA32
