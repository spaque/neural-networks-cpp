#include "UmrPhysics.h"

UmrPhysicsPtr UmrPhysics::s_instance;

/**
	Constructor
*/
UmrPhysics::UmrPhysics(void)
{
	Ogre::LogManager* logMgr = Ogre::LogManager::getSingletonPtr();

	logMgr->logMessage("UmrPhysics: Initializing havok engine");

	m_memoryMgr = new hkPoolMemory();
	//m_memoryMgr = new hkStlDebugMemory();
	m_threadMemory = new hkThreadMemory(m_memoryMgr);

	hkBaseSystem::init(m_memoryMgr, m_threadMemory, errorReport);
	m_memoryMgr->removeReference();

	// Initialize the stack area to 100k 
	{
		int stackSize = 0x100000;
		m_stackBuffer = 
			hkAllocate<char>(stackSize, HK_MEMORY_CLASS_BASE);
		hkThreadMemory::getInstance().setStackArea(
			m_stackBuffer, stackSize);
	}

	// Create the world
	{
		hkpWorldCinfo info;
		info.m_simulationType = 
			hkpWorldCinfo::SIMULATION_TYPE_DISCRETE;
		info.m_gravity.set(0, -9.8f, 0);
		info.setBroadPhaseWorldSize(1000.0f);
		info.setupSolverInfo(
			hkpWorldCinfo::SOLVER_TYPE_4ITERS_MEDIUM);
		m_world = new hkpWorld(info);
	}

	// Pre-allocate some larger memory blocks. 
	// These are used by the physics system 
	// when in multithreaded mode.
	// The amount and size of these depends on your physics usage. 
	// Larger simulation islands will use larger memory blocks.
	{
		hkMemory::getInstance().preAllocateRuntimeBlock(
			512000, HK_MEMORY_CLASS_BASE);
		hkMemory::getInstance().preAllocateRuntimeBlock(
			256000, HK_MEMORY_CLASS_BASE);
		hkMemory::getInstance().preAllocateRuntimeBlock(
			128000, HK_MEMORY_CLASS_BASE);
		hkMemory::getInstance().preAllocateRuntimeBlock(
			64000, HK_MEMORY_CLASS_BASE);
		hkMemory::getInstance().preAllocateRuntimeBlock(
			32000, HK_MEMORY_CLASS_BASE);
		hkMemory::getInstance().preAllocateRuntimeBlock(
			16000, HK_MEMORY_CLASS_BASE);
		hkMemory::getInstance().preAllocateRuntimeBlock(
			16000, HK_MEMORY_CLASS_BASE);
	}

	// Register all agents
	hkpAgentRegisterUtil::registerAllAgents(
		m_world->getCollisionDispatcher());

	{
		hkpGroupFilter* filter = new hkpGroupFilter();

		// disable all collisions by default
		filter->disableCollisionsUsingBitfield(
			0xfffffffe, 0xfffffffe);

		// Enable collision filter for vehicles
		for (int i = 0; i < NCARS; i++) {
			filter->enableCollisionsUsingBitfield(
				1 << (CAR_FILTER + i), 
					(1 << TRACK_FILTER) | 
					(1 << OUTFIELD_FILTER) |
					(1 << SECTOR_FILTER));
		}

		// Enable collision between vehicles
		filter->enableCollisionsUsingBitfield(1<<CAR_FILTER, 
				(1 << (CAR_FILTER+1))|
				(1 << (CAR_FILTER+2)));
		filter->enableCollisionsUsingBitfield(1<<CAR_FILTER+1, 
			(1 << (CAR_FILTER+2)));

		// Enable collision filter for ray casting
		filter->enableCollisionsUsingBitfield(1<<RAY_FILTER, 
			(1 << OUTFIELD_FILTER) |
			(1 << CAR_FILTER));

		m_world->setCollisionFilter(filter);
		filter->removeReference();
	}

#ifdef DEBUG_PHYSICS
	// Instantiate a context so that the VDB will 
	// know the physics worlds it can visualize
	m_physicsCtx = new hkpPhysicsContext();
	m_physicsCtx->addWorld(m_world);

	// Register all processes to expose to the VDB
	hkpPhysicsContext::registerAllPhysicsProcesses();

	// Create the Visual Debugger server
	hkArray<hkProcessContext*> contexts;
	contexts.pushBack(m_physicsCtx);
	m_vdb = new hkVisualDebugger(contexts);
	m_vdb->serve();
#endif
}

/**
	Destructor.
*/
UmrPhysics::~UmrPhysics(void)
{
}

/**
	Gets the instance of this class.
*/
UmrPhysicsPtr& UmrPhysics::getInstance()
{
	if (!s_instance)
		s_instance = UmrPhysicsPtr(new UmrPhysics());
	return s_instance;
}

/**
	Gets the physics world.
*/
hkpWorld* UmrPhysics::getWorld()
{
	return m_world;
}

/**
	Initializes thread for physics simulation.
*/
void UmrPhysics::initThread()
{
	// Need to call this for each active thread which uses Havok
	hkBaseSystem::initThread(m_threadMemory);
}

/**
	Clears resources used by a thread.
*/
void UmrPhysics::quitThread()
{
	hkBaseSystem::clearThreadResources();
}

/**
	Shut down physic system.
*/
void UmrPhysics::quit()
{
	Ogre::LogManager* logMgr = Ogre::LogManager::getSingletonPtr();

	logMgr->logMessage("UmrPhysics: Quitting havok engine");
	
	// Deallocate stack area
	m_threadMemory->setStackArea(0, 0);
	hkDeallocate(m_stackBuffer);

	//Deallocate runtime blocks
	hkMemory::getInstance().freeRuntimeBlocks();
	m_memoryMgr->removeReference();
	m_world->removeReference();
#ifdef DEBUG_PHYSICS
	m_physicsCtx->removeReference();
	m_vdb->removeReference();
#endif
	hkBaseSystem::quit();
}

/**
	Error report function for physics system.
*/
static void errorReport(const char *str, void *errorOutputObject)
{
	Ogre::LogManager *logMgr;

	logMgr = Ogre::LogManager::getSingletonPtr();
	if (logMgr != NULL) {
		// Redirect Havok messages to Ogre3D log file.
		logMgr->logMessage(Ogre::String(str));
	}
}

/**
	Performs a step in the physics system.
*/
void UmrPhysics::stepPhysics()
{
	m_world->stepDeltaTime(TIME_STEP_S);
#ifdef DEBUG_PHYSICS
	// Step the VDB display
	m_vdb->step(TIME_STEP_S);
#endif
}
