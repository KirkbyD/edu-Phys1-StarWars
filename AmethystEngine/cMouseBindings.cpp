#include "GLCommon.hpp"
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "cCallbacks.hpp"
#include "Globals.hpp"

#include <stdio.h>

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	//// Move the sphere to where the camera is and shoot the ball from there...

	//cGameObject* pTheBall = pFindObjectByFriendlyName("Sphere#1");

	//// What's the velocity
	//// Target - eye = direction
	//glm::vec3 direction = glm::normalize(cameraTarget - cameraEye);

	//float speed = 5.0f;

	//pTheBall->velocity = direction * speed;
	//pTheBall->positionXYZ = cameraEye;

	//return;
}


float mouseLastX = 320;
float mouseLastY = 240;

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) 
{
	float xoffset = xpos - mouseLastX;
	float yoffset = mouseLastY - ypos;
	mouseLastX = xpos;
	mouseLastY = ypos;

	float sensitivity = 0.05;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	if (useFreeCam) {
		FreeCamera.OffsetYaw(xoffset);
		FreeCamera.OffsetPitch(yoffset);

		FreeCamera.MoveFront();
	}
	else {
		mainCamera.OffsetYaw(xoffset);
		mainCamera.OffsetPitch(yoffset);

		mainCamera.MoveFront();

		if (isComplexSelect) {
			float roll = g_vec_pComplexObjects[vpComplexObjectsCurrentPos]->GetRoll();
			g_vec_pComplexObjects[vpComplexObjectsCurrentPos]->SetOrientation(glm::quatLookAt(-mainCamera.GetFront(), mainCamera.GetUpVector()));
			g_vec_pComplexObjects[vpComplexObjectsCurrentPos]->UpdateOrientation(glm::vec3(0.0f, 0.0f, roll));
		}
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) { }
