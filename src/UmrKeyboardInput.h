#pragma once

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION         DIRECTINPUT_HEADER_VERSION
#endif

#include "UmrConfig.h"
#include "UmrController.h"

#include <OgreConfigFile.h>
#include <dinput.h>
#include <map>

#include <Physics/Vehicle/DriverInput/Default/hkpVehicleDefaultAnalogDriverInput.h>

#define KEYDOWN(name, key) (name[key] & 0x80)

#define CLAMP(value, min, max) ((value <= min) ? min : ((value >= max) ? max : value))

#define MAX_ACTIONS	9
#define ACCELERATE	0
#define BRAKE		1
#define STEER_LEFT	2
#define STEER_RIGHT	3
#define SHIFT_UP	4
#define SHIFT_DOWN	5
#define HAND_BRAKE	6
#define QUIT		7
#define TRAIN		8

using namespace std;

/**
	Describes the state of certain input actions.
*/
__declspec(align(MEM_ALIGNMENT))
struct UmrKeyboardInputState : public UmrObject
{
	bool	accelerate;
	bool	brake;
	bool	steerLeft;
	bool	steerRight;
	bool	shiftUp;
	bool	shiftDown;
	bool	handBrake;
};

/**
	Keyboard input class.
	Provides the input to the controller through a keyboard device.
*/
__declspec(align(MEM_ALIGNMENT))
class UmrKeyboardInput : public UmrController
{
public:
	UmrKeyboardInput(const Ogre::ConfigFile &cf);
	~UmrKeyboardInput(void);

	virtual bool updateState(float timeStep);
	virtual void setInputStatus(
		hkpVehicleDriverInputAnalogStatus* status);

protected:
	virtual void processInput(float timeStep);
	void setupKeyMap();
	void setupActionMap(const Ogre::ConfigFile &cf);

	/// Pointer to the DirectInput subsystem
	LPDIRECTINPUT8			m_pDI;

	/// Pointer to the joystick input device
	LPDIRECTINPUTDEVICE8	m_pKeyboard;

	/// Last state of the device
	UmrKeyboardInputState	m_state;

	/// Holds the key binding for every input action
	byte					m_actionMap[MAX_ACTIONS];

	/// Maps a string to a DirectInput key definition
	map<string, byte>		m_keyMap;

	/// True to quit the application
	bool					m_quit;
};

typedef boost::shared_ptr<UmrKeyboardInput> UmrInputPtr;
