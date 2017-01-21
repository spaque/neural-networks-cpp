#pragma once

#include "UmrConfig.h"
#include "UmrController.h"
#include "UmrKeyboardInput.h"
#include "UmrJoystickInput.h"
#include "UmrStopwatch.h"

#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreSceneManager.h>
#include <OgreSubEntity.h>
#include <OgreOverlayManager.h>
#include <OgreOverlayElement.h>

// #include <Common/Base/System/Io/IStream/hkIStream.h>
// #include <Common/Serialize/Packfile/Binary/hkBinaryPackfileReader.h>
// #include <Common/Serialize/Util/hkRootLevelContainer.h>
#include <Physics/Collide/Filter/Group/hkpGroupFilter.h>
#include <Physics/Collide/Query/CastUtil/hkpWorldRayCastInput.h>
#include <Physics/Collide/Query/CastUtil/hkpWorldRayCastOutput.h>
#include <Physics/Collide/Query/Collector/RayCollector/hkpClosestRayHitCollector.h>
#include <Physics/Collide/Query/Collector/BodyPairCollector/hkpAllCdBodyPairCollector.h>
#include <Physics/Collide/Shape/Convex/Box/hkpBoxShape.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include <Physics/Dynamics/Phantom/hkpSimpleShapePhantom.h>
#include <Physics/Dynamics/World/hkpWorld.h>
//#include <Physics/Utilities/Serialize/hkpPhysicsData.h>
#include <Physics/Utilities/Actions/Reorient/hkpReorientAction.h>
#include <Physics/Utilities/Dynamics/Inertia/hkpInertiaTensorComputer.h>
#include <Physics/Collide/Shape/Convex/ConvexVertices/hkpConvexVerticesShape.h>
#include <Common/Internal/ConvexHull/hkGeometryUtility.h>

#include <Physics/Vehicle/AeroDynamics/Default/hkpVehicleDefaultAerodynamics.h>
#include <Physics/Vehicle/DriverInput/Default/hkpVehicleDefaultAnalogDriverInput.h>
#include <Physics/Vehicle/Brake/Default/hkpVehicleDefaultBrake.h>
#include <Physics/Vehicle/Engine/Default/hkpVehicleDefaultEngine.h>
#include <Physics/Vehicle/VelocityDamper/Default/hkpVehicleDefaultVelocityDamper.h>
#include <Physics/Vehicle/Steering/Default/hkpVehicleDefaultSteering.h>
#include <Physics/Vehicle/Suspension/Default/hkpVehicleDefaultSuspension.h>
#include <Physics/Vehicle/Transmission/Default/hkpVehicleDefaultTransmission.h>
#include <Physics/Vehicle/WheelCollide/RayCast/hkpVehicleRaycastWheelCollide.h>
#include <Physics/Vehicle/TyreMarks/hkpTyremarksInfo.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <fstream>

#define MAX_STEERING 35.0f

#define NRAYS 7

#define NNINPUTS 19
#define NNOUTPUTS 2

using namespace Ogre;

class UmrAIControlFast;
class UmrAIControlSlow;

/**
	Vehicle class.
	Container for all the runtime data the vehicle needs.
	Manages the vehicle content loading, the graphics
	updating (to keep coherence with the physics information),
	the player input updating and the recording of training data.
*/
__declspec(align(MEM_ALIGNMENT))
class UmrVehicle : public UmrObject
{
public:
	friend class UmrAIControlFast;
	friend class UmrAIControlSlow;

	UmrVehicle (short id, String &path);
	~UmrVehicle (void);

	void load (const String& cfgFile, int controlType);
	void destroy ();
	void buildVehicle (hkpWorld* world);
	const hkpRigidBody* getChassis() const;
	void updateGraphics ();
	bool updateControl ();
	void firstSectorFinished ();
	void secondSectorFinished ();
	void lastSectorFinished ();
	void setStartPosition (const Vector3& position, bool reverse);

protected:
	// Loading
	void loadGraphics (const ConfigFile &cf);
	void loadPhysics (const ConfigFile &cf);
	void loadController (int controlType);

