#include "UmrKeyboardInput.h"

// Two last keyboard device states
char g_keyboardState[256];
char g_lastKeyboardState[256];

/**
	Constructor.
*/
UmrKeyboardInput::UmrKeyboardInput(const Ogre::ConfigFile &cf) : 
	m_pDI(NULL), m_pKeyboard(NULL), m_quit(false)
{
	HWND hWnd;
	DWORD _dwCoopFlags = DISCL_EXCLUSIVE | DISCL_FOREGROUND;
	HRESULT _hr;

	m_inputXPosition = 0;
	m_inputYPosition = 0;
	//m_trainNNetwork = true;

	// Get window handle
	hWnd = FindWindow("OgreD3D9Wnd", TITLE);
	if (!IsWindow(hWnd))
		hWnd = FindWindow("OgreGLWindow", TITLE);

	// Create a DInput object
	_hr = DirectInput8Create(
		GetModuleHandle(NULL), DIRECTINPUT_VERSION,
		IID_IDirectInput8, ( VOID** )&m_pDI, NULL);
	if (FAILED(_hr)) {
		m_quit = true;
		return;
	}

	// Obtain an interface to the system keyboard device.
	_hr = m_pDI->CreateDevice(GUID_SysKeyboard, &m_pKeyboard, NULL);
	if (FAILED(_hr)) {
		m_quit = true;
		return;
	}

	// Set the data format to "keyboard format" - a predefined 
	// data format.
	//
	// A data format specifies which controls on a device we
	// are interested in, and how they should be reported.
	//
	// This tells DirectInput that we will be passing an array
	// of 256 bytes to IDirectInputDevice::GetDeviceState.
	_hr = m_pKeyboard->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(_hr)) {
		m_quit = true;
		return;
	}

	// Set the cooperativity level to let DirectInput know how
	// this device should interact with the system and with other
	// DirectInput applications.
	_hr = m_pKeyboard->SetCooperativeLevel(hWnd, _dwCoopFlags);
	if (FAILED(_hr)) {
		m_quit = true;
		return;
	}

	// Acquire the newly created device
	m_pKeyboard->Acquire();

	ZeroMemory(g_keyboardState, sizeof(g_keyboardState));
	ZeroMemory(&m_state, sizeof(m_state));

	setupKeyMap();
	setupActionMap(cf);
}

/**
	Destructor.
*/
UmrKeyboardInput::~UmrKeyboardInput(void)
{
	m_pControllerStatus->removeReference();
	if (m_pDI) {
		if (m_pKeyboard) {
			// Always unacquire device before calling Release()
			m_pKeyboard->Unacquire();
			m_pKeyboard->Release();
			m_pKeyboard = NULL;
		}
		m_pDI->Release();
		m_pDI = NULL;
	}
}

/**
	Sets the physics input controller.
*/
void UmrKeyboardInput::setInputStatus(
	hkpVehicleDriverInputAnalogStatus* status)
{
	m_pControllerStatus = status;
	m_pControllerStatus->addReference();
}

/**
	Updates the input state of the keyboard.
*/
void UmrKeyboardInput::processInput(float timeStep)
{
	HRESULT hr;

	memcpy(g_lastKeyboardState, g_keyboardState, sizeof(char)*256);
	hr = m_pKeyboard->GetDeviceState(
			sizeof(g_keyboardState),(LPVOID)&g_keyboardState);
	if (FAILED(hr)) {
		hr = m_pKeyboard->Acquire();
		while(hr == DIERR_INPUTLOST)
			hr = m_pKeyboard->Acquire();

		// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
		// may occur when the app is minimized or in the process of 
		// switching, so just try again later 
		return;
	}

	m_quit = KEYDOWN(g_keyboardState, m_actionMap[QUIT]) != 0;
	if (m_quit) return;

	m_state.accelerate = 
		KEYDOWN(g_keyboardState, m_actionMap[ACCELERATE]) != 0;

	m_state.brake = 
		KEYDOWN(g_keyboardState, m_actionMap[BRAKE]) != 0;

	m_state.steerLeft = 
		KEYDOWN(g_keyboardState, m_actionMap[STEER_LEFT]) != 0;

	m_state.steerRight = 
		KEYDOWN(g_keyboardState, m_actionMap[STEER_RIGHT]) != 0;

	m_state.handBrake = 
		KEYDOWN(g_keyboardState, m_actionMap[HAND_BRAKE]) != 0;

	if (KEYDOWN(g_keyboardState, m_actionMap[TRAIN]) == 0 &&
		KEYDOWN(g_lastKeyboardState, m_actionMap[TRAIN]) != 0)
		m_trainNNetwork = !m_trainNNetwork;
}

/**
	Updates the state of the controller.
*/
bool UmrKeyboardInput::updateState(float timeStep)
{
	const float steerSpeed     =  3.0f * timeStep;
	const float backSteerSpeed = 10.0f * timeStep;
	hkReal deltaY = -m_inputYPosition  * 0.2f;
	hkReal deltaX = -m_inputXPosition  * backSteerSpeed;

	if (m_pControllerStatus == NULL) return false;

	processInput(timeStep);
	if (m_quit) return true;

	m_active = m_state.accelerate || m_state.brake || 
				m_state.handBrake || m_state.steerRight || 
				m_state.steerLeft;

	if (m_state.accelerate) deltaY = -0.1f;
	else if (m_state.brake) deltaY = 0.1f;

	if (m_state.steerRight) {
		if (m_inputXPosition >= 0.0f) {	deltaX = 0.0f;	}
		deltaX += steerSpeed;
	} else if (m_state.steerLeft) {
		if ( m_inputXPosition <= 0.0f){	deltaX = 0.0f;	}
		deltaX -= steerSpeed;
	}

	m_inputXPosition = 
		hkMath::clamp( m_inputXPosition+deltaX, -1.0f, 1.0f);
	m_inputYPosition = 
		hkMath::clamp( m_inputYPosition+deltaY, -1.0f, 1.0f);

	// Now  -1 <= m_inputXPosition <= 1 and
	//		-1 <= m_inputYPosition <= 1
	m_pControllerStatus->m_positionX = m_inputXPosition;
	m_pControllerStatus->m_positionY = m_inputYPosition;

	m_pControllerStatus->m_handbrakeButtonPressed = m_state.handBrake;

	return false;
}

