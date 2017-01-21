#include "UmrObject.h"
#include "nedmalloc/nedmalloc.c.h"

UmrObject::UmrObject(void)
{
}

UmrObject::~UmrObject(void)
{
}

/**
	New operator. Allocates a new block of memory.
	@param
		sz Size of the memory block, in bytes.
*/
void* UmrObject::operator new(size_t sz)
{
	return nedalloc::nedmemalign(MEM_ALIGNMENT, sz);
}

/**
	Placement new operator. Avoid allocating new memory.
	@param
		ptr Pointer to a memory block to place the object.
*/
void* UmrObject::operator new(size_t sz, void *ptr)
{
	return ptr;
}

/**
	New operator for arrays.
*/
void* UmrObject::operator new[](size_t sz)
{
	return nedalloc::nedmemalign(MEM_ALIGNMENT, sz);
}

/**
	Deallocates a block of memory.
	@param
		ptr Pointer to a memory block previously allocated.
*/
void UmrObject::operator delete(void *ptr)
{
	if (!ptr) return;
	nedalloc::nedfree(ptr);
}

/**
	Deallocates a block of memory previously allocated.
*/
void UmrObject::operator delete(void *ptr, void *)
{
	if (!ptr) return;
	nedalloc::nedfree(ptr);
}

/**
	Deallocates a block of memory previously allocated.
*/
void UmrObject::operator delete[](void *ptr)
{
	if (!ptr) return;
	nedalloc::nedfree(ptr);
}
