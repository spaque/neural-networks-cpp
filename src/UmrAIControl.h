#pragma once

#include "UmrConfig.h"
#include "UmrController.h"
#include "UmrVehicle.h"

#include <Flood/Utilities/Vector.h>
#include <Flood/MultilayerPerceptron/MultilayerPerceptron.h>

using namespace Flood;

typedef boost::shared_ptr<MultilayerPerceptron> 
MultilayerPerceptronPtr;

/**
	AI Control class.
	Uses two previously trained neural networks to provide
	input values in order to control the vehicle.
*/
__declspec(align(MEM_ALIGNMENT))
class UmrAIControl : public UmrController
{
public:
	UmrAIControl(UmrVehicle* vehicle);
	virtual ~UmrAIControl(void);

	virtual bool updateState(float timeStep) = 0;
	virtual void setInputStatus(
		hkpVehicleDriverInputAnalogStatus* status);
protected:
	/// Neural network for vehicle turning
	MultilayerPerceptronPtr m_pNNetwork1;

	/// Neural network for vehicle acceleration
	MultilayerPerceptronPtr m_pNNetwork2;

	/// Controlled vehicle
	UmrVehicle* m_pVehicle;
};

typedef boost::shared_ptr<UmrAIControl> UmrAIControlPtr;