	// Updating
	void updateStats ();
	void updateVehicleAxis ();
	void updateSpeed ();
	void updateRays ();
	float checkHit (const hkVector4 &from, const hkVector4 &to);

	// Training
	void prepareNNInputs ();
	void prepareNNOutputs ();
	void writeTrainingData ();

	// Physics configuration
	void setupVehicleData (hkpWorld* world, hkpVehicleData& data);
	void setupComponent (
		const hkpVehicleData& data, 
		hkpVehicleDefaultAnalogDriverInput& driverInput);
	void setupComponent (
		const hkpVehicleData& data, 
		hkpVehicleDefaultSteering& steering);
	void setupComponent (
		const hkpVehicleData& data, 
		hkpVehicleDefaultEngine& engine);
	void setupComponent (
		const hkpVehicleData& data, 
		hkpVehicleDefaultTransmission& transmission);
	void setupComponent (
		const hkpVehicleData& data, 
		hkpVehicleDefaultBrake& brake);
	void setupComponent (
		const hkpVehicleData& data, 
		hkpVehicleDefaultSuspension& suspension);
	void setupComponent (
		const hkpVehicleData& data, 
		hkpVehicleDefaultAerodynamics& aerodynamics);
	void setupComponent (
		const hkpVehicleData& data, 
		hkpVehicleDefaultVelocityDamper& velocityDamper);
	void setupWheelCollide (
		hkpWorld* world, 
		const hkpVehicleInstance& vehicle, 
		hkpVehicleRaycastWheelCollide& wheelCollide);
	void setupTyremarks (
		const hkpVehicleData& data, 
		hkpTyremarksInfo& tyremarkscontroller);

	/// Vehicle unique id. Used for collision filtering, 
	/// scene graph identification and positioning.
	short m_id;

	/// Vehicle name. Used for content loading 
	/// and scene graph identification.
	String m_sName;			// 32 bytes

	/// Relative path for content loading.
	String m_sPath;

	/// Scene graph nodes for body and wheels.
	SceneNode* m_snBody;	// 4 bytes
	SceneNode* m_snTire_lf;
	SceneNode* m_snTire_lr;
	SceneNode* m_snTire_rf;
	SceneNode* m_snTire_rr;
	//SceneNode*		m_snWheel;
	//hkPackfileData* m_pLoadedData;

	/// Vehicle chassis rigid body.
	hkpRigidBody* m_pChassis;

	/// Havok vehicle action instance.
	hkpVehicleInstance*	m_pVehicle;

	/// Vehicle controller. Joystick, keyboard or AI.
	UmrControllerPtr m_pController;

	/// Vehicle stopwatch used to measure lap times.
	UmrStopwatchPtr m_pStopwatch;

	/// Vehicle's last valid transformation. Used to 
	/// reorient the vehicle when it's stuck.
	hkTransform m_lastValidTrans;
	hkVector4	m_lastValidDir;

	/// Timer used to poll at regular intervals
	/// whether the vehicle is stuck.
	TimerPtr m_pTimer;
	ULONG	 m_elapsedTime, m_lastTime;

	/// Calculated current speed of the vehicle.
	float m_speed;

	/// Vehicle coordinate system.
	hkVector4 m_forward;
	hkVector4 m_right;
	hkVector4 m_up;

	/// Sector the vehicle is currently in.
	int m_sector;

	/// Rays used to detect distance to the road's borders.
	/// Rays are casted in the angles: 45, 22.5, 0, -22.5, -45
	float m_rays[NRAYS];
	float m_lastRays[NRAYS];
	float m_backRay;

	/// Current road tilt.
	float m_tilt;

	/// Difference between vehicle direction and actual direction.
	float m_dirDiff;

	/// Training sample to be recorded on the training file.
	float m_NNInput[NNINPUTS];
	float m_NNOutput[NNOUTPUTS];

	/// File which contains training data 
	/// from the last training session.
	std::ofstream m_trainingData;
};

typedef boost::shared_ptr<UmrVehicle> UmrVehiclePtr;
