#pragma once

#include "UmrConfig.h"

#define MAXTHREADSINPOOL 4

#ifdef new
#undef new
#endif
#ifdef delete
#undef delete
#endif

/**
	Base class for all game classes.
	Overrides memory allocation and dealocation 
	operators to use the NedMalloc allocator.
*/
__declspec(align(MEM_ALIGNMENT))
class UmrObject
{
public:
	explicit UmrObject(void);
	~UmrObject(void);

	void* operator new(size_t sz);
	void* operator new(size_t sz, void* ptr);
	void* operator new[](size_t sz);
	void operator delete(void* ptr);
	void operator delete(void* ptr, void*);
	void operator delete[](void* ptr);
};
