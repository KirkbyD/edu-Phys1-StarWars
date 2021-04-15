#pragma once
#include "GLCommon.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "cModelObject.hpp"
#include "cVAOManager.hpp"
#include "cParticleEmitter.hpp"
#include "Globals.hpp"

class cRenderer {

	float StarDestroyerExplosionOffset = 0.0f;



public:
	cRenderer() { };
	~cRenderer() { };
	void Render3DObject(cModelObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager);
	void Render3DObjectStarDestroyer(cModelObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager);
	void RenderDebugObject();
	void RenderTextObject();
	void RenderSkyBoxObject(cModelObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager);
	void RenderImposterObject();
	void RenderParticleObject();
	void RenderInstanced3DObject();
	void RenderInstancedParticleObject(cModelObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager,
		cParticleEmitter* pParticleEmitter, GLuint particles_mat_model_buffer);
	void RenderInstancedImposterObject();

	void SetUpTextureBindingsForObject(cModelObject* pCurrentObject, GLint shaderProgID);
};