#pragma once

#include "UmrConfig.h"

#include <OgreLogManager.h>

#include <Common/Base/hkBase.h>
#include <Common/Base/System/hkBaseSystem.h>
#include <Common/Base/Memory/Memory/Pool/hkPoolMemory.h>
//#include <Common/Base/Memory/Memory/Debug/hkStlDebugMemory.h>
#include <Physics/Collide/Dispatch/hkpAgentRegisterUtil.h>
#include <Physics/Collide/Filter/Group/hkpGroupFilter.h>
#include <Physics/Dynamics/World/hkpWorld.h>
#include <Physics/Dynamics/World/hkpWorldCinfo.h>

#include <Common/Visualize/hkVisualDebugger.h>
#include <Physics/Utilities/VisualDebugger/hkpPhysicsContext.h>

#include <OgreLogManager.h>

#define DEBUG_PHYSICS 1

class UmrPhysics;
typedef boost::shared_ptr<UmrPhysics> UmrPhysicsPtr;

/**
	Physics class.
	Manages the initialization and shut down of the
	Havok physics system and performs simulation steps.
*/
__declspec(align(MEM_ALIGNMENT))
class UmrPhysics : public UmrObject
{
public:
	~UmrPhysics(void);

	void initThread();
	void quitThread();
	void quit();
	void stepPhysics();
	static UmrPhysicsPtr& getInstance();
	hkpWorld* getWorld();
protected:
	UmrPhysics(void);

	static UmrPhysicsPtr s_instance;
	hkpWorld*			m_world;
	hkPoolMemory*		m_memoryMgr;
	//hkStlDebugMemory *m_memoryMgr;
	hkThreadMemory*		m_threadMemory;
	char*				m_stackBuffer;
	hkpPhysicsContext*	m_physicsCtx;
	hkVisualDebugger*	m_vdb;
};

static void errorReport(const char *str, void *errorOutputObject);
