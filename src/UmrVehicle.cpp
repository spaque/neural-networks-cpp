
// Register Havok physics classes for serialization
#define INCLUDE_HAVOK_PHYSICS_CLASSES
#define HK_CLASSES_FILE <Common/Serialize/Classlist/hkClasses.h>
#include <Common/Serialize/Util/hkBuiltinTypeRegistry.cxx>

// Havok files version compatibility for serialization
#define HK_COMPAT_FILE <Common/Compat/hkCompatVersions.h>
#include <Common/Compat/hkCompat_None.cxx>

// Intrinsics functions
#include <xmmintrin.h>

const hkQuadReal hkQuadRealMinusHalf = _mm_set1_ps(-0.5f);

#include "UmrVehicle.h"
#include "UmrAIControlFast.h"
#include "UmrAIControlSlow.h"

/**
	Constructor.
	@param
		id Vehicle id.
	@param
		path Relative path where the vehicle content is located.
*/
UmrVehicle::UmrVehicle(short id, String &path) : 
	m_id(id), m_sPath(path), m_pChassis(HK_NULL), 
	m_pVehicle(HK_NULL), m_sector(2), m_speed(0.0f),
	m_elapsedTime(0), m_lastTime(0)
{
	String filename = path + "TrainingData.ini";

	m_pTimer = TimerPtr(new Ogre::Timer());
	m_pTimer->reset();

	for (int i = 0; i < NRAYS; i++)
		m_rays[i] = 1.0f;
	// Get training data only for human player
	if (id == 1) {
		m_trainingData.open(filename.c_str(), ios::out | ios::trunc);
		m_trainingData.setf(ios::fixed, ios::floatfield);
		m_trainingData.setf(ios::showpoint);
		m_trainingData.precision(4);
	}
}

/**
	Destructor.
*/
UmrVehicle::~UmrVehicle(void)
{
	if (m_id == 1) {
		m_trainingData.flush();
		m_trainingData.close();
	}
}

/**
	Gets the chassis rigidbody of the vehicle.
*/
const hkpRigidBody* UmrVehicle::getChassis() const
{
	return m_pChassis;
}

/**
	Loads the vehicle configuration from a file.
	@param
		cfgFile Configuration file name.
	@param
		controlType Controller to be used by the vehicle.
*/
void UmrVehicle::load(const String& cfgFile, int controlType)
{
	ConfigFile cf;

	cf.load(m_sPath + cfgFile, "\t:=", false);

	m_sName = cf.getSetting("Name", "General", "Vehicle");

	loadGraphics(cf);
	loadPhysics(cf);
	loadController(controlType);
}

/**
	Loads a controller for the vehicle.
*/
void UmrVehicle::loadController(int controlType)
{
	if (controlType == CONTROL_AI) {
		if (m_id == 2)
			m_pController = 
				UmrControllerPtr(new UmrAIControlFast(this));
		else
			m_pController = 
				UmrControllerPtr(new UmrAIControlSlow(this));
	} else {
		ConfigFile cf;

		cf.load("./gamedata/config/input.cfg");
		String device = 
			cf.getSetting("Device", "General", "Keyboard");

		if (device.compare("Keyboard") == 0) {
			m_pController = 
				UmrControllerPtr(new UmrKeyboardInput(cf));
		} else {
			m_pController = 
				UmrControllerPtr(new UmrJoystickInput(cf));
		}
	}
}

/**
	Loads graphic data and configuration from a file.
	@param
		cf Configuration file.
*/
void UmrVehicle::loadGraphics(const ConfigFile &cf)
{
	Vector3 vec;
	Entity *ent;
	SubEntity *sub;
	Root *root = Root::getSingletonPtr();
	SceneManager *sceneMgr = root->getSceneManager("SMInstance");
	SceneNode *rootNode = sceneMgr->getRootSceneNode();
	String id = StringConverter::toString(m_id);

	ent = sceneMgr->createEntity(m_sName + "_body" + id, 
		cf.getSetting("Body", "Graphics"));
	if (m_id > 1) {
		sub = ent->getSubEntity(0);
		sub->setMaterialName("BodyMat" + id);
	}
	vec = StringConverter::parseVector3(
		cf.getSetting("BodyPos", "Graphics"));
	m_snBody = rootNode->createChildSceneNode(
		m_sName + "_body" + id, vec);
	m_snBody->attachObject(ent);

	ent = sceneMgr->createEntity(m_sName + "_tire_lf" + id, 
		cf.getSetting("TireLeftFront", "Graphics"));
	vec = StringConverter::parseVector3(
		cf.getSetting("TireLeftFrontPos", "Graphics"));
	m_snTire_lf = m_snBody->createChildSceneNode(
		m_sName + "_tire_lf" + id, vec);
	m_snTire_lf->attachObject(ent);

	ent = sceneMgr->createEntity(m_sName + "_tire_lr" + id, 
		cf.getSetting("TireLeftRear", "Graphics"));
	vec = StringConverter::parseVector3(
		cf.getSetting("TireLeftRearPos", "Graphics"));
	m_snTire_lr = m_snBody->createChildSceneNode(
		m_sName + "_tire_lr" + id, vec);
	m_snTire_lr->attachObject(ent);

	ent = sceneMgr->createEntity(m_sName + "_tire_rf" + id, 
		cf.getSetting("TireRightFront", "Graphics"));
	vec = StringConverter::parseVector3(
		cf.getSetting("TireRightFrontPos", "Graphics"));
	m_snTire_rf = m_snBody->createChildSceneNode(
		m_sName + "_tire_rf" + id, vec);
	m_snTire_rf->attachObject(ent);

	ent = sceneMgr->createEntity(m_sName + "_tire_rr" + id, 
		cf.getSetting("TireRightRear", "Graphics"));
	vec = StringConverter::parseVector3(
		cf.getSetting("TireRightRearPos", "Graphics"));
	m_snTire_rr = m_snBody->createChildSceneNode(
		m_sName + "_tire_rr" + id, vec);
	m_snTire_rr->attachObject(ent);
}

