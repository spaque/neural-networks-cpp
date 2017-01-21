#pragma once

#include "UmrConfig.h"
#include "UmrVehicle.h"

#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreSceneManager.h>

#include <Common/Base/System/Io/IStream/hkIStream.h>
#include <Common/Serialize/Packfile/Binary/hkBinaryPackfileReader.h>
#include <Common/Serialize/Util/hkRootLevelContainer.h>

#include <Physics/Collide/Filter/Group/hkpGroupFilter.h>
#include <Physics/Collide/Shape/Convex/Box/hkpBoxShape.h>
#include <Physics/Collide/Shape/Misc/Bv/hkpBvShape.h>
#include <Physics/Collide/Shape/Misc/PhantomCallback/hkpPhantomCallbackShape.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include <Physics/Dynamics/World/hkpWorld.h>
#include <Physics/Utilities/Serialize/hkpPhysicsData.h>

using namespace Ogre;

/**
	Track class.
	Container for the physics and graphics information of the track.
	Manages the loading of the track content.
*/
__declspec(align(MEM_ALIGNMENT))
class UmrTrack : public UmrObject
{
public:
	UmrTrack(String &path, UmrVehiclePtr (&vehicles)[NCARS]);
	~UmrTrack(void);

	void load(const String& cfgFile);
	void destroy();
	void addToPhysicsWorld (hkpWorld *world);
protected:
	void loadGraphics(const ConfigFile &cf);
	void loadPhysics(const ConfigFile &cf);

	/// Track name. Used for content loading and scene graph identification.
	String m_sName;

	/// Relative path for content loading.
	String m_sPath;

	/// Scene graph nodes for the track and the surroundings
	SceneNode* m_snTrack;
	SceneNode* m_snOutfield;

	/// Vehicles running on the track
	UmrVehiclePtr m_vehicles[NCARS];

	/// Track physics information read from file
	hkPackfileData* m_pTrackLoadedData;

	/// Track rigid body
	hkpRigidBody* m_pTrackRigidBody;

	/// Outfield physics information read from file
	hkPackfileData* m_pOutfieldLoadedData;

	/// Outfield rigid body
	hkpRigidBody* m_pOutfieldRigidBody;

	/// Track sectors' rigid bodies
	hkpRigidBody* m_pFinish;
	hkpRigidBody* m_pSector1;
	hkpRigidBody* m_pSector2;
};

typedef boost::shared_ptr<UmrTrack> UmrTrackPtr;
