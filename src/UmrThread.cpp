#include "UmrThread.h"

/**
	Constructor.
	@param
		pFun Pointer to the function to be executed by the thread.
		pArg Pointer to the arguments of the function.
*/
UmrThread::UmrThread(
	unsigned (__stdcall *pFun) (void* arg), void* pArg)
{
	m_handle = (HANDLE)_beginthreadex(
					NULL, 0, pFun, pArg, CREATE_SUSPENDED, &m_tid);
}

/**
	Destructor.
*/
UmrThread::~UmrThread(void)
{
	CloseHandle(m_handle);
}

/**
	Decrements a thread's suspend count. When the suspend count is 
	decremented to zero, the execution of the thread is resumed.
*/
void UmrThread::resume()
{
	ResumeThread(m_handle);
}

/**
	Suspends the thread.
*/
void UmrThread::suspend()
{
	SuspendThread(m_handle);
}

/**
	Waits for this thread to die.
*/
void UmrThread::waitForDeath()
{
	WaitForSingleObject(m_handle, 1000);
}
