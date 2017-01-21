#pragma once

#include "UmrConfig.h"
#include "UmrThread.h"

/**
	Runnable class.
	Abstract class that should be implemented by any class
	whose instances are intended to be executed by a thread.
*/
__declspec(align(MEM_ALIGNMENT))
class UmrRunnable : public UmrObject
{
public:
	UmrRunnable(void);
	virtual ~UmrRunnable(void) {}
	void start();
	void join();
	void suspend();
	void resume();

protected:
	virtual unsigned run () = 0;

	static unsigned __stdcall threadEntry (void *pArg);

	/// True when signaled to end
	bool m_isDying;

	/// Execution thread
	UmrThread m_thread;
};

typedef boost::shared_ptr<UmrRunnable> UmrRunnablePtr;