/**
	Sets up physics data for the vehicle.
*/
void UmrVehicle::loadPhysics(const ConfigFile &cf)
{
	/*
	String path;
	hkBinaryPackfileReader reader;
	hkpPhysicsData* physicsData;
	Ogre::LogManager* logMgr = Ogre::LogManager::getSingletonPtr();

	path = m_sPath + cf.getSetting("Packfile", "Physics");
	logMgr->logMessage("UmrVehicle: Loading " + path);

	// Load the data
	hkIstream infile(path.c_str());
	reader.loadEntireFile(infile.getStreamReader());
	m_pLoadedData = reader.getPackfileData();
	m_pLoadedData->addReference();

	// Get the top level object in the file, 
	// which we know is a hkRootLevelContainer
	hkRootLevelContainer* container = 
		m_pLoadedData->getContents<hkRootLevelContainer>();
	// Get the physics data
	physicsData = static_cast<hkpPhysicsData*>(
		container->findObjectByType(hkpPhysicsDataClass.getName()));
	m_pChassis = physicsData->findRigidBodyByName("RZ_COLL_BODY");
	*/
	// Create rigid body manually
	hkReal xSize = 1.75f;
	hkReal ySize = 0.3f;
	hkReal zSize = 0.85f;

	hkReal xBumper = 1.9f;
	hkReal yBumper = 0.2f;
	hkReal zBumper = 0.80f;

	hkReal xRoofFront = 0.4f;
	hkReal xRoofBack = -1.2f;
	hkReal yRoof = ySize + 0.45f;
	hkReal zRoof = 0.6f;

	hkReal xDoorFront = xRoofFront;
	hkReal xDoorBack = xRoofBack;
	hkReal yDoor = ySize;
	hkReal zDoor = zSize + 0.1f;

	int numVertices = 22;

	// 16 = 4 (size of "each float group", 3 for x,y,z, 1 for padding)
	//	  * 4 (size of float).
	int stride = sizeof(float) * 4;

	float vertices[] = { 
		xSize, ySize, zSize, 0.0f,		// v0
		xSize, ySize, -zSize, 0.0f,		// v1
		xSize, -ySize, zSize, 0.0f,		// v2
		xSize, -ySize, -zSize, 0.0f,	// v3
		-xSize, -ySize, zSize, 0.0f,	// v4
		-xSize, -ySize, -zSize, 0.0f,	// v5

		xBumper, yBumper, zBumper, 0.0f,	// v6
		xBumper, yBumper, -zBumper, 0.0f,	// v7
		-xBumper, yBumper, zBumper, 0.0f,	// v8
		-xBumper, yBumper, -zBumper, 0.0f,	// v9

		xRoofFront, yRoof, zRoof, 0.0f,		// v10
		xRoofFront, yRoof, -zRoof, 0.0f,	// v11
		xRoofBack, yRoof, zRoof, 0.0f,		// v12
		xRoofBack, yRoof, -zRoof, 0.0f,		// v13

		xDoorFront, yDoor, zDoor, 0.0f,		// v14
		xDoorFront, yDoor, -zDoor, 0.0f,	// v15
		xDoorFront, -yDoor, zDoor, 0.0f,	// v16
		xDoorFront, -yDoor, -zDoor, 0.0f,	// v17

		xDoorBack, yDoor, zDoor, 0.0f,		// v18
		xDoorBack, yDoor, -zDoor, 0.0f,		// v19
		xDoorBack, -yDoor, zDoor, 0.0f,		// v20
		xDoorBack, -yDoor, -zDoor, 0.0f,	// v21
	};

	//
	// SHAPE CONSTRUCTION.
	//
	hkpConvexVerticesShape* chassisShape;
	hkArray<hkVector4> planeEquations;
	hkGeometry geom;
	{
		hkStridedVertices stridedVerts;
		{
			stridedVerts.m_numVertices = numVertices;
			stridedVerts.m_striding = stride;
			stridedVerts.m_vertices = vertices;
		}

		hkGeometryUtility::createConvexGeometry(
			stridedVerts, geom, planeEquations);

		{
			stridedVerts.m_numVertices = geom.m_vertices.getSize();
			stridedVerts.m_striding = sizeof(hkVector4);
			stridedVerts.m_vertices = &(geom.m_vertices[0](0));
		}

		chassisShape = 
			new hkpConvexVerticesShape(stridedVerts, planeEquations);
	}

	chassisShape->setRadius(0.1f);

	{
		hkpRigidBodyCinfo chassisInfo;

		chassisInfo.m_mass = 1300.0f;
		chassisInfo.m_shape = chassisShape;
		chassisInfo.m_restitution = 0.4f;
		chassisInfo.m_friction = 0.5f;

		// The chassis MUST have m_motionType 
		// hkpMotion::MOTION_BOX_INERTIA to correctly simulate 
		// vehicle roll, pitch and yaw.
		chassisInfo.m_motionType = hkpMotion::MOTION_BOX_INERTIA;
		chassisInfo.m_position.set(0, 0, 0);
		chassisInfo.m_collisionFilterInfo = 
			hkpGroupFilter::calcFilterInfo(CAR_FILTER+m_id-1);
		hkpInertiaTensorComputer::setShapeVolumeMassProperties(
			chassisInfo.m_shape,
			chassisInfo.m_mass,
			chassisInfo);

		m_pChassis = new hkpRigidBody(chassisInfo);
		m_pChassis->setUserData(m_id);

		// No longer need reference to shape 
		// as the hkpRigidBody holds one.
		chassisShape->removeReference();
	}
	m_lastValidTrans = m_pChassis->getTransform();
	m_lastValidDir = hkVector4(1, 0, 0);
}

