#pragma once

#include "UmrAIControl.h"
#include "UmrConfig.h"
#include "UmrJoystickInput.h"
#include "UmrMutex.h"
#include "UmrPhysics.h"
#include "UmrScene.h"
#include "UmrTrack.h"
#include "UmrUpdateThread.h"
#include "UmrVehicle.h"
#include "UmrWindow.h"

#include <OgreWindowEventUtilities.h>

/**
	Game class.
	Main game class used to run the game. Configures the
	enviroment for execution, starts game processes and
	executes the game loop where rendering and syhnchronization
	with the physics thread takes place.
*/
__declspec(align(MEM_ALIGNMENT))
class UmrGame : public UmrObject
{
public:
	UmrGame(void);
	~UmrGame(void);

	void start ();
protected:
	bool setup ();
	void destroy ();
	void startRenderLoop ();
	void setupResources();

	/// Root class for Ogre3D system
	RootPtr m_pRoot;

	/// Window used by the application
	UmrWindowPtr m_pWindow;

	/// Graphic scene of the game
	UmrScenePtr m_pScene;

	/// Thread which execute physics simulation and input update.
	UmrUpdateThreadPtr m_updateThread;

	/// Mutex used for graphics and physics threads synchronization.
	UmrMutexPtr m_pMutex;

	/// Vehicles and track of the game
	UmrVehiclePtr	m_vehicles[NCARS];
	UmrTrackPtr		m_track;
};

typedef boost::shared_ptr<UmrGame> UmrGamePtr;
