#pragma once

// Workaround for a CoreCLR bug on x86-64 Windows: when an [UnmanagedCallersOnly]
// function body completes a catch (Exception) block, the managed->native return
// transition corrupts caller-saved XMM6-XMM15 (and sometimes MXCSR / x87 state).
//
// Measured with RtlCaptureContext at 4 probe points — corruption is isolated to
// the catch-exit + UCO return stub. All other phases (reflection dispatch,
// exception propagation, catch body, callbacks out) preserve XMMs correctly.
//
// FPUGuard is a zero-cost RAII wrapper around _fxsave/_fxrstor. Drop one at the
// start of any native scope that calls through a function pointer into managed
// code that might catch exceptions. Cost is ~100 cycles per save+restore.

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
	#include <immintrin.h>
	#define CORAL_HAS_FXSAVE 1
#else
	#define CORAL_HAS_FXSAVE 0
#endif

namespace Coral {

	struct FPUGuard
	{
#if CORAL_HAS_FXSAVE
		alignas(16) char State[512];
		FPUGuard()  { _fxsave(State); }
		~FPUGuard() { _fxrstor(State); }
#endif
		FPUGuard(const FPUGuard&) = delete;
		FPUGuard& operator=(const FPUGuard&) = delete;
	};

}