/**
	Removes graphic and physics elements of the vehicle.
*/
void UmrVehicle::destroy()
{
	String id = StringConverter::toString(m_id);
	hkpWorld* world = m_pChassis->getWorld();

	world->removePhantom(
		(hkpPhantom*)(static_cast< hkpVehicleRaycastWheelCollide*>(
			m_pVehicle->m_wheelCollide)->m_phantom));

	world->removeAction(m_pVehicle);

	m_pVehicle->removeReference();

	world->removeEntity(m_pChassis);
	m_pChassis->removeReference();

	//m_pLoadedData->removeReference();

	Root *root = Root::getSingletonPtr();
	SceneManager *sceneMgr = root->getSceneManager("SMInstance");
	SceneNode *rootNode = sceneMgr->getRootSceneNode();

	m_snTire_rr->detachObject(m_sName + "_tire_rr" + id);
	sceneMgr->destroyEntity(m_sName + "_tire_rr" + id);

	m_snTire_rf->detachObject(m_sName + "_tire_rf" + id);
	sceneMgr->destroyEntity(m_sName + "_tire_rf" + id);

	m_snTire_lr->detachObject(m_sName + "_tire_lr" + id);
	sceneMgr->destroyEntity(m_sName + "_tire_lr" + id);

	m_snTire_lf->detachObject(m_sName + "_tire_lf" + id);
	sceneMgr->destroyEntity(m_sName + "_tire_lf" + id);

	m_snBody->detachObject(m_sName + "_body" + id);
	sceneMgr->destroyEntity(m_sName + "_body" + id);

	m_snBody->removeAndDestroyChild(m_sName + "_tire_rr" + id);
	m_snBody->removeAndDestroyChild(m_sName + "_tire_rf" + id);
	m_snBody->removeAndDestroyChild(m_sName + "_tire_lr" + id);
	m_snBody->removeAndDestroyChild(m_sName + "_tire_lf" + id);

	rootNode->removeAndDestroyChild(m_sName + "_body" + id);
}

/**
	Creates vehicles components and adds it to the physics world.
	@param
		world Pointer to the physics world.
*/
void UmrVehicle::buildVehicle(hkpWorld* world)
{
	world->addEntity(m_pChassis);

	m_pVehicle = new hkpVehicleInstance(m_pChassis);

	m_pVehicle->m_data	= 
		new hkpVehicleData;
	m_pVehicle->m_driverInput = 
		new hkpVehicleDefaultAnalogDriverInput;
	m_pVehicle->m_steering = 
		new hkpVehicleDefaultSteering;
	m_pVehicle->m_engine = 
		new hkpVehicleDefaultEngine;
	m_pVehicle->m_transmission = 
		new hkpVehicleDefaultTransmission;
	m_pVehicle->m_brake = 
		new hkpVehicleDefaultBrake;
	m_pVehicle->m_suspension = 
		new hkpVehicleDefaultSuspension;
	m_pVehicle->m_aerodynamics = 
		new hkpVehicleDefaultAerodynamics;
	m_pVehicle->m_velocityDamper = 
		new hkpVehicleDefaultVelocityDamper;
	m_pVehicle->m_wheelCollide = 
		new hkpVehicleRaycastWheelCollide;
	m_pVehicle->m_deviceStatus = 
		new hkpVehicleDriverInputAnalogStatus;

	setupVehicleData(world, *(m_pVehicle->m_data));

	// Initialise the tyremarks controller with 128 tyremark points
	m_pVehicle->m_tyreMarks	= 
		new hkpTyremarksInfo(*(m_pVehicle->m_data), 128);

	// Setup components configuration
	setupComponent(
		*(m_pVehicle->m_data), 
		*static_cast<hkpVehicleDefaultAnalogDriverInput* >(
			m_pVehicle->m_driverInput));
	setupComponent(
		*(m_pVehicle->m_data), 
		*static_cast<hkpVehicleDefaultSteering*>(
			m_pVehicle->m_steering));
	setupComponent(
		*(m_pVehicle->m_data), 
		*static_cast<hkpVehicleDefaultEngine*>(
			m_pVehicle->m_engine));
	setupComponent( 
		*(m_pVehicle->m_data), 
		*static_cast<hkpVehicleDefaultTransmission*>(
			m_pVehicle->m_transmission));
	setupComponent( 
		*(m_pVehicle->m_data), 
		*static_cast<hkpVehicleDefaultBrake*>(
			m_pVehicle->m_brake));
	setupComponent( 
		*(m_pVehicle->m_data), 
		*static_cast<hkpVehicleDefaultSuspension*>(
			m_pVehicle->m_suspension));
	setupComponent( 
		*(m_pVehicle->m_data), 
		*static_cast<hkpVehicleDefaultAerodynamics*>(
			m_pVehicle->m_aerodynamics));
	setupComponent( 
		*(m_pVehicle->m_data), 
		*static_cast<hkpVehicleDefaultVelocityDamper*>(
			m_pVehicle->m_velocityDamper));

	// The wheel collide component performs collision detection. 
	// To do this, it needs to create an aabbPhantom from the 
	// vehicle information that has been set here already.
	setupWheelCollide( 
		world, 
		*m_pVehicle, 
		*static_cast<hkpVehicleRaycastWheelCollide*>(
			m_pVehicle->m_wheelCollide));

	/*setupTyremarks( 
		*(m_pVehicle->m_data), 
		*static_cast<hkpTyremarksInfo*>(m_pVehicle->m_tyreMarks));*/

	// Don't forget to call init! 
	// (This function is necessary to set up derived data)
	m_pVehicle->init();

	// The phantom for collision detection needs 
	// to be explicitly added to the world
	world->addPhantom( 
		(hkpPhantom*)(static_cast<hkpVehicleRaycastWheelCollide*>(
			m_pVehicle->m_wheelCollide)->m_phantom));

	world->addAction(m_pVehicle);

	m_pController->setInputStatus(
		(hkpVehicleDriverInputAnalogStatus*)(
			m_pVehicle->m_deviceStatus));

	m_pStopwatch = UmrStopwatchPtr(new UmrStopwatch());
}

