#include "UmrUpdateThread.h"

/**
	Constructor.
	@param
		mutex Mutex for graphics and physics updates synchronization.
*/
UmrUpdateThread::UmrUpdateThread(UmrMutexPtr &mutex) : m_pMutex(mutex)
{
	m_pPhysics = UmrPhysics::getInstance();
}

/**
	Destructor.
*/
UmrUpdateThread::~UmrUpdateThread(void)
{
}

/**
	Sets the vehicles to simulate.
*/
void UmrUpdateThread::setVehicles(UmrVehiclePtr (&vehicles)[NCARS])
{
	for (int i = 0; i < NCARS; i++)	{
		m_vehicles[i] = vehicles[i];
	}
}

/**
	Executes the thread's code.
*/
unsigned UmrUpdateThread::run()
{
	TimerPtr timer = TimerPtr(new Ogre::Timer());
	ULONG elapsedTime, timeLastUpdate = 0;
	bool quit = 0;

	// Init physics thread before performing 
	// any operation on the world.
	m_pPhysics->initThread();

	timer->reset();
	while(!m_isDying && !m_quit) {
		elapsedTime = timer->getMilliseconds() - timeLastUpdate;
		if (elapsedTime < TIME_STEP_MS) {
			Sleep(TIME_STEP_MS - elapsedTime);
		}
		// Only human player can quit the game
		quit = m_vehicles[0]->updateControl();
		if (quit) {
			_InterlockedIncrement(&m_quit);
			break;
		}
		for (int i = 1; i < NCARS; i++) {
			m_vehicles[i]->updateControl();
		}

		timeLastUpdate = timer->getMilliseconds();
		// Step physics when it's safe, after acquiring mutex
		m_pMutex->acquire();
			m_pPhysics->stepPhysics();
		m_pMutex->release();
	}

	m_pPhysics->quitThread();
	return 0;
}

/**
	Sets the quit flag for next update.
*/
const bool UmrUpdateThread::stopped() const
{
	return m_quit != 0;
}
