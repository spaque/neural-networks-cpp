#pragma once

#include "UmrConfig.h"

#include <windows.h>
#include <process.h>

/**
	Thread class.
	Thin encapsulation of the Windows thread API.
	The thread will start in suspended state.
*/
__declspec(align(MEM_ALIGNMENT))
class UmrThread : public UmrObject
{
public:
	UmrThread(unsigned (__stdcall * pFun) (void* arg), void* pArg);
	~UmrThread(void);
	void resume();
	void suspend();
	void waitForDeath();
protected:
	/// Windows thread handle
	HANDLE m_handle;	// 4 bytes

	/// Thread id
	unsigned m_tid;		// 4 bytes
};