/**
	Updates vehicle status information on the HUD.
*/
void UmrVehicle::updateStats()
{
	static String strSpeed = "Speed (kmh): ";
	static String strBestLap = "Best lap: ";
	static String strLastLap = "Last lap: ";
	static String strCurLap = "Current lap: ";
	static String strTrain = "Training";
	String id = StringConverter::toString(m_id);

	OverlayManager* overlayMgr = OverlayManager::getSingletonPtr();
	OverlayElement* speed = 
		overlayMgr->getOverlayElement("Speed"+id);
	OverlayElement* bestLap = 
		overlayMgr->getOverlayElement("BestLap"+id);
	OverlayElement* lastLap = 
		overlayMgr->getOverlayElement("LastLap"+id);
	OverlayElement* curLap = 
		overlayMgr->getOverlayElement("CurrentLap"+id);

	speed->setCaption(
		strSpeed + StringConverter::toString((int)m_speed));
	bestLap->setCaption(strBestLap + m_pStopwatch->getBestTime());
	lastLap->setCaption(strLastLap + m_pStopwatch->getLastTime());
	curLap->setCaption(strCurLap + m_pStopwatch->getCurrentTime());

	if (m_id > 1) {
		float xpos = m_pController->getXPosition();
		float ypos = m_pController->getYPosition();
		float width = hkMath::fabs(50.0f * xpos);
		float height = hkMath::fabs(50.0f * ypos);
		float left = 385.0f + (m_id-2) * 300.0f;
		float top = 55.0f;
		if (xpos < 0)
			left -= width;
		if (ypos < 0)
			top -= height;
		OverlayElement* turn = 
			overlayMgr->getOverlayElement("TurnPanel"+id);
		turn->setWidth(width);
		turn->setLeft(left);
		OverlayElement* accel = 
			overlayMgr->getOverlayElement("AccelPanel"+id);
		accel->setHeight(height);
		accel->setTop(top);
	} else {
		OverlayElement* train = 
			overlayMgr->getOverlayElement("Training");
		train->setCaption(
			(m_pController->getTrainNNetwork()) ? strTrain : "");
//#ifdef _DEBUG
		OverlayElement* debugText1 = 
			overlayMgr->getOverlayElement("DebugText1");
		OverlayElement* debugText2 = 
			overlayMgr->getOverlayElement("DebugText2");
		String sXpos = 
			StringConverter::toString(m_pController->getXPosition());
		String sYpos = 
			StringConverter::toString(m_pController->getYPosition());

		String dir = StringConverter::toString(m_dirDiff, 3, 6);
		String tilt = StringConverter::toString(m_tilt, 3, 6);
 		String ray0 = StringConverter::toString(m_rays[0], 3, 6);
 		String ray1 = StringConverter::toString(m_rays[1], 3, 6);
		String ray2 = StringConverter::toString(m_rays[2], 3, 6);
 		String ray3 = StringConverter::toString(m_rays[3], 3, 6);
 		String ray4 = StringConverter::toString(m_rays[4], 3, 6);
		String ray5 = StringConverter::toString(m_rays[5], 3, 6);
		String ray6 = StringConverter::toString(m_rays[6], 3, 6);
		debugText1->setCaption(
			sXpos + " " + sYpos + "   " + dir + " " + tilt);
		debugText2->setCaption(
			ray0 + " " + ray1 + " " + ray2 + " " + ray3 + " " + 
			ray4 + "    " + ray5 + " " + ray6);
//#endif
	}
}

/**
	Updates graphic rotation and position of the vehicle
	to match those of the physic world.
*/
void UmrVehicle::updateGraphics()
{
	// Get front wheels turning rotation
	float wheelRot = 
		-MAX_STEERING * 
		m_pController->getXPosition() * 
		(HK_REAL_PI / 180.0f);
	Ogre::Quaternion wRot(
		Ogre::Radian(wheelRot), Ogre::Vector3(0,1,0));
	hkVector4 pos(m_pChassis->getPosition());
	Ogre::Vector3 oPos(pos(0), pos(1) - 0.3f, pos(2));

	// Update body
	hkVector4 up(0, 1, 0);
	hkQuaternion rot(m_pChassis->getRotation());
	hkQuaternion q(up, M_PI_2);
	rot.mul(q);
	Ogre::Quaternion oRot(rot(3), rot(0), rot(1), rot(2));
	m_snBody->setPosition(oPos);
	m_snBody->setOrientation(oRot);

	// Update left front wheel
	m_pVehicle->calcCurrentPositionAndRotation(m_pChassis,
		m_pVehicle->m_suspension, 0, pos, rot);
	oPos = m_snTire_lf->getPosition();
	oRot = Ogre::Quaternion(rot(3), rot(2), 0, 0);
	oRot = wRot * oRot;
	m_snTire_lf->setOrientation(oRot);
	m_snTire_lf->setPosition(oPos.x, pos(3) + 0.2f, oPos.z);

	// Update right front wheel
	m_pVehicle->calcCurrentPositionAndRotation(m_pChassis,
		m_pVehicle->m_suspension, 1, pos, rot);
	oRot = Ogre::Quaternion(rot(3), rot(2), 0, 0);
	oRot = wRot * oRot;
	oPos = m_snTire_rf->getPosition();
	m_snTire_rf->setOrientation(oRot);
	m_snTire_rf->setPosition(oPos.x, pos(3) + 0.2f, oPos.z);

	// Update left rear wheel
	m_pVehicle->calcCurrentPositionAndRotation(m_pChassis,
			m_pVehicle->m_suspension, 2, pos, rot);
	oRot = Ogre::Quaternion(rot(3), rot(2), 0, 0);
	oPos = m_snTire_lr->getPosition();
	m_snTire_lr->setOrientation(oRot);
	m_snTire_lr->setPosition(oPos.x, pos(3) + 0.2f, oPos.z);

	// Update right rear wheel
	m_pVehicle->calcCurrentPositionAndRotation(m_pChassis,
		m_pVehicle->m_suspension, 3, pos, rot);
	oRot = Ogre::Quaternion(rot(3), rot(2), 0, 0);
	oPos = m_snTire_rr->getPosition();
	m_snTire_rr->setOrientation(oRot);
	m_snTire_rr->setPosition(oPos.x, pos(3) + 0.2f, oPos.z);
}

/**
	Updates the axis vectors of the vehicle.
*/
void UmrVehicle::updateVehicleAxis()
{
	hkQuaternion rotation(m_pChassis->getRotation());
	hkVector4 forward(1, 0, 0);
	hkVector4 right(0, 0, 1);
	hkVector4 up(0, 1, 0);
	m_forward.setRotatedDir(rotation, forward);
	m_forward.normalize3();
	m_right.setRotatedDir(rotation, right);
	m_right.normalize3();
	m_up.setCross(m_right, m_forward);
	m_up.normalize3();
	m_tilt = m_forward.dot3(up);
}

