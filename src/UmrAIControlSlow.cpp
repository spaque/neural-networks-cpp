#include "UmrAIControlSlow.h"

/**
	Constructor.
*/
UmrAIControlSlow::UmrAIControlSlow(UmrVehicle* vehicle) 
	: UmrAIControl(vehicle)
{
	m_pNNetwork1->load(
		"./gamedata/vehicles/Razor/MultilayerPerceptron11.ini");
	m_pNNetwork2->load(
		"./gamedata/vehicles/Razor/MultilayerPerceptron12.ini");
}

/**
	Updates the state of the controller.
*/
bool UmrAIControlSlow::updateState(float timeStep)
{
	Vector<double> input1(4);
	Vector<double> input2(5);
	Vector<double> output;

	input1[0] = m_pVehicle->m_rays[0];
	input1[1] = m_pVehicle->m_rays[1];
	input1[2] = m_pVehicle->m_rays[3];
	input1[3] = m_pVehicle->m_rays[4];

	input2[0] = m_inputYPosition;
	input2[1] = m_pVehicle->m_speed * 0.01f;
	input2[2] = m_pVehicle->m_rays[2];
	input2[3] = m_pVehicle->m_lastRays[2];
	input2[4] = m_pVehicle->m_tilt;

	output = m_pNNetwork1->calculateOutput(input1);
	m_inputXPosition = output[0];
	output = m_pNNetwork2->calculateOutput(input2);
	m_inputYPosition = output[0];

	m_pControllerStatus->m_positionX = 
		hkMath::clamp(m_inputXPosition, -1.0f, 1.0f);
	m_pControllerStatus->m_positionY = 
		hkMath::clamp(m_inputYPosition, -1.0f, 1.0f);

	return false;
}