/**
	Sets up the keys mapping.
	@remarks
		Each key string is mapped to its DirectInput definition.
*/
void UmrKeyboardInput::setupKeyMap()
{
	m_keyMap["ESCAPE"] = DIK_ESCAPE;
	m_keyMap["A"] = DIK_A;
	m_keyMap["B"] = DIK_B;
	m_keyMap["C"] = DIK_C;
	m_keyMap["D"] = DIK_D;
	m_keyMap["E"] = DIK_E;
	m_keyMap["F"] = DIK_F;
	m_keyMap["G"] = DIK_G;
	m_keyMap["H"] = DIK_H;
	m_keyMap["I"] = DIK_I;
	m_keyMap["J"] = DIK_J;
	m_keyMap["K"] = DIK_K;
	m_keyMap["L"] = DIK_L;
	m_keyMap["M"] = DIK_M;
	m_keyMap["N"] = DIK_N;
	m_keyMap["O"] = DIK_O;
	m_keyMap["P"] = DIK_P;
	m_keyMap["Q"] = DIK_Q;
	m_keyMap["R"] = DIK_R;
	m_keyMap["S"] = DIK_S;
	m_keyMap["T"] = DIK_T;
	m_keyMap["U"] = DIK_U;
	m_keyMap["V"] = DIK_V;
	m_keyMap["W"] = DIK_W;
	m_keyMap["X"] = DIK_X;
	m_keyMap["Y"] = DIK_Y;
	m_keyMap["Z"] = DIK_Z;
	m_keyMap["1"] = DIK_1;
	m_keyMap["2"] = DIK_2;
	m_keyMap["3"] = DIK_3;
	m_keyMap["4"] = DIK_4;
	m_keyMap["5"] = DIK_5;
	m_keyMap["6"] = DIK_6;
	m_keyMap["7"] = DIK_7;
	m_keyMap["8"] = DIK_8;
	m_keyMap["9"] = DIK_9;
	m_keyMap["0"] = DIK_0;
	m_keyMap["LCONTROL"] = DIK_LCONTROL;
	m_keyMap["LSHIFT"] = DIK_LSHIFT;
	m_keyMap["TAB"] = DIK_TAB;
	m_keyMap["LALT"] = DIK_LALT;
	m_keyMap["SPACE"] = DIK_SPACE;
	m_keyMap["RALT"] = DIK_RALT;
	m_keyMap["RCONTROL"] = DIK_RCONTROL;
	m_keyMap["COMMA"] = DIK_COMMA;
	m_keyMap["PERIOD"] = DIK_PERIOD;
	m_keyMap["RSHIFT"] = DIK_RSHIFT;
	m_keyMap["ENTER"] = DIK_RETURN;
	m_keyMap["BACKSPACE"] = DIK_BACKSPACE;
	m_keyMap["HOME"] = DIK_HOME;
	m_keyMap["END"] = DIK_END;
	m_keyMap["DELETE"] = DIK_DELETE;
	m_keyMap["PRIOR"] = DIK_PRIOR;
	m_keyMap["NEXT"] = DIK_NEXT;
	m_keyMap["UP"] = DIK_UP;
	m_keyMap["DOWN"] = DIK_DOWN;
	m_keyMap["LEFT"] = DIK_LEFT;
	m_keyMap["RIGHT"] = DIK_RIGHT;
	m_keyMap["F1"] = DIK_F1;
	m_keyMap["F2"] = DIK_F2;
	m_keyMap["F3"] = DIK_F3;
	m_keyMap["F4"] = DIK_F4;
	m_keyMap["F5"] = DIK_F5;
	m_keyMap["F6"] = DIK_F6;
	m_keyMap["F7"] = DIK_F7;
	m_keyMap["F8"] = DIK_F8;
	m_keyMap["F9"] = DIK_F9;
	m_keyMap["F10"] = DIK_F10;
	m_keyMap["F11"] = DIK_F11;
	m_keyMap["F12"] = DIK_F12;
}

/**
	Reads the key binding configuration from a file.
*/
void UmrKeyboardInput::setupActionMap(const Ogre::ConfigFile &cf)
{
	string key;

	key = cf.getSetting("Accelerate", "Keyboard", "UP");
	m_actionMap[ACCELERATE] = m_keyMap[key];

	key = cf.getSetting("Brake", "Keyboard", "DOWN");
	m_actionMap[BRAKE] = m_keyMap[key];

	key = cf.getSetting("SteerLeft", "Keyboard", "LEFT");
	m_actionMap[STEER_LEFT] = m_keyMap[key];

	key = cf.getSetting("SteerRight", "Keyboard", "RIGHT");
	m_actionMap[STEER_RIGHT] = m_keyMap[key];

	key = cf.getSetting("HandBrake", "Keyboard", "SPACE");
	m_actionMap[HAND_BRAKE] = m_keyMap[key];

	key = cf.getSetting("Quit", "Keyboard", "ESCAPE");
	m_actionMap[QUIT] = m_keyMap[key];

	key = cf.getSetting("Train", "Keyboard", "T");
	m_actionMap[TRAIN] = m_keyMap[key];
}
