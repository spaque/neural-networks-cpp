#include "UmrTrack.h"

/**
	Custom shape used to detect when a 
	vehicle crosses the sector end.
*/
class UmrSectorEnd : public hkpPhantomCallbackShape
{
public:
	UmrSectorEnd(UmrVehiclePtr (&vehicles)[NCARS])
	{
		for (int i = 0; i < NCARS; i++) {
			m_vehicles[i] = vehicles[i];
		}
	}

	/**
		This callback is called when the phantom shape 
		starts intersecting with another shape.
	*/
	virtual void phantomEnterEvent(const hkpCollidable* collidableA, 
									const hkpCollidable* collidableB, 
									const hkpCollisionInput& env) = 0;
	/**
		This callback is called when the phantom shape
		stops intersecting with another shape.
	*/
	virtual void phantomLeaveEvent(const hkpCollidable* collidableA, 
								const hkpCollidable* collidableB) = 0;
protected:
	UmrVehiclePtr	m_vehicles[NCARS];
};

/**
	Detects when a vehicle finishes the first sector.
*/
class UmrFirstSectorEnd : public UmrSectorEnd
{
public:
	UmrFirstSectorEnd(UmrVehiclePtr (&vehicles)[NCARS]) : 
	  UmrSectorEnd(vehicles) {}

	virtual void phantomEnterEvent(
		const hkpCollidable* collidableA, 
		const hkpCollidable* collidableB, 
		const hkpCollisionInput& env)
	{
		hkpRigidBody* owner = hkGetRigidBody(collidableB);
		// Get vehicle's id
		hkUlong id = owner->getUserData();
		m_vehicles[id-1]->firstSectorFinished();
	}

	virtual void phantomLeaveEvent(
		const hkpCollidable* collidableA, 
		const hkpCollidable* collidableB) {}
};

/**
	Detects when a vehicle finishes the second sector.
*/
class UmrSecondSectorEnd : public UmrSectorEnd
{
public:
	UmrSecondSectorEnd(UmrVehiclePtr (&vehicles)[NCARS]) : 
	  UmrSectorEnd(vehicles) {}

	virtual void phantomEnterEvent(
		const hkpCollidable* collidableA, 
		const hkpCollidable* collidableB, 
		const hkpCollisionInput& env)
	{
		hkpRigidBody* owner = hkGetRigidBody(collidableB);
		hkUlong id = owner->getUserData();
		m_vehicles[id-1]->secondSectorFinished();
	}

	virtual void phantomLeaveEvent(
		const hkpCollidable* collidableA, 
		const hkpCollidable* collidableB) {}
};

/**
	Detects when a vehicle finishes the last sector.
*/
class UmrLastSectorEnd : public UmrSectorEnd
{
public:
	UmrLastSectorEnd(UmrVehiclePtr (&vehicles)[NCARS]) : 
	  UmrSectorEnd(vehicles) {}

	virtual void phantomEnterEvent(
		const hkpCollidable* collidableA, 
		const hkpCollidable* collidableB, 
		const hkpCollisionInput& env)
	{
		hkpRigidBody* owner = hkGetRigidBody(collidableB);
		hkUlong id = owner->getUserData();
		m_vehicles[id-1]->lastSectorFinished();
	}

	virtual void phantomLeaveEvent(
		const hkpCollidable* collidableA, 
		const hkpCollidable* collidableB) {}
};

/**
	Constructor.
	@param
		path Relative path where track content is located.
	@param
		vehicles Vehicles array.
*/
UmrTrack::UmrTrack(String &path, UmrVehiclePtr (&vehicles)[NCARS]) : 
	m_sPath(path)
{
	for (int i = 0; i < NCARS; i++) {
		m_vehicles[i] = vehicles[i];
	}
}

/**
	Destructor.
*/
UmrTrack::~UmrTrack(void)
{
}

/**
	Loads track configuration from a file.
	@param
		cfgFile Configuration file name.
*/
void UmrTrack::load(const Ogre::String &cfgFile)
{
	ConfigFile cf;

	cf.load(m_sPath + cfgFile, "\t:=", false);

	m_sName = cf.getSetting("Name", "General", "Track");
	loadGraphics(cf);
	loadPhysics(cf);
}