/**
	Updates speed of the vehicle.
*/
void UmrVehicle::updateSpeed()
{
	hkVector4 velocity = m_pChassis->getLinearVelocity();
	float speed = velocity.length3(); //m_pVehicle->calcKMPH();
	m_speed = speed * 2.0f;
	hkVector4 vSpeed(velocity);
	vSpeed.normalize3IfNotZero();
	m_dirDiff = m_right.dot3(vSpeed);
	m_dirDiff *= -1.0f;
	vSpeed.add3clobberW(m_forward);
	if (vSpeed.equals3(hkVector4::getZero(), 0.5f)) m_speed *= -1.0f;
}

/**
	Checks if a ray hits against an obstacle.
	@param
		from The starting point of the ray in world space.
	@param
		to The end point of the ray in world space.
*/
float UmrVehicle::checkHit(const hkVector4 &from, const hkVector4 &to)
{
	hkpWorldRayCastInput input;
	hkpClosestRayHitCollector output;
	hkpWorld* world = m_pChassis->getWorld();

	input.m_filterInfo = hkpGroupFilter::calcFilterInfo(RAY_FILTER);
	input.m_from = from;
	input.m_to = to;

	world->castRay(input, output);
	if (output.hasHit()) {
		const hkpWorldRayCastOutput hit = output.getHit();
		return hit.m_hitFraction;
	} else {
		return 1.0f;
	}
}

/**
	Update rays to measure the distance to the border of the track.
*/
void UmrVehicle::updateRays()
{
	hkVector4 pos(m_pChassis->getPosition());
	float degToRad = HK_REAL_PI / 180.0f;
	// 75 m ahead of the vehicle's position
	hkVector4 to(pos(0) + 75.0f * m_forward(0),
				 pos(1) + 75.0f * m_forward(1),
				 pos(2) + 75.0f * m_forward(2));

	for (int i = 0; i < NRAYS; i++) {
		m_lastRays[i] = m_rays[i];
	}

	// front ray
	m_rays[2] = checkHit(pos, to);

	// back ray
	to = hkVector4 (pos(0) + 75.0f * -m_forward(0),
					pos(1) + 75.0f * -m_forward(1),
					pos(2) + 75.0f * -m_forward(2));
	m_backRay = checkHit(pos, to);

	float front;
	to = hkVector4 (pos(0) + 25.0f * m_forward(0),
					pos(1) + 25.0f * m_forward(1),
					pos(2) + 25.0f * m_forward(2));
	front = checkHit(pos, to);

	hkVector4 from(pos(0) + 24.5f * front * m_forward(0),
					pos(1) + 24.5f * front * m_forward(1),
					pos(2) + 24.5f * front * m_forward(2));

	// left turn detection ray
	to = hkVector4(from(0) + 20.0f * -m_right(0),
					from(1) + 20.0f * -m_right(1),
					from(2) + 20.0f * -m_right(2));
	m_rays[5] = checkHit(from, to);

	// right turn detection ray
	to = hkVector4(from(0) + 20.0f * m_right(0),
					from(1) + 20.0f * m_right(1),
					from(2) + 20.0f * m_right(2));
	m_rays[6] = checkHit(from, to);

	// left side ray
	{
		float angle = 45.0f * degToRad;
		hkQuaternion quat(m_up, angle);
		hkVector4 forward;
		forward.setRotatedDir(quat, m_forward);
		forward.normalize3();
		to = hkVector4(pos(0) + 75.0f * forward(0),
						pos(1) + 75.0f * forward(1),
						pos(2) + 75.0f * forward(2));
		m_rays[0] = checkHit(pos, to);
	}

	// front left side ray
	{
		float angle = 22.5f * degToRad;
		hkQuaternion quat(m_up, angle);
		hkVector4 forward;
		forward.setRotatedDir(quat, m_forward);
		forward.normalize3();
		to = hkVector4(pos(0) + 75.0f * forward(0),
						pos(1) + 75.0f * forward(1),
						pos(2) + 75.0f * forward(2));
		m_rays[1] = checkHit(pos, to);
	}

	// front right side ray
	{
		float angle = -22.5f * degToRad;
		hkQuaternion quat(m_up, angle);
		hkVector4 forward;
		forward.setRotatedDir(quat, m_forward);
		forward.normalize3();
		to = hkVector4(pos(0) + 75.0f * forward(0),
						pos(1) + 75.0f * forward(1),
						pos(2) + 75.0f * forward(2));
		m_rays[3] = checkHit(pos, to);
	}

	// right side ray
	{
		float angle = -45.0f * degToRad;
		hkQuaternion quat(m_up, angle);
		hkVector4 forward;
		forward.setRotatedDir(quat, m_forward);
		forward.normalize3();
		to = hkVector4(pos(0) + 75.0f * forward(0),
						pos(1) + 75.0f * forward(1),
						pos(2) + 75.0f * forward(2));
		m_rays[4] = checkHit(pos, to);
	}
}

/**
	Stores the inputs for the neural network training.
*/
void UmrVehicle::prepareNNInputs()
{
	int i = 3;
	m_NNInput[0] = m_pController->getXPosition();
	m_NNInput[1] = m_pController->getYPosition();
	// Normalize speed into the range [-1, 1]
	m_NNInput[2] = m_speed * 0.01f;
	for (int j = 0; j < NRAYS; i++, j++)
		m_NNInput[i] = m_rays[j];
	for (int j = 0; j < NRAYS; i++, j++)
		m_NNInput[i] = m_lastRays[j];
	m_NNInput[17] = m_tilt;
	m_NNInput[18] = m_dirDiff;
}

/**
	Stores the outputs for the neural network training.
*/
void UmrVehicle::prepareNNOutputs()
{
	m_NNOutput[0] = m_pController->getXPosition();
	m_NNOutput[1] = m_pController->getYPosition();
}

/**
	Writes the last stored training data to a file.
*/
void UmrVehicle::writeTrainingData()
{
	for (int i = 0; i < NNINPUTS; i++)
		m_trainingData << m_NNInput[i] << " ";

	for (int i = 0; i < NNOUTPUTS; i++)
		m_trainingData << m_NNOutput[i] << " ";
	m_trainingData << endl;
}

