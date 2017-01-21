#include "UmrScene.h"

/**
	Constructor.
	@param
		root Reference to the Ogre3D root class.
*/
UmrScene::UmrScene(RootPtr &root) : m_pRoot(root)
{
	// create the scene manager
	//m_pSceneMgr=root->createSceneManager(ST_GENERIC, "SMInstance");
	m_pSceneMgr = 
		root->createSceneManager(ST_EXTERIOR_CLOSE, "SMInstance");

	// create the camera
	m_pCamera = m_pSceneMgr->createCamera("PlayerCam");
	// Position the camera
	m_pCamera->setPosition(Vector3(-6,1.5f,0));
	// Look back along -Z
	m_pCamera->lookAt(Vector3(2,0,0));
	m_pCamera->setNearClipDistance(1);
	m_pCamera->setFarClipDistance(2000);
	m_pCamera->setFOVy(Radian(60.0f * M_PI / 180.0f));
}

/**
	Destructor.
*/
UmrScene::~UmrScene(void)
{
	m_pSceneMgr->destroyCamera("PlayerCam");
	m_pRoot->destroySceneManager(m_pSceneMgr);
}

/**
	Creates the elements of the graphic scene.
	@param
		vehicle Reference to the vehicle to be followed by the camera.
*/
bool UmrScene::createScene(UmrVehiclePtr &vehicle)
{
	m_pFollowVehicle = vehicle;
	createFollowCamera();

	m_pSceneMgr->setAmbientLight(ColourValue(0.8f,0.8f,0.8f));

	// Create a directional light
	Light *l = m_pSceneMgr->createLight("MainLight");
	Ogre::Vector3 dir(1,-1,-1);
	dir.normalise();
	l->setType(Ogre::Light::LT_DIRECTIONAL);
	l->setDirection(dir);
	l->setDiffuseColour(1, 1, 1);
	l->setSpecularColour(1, 1, 1);

	m_pSceneMgr->setSkyBox(true, "Sky1", 1000);

	return true;
}

/**
	Destroys the scene.
*/
void UmrScene::destroyScene()
{
	m_pSceneMgr->destroyLight("MainLight");
}

/**
	Gets a pointer to the camera of the scene.
*/
Camera * UmrScene::getCamera()
{
	return m_pCamera;
}

/**
	Creates a camera that follows a vehicle.
*/
void UmrScene::createFollowCamera()
{
	hkp1dAngularFollowCamCinfo cinfo;

	cinfo.m_yawSignCorrection = 1.0f;
	cinfo.m_upDirWS.set(0.0f, 1.0f, 0.0f);
	cinfo.m_rigidBodyForwardDir.set(1.0f, 0.0f, 0.0f);

	// Depending on the velocity of the car, the parameters actually
	// used are calculated by interpolating between two parameter sets
	cinfo.m_set[0].m_velocity = 10.0f;
	cinfo.m_set[1].m_velocity = 80.0f;
	cinfo.m_set[0].m_speedInfluenceOnCameraDirection = 1.0f;
	cinfo.m_set[1].m_speedInfluenceOnCameraDirection = 1.0f;
	cinfo.m_set[0].m_angularRelaxation = 3.0f;
	cinfo.m_set[1].m_angularRelaxation = 6.0f;

	cinfo.m_set[0].m_positionUS.set( -6.0f, 2.0f, 0.0f);
	cinfo.m_set[1].m_positionUS.set( -9.0f, 2.5f, 0.0f);

	cinfo.m_set[0].m_lookAtUS.set ( 2.0f, 0.0f, 0.0f );
	cinfo.m_set[1].m_lookAtUS.set ( 2.0f, 0.0f, 0.0f );

	cinfo.m_set[0].m_fov = 60.0f;
	cinfo.m_set[1].m_fov = 60.0f;

	m_followCamera.reinitialize( cinfo );
}

/**
	Updates the camera, based on where the vehicle is.
*/
void UmrScene::updateCamera()
{
	const hkpRigidBody *vehicleChassis = 
		m_pFollowVehicle->getChassis();
	// Vehicle specific camera settings
	hkp1dAngularFollowCam::CameraInput in;
	{
		hkpWorld *world = vehicleChassis->getWorld();
		hkReal time = world->getCurrentTime();
		vehicleChassis->approxTransformAt( time, in.m_fromTrans );

		in.m_linearVelocity = vehicleChassis->getLinearVelocity();
		in.m_angularVelocity = vehicleChassis->getAngularVelocity();
		in.m_deltaTime = TIME_STEP_S;
	}

	hkp1dAngularFollowCam::CameraOutput out;
	{
		m_followCamera.calculateCamera( in, out );
	}

	// General camera settings
	Vector3 position(
		out.m_positionWS(0), 
		out.m_positionWS(1), 
		out.m_positionWS(2));
	Vector3 lookAt(
		out.m_lookAtWS(0), 
		out.m_lookAtWS(1), 
		out.m_lookAtWS(2));
	//float fov = out.m_fov * M_PI / 180.0f;
	m_pCamera->setPosition(position);
	m_pCamera->lookAt(lookAt);
	//m_pCamera->setFOVy(Radian(fov));
}

/**
	Updates the camera of the scene.
*/
void UmrScene::updateScene()
{
	updateCamera();
}