/**
	Loads graphic data and configuration from a file.
	@param
		cf Configuration file.
*/
void UmrTrack::loadGraphics(const ConfigFile &cf)
{
	Vector3 vec;
	Entity *ent;
	Root *root = Root::getSingletonPtr();
	SceneManager *sceneMgr = root->getSceneManager("SMInstance");
	SceneNode *rootNode = sceneMgr->getRootSceneNode();

	ent = sceneMgr->createEntity(m_sName + "_track", 
		cf.getSetting("Track", "Graphics"));
	vec = StringConverter::parseVector3(
		cf.getSetting("TrackPos", "Graphics"));
	m_snTrack = 
		rootNode->createChildSceneNode(m_sName + "_track", vec);
	m_snTrack->attachObject(ent);

	ent = sceneMgr->createEntity(m_sName + "_outfield", 
		cf.getSetting("Outfield", "Graphics"));
	vec = StringConverter::parseVector3(
		cf.getSetting("OutfieldPos", "Graphics"));
	m_snOutfield = 
		rootNode->createChildSceneNode(m_sName + "_outfield", vec);
	m_snOutfield->attachObject(ent);
}

/**
	Loads physics data and configuration from a file.
	@param
		cf Configuration file.
*/
void UmrTrack::loadPhysics(const ConfigFile &cf)
{
	String path;
	hkBinaryPackfileReader* reader;
	hkIstream* infile;
	hkRootLevelContainer* container;
	hkpPhysicsData* physicsData;
	Ogre::LogManager* logMgr = Ogre::LogManager::getSingletonPtr();
	Vector3 vec;

	path = m_sPath + cf.getSetting("Track", "Physics");
	logMgr->logMessage("UmrTrack: Loading " + path);

	// Load the data
	infile = new hkIstream(path.c_str());
	reader = new hkBinaryPackfileReader();
	reader->loadEntireFile(infile->getStreamReader());
	m_pTrackLoadedData = reader->getPackfileData();
	infile->removeReference();
	m_pTrackLoadedData->addReference();
	reader->removeReference();

	// Get the top level object in the file, 
	// which we know is a hkRootLevelContainer.
	container = 
		m_pTrackLoadedData->getContents<hkRootLevelContainer>();
	// Get the physics data
	physicsData = static_cast<hkpPhysicsData*>(
		container->findObjectByType(hkpPhysicsDataClass.getName()));
	m_pTrackRigidBody = physicsData->findRigidBodyByName("road");

	path = m_sPath + cf.getSetting("Outfield", "Physics");
	logMgr->logMessage("UmrTrack: Loading " + path);

	// Load the data
	infile = new hkIstream(path.c_str());
	reader = new hkBinaryPackfileReader();
	reader->loadEntireFile(infile->getStreamReader());
	m_pOutfieldLoadedData = reader->getPackfileData();
	infile->removeReference();
	m_pOutfieldLoadedData->addReference();
	reader->removeReference();

	// Get the top level object in the file, 
	// which we know is a hkRootLevelContainer.
	container = 
		m_pOutfieldLoadedData->getContents<hkRootLevelContainer>();
	// Get the physics data
	physicsData = static_cast<hkpPhysicsData*>(
		container->findObjectByType(hkpPhysicsDataClass.getName()));
	m_pOutfieldRigidBody = physicsData->findRigidBodyByName("gravel");

	// Create last sector end
	{
		hkpRigidBodyCinfo boxInfo;
		vec = StringConverter::parseVector3(
				cf.getSetting("LastSectorSize", "Physics"));
		hkVector4 boxSize(vec[0], vec[1], vec[2]);
		hkpShape* boxShape = new hkpBoxShape(boxSize, 0);
		boxInfo.m_motionType = hkpMotion::MOTION_FIXED;
		boxInfo.m_collisionFilterInfo = 
			hkpGroupFilter::calcFilterInfo(SECTOR_FILTER);
		vec = StringConverter::parseVector3(
				cf.getSetting("LastSectorPosition", "Physics"));
		boxInfo.m_position.set(vec[0], vec[1], vec[2]);

		UmrLastSectorEnd* myPhantomShape = 
			new UmrLastSectorEnd(m_vehicles);
		hkpBvShape* bvShape = 
			new hkpBvShape(boxShape, myPhantomShape);
		boxShape->removeReference();
		myPhantomShape->removeReference();

		boxInfo.m_shape = bvShape;

		m_pFinish = new hkpRigidBody(boxInfo);
		bvShape->removeReference();
	}

	// Create first sector end
	{
		hkpRigidBodyCinfo boxInfo;
		vec = StringConverter::parseVector3(
				cf.getSetting("Sector1Size", "Physics"));
		hkVector4 boxSize(vec[0], vec[1], vec[2]);
		hkpShape* boxShape = new hkpBoxShape(boxSize, 0);
		boxInfo.m_motionType = hkpMotion::MOTION_FIXED;
		boxInfo.m_collisionFilterInfo = 
			hkpGroupFilter::calcFilterInfo(SECTOR_FILTER);
		vec = StringConverter::parseVector3(
				cf.getSetting("Sector1Position", "Physics"));
		boxInfo.m_position.set(vec[0], vec[1], vec[2]);

		UmrFirstSectorEnd* myPhantomShape = 
			new UmrFirstSectorEnd(m_vehicles);
		hkpBvShape* bvShape = 
			new hkpBvShape(boxShape, myPhantomShape);
		boxShape->removeReference();
		myPhantomShape->removeReference();

		boxInfo.m_shape = bvShape;

		m_pSector1 = new hkpRigidBody(boxInfo);
		bvShape->removeReference();
	}

	// Create second sector end
	{
		hkpRigidBodyCinfo boxInfo;
		vec = StringConverter::parseVector3(
				cf.getSetting("Sector2Size", "Physics"));
		hkVector4 boxSize(vec[0], vec[1], vec[2]);
		hkpShape* boxShape = new hkpBoxShape(boxSize, 0);
		boxInfo.m_motionType = hkpMotion::MOTION_FIXED;
		boxInfo.m_collisionFilterInfo = 
			hkpGroupFilter::calcFilterInfo(SECTOR_FILTER);
		vec = StringConverter::parseVector3(
				cf.getSetting("Sector2Position", "Physics"));
		boxInfo.m_position.set(vec[0], vec[1], vec[2]);

		UmrSecondSectorEnd* myPhantomShape = 
			new UmrSecondSectorEnd(m_vehicles);
		hkpBvShape* bvShape = 
			new hkpBvShape(boxShape, myPhantomShape);
		boxShape->removeReference();
		myPhantomShape->removeReference();

		boxInfo.m_shape = bvShape;

		m_pSector2 = new hkpRigidBody(boxInfo);
		bvShape->removeReference();
	}

	// Set vehicles' initial position
	{
		vec = StringConverter::parseVector3(
				cf.getSetting("StartPosition", "Physics"));
		bool reverse = StringConverter::parseBool(
						cf.getSetting("Reverse", "Physics"));
		for (int i = 0; i < NCARS; i++)
			m_vehicles[i]->setStartPosition(vec, reverse);
	}
}

