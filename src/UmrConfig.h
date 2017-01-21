#pragma once

#define _WIN32_WINNT 0x501
#define WIN32_LEAN_AND_MEAN

#define TITLE		"UMR"
#define RESOURCES	"./gamedata"
#define PLUGINS		"/config/plugins.cfg"
#define GRAPHICS	"/config/graphics.cfg"
#define LOG			"/umr.log"

#define NCARS		3

// Memory alignment for class storage
#define MEM_ALIGNMENT	16

#define TIME_STEP_MS	16
#define TIME_STEP_S		0.01667f

// Collision filters
#define CAR_FILTER		2
#define TRACK_FILTER	6
#define	OUTFIELD_FILTER	7
#define SECTOR_FILTER	8
#define RAY_FILTER		9

#define CONTROL_MANUAL		1
#define CONTROL_AI			2

// safe release/delete macros

// use this in classes with a release method
#define SAFE_RELEASE(p) \
	if (p) { \
	(p)->Release(); \
	(p) = 0; \
	} \

// use this do delete a pointer
#define SAFE_DELETE(p) \
	if(p) { \
	delete (p); \
	(p) = 0; \
	} \

// use this to delete an array
#define SAFE_DELETE_ARRAY(p)  \
	if(p) { \
	delete[] (p); \
	(p) = 0; \
	} \

// Smart pointers
#include <boost/shared_ptr.hpp>

#include <OgreConfigFile.h>
#include <OgreRoot.h>
#include <OgreSceneNode.h>
#include <OgreTimer.h>

#include "UmrObject.h"

typedef boost::shared_ptr<Ogre::Root> RootPtr;
typedef boost::shared_ptr<Ogre::Timer> TimerPtr;
