#include "UmrRunnable.h"

/**
	Constructor.
*/
UmrRunnable::UmrRunnable(void) : 
	m_isDying(false), 
// 'this' : used in base member initializer list
#pragma warning(disable: 4355)
	m_thread(threadEntry, this)
{
}

/**
	Destructor.
*/
void UmrRunnable::join()
{
	m_isDying = true;
	m_thread.waitForDeath();
}

/**
	Executes the thread function.
	@param
		pArg Pointer to the UmrRunnable class to be executed.
*/
unsigned __stdcall UmrRunnable::threadEntry(void *pArg)
{
	UmrRunnable *pRunnable = (UmrRunnable *)pArg;
	return pRunnable->run();
}

/**
	Starts executing the thread.
*/
void UmrRunnable::start()
{
	m_thread.resume();
}

/**
	Suspends the execution of the thread.
*/
void UmrRunnable::suspend()
{
	m_thread.suspend();
}

/**
	Resumes the execution of the thread.
*/
void UmrRunnable::resume()
{
	m_thread.resume();
}
