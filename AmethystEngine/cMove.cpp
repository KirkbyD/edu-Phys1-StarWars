#include "cMove.hpp"
#include <glm/gtx/easing.hpp>

void cMove::Init(sData data) {
	this->_subsystem = data.getSubsystem();
	this->_target = data.getTarget();
	this->_end_position = glm::vec3(data.getVec4Data()[0]);
	if (data.getVec4Data().size() == 2) { _isRelative = false; }
	if (!_isRelative) { _start_position = glm::vec3(data.getVec4Data()[1]); }
	this->_time = data.getFloatData()[0];
	if (data.getIsEaseInOut()) { this->_easeOut = data.getFloatData()[1]; this->_easeOut = data.getFloatData()[2]; }
	else if (data.getIsEaseIn()) { this->_easeIn = data.getFloatData()[1]; }
	else if (data.getIsEaseOut()) { this->_easeOut = data.getFloatData()[1]; }
	test = 0.0f;
}

bool cMove::Update(double deltatime) {
	cCommandManager* pcmdmngr = cCommandManager::GetCommandManager();

	// check to see if this is the first time this command was called for this object
	if (!this->_UpdateHasBeenCalled) {
		if (this->_isRelative) {
			sData data;
			data.setSubsystem(_subsystem);
			data.setCMD(GET_POSITION);
			data.setTarget(_target);
			pcmdmngr->IssueCommand(data);
			this->_start_position = glm::vec3(data.getVec4Data()[0].x, data.getVec4Data()[0].y, data.getVec4Data()[0].z);
		}

		// need to set the position of the model selected to the starting position...

		// Calculate the velocity...
		glm::vec3 moveVector = this->_end_position - this->_start_position;
		this->_total_distance = glm::length(moveVector);
		float speed = this->_total_distance / this->_time;
		glm::vec3 direction = glm::normalize(moveVector);
		this->_velocity = direction * speed;

		// set the velocity on the model
		sData data;
		data.setSubsystem(MODELS);
		data.setCMD(SET_VELOCITY);
		data.setTarget(_target);
		data.setVec4Data(_velocity);
		pcmdmngr->IssueCommand(data);

		this->_UpdateHasBeenCalled = true;
	}

	// call mediator to get the position of the object.
	sData data;
	data.setSubsystem(_subsystem);
	data.setCMD(GET_POSITION);
	data.setTarget(_target);
	pcmdmngr->IssueCommand(data);

	_current_pos = glm::vec3(data.getVec4Data()[0].x, data.getVec4Data()[0].y, data.getVec4Data()[0].z);

	// do ease-in and ease-out checks here...
	// make a check for distance based or time based...
	// retrieve the current distance to check for ease-in / ease-out
	// retrieve the current time to check for ease-in / ease-out
	/*
		DISTANCE CALCULATION!!!!
		edge0 = 0 / totaldistance
		edge1 = in/out distance%
		x = currentdistance

		TIME CALCULATION!!!
		edge0 = 0 / totaltime
		edge1 = easein/easeout time
		x = currenttime
	*/


	float ease_in_out = 1.0f;
	float current_distance = glm::distance(this->_current_pos, this->_start_position);
	float start_distance = glm::distance(this->_current_pos, this->_start_position);
	float current_time;


	// for testing remove later
	_easeIn = 0.15f;
	_easeOut = 0.0f;
	//

	float easein_percent = _easeIn * _total_distance;
	if (current_distance < easein_percent)
		test += glm::quadraticEaseIn(_easeIn);
	glm::vec3 deltaStep = this->_velocity * (float)deltatime * test;

	// This could be done in the physics engine...
	this->_current_pos += deltaStep;
	

	// call the physics engine to do the work; check to see if the distance is greater or equal to the total distance
	float distance = glm::distance(this->_current_pos, this->_start_position);

	// set the position on the model
	data.setCMD(SET_POSITION);
	data.setVec4Data(_current_pos);
	pcmdmngr->IssueCommand(data);

	if (distance >= this->_total_distance)
		return true;

	return false;
}
