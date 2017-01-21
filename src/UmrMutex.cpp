#include "UmrMutex.h"

/**
	Constructor.
*/
UmrMutex::UmrMutex(void)
{
	InitializeCriticalSectionAndSpinCount(
		&m_criticalSection, SPIN_COUNT);
}

/**
	Constructor.
	@param
		spinCount Spin count for the critical section.
*/
UmrMutex::UmrMutex(DWORD spinCount)
{
	InitializeCriticalSectionAndSpinCount(
		&m_criticalSection, spinCount);
}

/**
	Destructor.
*/
UmrMutex::~UmrMutex(void)
{
	DeleteCriticalSection(&m_criticalSection);
}

/**
	Waits for ownership of the mutex object. The method 
	returns when the calling thread is granted ownership.
*/
void UmrMutex::acquire()
{
	EnterCriticalSection(&m_criticalSection);
}

/**
	Attempts to acquire the mutex without blocking. 
	If the call is successful, the calling thread 
	takes ownership of the mutex.
*/
bool UmrMutex::tryAcquire()
{
	if (TryEnterCriticalSection(&m_criticalSection)) {
		return true;
	} else {
		return false;
	}
}

/**
	Releases the ownership of the mutex.
*/
void UmrMutex::release()
{
	LeaveCriticalSection(&m_criticalSection);
}