/**
	Updates the input to the vehicle from the player's controller.
*/
bool UmrVehicle::updateControl()
{
	bool trainNN = m_pController->getTrainNNetwork();
	bool wrongDir = false;

	// Save transform only when it's safe
	if (m_speed > 40.0f && m_rays[2] > 0.2f) {
		m_lastValidTrans = m_pChassis->getTransform();
		m_lastValidDir = m_forward;
	}

	wrongDir = m_forward.dot3(m_lastValidDir) < -0.85f;

	m_elapsedTime = m_pTimer->getMilliseconds() - m_lastTime;
	// Check every second if the car is stuck
	if (m_elapsedTime > 1000) {
		m_lastTime = m_elapsedTime;
		if (m_speed >= -0.5f && m_speed <= 0.5f && 
			(m_rays[2] < 0.05f || m_rays[0] < 0.05f || 
			m_rays[4] < 0.05f || m_backRay < 0.05f))
			m_pChassis->setTransform(m_lastValidTrans);
		else if (wrongDir)
			m_pChassis->setTransform(m_lastValidTrans);
		else if (m_up(1) < 0)
			m_pChassis->setTransform(m_lastValidTrans);
	}

	updateVehicleAxis();
	updateSpeed();
	updateRays();

	if (trainNN) {
		prepareNNInputs();
	}
	// Get input from the controller
	bool quit = m_pController->updateState(TIME_STEP_S);
	if (trainNN) {
		prepareNNOutputs();
		// write training set
		writeTrainingData();
	}

	if (m_pController->isActive()) m_pChassis->activate();
	updateStats();

	return quit;
}

/**
	Sets up vehicle data for the physics simulation.
	@param
		world Pointer to the physics world.
	@param
		data Reference to the class to store the 
			 vehicle's static information.
*/
void UmrVehicle::setupVehicleData(
	hkpWorld* world, hkpVehicleData& data)
{
	//
	// The vehicleData contains information about the chassis.
	//

	data.m_gravity = world->getGravity();

	hkVector4 up(0, 1, 0);
	hkVector4 forward(1, 0, 0);
	hkVector4 right(0, 0, 1);
	// The coordinates of the chassis system, 
	// used for steering the vehicle.
	data.m_chassisOrientation.setCols(up, forward, right);

	// Specifies how the effect of dynamic load distribution is 
	// averaged with static load distribution.
	data.m_frictionEqualizer = 0.5f;

	// Change this value to clip the normal in suspension force 
	// calculations. In particular, when mounting a curb, the 
	// raycast vehicle calculations can produce large impulses as 
	// the rays hit the face of the step, causing the 
	// vehicle to spin around its up-axis.
	data.m_normalClippingAngle = 0.2f;

	// Inertia tensor for each axis is calculated by using : 
	// (1 / chassis_mass) * (torque(axis)Factor / chassisUnitInertia)
	data.m_torqueRollFactor = 0.625f;
	data.m_torquePitchFactor = 0.5f;
	data.m_torqueYawFactor = 0.4f;

	// Rotation intertia
	data.m_chassisUnitInertiaYaw = 1.0f; 
	data.m_chassisUnitInertiaRoll = 1.0f;
	data.m_chassisUnitInertiaPitch = 1.0f; 

	// Adds or removes torque around the yaw axis based on the 
	// current steering angle. This will affect steering.
	data.m_extraTorqueFactor = -0.5f;

	data.m_maxVelocityForPositionalFriction = 180.0f;

	//
	// Wheel specifications
	//
	data.m_numWheels = 4;

	data.m_wheelParams.setSize( data.m_numWheels );

	// The axle the wheel is on
	data.m_wheelParams[0].m_axle = 0;
	data.m_wheelParams[1].m_axle = 0;
	data.m_wheelParams[2].m_axle = 1;
	data.m_wheelParams[3].m_axle = 1;

	// Wheel friction coefficient
	data.m_wheelParams[0].m_friction = 1.5f;
	data.m_wheelParams[1].m_friction = 1.5f;
	data.m_wheelParams[2].m_friction = 1.5f;
	data.m_wheelParams[3].m_friction = 1.5f;

	// The slip angle of the tyre for a car 
	// where the cornering forces are one G
	data.m_wheelParams[0].m_slipAngle = 0.0f;
	data.m_wheelParams[1].m_slipAngle = 0.0f;
	data.m_wheelParams[2].m_slipAngle = 0.0f;
	data.m_wheelParams[3].m_slipAngle = 0.0f;

	for (int i = 0; i < data.m_numWheels; i++) {
		// This value is also used to calculate 
		// the m_primaryTransmissionRatio.
		data.m_wheelParams[i].m_radius = 0.25f;
		data.m_wheelParams[i].m_width = 0.3f;
		data.m_wheelParams[i].m_mass = 10.0f;

		// An extra velocity dependent friction factor. 
		// This factor allows us to increase the friction 
		// if the car slides.
		data.m_wheelParams[i].m_viscosityFriction = 0.25f;

		// Clips the final friction
		data.m_wheelParams[i].m_maxFriction = 
			2.0f * data.m_wheelParams[i].m_friction;
		
		data.m_wheelParams[i].m_forceFeedbackMultiplier = 0.1f;
		data.m_wheelParams[i].m_maxContactBodyAcceleration = 
			hkReal(data.m_gravity.length3()) * 2;
	}
}

/**
	Sets up the driver input configuration.
*/
void UmrVehicle::setupComponent(
	const hkpVehicleData& data, 
	hkpVehicleDefaultAnalogDriverInput& driverInput)
{
	// We also use an analog "driver input" class to 
	// help converting user input to vehicle behavior.

	// The initial slope. Used for small steering angles.
	driverInput.m_initialSlope = 0.7f;

	// The input value, up to which the m_initialSlope is valid.
	driverInput.m_slopeChangePointX = 0.8f;

	// The deadZone of the joystick.
	driverInput.m_deadZone = 0.0f;

	// If true, the car will start reversing when 
	// the brake is applied and the car is stopped.
	driverInput.m_autoReverse = true;
}

