#pragma once

#include "UmrConfig.h"

#include <Physics/Vehicle/DriverInput/Default/hkpVehicleDefaultAnalogDriverInput.h>

/**
	Controller class.
	Base class for any class able to control a vehicle. 
	This class is an abstraction for the input of a driver 
	to the controls of the vehicle.
*/
__declspec(align(MEM_ALIGNMENT))
class UmrController : public UmrObject
{
public:
	UmrController (void) {}
	virtual ~UmrController (void) {}

	virtual bool updateState (float stepTime) = 0;
	const bool isActive () const { return m_active; }
	const float getXPosition () const { return m_inputXPosition; }
	const float getYPosition () const { return m_inputYPosition; }
	const bool getTrainNNetwork () const { return m_trainNNetwork; }
	virtual void setInputStatus (
		hkpVehicleDriverInputAnalogStatus* status) = 0;
protected:
	virtual void processInput(float stepTime) {}

	/// Container for the input values obtained from the contoller.
	hkpVehicleDriverInputAnalogStatus* m_pControllerStatus;

	/// Whether the controller is active or not.
	bool	m_active;

	/// True when the vehicle should record training data.
	bool	m_trainNNetwork;

	/// The current input for the steering wheel
	float	m_inputXPosition;

	/// The current input for the acceleration and brake pedal
	float	m_inputYPosition;
};

typedef boost::shared_ptr<UmrController> UmrControllerPtr;
