#pragma once

#include "UmrConfig.h"
#include "UmrMutex.h"
#include "UmrPhysics.h"
#include "UmrRunnable.h"
#include "UmrVehicle.h"

#include <intrin.h>

/**
	Update thread class.
	Thread implementation used to update vehicles' input
	and perform physics simulation steps.
*/
__declspec(align(MEM_ALIGNMENT))
class UmrUpdateThread : public UmrRunnable
{
public:
	UmrUpdateThread(UmrMutexPtr &mutex);
	~UmrUpdateThread(void);

	const bool stopped() const;
	void setVehicles(UmrVehiclePtr (&vehicles)[NCARS]);
protected:
	unsigned run ();
	
	/// True when the player wants to quit the game
	LONG m_quit;

	/// Reference to physics system
	UmrPhysicsPtr m_pPhysics;

	/// Mutex used to gain exclusive access to vehicles
	UmrMutexPtr	m_pMutex;

	/// Vehicles of the game
	UmrVehiclePtr m_vehicles[NCARS];
};

typedef boost::shared_ptr<UmrUpdateThread> UmrUpdateThreadPtr;
