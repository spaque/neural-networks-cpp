#pragma once

#include "UmrAIControl.h"

/**
	Fast AI Control class.
	AI Control version that uses a neural 
	network trained with high speed driving data.
*/
__declspec(align(MEM_ALIGNMENT))
class UmrAIControlFast : public UmrAIControl
{
public:
	UmrAIControlFast(UmrVehicle* vehicle);
	~UmrAIControlFast(void) {}

	virtual bool updateState(float timeStep);
};