/**
	Removes graphic and physics elements of the track.
*/
void UmrTrack::destroy()
{
	hkpWorld* world = m_pTrackRigidBody->getWorld();

 	world->removeEntity(m_pTrackRigidBody);
 	world->removeEntity(m_pOutfieldRigidBody);
	world->removeEntity(m_pFinish);
 	world->removeEntity(m_pSector1);
 	world->removeEntity(m_pSector2);
	m_pTrackLoadedData->removeReference();
	m_pOutfieldLoadedData->removeReference();
	m_pFinish->removeReference();
 	m_pSector1->removeReference();
 	m_pSector2->removeReference();

	Root *root = Root::getSingletonPtr();
	SceneManager *sceneMgr = root->getSceneManager("SMInstance");
	SceneNode *rootNode = sceneMgr->getRootSceneNode();

	m_snTrack->detachObject(m_sName + "_track");
	sceneMgr->destroyEntity(m_sName + "_track");
	rootNode->removeAndDestroyChild(m_sName + "_track");

	m_snOutfield->detachObject(m_sName + "_outfield");
	sceneMgr->destroyEntity(m_sName + "_outfield");
	rootNode->removeAndDestroyChild(m_sName + "_outfield");
}

/**
	Adds track to the physics simulation.
	@param
		world Pointer to physics world.
*/
void UmrTrack::addToPhysicsWorld(hkpWorld *world)
{
  	world->addEntity(m_pTrackRigidBody);
  	world->addEntity(m_pOutfieldRigidBody);
	world->addEntity(m_pFinish);
 	world->addEntity(m_pSector1);
 	world->addEntity(m_pSector2);
}
