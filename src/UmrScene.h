#pragma once

#include "UmrConfig.h"
#include "UmrVehicle.h"

#include <OgreCamera.h>
#include <OgreMeshManager.h>
#include <OgreRenderWindow.h>
#include <OgreSceneManager.h>

#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include <Physics/Dynamics/World/hkpWorld.h>
#include <Physics/Vehicle/Camera/hkp1dAngularFollowCam.h>

using namespace Ogre;

/**
	Scene class.
	Manages Ogre3D scene creation and updating.
	Object loading into the scene is delegated to
	the Vehicle and Track classes.
*/
__declspec(align(MEM_ALIGNMENT))
class UmrScene : public UmrObject
{
public:
	UmrScene(RootPtr &root);
	~UmrScene(void);

	bool createScene (UmrVehiclePtr &vehicle);
	void destroyScene ();
	Camera * getCamera();
	void updateScene ();
protected:
	void createFollowCamera ();
	void updateCamera ();

	/// Root class for Ogre3D system
	RootPtr m_pRoot;

	/// Camera from which the scene will be rendered
	Camera* m_pCamera;

	/// Ogre3D scene manager. Manages the organization
	/// and rendering of the scene.
	SceneManager* m_pSceneMgr;

	/// Vehicle followed by the scene camera
	UmrVehiclePtr m_pFollowVehicle;

	/// Havok camera helper. Camera attached to the vehicle
	hkp1dAngularFollowCam m_followCamera;

	/// Havok physics world
	hkpWorld* m_pWorld;
};

typedef boost::shared_ptr<UmrScene> UmrScenePtr;
