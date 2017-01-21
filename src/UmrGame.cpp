#include "UmrGame.h"

/**
	Constructor
 */
UmrGame::UmrGame(void)
{
}

/**
	Destructor
 */
UmrGame::~UmrGame(void)
{
}

/**
	Starts the game.
 */
void UmrGame::start()
{
	if (!setup()) return;

	m_updateThread->start();
	startRenderLoop();
	m_updateThread->join();

	destroy();

	m_pScene->destroyScene();
}

/**
	Initializes game components.
 */
bool UmrGame::setup()
{
	String _resources(RESOURCES);
	m_pRoot = RootPtr(new Ogre::Root(_resources + PLUGINS, 
					 _resources + GRAPHICS, 
					 _resources + LOG));

	setupResources();

	m_pMutex = UmrMutexPtr(new UmrMutex());

	// Create the render window
	m_pWindow = UmrWindowPtr(new UmrWindow(m_pRoot));
	if (!m_pWindow->loadGraphicsConfiguration())
		return false;
	m_pWindow->initialize();

	m_pScene = UmrScenePtr(new UmrScene(m_pRoot));

	m_pWindow->setViewport(m_pScene->getCamera());

	ResourceGroupManager::
		getSingleton().initialiseAllResourceGroups();

	m_updateThread = 
		UmrUpdateThreadPtr(new UmrUpdateThread(m_pMutex));

	UmrPhysicsPtr physics = UmrPhysics::getInstance();
	hkpWorld* world = physics->getWorld();
	// Load and prepare game vehicles
	for (int i = 0; i < NCARS; i++) {
		m_vehicles[i] = UmrVehiclePtr(
			new UmrVehicle(
				i+1, String("./gamedata/vehicles/Razor/")));
		m_vehicles[i]->load(
			"razor.cfg", (i == 0) ? CONTROL_MANUAL : CONTROL_AI);
		m_vehicles[i]->buildVehicle(world);
	}

	// Create a scene where camera follows the first vehicle
	m_pScene->createScene(m_vehicles[0]);
	m_updateThread->setVehicles(m_vehicles);

	// Load and prepare track
	m_track = UmrTrackPtr(
		new UmrTrack(
			String("./gamedata/tracks/CoyotePoint/"), m_vehicles));
	m_track->load("CoyotePoint.cfg");
	m_track->addToPhysicsWorld(world);

	OverlayManager::
		getSingletonPtr()->getByName("StatsOverlay")->show();

	return true;
}

void UmrGame::destroy()
{
	for (int i = 0; i < NCARS; i++) {
		m_vehicles[i]->destroy();
	}
	m_track->destroy();
}

/**
	Starts the game loop.
 */
void UmrGame::startRenderLoop()
{
	ULONG elapsedTime, lastTime = 0;
	bool quit = false;
	const RenderWindow* window = m_pWindow->getRenderWindow();
	TimerPtr timer = TimerPtr(new Ogre::Timer());
	timer->reset();
	OverlayElement* fps = 
		OverlayManager::getSingletonPtr()->getOverlayElement("fps");

	while(!quit) {
		if (window->isClosed() || m_updateThread->stopped()) break;

		/*if (!window->isActive()) {
			m_updateThread->suspend();
			WaitMessage();
			m_updateThread->resume();
		}*/

		elapsedTime = timer->getMilliseconds() - lastTime;
		if (elapsedTime > 8) {
			// Gain exclusive access to vehicles
			m_pMutex->acquire();
				// Update camera and vehicles transform
				m_pScene->updateScene();
				for (int i = 0; i < NCARS; i++)	{
					m_vehicles[i]->updateGraphics();
				}
			m_pMutex->release();
			lastTime = timer->getMilliseconds();
		}

		fps->setCaption(
			StringConverter::toString(m_pWindow->getLastFps()));

		m_pRoot->renderOneFrame();

		// Handle windows messages
		Ogre::WindowEventUtilities::messagePump();
	}
}

/**
	Setup game resources locations.
 */
void UmrGame::setupResources()
{
	// Load resource paths from config file
	Ogre::ConfigFile cf;
	Ogre::String _resourcesPath(RESOURCES);
	cf.load(_resourcesPath + "/config/resources.cfg");

	// Go through all sections and settings in the file
	Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

	Ogre::String secName, typeName, archName;
	while (seci.hasMoreElements())
	{
		secName = seci.peekNextKey();
		Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
		Ogre::ConfigFile::SettingsMultiMap::iterator i;
		for (i = settings->begin(); i != settings->end(); ++i)
		{
			typeName = i->first;
			archName = i->second;
			Ogre::ResourceGroupManager::
				getSingleton().addResourceLocation(
					archName, typeName, secName);
		}
	}
}
