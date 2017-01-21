#include "UmrWindow.h"

/**
	Constructor.
	@param
		root Reference to the Ogre3D root class.
*/
UmrWindow::UmrWindow(RootPtr &root) : m_pRoot(root)
{
	m_sTitle = Ogre::String(TITLE);
}

/**
	Destructor.
*/
UmrWindow::~UmrWindow(void)
{
}

/**
	Loads rendering options from a previously created
	configuration file or shows a dialog to choose the settings.
*/
bool UmrWindow::loadGraphicsConfiguration()
{
	if (!m_pRoot->restoreConfig()) {
		if (!m_pRoot->showConfigDialog()) {
			return false;
		}
	}
	return true;
}

/**
	Initializes the render window.
*/
bool UmrWindow::initialize()
{
	m_pWindow = m_pRoot->initialise(true, m_sTitle);

	// set window handle
	m_hWnd = FindWindow("OgreD3D9Wnd", TITLE);
	if (!IsWindow(m_hWnd))
		m_hWnd = FindWindow("OgreGLWindow", TITLE);
	if (!IsWindow(m_hWnd))
		return false;

	// set window icon
	char buf[1024];
	GetModuleFileName(0, (LPCH)&buf, 1024);
	HINSTANCE hInstance = GetModuleHandle(buf);
	HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
	if (hIcon) {
		SendMessage(m_hWnd, WM_SETICON, 1, (LPARAM)hIcon);
		SendMessage(m_hWnd, WM_SETICON, 0, (LPARAM)hIcon);
	}

	return true;
}

/**
	Destroys the render window.
*/
void UmrWindow::destroy()
{
	m_pWindow->removeAllViewports();
	m_pRoot->detachRenderTarget(m_pWindow);
}

/**
	Gets the window handle.
*/
const HWND UmrWindow::getWindowHandle() const
{
	return m_hWnd;
}

/**
	Gets a pointer to the render window.
*/
RenderWindow* UmrWindow::getRenderWindow()
{
	return m_pWindow;
}

/**
	Gets the number of frames per second (FPS).
*/
const float UmrWindow::getLastFps() const
{
	return m_pWindow->getLastFPS();
}

/**
	Sets the viewport for the render window.
	@param
		camera Pointer to the scene camera.
*/
void UmrWindow::setViewport(Ogre::Camera *camera)
{
	Viewport *vp = m_pWindow->addViewport(camera);
	vp->setBackgroundColour(ColourValue(0,0,0));
	// Alter the camera aspect ratio to match the viewport
	camera->setAspectRatio(
		Real(vp->getActualWidth()) / Real(vp->getActualHeight()));
}
