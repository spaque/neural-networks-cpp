#pragma once

#include "UmrConfig.h"

#include <Windows.h>

#define SPIN_COUNT	4000

/**
	Mutex class.
	Provides synchronization between threads to avoid the
	simultaneous use of a common resource.
*/
__declspec(align(MEM_ALIGNMENT))
class UmrMutex : public UmrObject
{
public:
	UmrMutex(void);
	explicit UmrMutex(DWORD spinCount);
	~UmrMutex(void);

	void acquire();
	bool tryAcquire();
	void release();
protected:
	/// Windows synchronization object
	CRITICAL_SECTION m_criticalSection;
};

typedef boost::shared_ptr<UmrMutex> UmrMutexPtr;
