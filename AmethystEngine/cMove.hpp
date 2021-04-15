#pragma once
#include "cCommandManager.hpp"

class cMove : public iCommand {
	bool _UpdateHasBeenCalled;
	bool _isRelative;
	size_t _subsystem;
	std::string _target;
	glm::vec3 _start_position;
	glm::vec3 _end_position;
	glm::vec3 _current_pos;
	glm::vec3 _velocity;
	float _time;
	float _total_distance;
	float _easeIn, _easeOut;
	float test;

public:
	// move time
	cMove() {
		_UpdateHasBeenCalled = false;
		_isRelative = true;
	}

	virtual void Init(sData data);
	virtual bool Update(double deltatime);
};
