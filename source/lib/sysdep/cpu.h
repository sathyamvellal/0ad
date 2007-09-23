/**
 * =========================================================================
 * File        : cpu.h
 * Project     : 0 A.D.
 * Description : CPU and memory detection.
 * =========================================================================
 */

// license: GPL; see lib/license.txt

#ifndef INCLUDED_CPU
#define INCLUDED_CPU

namespace ERR
{
	const LibError CPU_FEATURE_MISSING     = -130000;
	const LibError CPU_UNKNOWN_OPCODE      = -130001;
	const LibError CPU_UNKNOWN_VENDOR      = -130002;
	const LibError CPU_RESTRICTED_AFFINITY = -130003;
}

// (some of these functions may be implemented in external asm files)
#ifdef __cplusplus
extern "C" {
#endif


//-----------------------------------------------------------------------------
// CPU detection

/**
 * @return string identifying the CPU (usually a cleaned-up version of the
 * brand string)
 **/
extern const char* cpu_IdentifierString();

/**
 * @return a rough estimate of the CPU clock frequency.
 *
 * note: the accuracy of this value is not important. while it is used by
 * the TSC timing backend, thermal drift is an issue that requires
 * continual recalibration anyway, which makes the initial accuracy moot.
 * querying frequency via OS is also much faster than ia32's measurement loop.
 **/
extern double cpu_ClockFrequency();

/**
 * @return the number of what the OS deems "processors" or -1 on failure.
 *
 * this is used by ia32 when it cannot determine the number via APIC IDs.
 * in other situations, the cpu_NumPackages function is preferable since
 * it is more specific.
 *
 * note: this function is necessary because POSIX sysconf _SC_NPROCESSORS_CONF
 * is not suppored on MacOSX, else we would use that.
 **/
extern uint cpu_NumProcessors();

/**
 * @return number of *enabled* CPU packages / sockets.
 **/
extern uint cpu_NumPackages();

/**
 * @return number of *enabled* CPU cores per package.
 * (2 on dual-core systems)
 **/
extern uint cpu_CoresPerPackage();

/**
 * @return number of *enabled* hyperthreading units per core.
 * (2 on P4 EE)
 **/
extern uint cpu_LogicalPerCore();

/**
 * @return the size [bytes] of a MMU page.
 * (4096 on most IA-32 systems)
 **/
extern size_t cpu_PageSize();

enum CpuMemoryIndicators
{
	CPU_MEM_TOTAL,
	CPU_MEM_AVAILABLE
};

/**
 * @return the amount [bytes] of available or total physical memory.
 **/
extern size_t cpu_MemorySize(CpuMemoryIndicators mem_type);


//-----------------------------------------------------------------------------
// lock-free support routines

/**
 * atomic "compare and swap".
 *
 * @param location address of the word to compare and possibly overwrite
 * @param expected its expected value
 * @param newValue the value with which to replace it
 * @return false if the target word doesn't match the expected value,
 * otherwise true (also overwriting the contents of location)
 **/
extern bool cpu_CAS(volatile uintptr_t* location, uintptr_t expected, uintptr_t newValue);

/**
 * add a signed value to a variable without the possibility of interference
 * from other threads/CPUs.
 **/
extern void cpu_AtomicAdd(volatile intptr_t* location, intptr_t increment);

/**
 * enforce strict instruction ordering in the CPU pipeline.
 **/
extern void cpu_Serialize();

/**
 * enforce strong memory ordering.
 **/
extern void cpu_MemoryFence();


//-----------------------------------------------------------------------------
// misc

/**
 * drop-in replacement for libc memcpy(). highly optimized for Athlon and
 * Pentium III microarchitectures; significantly outperforms VC7.1 memcpy and
 * memcpy_amd. for details, see accompanying article.
 **/
extern void* cpu_memcpy(void* RESTRICT dst, const void* RESTRICT src, size_t size);

/**
 * execute the specified function once on each CPU.
 * this includes logical HT units and proceeds serially (function
 * is never re-entered) in order of increasing OS CPU ID.
 * note: implemented by switching thread affinity masks and forcing
 * a reschedule, which is apparently not possible with POSIX.
 *
 * may fail if e.g. OS is preventing us from running on some CPUs.
 **/
typedef void (*CpuCallback)(void* param);
extern LibError cpu_CallByEachCPU(CpuCallback cb, void* param);

/**
 * set the FPU control word to "desirable" values (see implementation)
 **/
extern void cpu_ConfigureFloatingPoint();

// convert float to int much faster than _ftol2, which would normally be
// used by (int) casts.
#if !USE_IA32_FLOAT_TO_INT
#define cpu_i32FromFloat(f) ((i32)f)
#define cpu_i32FromDouble(d) ((i32)d)
#define cpu_i64FromDouble(d) ((i64)d)
#else
extern i32 cpu_i32FromFloat(float f);
extern i32 cpu_i32FromDouble(double d);
extern i64 cpu_i64FromDouble(double d);
#endif


#ifdef __cplusplus
}
#endif


/**
 * specialization of cpu_CAS for pointer types. this avoids error-prone
 * casting in user code.
 **/
template<typename T>
extern bool cpu_CAS(volatile T* location, T expected, T new_value)
{
	return cpu_CAS((volatile uintptr_t*)location, (uintptr_t)expected, (uintptr_t)new_value);
}

#endif	// #ifndef INCLUDED_CPU
