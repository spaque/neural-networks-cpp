#include "UmrAIControl.h"

/**
	Constructor.
*/
UmrAIControl::UmrAIControl(UmrVehicle* vehicle)
{
	m_pVehicle = vehicle;
	m_pControllerStatus = HK_NULL;
	m_active = true;
	m_trainNNetwork = false;

	m_pNNetwork1 = 
		MultilayerPerceptronPtr(new MultilayerPerceptron());
	m_pNNetwork2 = 
		MultilayerPerceptronPtr(new MultilayerPerceptron());
}

/**
	Destructor.
*/
UmrAIControl::~UmrAIControl(void)
{
	if (m_pControllerStatus != HK_NULL) 
		m_pControllerStatus->removeReference();
}

/**
	Sets the physics input controller.
*/
void UmrAIControl::setInputStatus(
	hkpVehicleDriverInputAnalogStatus *status)
{
	m_pControllerStatus = status;
	m_pControllerStatus->addReference();
	m_pControllerStatus->m_handbrakeButtonPressed = false;
	m_pControllerStatus->m_reverseButtonPressed = false;
}
