#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include "sData.hpp"

class iCommand {
public:
	virtual ~iCommand() { };

	virtual void Init(sData data) = 0;
	virtual bool Update( double deltatime ) = 0;			// IsDone & Update now in the same function call, returns if it is done processing the update or not

	// for ease-in / ease-out
	virtual float SmootherStep(float edge0, float edge1, float x) = 0;
	virtual float Clamp(float x, float lowerlimit, float upperlimit) = 0;
	// Think about the implementation of the command groups and if they are to be a part of the commands or not...
	// virtual void AddCommandSerial(iCommand* pCommand) = 0;
	// virtual void AddCommandParallel( iCommand* pCommand ) = 0;
};