/**
	Sets up the vehicle steering configuration.
*/
void UmrVehicle::setupComponent(
	const hkpVehicleData& data, 
	hkpVehicleDefaultSteering& steering)
{   
	steering.m_doesWheelSteer.setSize(data.m_numWheels);

	// degrees
	steering.m_maxSteeringAngle = MAX_STEERING * (HK_REAL_PI / 180);

	// The steering angle decreases linearly based
	// on your overall max speed of the vehicle.
	steering.m_maxSpeedFullSteeringAngle = 70.0f * (1.605f / 3.6f);
	steering.m_doesWheelSteer[0] = true;
	steering.m_doesWheelSteer[1] = true;
	steering.m_doesWheelSteer[2] = false;
	steering.m_doesWheelSteer[3] = false;
}

/**
	Sets up the vehicle engine configuration.
*/
void UmrVehicle::setupComponent(
	const hkpVehicleData& data, 
	hkpVehicleDefaultEngine& engine)
{
	// The maximum gross torque the engine 
	// can supply at the optimum RPM.
	engine.m_maxTorque = 600.0f;

	engine.m_minRPM = 1000.0f;
	// The optimum RPM, where the gross 
	// torque of the engine is maximal.
	engine.m_optRPM = 5500.0f;

	// This value is also used to calculate 
	// the m_primaryTransmissionRatio.
	engine.m_maxRPM = 7500.0f;

	// Defines the gross torque at the min/max rpm as 
	// a factor to the torque at optimal RPM.
	engine.m_torqueFactorAtMinRPM = 0.8f;
	engine.m_torqueFactorAtMaxRPM = 0.8f;

	// Defines the engine resistance torque at the min/opt/max rpm
	// as a factor to the torque at optimal RPM.
	engine.m_resistanceFactorAtMinRPM = 0.05f;
	engine.m_resistanceFactorAtOptRPM = 0.1f;
	engine.m_resistanceFactorAtMaxRPM = 0.3f;
}

/**
	Sets up the vehicle transmission configuration.
*/
void UmrVehicle::setupComponent(
	const hkpVehicleData& data, 
	hkpVehicleDefaultTransmission& transmission)
{
	int numGears = 4;

	transmission.m_gearsRatio.setSize( numGears );
	transmission.m_wheelsTorqueRatio.setSize( data.m_numWheels );

	// The RPM of the engine the transmission shifts down and up
	transmission.m_downshiftRPM = 3500.0f;
	transmission.m_upshiftRPM = 6500.0f;

	// The time needed [seconds] to shift a gear
	transmission.m_clutchDelayTime = 0.0f;

	// The back gear ratio
	transmission.m_reverseGearRatio = 1.0f;

	// The ratio of the forward gears
	transmission.m_gearsRatio[0] = 2.0f;
	transmission.m_gearsRatio[1] = 1.5f;
	transmission.m_gearsRatio[2] = 1.0f;
	transmission.m_gearsRatio[3] = 0.75f;

	// The transmission ratio for every wheel
	transmission.m_wheelsTorqueRatio[0] = 0.5f;
	transmission.m_wheelsTorqueRatio[1] = 0.5f;
	transmission.m_wheelsTorqueRatio[2] = 0.0f;
	transmission.m_wheelsTorqueRatio[3] = 0.0f;

	const hkReal vehicleTopSpeed = 180.0f; 	 
	const hkReal wheelRadius = 0.25f;
	const hkReal maxEngineRpm = 7500.0f;
	// An extra factor to the gear ratio
	transmission.m_primaryTransmissionRatio = 
	 hkpVehicleDefaultTransmission::calculatePrimaryTransmissionRatio(
			vehicleTopSpeed,
			wheelRadius,
			maxEngineRpm,
			transmission.m_gearsRatio[numGears - 1]);
}

/**
	Sets up the vehicle brake configuration.
*/
void UmrVehicle::setupComponent(
	const hkpVehicleData& data, 
	hkpVehicleDefaultBrake& brake)
{
	brake.m_wheelBrakingProperties.setSize( data.m_numWheels );

	const float bt = 1500.0f;
	// The maximum torque the wheel can apply when braking
	brake.m_wheelBrakingProperties[0].m_maxBreakingTorque = bt;
	brake.m_wheelBrakingProperties[1].m_maxBreakingTorque = bt;
	brake.m_wheelBrakingProperties[2].m_maxBreakingTorque = bt;
	brake.m_wheelBrakingProperties[3].m_maxBreakingTorque = bt;

	// Handbrake is attached to rear wheels only
	brake.m_wheelBrakingProperties[0].m_isConnectedToHandbrake =false;
	brake.m_wheelBrakingProperties[1].m_isConnectedToHandbrake =false;
	brake.m_wheelBrakingProperties[2].m_isConnectedToHandbrake = true;
	brake.m_wheelBrakingProperties[3].m_isConnectedToHandbrake = true;

	// The minimum amount of braking from the driver
	// that could cause the wheel to block.
	brake.m_wheelBrakingProperties[0].m_minPedalInputToBlock = 0.9f;
	brake.m_wheelBrakingProperties[1].m_minPedalInputToBlock = 0.9f;
	brake.m_wheelBrakingProperties[2].m_minPedalInputToBlock = 0.9f;
	brake.m_wheelBrakingProperties[3].m_minPedalInputToBlock = 0.9f;

	// The time (in secs) after which, if the user 
	// applies enough brake input wheel will block.
	brake.m_wheelsMinTimeToBlock = 1000.0f;
}

