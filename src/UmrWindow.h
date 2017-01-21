#pragma once

#include "UmrConfig.h"
#include "resource.h"

#include <OgreCamera.h>
#include <OgreRenderWindow.h>
#include <OgreViewport.h>

using namespace Ogre;

/**
	Window class.
	Manages Ogre3D window creation, configuration and destruction.
*/
__declspec(align(MEM_ALIGNMENT))
class UmrWindow : public UmrObject
{
public:
	UmrWindow(RootPtr &root);
	~UmrWindow(void);

	bool			loadGraphicsConfiguration ();
	bool			initialize ();
	void			destroy ();
	const HWND		getWindowHandle () const;
	RenderWindow*	getRenderWindow ();
	const float		getLastFps() const;
	void			setViewport(Camera* camera);
protected:
	/// Root class for Ogre3D system
	RootPtr m_pRoot;

	/// Target rendering window
	RenderWindow* m_pWindow;

	/// Window title
	String m_sTitle;

	/// Window handle
	HWND m_hWnd;
};

typedef boost::shared_ptr<UmrWindow> UmrWindowPtr;
