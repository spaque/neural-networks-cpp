#include "UmrJoystickInput.h"

/**
	Constructor.
*/
UmrJoystickInput::UmrJoystickInput(const Ogre::ConfigFile &cf) :
	m_pDI(NULL), m_pJoystick(NULL), m_quit(false)
{
	HWND hWnd;
	HRESULT hr;

	m_pControllerStatus = HK_NULL;
	m_inputXPosition = 0;
	m_inputYPosition = 0;
	m_active = true;
	m_trainNNetwork = false;

	// Get window handle
	hWnd = FindWindow("OgreD3D9Wnd", TITLE);
	if (!IsWindow(hWnd))
		hWnd = FindWindow("OgreGLWindow", TITLE);

	// Register with the DirectInput subsystem and get a pointer
	// to a IDirectInput interface we can use.
	// Create a DInput object
	hr = DirectInput8Create(GetModuleHandle(NULL),DIRECTINPUT_VERSION,
		IID_IDirectInput8, (VOID**)&m_pDI, NULL);
	if (FAILED(hr)) {
		m_quit = true;
		return;
	}

	// Look for a simple joystick we can use for this sample program.
	hr = m_pDI->EnumDevices(DI8DEVCLASS_GAMECTRL,
		&UmrJoystickInput::enumJoystickCallback,
		this, DIEDFL_ATTACHEDONLY);
	if (FAILED(hr) || m_pJoystick == NULL) {
		m_quit = true;
		return;
	}

	// Set the data format to "simple joystick" - a predefined 
	// data format.
	//
	// A data format specifies which controls on a device we 
	// are interested in, and how they should be reported. This 
	// tells DInput that we will be passing a DIJOYSTATE2 structure 
	// to IDirectInputDevice::GetDeviceState().
	hr = m_pJoystick->SetDataFormat(&c_dfDIJoystick2);
	if (FAILED(hr)) {
		m_quit = true;
		return;
	}

	// Set the cooperative level to let DInput 
	// know how this device should interact with 
	// the system and with other DInput applications.
	hr = m_pJoystick->SetCooperativeLevel(
			hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
	if (FAILED(hr)) {
		m_quit = true;
		return;
	}

	// Enumerate the joystick objects. The callback 
	// function enabled user interface elements for 
	// objects that are found, and sets the min/max
	// values property for discovered axes.
	hr = m_pJoystick->EnumObjects(
		&UmrJoystickInput::enumObjectsCallback,
		this, DIDFT_ALL);
	if (FAILED(hr)) {
		m_quit = true;
		return;
	}

	m_pJoystick->Acquire();

	setupKeys(cf);
}

/**
	Destructor.
*/
UmrJoystickInput::~UmrJoystickInput(void)
{
	if (m_pControllerStatus != HK_NULL) 
		m_pControllerStatus->removeReference();
	if (m_pDI) {
		if (m_pJoystick) {
			// Always unacquire device before calling Release()
			m_pJoystick->Unacquire();
			m_pJoystick->Release();
			m_pJoystick = NULL;
		}
		m_pDI->Release();
		m_pDI = NULL;
	}
}

/**
	Sets the physics input controller.
*/
void UmrJoystickInput::setInputStatus(
	hkpVehicleDriverInputAnalogStatus* status)
{
	m_pControllerStatus = status;
	m_pControllerStatus->addReference();
// 	m_pControllerStatus->m_handbrakeButtonPressed = false;
// 	m_pControllerStatus->m_reverseButtonPressed = false;
}

/**
	Callback function called once for each enumerated joystick. If we
	find one, create a device interface on it so we can play with it.
	@param
		pdidInstance Pointer to an instance of a DirectInput device.
	@param
		pContext Pointer to this class.
*/
int CALLBACK UmrJoystickInput::enumJoystickCallback(
	const DIDEVICEINSTANCE *pdidInstance, void *pContext)
{
	UmrJoystickInput* pThis = (UmrJoystickInput*)pContext;
	HRESULT hr;

	// Obtain an interface to the enumerated joystick.
	hr = pThis->m_pDI->CreateDevice(
			pdidInstance->guidInstance, &(pThis->m_pJoystick), NULL);

	// If it failed, then we can't use this joystick. 
	// (Maybe the user unplugged it while we were in 
	// the middle of enumerating it.)
	if( FAILED( hr ) )
		return DIENUM_CONTINUE;

	// Stop enumeration. Note: we're just taking the 
	// first joystick we get. You could store all the 
	// enumerated joysticks and let the user pick.
	return DIENUM_STOP;
}

/**
	Callback function for enumerating objects (axes, buttons, POVs) 
	on a joystick. This function enables user interface elements for 
	objects	that are found to exist, and scales axes min/max values.
	@param
		pdidoi Pointer to a device object instance.
	@param
		pContext Pointer to this class.
*/
int CALLBACK UmrJoystickInput::enumObjectsCallback(
	const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext)
{
	UmrJoystickInput* pThis = (UmrJoystickInput*)pContext;

	// Number of returned slider controls
	static int nSliderCount = 0;
	// Number of returned POV controls
	static int nPOVCount = 0;

	// For axes that are returned, set the DIPROP_RANGE property 
	// for the enumerated axis in order to scale min/max values.
	if( pdidoi->dwType & DIDFT_AXIS )
	{
		DIPROPRANGE diprg;
		diprg.diph.dwSize = sizeof( DIPROPRANGE );
		diprg.diph.dwHeaderSize = sizeof( DIPROPHEADER );
		diprg.diph.dwHow = DIPH_BYID;
		// Specify the enumerated axis
		diprg.diph.dwObj = pdidoi->dwType;
		diprg.lMin = -1000;
		diprg.lMax = +1000;

		// Set the range for the axis
		if(FAILED(pThis->m_pJoystick->SetProperty(
					DIPROP_RANGE, &diprg.diph)))
			return DIENUM_STOP;
	}
	return DIENUM_CONTINUE;
}

void UmrJoystickInput::setupKeys(const Ogre::ConfigFile &cf)
{
	m_keys.handbrake = Ogre::StringConverter::parseUnsignedInt(
						cf.getSetting("HandBrake", "Joystick", "7"));
	m_keys.train = Ogre::StringConverter::parseUnsignedInt(
						cf.getSetting("Train", "Joystick", "0"));
	m_keys.quit = Ogre::StringConverter::parseUnsignedInt(
						cf.getSetting("Quit", "Joystick", "3"));
}

/**
	Updates the input state of the joystick.
*/
void UmrJoystickInput::processInput(float timeStep)
{
    HRESULT hr;

	if (m_pJoystick == NULL) return;

	// Poll the device to read the current state
	hr = m_pJoystick->Poll();
	if(FAILED(hr))
	{
		// DInput is telling us that the input stream has been
		// interrupted. We aren't tracking any state between polls,
		// so we don't have any special reset that needs to be done.
		// We just re-acquire and try again.
		hr = m_pJoystick->Acquire();
		while(hr == DIERR_INPUTLOST)
			hr = m_pJoystick->Acquire();

		// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
		// may occur when the app is minimized or in the process of 
		// switching, so just try again later 
		return;
	}

	memcpy(&m_lastState, &m_state, sizeof(DIJOYSTATE2));
	// Get the input's device state
	if(FAILED(hr = m_pJoystick->GetDeviceState(
						sizeof(DIJOYSTATE2), &m_state)))
		// The device should have been acquired during the Poll()
		return;
	m_quit = m_state.rgbButtons[m_keys.quit] != 0;
}

/**
	Updates the state of the controller.
*/
bool UmrJoystickInput::updateState(float timeStep)
{
	if (m_pControllerStatus == NULL) return false;

	processInput(timeStep);
	
	if (m_quit) return true;

	float inputX = float(m_state.lX) * 0.001f;
	// Map acceleration to positive values and braking to negative 
	// values since they come from different joystick axis
	float accel = float(m_state.lY-1000) * 0.0005f;
	float brake = float(m_state.lRz-1000) * -0.0005f;
	float inputY = accel + brake;

	m_inputXPosition = inputX;
	m_inputYPosition = inputY;
	m_pControllerStatus->m_positionX = m_inputXPosition;
	m_pControllerStatus->m_positionY = m_inputYPosition;
	m_pControllerStatus->m_handbrakeButtonPressed = 
		m_state.rgbButtons[m_keys.handbrake] != 0;
	// Detect single keystroke
	if (m_lastState.rgbButtons[m_keys.train] != 0 && 
		 m_state.rgbButtons[m_keys.train] == 0)
		m_trainNNetwork = !m_trainNNetwork;

	return false;
}