/**
	Sets up the vehicle suspension configuration.
*/
void UmrVehicle::setupComponent(
	const hkpVehicleData& data, 
	hkpVehicleDefaultSuspension& suspension) 
{
	suspension.m_wheelParams.setSize( data.m_numWheels );
	suspension.m_wheelSpringParams.setSize( data.m_numWheels );

	// The suspension length at rest i.e. the maximum distance 
	// from the hardpoint to the wheel center.
	suspension.m_wheelParams[0].m_length = 0.5f;
	suspension.m_wheelParams[1].m_length = 0.5f;
	suspension.m_wheelParams[2].m_length = 0.5f;
	suspension.m_wheelParams[3].m_length = 0.5f;

	const float str = 50.0f;
	// The strength [N/m] of the suspension at each wheel
	suspension.m_wheelSpringParams[0].m_strength = str;
	suspension.m_wheelSpringParams[1].m_strength = str;
	suspension.m_wheelSpringParams[2].m_strength = str;
	suspension.m_wheelSpringParams[3].m_strength = str;

	const float wd = 3.0f; 
	// The damping force [N/(m/sec)] of the suspension at each wheel
	suspension.m_wheelSpringParams[0].m_dampingCompression = wd;
	suspension.m_wheelSpringParams[1].m_dampingCompression = wd;
	suspension.m_wheelSpringParams[2].m_dampingCompression = wd;
	suspension.m_wheelSpringParams[3].m_dampingCompression = wd;

	suspension.m_wheelSpringParams[0].m_dampingRelaxation = wd;
	suspension.m_wheelSpringParams[1].m_dampingRelaxation = wd;
	suspension.m_wheelSpringParams[2].m_dampingRelaxation = wd;
	suspension.m_wheelSpringParams[3].m_dampingRelaxation = wd;

	// The hardpoints MUST be positioned INSIDE the chassis
	{
		const hkReal hardPointFrontX = 1.2f;
		const hkReal hardPointBackX = -1.2f;
		const hkReal hardPointY = 0.1f;
		const hkReal hardPointZ = 0.85f;

		// A point INSIDE the chassis to which 
		// the wheel suspension is attached.
		suspension.m_wheelParams[0].m_hardpointChassisSpace.set(
			hardPointFrontX, hardPointY, -hardPointZ);
		suspension.m_wheelParams[1].m_hardpointChassisSpace.set(
			hardPointFrontX, hardPointY,  hardPointZ);
		suspension.m_wheelParams[2].m_hardpointChassisSpace.set(
			hardPointBackX, hardPointY, -hardPointZ);
		suspension.m_wheelParams[3].m_hardpointChassisSpace.set(
			hardPointBackX, hardPointY,  hardPointZ);
	}

	const hkVector4 downDirection( 0.0f, -1.0f, 0.0f );
	// The suspension direction (in Chassis Space)
	suspension.m_wheelParams[0].m_directionChassisSpace = 
		downDirection;
	suspension.m_wheelParams[1].m_directionChassisSpace = 
		downDirection;
	suspension.m_wheelParams[2].m_directionChassisSpace = 
		downDirection;
	suspension.m_wheelParams[3].m_directionChassisSpace = 
		downDirection;
}

/**
	Sets up the vehicle aerodynamics configuration.
*/
void UmrVehicle::setupComponent(
	const hkpVehicleData& data, 
	hkpVehicleDefaultAerodynamics& aerodynamics)
{
	// The density of the air that surrounds 
	// the vehicle, usually, 1.3 kg/m3.
	aerodynamics.m_airDensity = 1.3f;

	// The frontal area, in m2, of the car.
	aerodynamics.m_frontalArea = 1.0f;	

	// The drag and lift coefficients of the car.
	aerodynamics.m_dragCoefficient = 0.7f;
	aerodynamics.m_liftCoefficient = -0.3f;

	// Extra gavity applies in world space 
	// (independent of m_chassisCoordinateSystem).
	aerodynamics.m_extraGravityws.set(0.0f, -5.0f, 0.0f); 
}

/**
	Sets up the vehicle velocity damper configuration.
*/
void UmrVehicle::setupComponent(
	const hkpVehicleData& data, 
	hkpVehicleDefaultVelocityDamper& velocityDamper)
{
	// Caution: setting negative damping values 
	// will add energy to system. Setting the value 
	// to 0 will not affect the angular velocity. 

	// Damping the change of the chassis’ angular 
	// velocity when below m_collisionThreshold. 
	// This will affect turning radius and steering.
	velocityDamper.m_normalSpinDamping    = 0.0f; 

	// Positive numbers dampen the rotation of the chassis and 
	// reduce the reaction of the chassis in a collision. 
	velocityDamper.m_collisionSpinDamping = 4.0f; 

	// The threshold in m/s at which the algorithm switches from 
	// using the normalSpinDamping to the collisionSpinDamping. 	
	velocityDamper.m_collisionThreshold   = 1.0f; 
}

/**
	Sets up the vehicle wheel colliding configuration.
*/
void UmrVehicle::setupWheelCollide(
	hkpWorld* world, 
	const hkpVehicleInstance& vehicle, 
	hkpVehicleRaycastWheelCollide& wheelCollide)
{
	wheelCollide.m_wheelCollisionFilterInfo = 
	  vehicle.getChassis()->getCollidable()->getCollisionFilterInfo();
}

/**
	Sets up the vehicle tyremarks configuration.
*/
void UmrVehicle::setupTyremarks(
	const hkpVehicleData& data, 
	hkpTyremarksInfo& tyreMarks ) 
{
	tyreMarks.m_minTyremarkEnergy = 100.0f;
	tyreMarks.m_maxTyremarkEnergy  = 1000.0f;
}

/**
	Flags the finishing of the first sector.
*/
void UmrVehicle::firstSectorFinished()
{
	if (m_sector == 0) {
		m_pStopwatch->sector1Finished();
		m_sector = 1;
	}
}

/**
	Flags the finishing of the second sector.
*/
void UmrVehicle::secondSectorFinished()
{
	if (m_sector == 1) {
		m_pStopwatch->sector2Finished();
		m_sector = 2;
	}
}

/**
	Flags the finishing of the last sector.
*/
void UmrVehicle::lastSectorFinished()
{
	if (m_sector == 2) {
		m_pStopwatch->lapFinished();
		m_sector = 0;
	}
}

/**
	Sets the starting position of the vehicle.
*/
void UmrVehicle::setStartPosition(
	const Vector3& position, bool reverse)
{
	hkVector4 pos(
		position[0], position[1], position[2] + m_id * 2.5f);
	m_pChassis->setPosition(pos);
	// Rotate 180 degrees on reverse tracks
	if (reverse) {
		hkVector4 up(0, 1, 0);
		hkQuaternion rot(m_pChassis->getRotation());
		hkQuaternion q(up, M_PI);
		rot.mul(q);
		m_pChassis->setRotation(rot);
		m_lastValidDir = hkVector4(-1, 0, 0);
	}
}
