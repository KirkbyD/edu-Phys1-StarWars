#include "GLCommon.hpp"
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "cCallbacks.hpp"
#include "cCommandManager.hpp"
#include "Globals.hpp"

#include <stdio.h>
#include <iostream>
#include "cPhysicsManager.hpp"

extern GLFWwindow* _text_window;
extern GLFWwindow* _window;

bool isShiftKeyDownByAlone(int mods); 
bool isCtrlKeyDownByAlone(int mods);
float devSpeed = 180.0f;
float maxSpeed = 2.0;

cAudioManager* pAudioManager = cAudioManager::GetAudioManager();
cPhysicsManager* pPhysicsManager = cPhysicsManager::GetPhysicsManager();

// ADDITIONALLY CHANGE THE DEVELOPER CONSOLE TO USE A SECONDARY WINDOW WITH THE STUFF FROM OSCARS CLASS
// FUCNTION FOR ACTUALLY ISSUING THE COMMAND (THIS IS ESSENTIALLY THE MEDIATOR)
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	cCommandManager* pCmdMngr = cCommandManager::GetCommandManager();
	float accelSpeed = 180.0f * deltaTime;
	float rotationSpeed = 1.0f * deltaTime; //assuming 60 fps, move 3 unit a second
	float devMoveSpeed = devSpeed * deltaTime;

	// BECAUSE COMPLICATED MESSY IF/ELSE CRAP BELOW F KEY TO TURN ON/OFF/TOGGLE LIGHTS IS PLACED HERE
	if (!isDevCon && (key == GLFW_KEY_F && action == GLFW_PRESS)) {
		pCmdMngr->IssueCommand("light.visible toggle");
	}

	// OBJECT SELECTION USING THE ARROW KEYS
	if ((key == GLFW_KEY_RIGHT || (key == GLFW_KEY_RIGHT && isDevCon)) && action == GLFW_PRESS) {
		if (isModelSelect) {
			++itModelObjects;
			if (itModelObjects == mpModel.end())
				itModelObjects = mpModel.begin();

			mainCamera.SetObject((*itModelObjects).second);
			if ((*itModelObjects).second->scale < 1.0f)
				mainCamera.SetFollowDistance(1.0f);
			else
				mainCamera.SetFollowDistance((*itModelObjects).second->scale);
		}

		else if (isLightSelect) {
			if (vpLightObjectsCurrentPos == g_vec_pLightObjects.size() - 1) {
				vpLightObjectsCurrentPos = 0;
			}
			else {
				++vpLightObjectsCurrentPos;
			}
			mainCamera.SetObject(g_vec_pLightObjects[vpLightObjectsCurrentPos]);
			mainCamera.SetFollowDistance(0);
		}

		else if (isComplexSelect) {
			if (vpComplexObjectsCurrentPos == g_vec_pComplexObjects.size() - 1) {
				vpComplexObjectsCurrentPos = 0;
			}
			else {
				++vpComplexObjectsCurrentPos;
			}
			mainCamera.SetObject(g_vec_pComplexObjects[vpComplexObjectsCurrentPos]);
			mainCamera.SetFollowDistance(15.0f);
		}

		else if (isAudioSelect) {
			++itChannelGroup;
			if (itChannelGroup == mpChannelGoups.end())
				itChannelGroup = mpChannelGoups.begin();
		}
	}
	if ((key == GLFW_KEY_LEFT || (key == GLFW_KEY_LEFT && isDevCon)) && action == GLFW_PRESS) {
		if (isModelSelect) {
			if (itModelObjects == mpModel.begin())
				itModelObjects = mpModel.end();
			--itModelObjects;

			mainCamera.SetObject((*itModelObjects).second);
			if ((*itModelObjects).second->scale < 1.0f)
				mainCamera.SetFollowDistance(1.0f);
			else
				mainCamera.SetFollowDistance((*itModelObjects).second->scale);
		}

		else if (isLightSelect) {
			if (vpLightObjectsCurrentPos == 0) {
				vpLightObjectsCurrentPos = g_vec_pLightObjects.size() - 1;
			}
			else {
				--vpLightObjectsCurrentPos;
			}
			mainCamera.SetObject(g_vec_pLightObjects[vpLightObjectsCurrentPos]);
			mainCamera.SetFollowDistance(0);
		}

		else if (isComplexSelect) {
			if (vpComplexObjectsCurrentPos == g_vec_pComplexObjects.size() - 1) {
				vpComplexObjectsCurrentPos = 0;
			}
			else {
				++vpComplexObjectsCurrentPos;
			}
			mainCamera.SetObject(g_vec_pComplexObjects[vpComplexObjectsCurrentPos]);
			mainCamera.SetFollowDistance(15.0f);
		}
		
		else if (isAudioSelect) {
			if (itChannelGroup == mpChannelGoups.begin())
				itChannelGroup = mpChannelGoups.end();
			--itChannelGroup;
		}
	}

	// DEVELOPER CONSOLE ACTIVATION CHECK
	// THIS APPLIES TO BOTH WINDOWS NOW SO BE CAREFUL
	if (key == GLFW_KEY_GRAVE_ACCENT && action == GLFW_PRESS) {
		if (!isDevCon) {
			isDevCon = true;
			glfwSetCharCallback(_text_window, character_callback);
			glfwShowWindow(_text_window);
		}
		else {
			isDevCon = false;
			glfwSetCharCallback(_text_window, NULL);
			glfwHideWindow(_text_window);
		}
	}

	// DEVELOPER CONSOLE ENTER CHARACTER
	if (isDevCon && key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
		pCmdMngr->IssueCommand(cmdstr);
		cmdstr = "";
	}

	// DEVELOPER CONSOLE DELETE CHARACTER
	if (isDevCon && key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS || action == GLFW_REPEAT) {
		if (!cmdstr.empty()) {
			cmdstr.pop_back();
		}
	}
	
	if (!isDevCon && !isShiftKeyDownByAlone(mods) && !isCtrlKeyDownByAlone(mods) && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		// SWITCH THE + AND - FROM THE CAMERA SPEEDS TO THE OPPOSITE TO MAKE THE DIRECTION OF THE ROTATION NORMAL FEELING
		// Move the camera (A & D for left and right, along the x axis)
		// Working on creation of a first person camera with mouse motion.
		if (isObjCtrl) {
			if (isModelSelect) {
				if (key == GLFW_KEY_A) {
					(*itModelObjects).second->velocity.x += mainCamera.GetRight().x * accelSpeed;
					(*itModelObjects).second->velocity.z += mainCamera.GetRight().z * accelSpeed;
				}
				if (key == GLFW_KEY_D) {
					(*itModelObjects).second->velocity.x -= mainCamera.GetRight().x * accelSpeed;
					(*itModelObjects).second->velocity.z -= mainCamera.GetRight().z * accelSpeed;
				}
				if (key == GLFW_KEY_SPACE) {
					//clamp 'jumping'
					if ((*itModelObjects).second->velocity.y < 1.0f
						&& (*itModelObjects).second->positionXYZ.y < 1.5 * (*itModelObjects).second->scale) {
						(*itModelObjects).second->velocity.y += 10.0f;
						if (mpAudio.find("Jump") != mpAudio.end()) {
							//AudioIttem functions are self errorchecking
							mpAudio["Jump"]->Move((*itModelObjects).second->positionXYZ, glm::vec3(0.0f, 0.0f, 0.0f));

							FMOD::Channel* chnnl = mpAudio["Jump"]->GetChannel();
							pAudioManager->_result = pAudioManager->_system->playSound(mpAudio["Jump"]->GetSound(),
								0,
								false,
								&chnnl);
							pAudioManager->error_check();
							pAudioManager->_result = chnnl->setPaused(false);
							pAudioManager->error_check();
						}
					}
				}
				if (key == GLFW_KEY_W) {
					(*itModelObjects).second->velocity.x += mainCamera.GetFront().x * accelSpeed;
					(*itModelObjects).second->velocity.z += mainCamera.GetFront().z * accelSpeed;
				}
				if (key == GLFW_KEY_S) {
					(*itModelObjects).second->velocity.x -= mainCamera.GetFront().x * accelSpeed;
					(*itModelObjects).second->velocity.z -= mainCamera.GetFront().z * accelSpeed;
				}

				//clamps x y movement?
			/*	if ((*itModelObjects).second->velocity.x > maxSpeed)
					(*itModelObjects).second->velocity.x = maxSpeed;
				else if ((*itModelObjects).second->velocity.x < -maxSpeed)
					(*itModelObjects).second->velocity.x = -maxSpeed;

				if ((*itModelObjects).second->velocity.z > maxSpeed)
					(*itModelObjects).second->velocity.z = maxSpeed;
				else if ((*itModelObjects).second->velocity.z > -maxSpeed)
					(*itModelObjects).second->velocity.z = -maxSpeed;*/

			}
			else if (isComplexSelect) {
				glm::vec3 velocity = g_vec_pComplexObjects[vpComplexObjectsCurrentPos]->GetVelocity();
				if (key == GLFW_KEY_A) {
					g_vec_pComplexObjects[vpComplexObjectsCurrentPos]->SetVelocity(glm::vec3(velocity.x + mainCamera.GetRight().x * accelSpeed,
						velocity.y,
						velocity.z + mainCamera.GetRight().z * accelSpeed));
				}
				if (key == GLFW_KEY_D) {
					g_vec_pComplexObjects[vpComplexObjectsCurrentPos]->SetVelocity(glm::vec3(velocity.x - mainCamera.GetRight().x * accelSpeed,
						velocity.y,
						velocity.z - mainCamera.GetRight().z * accelSpeed));
				}
				if (key == GLFW_KEY_SPACE)
					g_vec_pComplexObjects[vpComplexObjectsCurrentPos]->SetVelocity(velocity * 0.5f);
				if (key == GLFW_KEY_W) {
					g_vec_pComplexObjects[vpComplexObjectsCurrentPos]->SetVelocity(glm::vec3(velocity.x + mainCamera.GetFront().x * accelSpeed,
						velocity.y + mainCamera.GetFront().y * accelSpeed,
						velocity.z + mainCamera.GetFront().z * accelSpeed));
				}
				if (key == GLFW_KEY_S) {
					g_vec_pComplexObjects[vpComplexObjectsCurrentPos]->SetVelocity(glm::vec3(velocity.x - mainCamera.GetFront().x * accelSpeed,
						velocity.y - mainCamera.GetFront().y * accelSpeed,
						velocity.z - mainCamera.GetFront().z * accelSpeed));
				}
				if (key == GLFW_KEY_Q) {
					g_vec_pComplexObjects[vpComplexObjectsCurrentPos]->UpdateRoll(1.0f);
				}
				if (key == GLFW_KEY_E) {
					g_vec_pComplexObjects[vpComplexObjectsCurrentPos]->UpdateRoll(-1.0f);
				}
			}
			//else if (isAudioSelect) {
			//	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
			//		if (g_vec_DSP.size() > 0) {
			//			g_vec_DSP[0].bypass = !g_vec_DSP[0].bypass;
			//			pAudioManager->_result = g_vec_DSP[0].dsp->setBypass(g_vec_DSP[0].bypass);
			//			pAudioManager->error_check();
			//		}
			//	}
			//	if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
			//		if (g_vec_DSP.size() > 1) {
			//			g_vec_DSP[1].bypass = !g_vec_DSP[1].bypass;
			//			pAudioManager->_result = g_vec_DSP[1].dsp->setBypass(g_vec_DSP[1].bypass);
			//			pAudioManager->error_check();
			//		}
			//	}
			//	if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
			//		if (g_vec_DSP.size() > 2) {
			//			g_vec_DSP[2].bypass = !g_vec_DSP[2].bypass;
			//			pAudioManager->_result = g_vec_DSP[2].dsp->setBypass(g_vec_DSP[2].bypass);
			//			pAudioManager->error_check();
			//		}
			//	}
			//	if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
			//		if (g_vec_DSP.size() > 3) {
			//			g_vec_DSP[3].bypass = !g_vec_DSP[3].bypass;
			//			pAudioManager->_result = g_vec_DSP[3].dsp->setBypass(g_vec_DSP[3].bypass);
			//			pAudioManager->error_check();
			//		}
			//	}
			//	if (key == GLFW_KEY_5 && action == GLFW_PRESS) {
			//		if (g_vec_DSP.size() > 4) {
			//			g_vec_DSP[4].bypass = !g_vec_DSP[4].bypass;
			//			pAudioManager->_result = g_vec_DSP[4].dsp->setBypass(g_vec_DSP[4].bypass);
			//			pAudioManager->error_check();
			//		}
			//	}
			//	if (key == GLFW_KEY_6 && action == GLFW_PRESS) {
			//		if (g_vec_DSP.size() > 5) {
			//			g_vec_DSP[5].bypass = !g_vec_DSP[5].bypass;
			//			pAudioManager->_result = g_vec_DSP[5].dsp->setBypass(g_vec_DSP[5].bypass);
			//			pAudioManager->error_check();
			//		}
			//	}
			//	if (key == GLFW_KEY_7 && action == GLFW_PRESS) {
			//		if (g_vec_DSP.size() > 6) {
			//			g_vec_DSP[6].bypass = !g_vec_DSP[6].bypass;
			//			pAudioManager->_result = g_vec_DSP[6].dsp->setBypass(g_vec_DSP[6].bypass);
			//			pAudioManager->error_check();
			//		}
			//	}
			//	if (key == GLFW_KEY_8 && action == GLFW_PRESS) {
			//		if (g_vec_DSP.size() > 7) {
			//			g_vec_DSP[7].bypass = !g_vec_DSP[7].bypass;
			//			pAudioManager->_result = g_vec_DSP[7].dsp->setBypass(g_vec_DSP[7].bypass);
			//			pAudioManager->error_check();
			//		}
			//	}
			//	if (key == GLFW_KEY_9 && action == GLFW_PRESS) {
			//		if (g_vec_DSP.size() > 8) {
			//			g_vec_DSP[8].bypass = !g_vec_DSP[8].bypass;
			//			pAudioManager->_result = g_vec_DSP[8].dsp->setBypass(g_vec_DSP[8].bypass);
			//			pAudioManager->error_check();
			//		}
			//	}

			//	if ((key == GLFW_KEY_KP_0 || key == GLFW_KEY_F1) && action == GLFW_PRESS) {
			//		bool tmp = false;
			//		itChannelGroup->second->getPaused(&tmp);
			//		itChannelGroup->second->setPaused(!tmp);

			//		//done for sounds that are started paused, because channel group doesn't unpause them???
			//		for (size_t i = 0; i < 3; i++) {
			//			FMOD::Channel* moretmp;
			//			itChannelGroup->second->getChannel(i, &moretmp);
			//			moretmp->setPaused(!tmp);
			//		}
			//	}
			//	if ((key == GLFW_KEY_KP_1 || key == GLFW_KEY_F2) && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			//		float tmp = 0.0f;
			//		itChannelGroup->second->getVolume(&tmp);
			//		tmp = (tmp >= 1.0f) ? 1.0f : tmp + 0.01;
			//		itChannelGroup->second->setVolume(tmp);
			//	}
			//	if ((key == GLFW_KEY_KP_2 || key == GLFW_KEY_F3) && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			//		float tmp = 0.0f;
			//		itChannelGroup->second->getVolume(&tmp);
			//		tmp = (tmp <= 0.0f) ? 0.0f : tmp - 0.01;
			//		itChannelGroup->second->setVolume(tmp);
			//	}
			//	if ((key == GLFW_KEY_KP_4 || key == GLFW_KEY_F4) && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			//		float tmp = 0.0f;
			//		itChannelGroup->second->getPitch(&tmp);
			//		tmp = (tmp >= 10.0f) ? 10.0f : tmp + 0.01;
			//		itChannelGroup->second->setPitch(tmp);
			//	}
			//	if ((key == GLFW_KEY_KP_5 || key == GLFW_KEY_F5) && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			//		float tmp = 0.0f;
			//		itChannelGroup->second->getPitch(&tmp);
			//		tmp = (tmp <= 0.0f) ? 0.0f : tmp - 0.01;
			//		itChannelGroup->second->setPitch(tmp);
			//	}
			//	if ((key == GLFW_KEY_KP_7 || key == GLFW_KEY_F6) && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			//		std::map<std::string, cAudioObject*>::iterator itAudioItem;
			//		for (itAudioItem = mpAudio.begin();
			//			itAudioItem != mpAudio.end();
			//			++itAudioItem) {
			//			if (itAudioItem->second->GetChannelGroup() == itChannelGroup->second) {
			//				break;
			//			}
			//		}
			//		itAudioItem->second->SetPan((itAudioItem->second->GetPan() <= -1.0f) ? -1.0f : itAudioItem->second->GetPan() - 0.05);
			//		itChannelGroup->second->setPan(itAudioItem->second->GetPan());
			//	}
			//	if ((key == GLFW_KEY_KP_8 || key == GLFW_KEY_F7) && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			//		std::map<std::string, cAudioObject*>::iterator itAudioItem;
			//		for (itAudioItem = mpAudio.begin();
			//			itAudioItem != mpAudio.end();
			//			itAudioItem++) {
			//			if (itAudioItem->second->GetChannelGroup() == itChannelGroup->second) {
			//				break;
			//			}
			//		}

			//		itAudioItem->second->SetPan((itAudioItem->second->GetPan() >= 1.0f) ? 1.0f : itAudioItem->second->GetPan() + 0.05);
			//		itChannelGroup->second->setPan(itAudioItem->second->GetPan());
			//	}

			//}
		}
		else if(!isObjCtrl) {
			if (useFreeCam) {
				if (key == GLFW_KEY_W)
					FreeCamera.MoveForBack(devSpeed * deltaTime);
				if (key == GLFW_KEY_S)
					FreeCamera.MoveForBack(-devSpeed * deltaTime);
				if (key == GLFW_KEY_A)
					FreeCamera.MoveStrafe(devSpeed * deltaTime);
				if (key == GLFW_KEY_D)
					FreeCamera.MoveStrafe(-devSpeed * deltaTime);
				if (key == GLFW_KEY_Q)
					FreeCamera.MoveUpDown(devSpeed * deltaTime);
				if (key == GLFW_KEY_E)
					FreeCamera.MoveUpDown(-devSpeed * deltaTime);
			}
		}
	}

	if (isDevCon && !isShiftKeyDownByAlone(mods) && isCtrlKeyDownByAlone(mods) && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		// Position
		if (key == GLFW_KEY_PERIOD) {
			devSpeed *= 1.1;
		}
		if (key == GLFW_KEY_COMMA) {
			devSpeed *= 0.9;
		}
		if (isModelSelect) {
			if (key == GLFW_KEY_A)
				(*itModelObjects).second->positionXYZ += mainCamera.GetRight() * devMoveSpeed;
			if (key == GLFW_KEY_D) 
				(*itModelObjects).second->positionXYZ -= mainCamera.GetRight() * devMoveSpeed;
			if (key == GLFW_KEY_Q)
				(*itModelObjects).second->positionXYZ += mainCamera.GetUp() * devMoveSpeed;
			if (key == GLFW_KEY_E)
				(*itModelObjects).second->positionXYZ -= mainCamera.GetUp() * devMoveSpeed;
			if (key == GLFW_KEY_W)
				(*itModelObjects).second->positionXYZ += mainCamera.GetFront() * devMoveSpeed;
			if (key == GLFW_KEY_S)
				(*itModelObjects).second->positionXYZ -= mainCamera.GetFront() * devMoveSpeed;
			if (key == GLFW_KEY_R)
				(*itModelObjects).second->updateOrientation(glm::vec3(0.0f, -1.0f, 0.0f));
			if (key == GLFW_KEY_T)
				(*itModelObjects).second->updateOrientation(glm::vec3(0.0f, 1.0f, 0.0f));
		}
		if (isLightSelect) {
			if (key == GLFW_KEY_A) {
				g_vec_pLightObjects[vpLightObjectsCurrentPos]->position.x += mainCamera.GetRight().x * devMoveSpeed;
				g_vec_pLightObjects[vpLightObjectsCurrentPos]->position.y += mainCamera.GetRight().y * devMoveSpeed;
				g_vec_pLightObjects[vpLightObjectsCurrentPos]->position.z += mainCamera.GetRight().z * devMoveSpeed;
			}
			if (key == GLFW_KEY_D) {
				g_vec_pLightObjects[vpLightObjectsCurrentPos]->position.x -= mainCamera.GetRight().x * devMoveSpeed;
				g_vec_pLightObjects[vpLightObjectsCurrentPos]->position.y -= mainCamera.GetRight().y * devMoveSpeed;
				g_vec_pLightObjects[vpLightObjectsCurrentPos]->position.z -= mainCamera.GetRight().z * devMoveSpeed;
			}
			if (key == GLFW_KEY_Q) {
				g_vec_pLightObjects[vpLightObjectsCurrentPos]->position.x += mainCamera.GetUp().x * devMoveSpeed;
				g_vec_pLightObjects[vpLightObjectsCurrentPos]->position.y += mainCamera.GetUp().y * devMoveSpeed;
				g_vec_pLightObjects[vpLightObjectsCurrentPos]->position.z += mainCamera.GetUp().z * devMoveSpeed;
			}
			if (key == GLFW_KEY_E) {
				g_vec_pLightObjects[vpLightObjectsCurrentPos]->position.x -= mainCamera.GetUp().x * devMoveSpeed;
				g_vec_pLightObjects[vpLightObjectsCurrentPos]->position.y -= mainCamera.GetUp().y * devMoveSpeed;
				g_vec_pLightObjects[vpLightObjectsCurrentPos]->position.z -= mainCamera.GetUp().z * devMoveSpeed;
			}
			if (key == GLFW_KEY_W) {
				g_vec_pLightObjects[vpLightObjectsCurrentPos]->position.x += mainCamera.GetFront().x * devMoveSpeed;
				g_vec_pLightObjects[vpLightObjectsCurrentPos]->position.y += mainCamera.GetFront().y * devMoveSpeed;
				g_vec_pLightObjects[vpLightObjectsCurrentPos]->position.z += mainCamera.GetFront().z * devMoveSpeed;
			}
			if (key == GLFW_KEY_S) {
				g_vec_pLightObjects[vpLightObjectsCurrentPos]->position.x -= mainCamera.GetFront().x * devMoveSpeed;
				g_vec_pLightObjects[vpLightObjectsCurrentPos]->position.y -= mainCamera.GetFront().y * devMoveSpeed;
				g_vec_pLightObjects[vpLightObjectsCurrentPos]->position.z -= mainCamera.GetFront().z * devMoveSpeed;
			}
			if (key == GLFW_KEY_1)
				g_vec_pLightObjects[vpLightObjectsCurrentPos]->atten.x *= 0.99f;			// 99% of what it was
			if (key == GLFW_KEY_2)
				g_vec_pLightObjects[vpLightObjectsCurrentPos]->atten.x *= 1.01f;			// 1% more of what it was
			if (key == GLFW_KEY_3)
				g_vec_pLightObjects[vpLightObjectsCurrentPos]->atten.y *= 0.99f;			// 99% of what it was
			if (key == GLFW_KEY_4)
				g_vec_pLightObjects[vpLightObjectsCurrentPos]->atten.y *= 1.01f;			// 1% more of what it was
			if (key == GLFW_KEY_5)
				g_vec_pLightObjects[vpLightObjectsCurrentPos]->atten.z *= 0.99f;			// 99% of what it was
			if (key == GLFW_KEY_6)
				g_vec_pLightObjects[vpLightObjectsCurrentPos]->atten.z *= 1.01f;			// 1% more of what it was
			if (key == GLFW_KEY_V)
				g_vec_pLightObjects[vpLightObjectsCurrentPos]->param1.y -= 0.1f;
			if (key == GLFW_KEY_B)
				g_vec_pLightObjects[vpLightObjectsCurrentPos]->param1.y += 0.1f;
			if (key == GLFW_KEY_N)
				g_vec_pLightObjects[vpLightObjectsCurrentPos]->param1.z -= 0.1f;
			if (key == GLFW_KEY_M)
				g_vec_pLightObjects[vpLightObjectsCurrentPos]->param1.z += 0.1f;
		}
		if (isComplexSelect) {
			if (key == GLFW_KEY_A)
				g_vec_pComplexObjects[vpComplexObjectsCurrentPos]->SetPosition(	g_vec_pComplexObjects[vpComplexObjectsCurrentPos]->GetPosition()
																				+ mainCamera.GetRight() * devMoveSpeed);
			if (key == GLFW_KEY_D)
				g_vec_pComplexObjects[vpComplexObjectsCurrentPos]->SetPosition(	g_vec_pComplexObjects[vpComplexObjectsCurrentPos]->GetPosition()
																				- mainCamera.GetRight() * devMoveSpeed);
			if (key == GLFW_KEY_Q)
				g_vec_pComplexObjects[vpComplexObjectsCurrentPos]->SetPosition(	g_vec_pComplexObjects[vpComplexObjectsCurrentPos]->GetPosition()
																				+ mainCamera.GetUp() * devMoveSpeed);
			if (key == GLFW_KEY_E)
				g_vec_pComplexObjects[vpComplexObjectsCurrentPos]->SetPosition(	g_vec_pComplexObjects[vpComplexObjectsCurrentPos]->GetPosition()
																				- mainCamera.GetUp() * devMoveSpeed);
			if (key == GLFW_KEY_W)
				g_vec_pComplexObjects[vpComplexObjectsCurrentPos]->SetPosition(	g_vec_pComplexObjects[vpComplexObjectsCurrentPos]->GetPosition()
																				+ mainCamera.GetFront() * devMoveSpeed);
			if (key == GLFW_KEY_S)
				g_vec_pComplexObjects[vpComplexObjectsCurrentPos]->SetPosition(	g_vec_pComplexObjects[vpComplexObjectsCurrentPos]->GetPosition()
																				- mainCamera.GetFront() * devMoveSpeed);
		}
	}

	if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		pPhysicsManager->FINALXWingPath();
	}

	// Swap iGameObject* selection vectors
	if (key == GLFW_KEY_LEFT_BRACKET) {
		if (g_vec_pGameObjects.size() != 0) {
			isModelSelect = true;
			isLightSelect = false;
			isComplexSelect = false;
			isAudioSelect = false;
			useFreeCam = false;
			isObjCtrl = true;

			mainCamera.SetObject((*itModelObjects).second);
			mainCamera.SetFollowDistance((*itModelObjects).second->scale);
		}
	}
	if (key == GLFW_KEY_RIGHT_BRACKET) {
		if (g_vec_pLightObjects.size() != 0) {
			isModelSelect = false;
			isLightSelect = true;
			isComplexSelect = false;
			isAudioSelect = false;
			useFreeCam = false;
			isObjCtrl = true;

			mainCamera.SetObject(g_vec_pLightObjects[vpLightObjectsCurrentPos]);
			mainCamera.SetFollowDistance(0);
		}
	}
	if (key == GLFW_KEY_BACKSLASH) {
		if (g_vec_pComplexObjects.size() != 0) {
			isModelSelect = false;
			isLightSelect = false;
			isComplexSelect = true;
			isAudioSelect = false;
			useFreeCam = false;
			isObjCtrl = true;

			mainCamera.SetObject(g_vec_pComplexObjects[vpComplexObjectsCurrentPos]);
			mainCamera.SetFollowDistance(15.0f);
		}
	}
	if (key == GLFW_KEY_EQUAL) {
		if (!mpChannelGoups.empty()) {
			isModelSelect = false;
			isLightSelect = false;
			isComplexSelect = false;
			isAudioSelect = true;
			useFreeCam = false;
			isObjCtrl = true;
		}
	}

	if (key == GLFW_KEY_P) {
		useFreeCam = true;
		isObjCtrl = false;

		FreeCamera.SetEye(mainCamera.GetEye());
	}

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS && !isDevCon)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	// Don't Quit but leave devcon mode
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS && isDevCon) {
		if (!isDevCon) {
			isDevCon = true;
			glfwSetCharCallback(_text_window, character_callback);
			glfwShowWindow(_text_window);
			glfwMakeContextCurrent(_text_window);
		}
		else {
			isDevCon = false;
			glfwSetCharCallback(_text_window, NULL);
			glfwHideWindow(_text_window);
			glfwMakeContextCurrent(_window);
		}
	}
}

bool isShiftKeyDownByAlone(int mods) {
	if (mods == GLFW_MOD_SHIFT)
		return true;
	return false;
}

bool isCtrlKeyDownByAlone(int mods) {
	if (mods == GLFW_MOD_CONTROL)
		return true;
	return false;
}

void character_callback(GLFWwindow* window, unsigned codepoint) {
	// STUFF FOR THE DEVELOPER CONSOLE
	if ((char)codepoint != '`')
		cmdstr.push_back((char)codepoint);
}