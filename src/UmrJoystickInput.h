#pragma once

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION         DIRECTINPUT_HEADER_VERSION
#endif

#include "UmrConfig.h"
#include "UmrController.h"

#include <cstring>
#include <dinput.h>
#include <dinputd.h>

#include <Physics/Vehicle/DriverInput/Default/hkpVehicleDefaultAnalogDriverInput.h>

/**
	Joystick keys binding definition.
*/
__declspec(align(MEM_ALIGNMENT))
struct UmrJoystickKeys : public UmrObject
{
	unsigned handbrake;
	unsigned train;
	unsigned quit;
};

/**
	Joystick input class.
	Provides the input to the controller through a joystick device.
*/
__declspec(align(MEM_ALIGNMENT))
class UmrJoystickInput : public UmrController
{
public:
	UmrJoystickInput (const Ogre::ConfigFile &cf);
	~UmrJoystickInput (void);

	virtual bool updateState(float timeStep);
	virtual void setInputStatus(
		hkpVehicleDriverInputAnalogStatus* status);
protected:
	static int CALLBACK enumJoystickCallback(
		const DIDEVICEINSTANCE* pdidInstance, 
		VOID* pContext);
	static int CALLBACK enumObjectsCallback(
		const DIDEVICEOBJECTINSTANCE* pdidoi, 
		VOID* pContext);
	virtual void processInput(float timeStep);
	void setupKeys (const Ogre::ConfigFile &cf);

	/// Pointer to the DirectInput subsystem
	LPDIRECTINPUT8 m_pDI;

	/// Pointer to the joystick input device
	LPDIRECTINPUTDEVICE8 m_pJoystick;

	/// Structures holding the two last joystick device states
	DIJOYSTATE2	m_state;
	DIJOYSTATE2	m_lastState;

	/// True to quit the application
	bool m_quit;

	UmrJoystickKeys m_keys;
};

typedef boost::shared_ptr<UmrJoystickInput> UmrJoystickInputPtr;
