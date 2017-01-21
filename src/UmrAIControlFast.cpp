#include "UmrAIControlFast.h"

/**
	Constructor.
*/
UmrAIControlFast::UmrAIControlFast(UmrVehicle* vehicle)
	: UmrAIControl(vehicle)
{
	m_pNNetwork1->load(
		"./gamedata/vehicles/Razor/MultilayerPerceptron21.ini");
	m_pNNetwork2->load(
		"./gamedata/vehicles/Razor/MultilayerPerceptron22.ini");
}

/**
	Updates the state of the controller.
*/
bool UmrAIControlFast::updateState(float timeStep)
{
	Vector<double> input1(17);
	Vector<double> input2(17);
	Vector<double> output;
	int i, j;
	float speed = m_pVehicle->m_speed * 0.01f;

	// Set inputs for the first net
	input1[0] = m_inputYPosition;
	input1[1] = speed;
	for (i = 2, j = 0; j < NRAYS; i++, j++) {
		input1[i] = m_pVehicle->m_rays[j];
	}
	for (j = 0; j < NRAYS; i++, j++) {
		input1[i] = m_pVehicle->m_lastRays[j];
	}
	input1[16] = m_pVehicle->m_dirDiff;

	// Set inputs for the second net
	input2[0] = m_inputXPosition;
	input2[1] = m_inputYPosition;
	input2[2] = speed;
	for (i = 3, j = 0; j < NRAYS; i++, j++) {
		input2[i] = m_pVehicle->m_rays[j];
	}
	for (j = 0; j < NRAYS; i++, j++) {
		input2[i] = m_pVehicle->m_lastRays[j];
	}

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
