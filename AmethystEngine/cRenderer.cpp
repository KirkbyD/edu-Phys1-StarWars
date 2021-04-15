#include "cRenderer.hpp"

void cRenderer::Render3DObject(cModelObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager) {
	if (pCurrentObject->isVisible == false)
		return;

	glEnable(GL_BLEND);										// Turns on "alpha transparency"
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);		// Reads what's on the buffer already, and blends it with the incoming colour based on the "alpha" value, which is the 4th colour output RGB+A

	// This block of code, where I generate the world matrix from the position, scale, and rotation (orientation) of the object has been placed into calculateWorldMatrix()

	GLint explosionTime = glGetUniformLocation(shaderProgID, "explosionTime");
	glUniform1f(explosionTime, (float)GL_FALSE);

	// ************
	// Set the texture bindings and samplers
	SetUpTextureBindingsForObject(pCurrentObject, shaderProgID);

	GLint bInstance = glGetUniformLocation(shaderProgID, "bInstance");
	glUniform1f(bInstance, (float)GL_FALSE);

	GLint bIsSkyBox_UL = glGetUniformLocation(shaderProgID, "bIsSkyBox");
	glUniform1f(bIsSkyBox_UL, (float)GL_FALSE);

	// Don't draw back facing triangles (default)
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);		// Don't draw "back facing" triangles

	// If there is something to discard
	if (pCurrentObject->discardTextureName != "") {
		GLint bDiscard_UL = glGetUniformLocation(shaderProgID, "bDiscard");
		glUniform1f(bDiscard_UL, (float)GL_TRUE);

		GLuint texSamp_ID = mpTexture[pCurrentObject->discardTextureName];
		glActiveTexture(GL_TEXTURE13);				// Texture Unit 13
		glBindTexture(GL_TEXTURE_2D, texSamp_ID);	// Texture now assoc with texture unit 0

		GLint texture_UL = glGetUniformLocation(shaderProgID, "discardTexture");
		glUniform1i(texture_UL, 13);	// Texture unit 13
	}
	else {
		GLint bDiscard_UL = glGetUniformLocation(shaderProgID, "bDiscard");
		glUniform1f(bDiscard_UL, (float)GL_FALSE);
	}

	// Choose which shader to use
	glUseProgram(shaderProgID);

	GLint matModel_UL = glGetUniformLocation(shaderProgID, "matModel");
	glUniformMatrix4fv(matModel_UL, 1, GL_FALSE, glm::value_ptr(pCurrentObject->matWorld));

	// Calcualte the inverse transpose of the model matrix and pass that...
	// Stripping away scaling and translation, leaving only rotation
	// Because the normal is only a direction, really
	GLint matModelIT_UL = glGetUniformLocation(shaderProgID, "matModelInverseTranspose");
	glm::mat4 matModelInverseTranspose = glm::inverse(glm::transpose(pCurrentObject->matWorld));
	glUniformMatrix4fv(matModelIT_UL, 1, GL_FALSE, glm::value_ptr(matModelInverseTranspose));

	// Find the location of the uniform variable newColour
	GLint newColour_location = glGetUniformLocation(shaderProgID, "newColour");
	GLint diffuseColour_UL = glGetUniformLocation(shaderProgID, "diffuseColour");
	GLint specularColour_UL = glGetUniformLocation(shaderProgID, "specularColour");

	glUniform3f(newColour_location, pCurrentObject->objectColourRGBA.r, pCurrentObject->objectColourRGBA.g, pCurrentObject->objectColourRGBA.b);
	glUniform4f(diffuseColour_UL, pCurrentObject->objectColourRGBA.r, pCurrentObject->objectColourRGBA.g, pCurrentObject->objectColourRGBA.b, pCurrentObject->alphaTransparency);
	glUniform4f(specularColour_UL, pCurrentObject->specularColour.r, pCurrentObject->specularColour.g, pCurrentObject->specularColour.b, pCurrentObject->specularColour.a);

	GLint debugColour_UL = glGetUniformLocation(shaderProgID, "debugColour");
	GLint bDoNotLight_UL = glGetUniformLocation(shaderProgID, "bDoNotLight");

	// LINES
	if (pCurrentObject->isWireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glUniform4f(debugColour_UL, pCurrentObject->debugColour.r, pCurrentObject->debugColour.g, pCurrentObject->debugColour.b, pCurrentObject->debugColour.a);
		glUniform1f(bDoNotLight_UL, (float)GL_TRUE);
	}

	// SOLID
	else {
		glUniform1f(bDoNotLight_UL, (float)GL_FALSE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// DEPTH Test OFF/ON
	(pCurrentObject->disableDepthBufferTest) ? glDisable(GL_DEPTH_TEST) : glEnable(GL_DEPTH_TEST);

	// DEPTH BUFFER Writting OFF/ON
	(pCurrentObject->disableDepthBufferWrite) ? glDisable(GL_DEPTH) : glEnable(GL_DEPTH);

	sModelDrawInfo drawInfo;
	if (pVAOManager->FindDrawInfoByModelName(pCurrentObject->meshName, drawInfo)) {
		glBindVertexArray(drawInfo.VAO_ID);
		glDrawElements(GL_TRIANGLES, drawInfo.numberOfIndices, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
	return;
}

void cRenderer::Render3DObjectStarDestroyer(cModelObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager) {
	if (pCurrentObject->isVisible == false)
		return;

	glEnable(GL_BLEND);										// Turns on "alpha transparency"
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);		// Reads what's on the buffer already, and blends it with the incoming colour based on the "alpha" value, which is the 4th colour output RGB+A

	// This block of code, where I generate the world matrix from the position, scale, and rotation (orientation) of the object has been placed into calculateWorldMatrix()

	GLint explosionTime = glGetUniformLocation(shaderProgID, "explosionTime");
	glUniform1f(explosionTime, (blow_up) ? (float)GL_TRUE : (float)GL_FALSE);

	StarDestroyerExplosionOffset += (blow_up) ? 0.01f : 0.0f;

	GLint offset = glGetUniformLocation(shaderProgID, "offset");
	glUniform1f(offset, StarDestroyerExplosionOffset);

	// ************
	// Set the texture bindings and samplers
	SetUpTextureBindingsForObject(pCurrentObject, shaderProgID);

	GLint bInstance = glGetUniformLocation(shaderProgID, "bInstance");
	glUniform1f(bInstance, (float)GL_FALSE);

	GLint bIsSkyBox_UL = glGetUniformLocation(shaderProgID, "bIsSkyBox");
	glUniform1f(bIsSkyBox_UL, (float)GL_FALSE);

	// Don't draw back facing triangles (default)
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);		// Don't draw "back facing" triangles

	// If there is something to discard
	if (pCurrentObject->discardTextureName != "") {
		GLint bDiscard_UL = glGetUniformLocation(shaderProgID, "bDiscard");
		glUniform1f(bDiscard_UL, (float)GL_TRUE);

		GLuint texSamp_ID = mpTexture[pCurrentObject->discardTextureName];
		glActiveTexture(GL_TEXTURE13);				// Texture Unit 13
		glBindTexture(GL_TEXTURE_2D, texSamp_ID);	// Texture now assoc with texture unit 0

		GLint texture_UL = glGetUniformLocation(shaderProgID, "discardTexture");
		glUniform1i(texture_UL, 13);	// Texture unit 13
	}
	else {
		GLint bDiscard_UL = glGetUniformLocation(shaderProgID, "bDiscard");
		glUniform1f(bDiscard_UL, (float)GL_FALSE);
	}

	// Choose which shader to use
	glUseProgram(shaderProgID);

	GLint matModel_UL = glGetUniformLocation(shaderProgID, "matModel");
	glUniformMatrix4fv(matModel_UL, 1, GL_FALSE, glm::value_ptr(pCurrentObject->matWorld));

	// Calcualte the inverse transpose of the model matrix and pass that...
	// Stripping away scaling and translation, leaving only rotation
	// Because the normal is only a direction, really
	GLint matModelIT_UL = glGetUniformLocation(shaderProgID, "matModelInverseTranspose");
	glm::mat4 matModelInverseTranspose = glm::inverse(glm::transpose(pCurrentObject->matWorld));
	glUniformMatrix4fv(matModelIT_UL, 1, GL_FALSE, glm::value_ptr(matModelInverseTranspose));

	// Find the location of the uniform variable newColour
	GLint newColour_location = glGetUniformLocation(shaderProgID, "newColour");
	GLint diffuseColour_UL = glGetUniformLocation(shaderProgID, "diffuseColour");
	GLint specularColour_UL = glGetUniformLocation(shaderProgID, "specularColour");

	glUniform3f(newColour_location, pCurrentObject->objectColourRGBA.r, pCurrentObject->objectColourRGBA.g, pCurrentObject->objectColourRGBA.b);
	glUniform4f(diffuseColour_UL, pCurrentObject->objectColourRGBA.r, pCurrentObject->objectColourRGBA.g, pCurrentObject->objectColourRGBA.b, pCurrentObject->alphaTransparency);
	glUniform4f(specularColour_UL, pCurrentObject->specularColour.r, pCurrentObject->specularColour.g, pCurrentObject->specularColour.b, pCurrentObject->specularColour.a);

	GLint debugColour_UL = glGetUniformLocation(shaderProgID, "debugColour");
	GLint bDoNotLight_UL = glGetUniformLocation(shaderProgID, "bDoNotLight");

	// LINES
	if (pCurrentObject->isWireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glUniform4f(debugColour_UL, pCurrentObject->debugColour.r, pCurrentObject->debugColour.g, pCurrentObject->debugColour.b, pCurrentObject->debugColour.a);
		glUniform1f(bDoNotLight_UL, (float)GL_TRUE);
	}

	// SOLID
	else {
		glUniform1f(bDoNotLight_UL, (float)GL_FALSE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// DEPTH Test OFF/ON
	(pCurrentObject->disableDepthBufferTest) ? glDisable(GL_DEPTH_TEST) : glEnable(GL_DEPTH_TEST);

	// DEPTH BUFFER Writting OFF/ON
	(pCurrentObject->disableDepthBufferWrite) ? glDisable(GL_DEPTH) : glEnable(GL_DEPTH);

	sModelDrawInfo drawInfo;
	if (pVAOManager->FindDrawInfoByModelName(pCurrentObject->meshName, drawInfo)) {
		glBindVertexArray(drawInfo.VAO_ID);
		glDrawElements(GL_TRIANGLES, drawInfo.numberOfIndices, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
	return;
}


void cRenderer::RenderDebugObject() {
	//TODO: Add code for the rendering of debug objects...
	return;
}


void cRenderer::RenderTextObject() {
	//TODO: Move the TextRender code here and call this to render the console window text...
	return;
}



void cRenderer::RenderSkyBoxObject(cModelObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager) {
	if (pCurrentObject->isVisible == false)
		return;

	glEnable(GL_BLEND);										// Turns on "alpha transparency"
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);		// Reads what's on the buffer already, and blends it with the incoming colour based on the "alpha" value, which is the 4th colour output RGB+A

	// This block of code, where I generate the world matrix from the position, scale, and rotation (orientation) of the object has been placed into calculateWorldMatrix()


	GLint explosionTime = glGetUniformLocation(shaderProgID, "explosionTime");
	glUniform1f(explosionTime, (float)GL_FALSE);


	// ************
	// Set the texture bindings and samplers
	SetUpTextureBindingsForObject(pCurrentObject, shaderProgID);

	GLint bInstance = glGetUniformLocation(shaderProgID, "bInstance");
	glUniform1f(bInstance, (float)GL_FALSE);

	// See if this is a skybox object? 
	GLint bIsSkyBox_UL = glGetUniformLocation(shaderProgID, "bIsSkyBox");

	// Draw the back facing triangles. 
	// Because we are inside the object, so it will force a draw on the "back" of the sphere 
	//glCullFace(GL_FRONT_AND_BACK);
	glDisable(GL_CULL_FACE);	// Draw everything

	glUniform1f(bIsSkyBox_UL, (float)GL_TRUE);

	GLuint skyBoxTextureID = mpTexture[pCurrentObject->v_textureNames[0]];
	glActiveTexture(GL_TEXTURE26);				// Texture Unit 26
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTextureID);	// Texture now assoc with texture unit 0

	// Tie the texture units to the samplers in the shader
	GLint skyBoxSampler_UL = glGetUniformLocation(shaderProgID, "skyBox");
	glUniform1i(skyBoxSampler_UL, 26);	// Texture unit 26


	// If there is something to discard
	if (pCurrentObject->discardTextureName != "") {
		GLint bDiscard_UL = glGetUniformLocation(shaderProgID, "bDiscard");
		glUniform1f(bDiscard_UL, (float)GL_TRUE);

		GLuint texSamp_ID = mpTexture[pCurrentObject->discardTextureName];
		glActiveTexture(GL_TEXTURE13);				// Texture Unit 13
		glBindTexture(GL_TEXTURE_2D, texSamp_ID);	// Texture now assoc with texture unit 0

		GLint texture_UL = glGetUniformLocation(shaderProgID, "discardTexture");
		glUniform1i(texture_UL, 13);	// Texture unit 13
	}
	else {
		GLint bDiscard_UL = glGetUniformLocation(shaderProgID, "bDiscard");
		glUniform1f(bDiscard_UL, (float)GL_FALSE);
	}

	// Choose which shader to use
	glUseProgram(shaderProgID);

	GLint matModel_UL = glGetUniformLocation(shaderProgID, "matModel");
	glUniformMatrix4fv(matModel_UL, 1, GL_FALSE, glm::value_ptr(pCurrentObject->matWorld));

	// Calcualte the inverse transpose of the model matrix and pass that...
	// Stripping away scaling and translation, leaving only rotation
	// Because the normal is only a direction, really
	GLint matModelIT_UL = glGetUniformLocation(shaderProgID, "matModelInverseTranspose");
	glm::mat4 matModelInverseTranspose = glm::inverse(glm::transpose(pCurrentObject->matWorld));
	glUniformMatrix4fv(matModelIT_UL, 1, GL_FALSE, glm::value_ptr(matModelInverseTranspose));

	// Find the location of the uniform variable newColour
	GLint newColour_location = glGetUniformLocation(shaderProgID, "newColour");
	GLint diffuseColour_UL = glGetUniformLocation(shaderProgID, "diffuseColour");
	GLint specularColour_UL = glGetUniformLocation(shaderProgID, "specularColour");

	glUniform3f(newColour_location, pCurrentObject->objectColourRGBA.r, pCurrentObject->objectColourRGBA.g, pCurrentObject->objectColourRGBA.b);
	glUniform4f(diffuseColour_UL, pCurrentObject->objectColourRGBA.r, pCurrentObject->objectColourRGBA.g, pCurrentObject->objectColourRGBA.b, pCurrentObject->alphaTransparency);
	glUniform4f(specularColour_UL, pCurrentObject->specularColour.r, pCurrentObject->specularColour.g, pCurrentObject->specularColour.b, pCurrentObject->specularColour.a);

	GLint debugColour_UL = glGetUniformLocation(shaderProgID, "debugColour");
	GLint bDoNotLight_UL = glGetUniformLocation(shaderProgID, "bDoNotLight");

	// LINES
	if (pCurrentObject->isWireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glUniform4f(debugColour_UL, pCurrentObject->debugColour.r, pCurrentObject->debugColour.g, pCurrentObject->debugColour.b, pCurrentObject->debugColour.a);
		glUniform1f(bDoNotLight_UL, (float)GL_TRUE);
	}

	// SOLID
	else {
		glUniform1f(bDoNotLight_UL, (float)GL_FALSE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// DEPTH Test OFF/ON
	(pCurrentObject->disableDepthBufferTest) ? glDisable(GL_DEPTH_TEST) : glEnable(GL_DEPTH_TEST);

	// DEPTH BUFFER Writting OFF/ON
	(pCurrentObject->disableDepthBufferWrite) ? glDisable(GL_DEPTH) : glEnable(GL_DEPTH);

	sModelDrawInfo drawInfo;
	if (pVAOManager->FindDrawInfoByModelName(pCurrentObject->meshName, drawInfo)) {
		glBindVertexArray(drawInfo.VAO_ID);
		glDrawElements(GL_TRIANGLES, drawInfo.numberOfIndices, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
	return;
}


void cRenderer::RenderImposterObject() {
	//TODO: Move imposter rendering here for any imposters that would be a onetime render...
	//NOTE: This would not happen very often and only in very rare cases...
	return;
}



void cRenderer::RenderParticleObject() {
	//TODO: Move rendering of particles here that would not be an instanced effect...
	//NOTE: Remove this function if it is never used (i suspect that this will never be the case)...
	return;
}



void cRenderer::RenderInstanced3DObject() {
	//TODO: Move the rendering of instanced objects here...
	//NOTE: These are objects that are not particles or imposters; thus in the case we actually want to instance render full meshes of objects (like a massive amount of
	//		the same weapon/armor being drawn to the screen)
	return;
}



void cRenderer::RenderInstancedParticleObject(cModelObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager,
	cParticleEmitter* pParticleEmitter, GLuint particles_mat_model_buffer) {
	if (pCurrentObject->isVisible == false)
		return;

	glEnable(GL_BLEND);										// Turns on "alpha transparency"
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);		// Reads what's on the buffer already, and blends it with the incoming colour based on the "alpha" value, which is the 4th colour output RGB+A

	// This block of code, where I generate the world matrix from the position, scale, and rotation (orientation) of the object has been placed into calculateWorldMatrix()


	// ************
	// Set the texture bindings and samplers
	SetUpTextureBindingsForObject(pCurrentObject, shaderProgID);

	GLint bInstance = glGetUniformLocation(shaderProgID, "bInstance");
	glUniform1f(bInstance, (float)GL_TRUE);

	GLint bIsSkyBox_UL = glGetUniformLocation(shaderProgID, "bIsSkyBox");
	glUniform1f(bIsSkyBox_UL, (float)GL_FALSE);

	// If there is something to discard
	if (pCurrentObject->discardTextureName != "") {
		GLint bDiscard_UL = glGetUniformLocation(shaderProgID, "bDiscard");
		glUniform1f(bDiscard_UL, (float)GL_TRUE);

		GLuint texSamp_ID = mpTexture[pCurrentObject->discardTextureName];
		glActiveTexture(GL_TEXTURE13);				// Texture Unit 13
		glBindTexture(GL_TEXTURE_2D, texSamp_ID);	// Texture now assoc with texture unit 0

		GLint texture_UL = glGetUniformLocation(shaderProgID, "discardTexture");
		glUniform1i(texture_UL, 13);	// Texture unit 13
	}
	else {
		GLint bDiscard_UL = glGetUniformLocation(shaderProgID, "bDiscard");
		glUniform1f(bDiscard_UL, (float)GL_FALSE);
	}

	// Don't draw back facing triangles (default)
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);		// Don't draw "back facing" triangles

	// Choose which shader to use
	glUseProgram(shaderProgID);

	GLint matModel_UL = glGetUniformLocation(shaderProgID, "matModel");
	glUniformMatrix4fv(matModel_UL, 1, GL_FALSE, glm::value_ptr(pCurrentObject->matWorld));

	// Calcualte the inverse transpose of the model matrix and pass that...
	// Stripping away scaling and translation, leaving only rotation
	// Because the normal is only a direction, really
	GLint matModelIT_UL = glGetUniformLocation(shaderProgID, "matModelInverseTranspose");
	glm::mat4 matModelInverseTranspose = glm::inverse(glm::transpose(pCurrentObject->matWorld));
	glUniformMatrix4fv(matModelIT_UL, 1, GL_FALSE, glm::value_ptr(matModelInverseTranspose));

	// Find the location of the uniform variable newColour
	GLint newColour_location = glGetUniformLocation(shaderProgID, "newColour");
	GLint diffuseColour_UL = glGetUniformLocation(shaderProgID, "diffuseColour");
	GLint specularColour_UL = glGetUniformLocation(shaderProgID, "specularColour");

	glUniform3f(newColour_location, pCurrentObject->objectColourRGBA.r, pCurrentObject->objectColourRGBA.g, pCurrentObject->objectColourRGBA.b);
	glUniform4f(diffuseColour_UL, pCurrentObject->objectColourRGBA.r, pCurrentObject->objectColourRGBA.g, pCurrentObject->objectColourRGBA.b, pCurrentObject->alphaTransparency);
	glUniform4f(specularColour_UL, pCurrentObject->specularColour.r, pCurrentObject->specularColour.g, pCurrentObject->specularColour.b, pCurrentObject->specularColour.a);

	GLint debugColour_UL = glGetUniformLocation(shaderProgID, "debugColour");
	GLint bDoNotLight_UL = glGetUniformLocation(shaderProgID, "bDoNotLight");

	// LINES
	if (pCurrentObject->isWireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glUniform4f(debugColour_UL, pCurrentObject->debugColour.r, pCurrentObject->debugColour.g, pCurrentObject->debugColour.b, pCurrentObject->debugColour.a);
		glUniform1f(bDoNotLight_UL, (float)GL_TRUE);
	}

	// SOLID
	else {
		glUniform1f(bDoNotLight_UL, (float)GL_FALSE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// DEPTH Test OFF/ON
	(pCurrentObject->disableDepthBufferTest) ? glDisable(GL_DEPTH_TEST) : glEnable(GL_DEPTH_TEST);

	// DEPTH BUFFER Writting OFF/ON
	(pCurrentObject->disableDepthBufferWrite) ? glDisable(GL_DEPTH) : glEnable(GL_DEPTH);


	sModelDrawInfo drawInfo;
	if (pVAOManager->FindDrawInfoByModelName(pCurrentObject->meshName, drawInfo)) {
		glBindVertexArray(drawInfo.VAO_ID);

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glDrawElementsInstanced(GL_TRIANGLES, drawInfo.numberOfIndices, GL_UNSIGNED_INT, 0, pParticleEmitter->getMaxParticles());

		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);
		glDisableVertexAttribArray(5);
		glDisableVertexAttribArray(6);

		glBindVertexArray(0);
	}

	return;
}



void cRenderer::RenderInstancedImposterObject() {
	//TODO: Move the rendering of imposter objects that are not particles here...
	//NOTE: This is likely to come up where an imposter will not be a particle but some texture effect on a 2d / 3d flat plane object that makes the
	//		player believe they are view some super cool effect when in reality they are not...
	//		i.e. light coronas.
	//		This effect on lightning or even the glow around lights would likely be a texture but here is a link for the idea of a light corona.
	//		https://en.wikipedia.org/wiki/Corona_discharge
	//		and son of a crap we now have another yes ANOTHER shader type as noted below
	//		https://en.wikipedia.org/wiki/Bloom_(shader_effect)
	//		this would also lead to both the tone mapping and colour mapping which may also lead to more textures being blended togeather!!!
	return;
}


void cRenderer::SetUpTextureBindingsForObject(cModelObject* pCurrentObject, GLint shaderProgID) {
	if (pCurrentObject->v_textureNames.size() == 0)
	{
		GLuint ID = mpTexture["Default"];
		glActiveTexture(GL_TEXTURE0);		// Texture Unit 0
		glBindTexture(GL_TEXTURE_2D, ID);	// Texture now assoc with texture unit 0


		GLint textSamp00_UL = glGetUniformLocation(shaderProgID, "textSamp00");
		glUniform1i(textSamp00_UL, 0);	// Texture unit 0

		GLint textSamp01_UL = glGetUniformLocation(shaderProgID, "textSamp01");
		glUniform1i(textSamp01_UL, 0);	// Texture unit 0

		GLint tex0_ratio_UL = glGetUniformLocation(shaderProgID, "tex_0_1_ratio");
		glUniform2f(tex0_ratio_UL, 1.0f, 0.0f);
	}
	else if (pCurrentObject->v_textureNames.size() == 1)
	{
		GLuint ID = mpTexture[pCurrentObject->v_textureNames[0]];
		glActiveTexture(GL_TEXTURE0);				// Texture Unit 0
		glBindTexture(GL_TEXTURE_2D, ID);	// Texture now assoc with texture unit 0


		GLint textSamp00_UL = glGetUniformLocation(shaderProgID, "textSamp00");
		glUniform1i(textSamp00_UL, 0);	// Texture unit 0

		GLint textSamp01_UL = glGetUniformLocation(shaderProgID, "textSamp01");
		glUniform1i(textSamp01_UL, 0);	// Texture unit 0

		GLint tex0_ratio_UL = glGetUniformLocation(shaderProgID, "tex_0_1_ratio");
		glUniform2f(tex0_ratio_UL, pCurrentObject->v_texureBlendRatio[0], 0.0f);
	}
	else if (pCurrentObject->v_textureNames.size() == 2)
	{
		GLuint ID0 = mpTexture[pCurrentObject->v_textureNames[0]];
		glActiveTexture(GL_TEXTURE0);				// Texture Unit 0
		glBindTexture(GL_TEXTURE_2D, ID0);	// Texture now assoc with texture unit 0

		GLuint ID1 = mpTexture[pCurrentObject->v_textureNames[1]];
		glActiveTexture(GL_TEXTURE1);				// Texture Unit 0
		glBindTexture(GL_TEXTURE_2D, ID1);	// Texture now assoc with texture unit 0


		GLint textSamp00_UL = glGetUniformLocation(shaderProgID, "textSamp00");
		glUniform1i(textSamp00_UL, 0);	// Texture unit 0

		GLint textSamp01_UL = glGetUniformLocation(shaderProgID, "textSamp01");
		glUniform1i(textSamp01_UL, 1);	// Texture unit 1

		GLint tex0_ratio_UL = glGetUniformLocation(shaderProgID, "tex_0_1_ratio");
		glUniform2f(tex0_ratio_UL, pCurrentObject->v_texureBlendRatio[0], pCurrentObject->v_texureBlendRatio[1]);
	}

	return;
}