#pragma once

#include "UmrAIControl.h"

/**
	Slow AI Control class.
	AI Control version that uses a neural 
	network trained with low speed driving data.
*/
__declspec(align(MEM_ALIGNMENT))
class UmrAIControlSlow : public UmrAIControl
{
public:
	UmrAIControlSlow(UmrVehicle* vehicle);
	~UmrAIControlSlow(void) {}

	virtual bool updateState(float timeStep);
};
