//#include <windows.h>
#include <iostream>
#include <sstream>
#include "cPhysicsManager.hpp"
#include "Globals.hpp"
#include "cGameObjectFactory.hpp"
#include <random>

#pragma region SINGLETON
cPhysicsManager cPhysicsManager::stonPhysMngr;
cPhysicsManager* cPhysicsManager::GetPhysicsManager() { return &(cPhysicsManager::stonPhysMngr); }
cPhysicsManager::cPhysicsManager() {
	pMediator = nullptr;
	pPhysics = new cPhysics();

	prevAABB = 0;

	vec_BoundingBox_Test_Points = {
		// 9 Points above
		glm::vec3(0.0f, 10000.0f, 0.0f),
		glm::vec3(0.0f, 10000.0f, -10000.0f),
		glm::vec3(-10000.0f, 10000.0f, -10000.0f),
		glm::vec3(-10000.0f, 10000.0f, 0.0f),
		glm::vec3(-10000.0f, 10000.0f, 10000.0f),
		glm::vec3(0.0f, 10000.0f, 10000.0f),
		glm::vec3(10000.0f, 10000.0f, 10000.0f),
		glm::vec3(10000.0f, 10000.0f, 0.0f),
		glm::vec3(10000.0f, 10000.0f, -100000.0f),

		// 8 at level
		glm::vec3(0.0f, 0.0f, -10000.0f),
		glm::vec3(-10000.0f, 0.0f, -10000.0f),
		glm::vec3(-10000.0f, 0.0f, 0.0f),
		glm::vec3(-10000.0f, 0.0f, 10000.0f),
		glm::vec3(0.0f, 0.0f, 10000.0f),
		glm::vec3(10000.0f, 0.0f, 10000.0f),
		glm::vec3(10000.0f, 0.0f, 0.0f),
		glm::vec3(10000.0f, 0.0f, -100000.0f),

		// 9 Points Below
		glm::vec3(0.0f, -10000.0f, 0.0f),
		glm::vec3(0.0f, -10000.0f, -10000.0f),
		glm::vec3(-10000.0f, -10000.0f, -10000.0f),
		glm::vec3(-10000.0f, -10000.0f, 0.0f),
		glm::vec3(-10000.0f, -10000.0f, 10000.0f),
		glm::vec3(0.0f, -10000.0f, 10000.0f),
		glm::vec3(10000.0f, -10000.0f, 10000.0f),
		glm::vec3(10000.0f, -10000.0f, 0.0f),
		glm::vec3(10000.0f, -10000.0f, -100000.0f)
	};

	for (size_t i = 0; i < vec_BoundingBox_Test_Points.size(); i++)
	{
		vec_BoundingBox_ClosestPoints.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
	}

	turned_around = false;
	blow_up = false;
	portHP = 100;
	starboardHP = 100;

	portPoint = glm::vec3(-103.0f, 245.0f, 556.0f);
	starboardPoint = glm::vec3(100.0f, 245.0f, 556.0f);
	deflectorShieldGeneratorGlobesRadius = 30.0f; //probably a bit big

	std::cout << "Physics Manager Created" << std::endl;
}
#pragma endregion


void cPhysicsManager::IntegrationStep(std::vector<cModelObject*>& vec_pGameObjects, float deltaTime) {
	pPhysics->IntegrationStep(::g_vec_pGameObjects, deltaTime);
}
void cPhysicsManager::TestForCollisions(std::vector<cModelObject*>& vec_pGameObjects) {
	pPhysics->TestForCollisions(::g_vec_pGameObjects);
}

void cPhysicsManager::TestForComplexCollisions(std::vector<cComplexObject*> g_vec_pComplexObjects, cDebugRenderer* pDebugRenderer) {
	//multi collision container
	std::vector<std::tuple<collisionPoint*, glm::vec3, cPhysics::sPhysicsTriangle>> vec_Collisions;
	cPhysics::sPhysicsTriangle closestTriangle;
	glm::vec3 closestPoint;

	for (cComplexObject* curObj : g_vec_pComplexObjects) {
		std::vector<collisionPoint*> testPoints = curObj->GetCollisionPoints();
		for (size_t i = 0; i < testPoints.size(); ++i) {
			if (curObj->friendlyName == "Luke Skywalker") {
				if (glm::distance(curObj->GetPosition(), endingPoint) < 2.5f) {
					curObj->SetVelocity(glm::vec3(0.0f, 0.0f, 0.0f));
					break;
				}
				else if (turned_around && glm::distance(curObj->GetPosition(), startingPoint) < 2.5f) {
					curObj->SetVelocity(glm::vec3(0.0f, 0.0f, 0.0f));
					break;
				}
			}
			//find AABB point is in
			unsigned long long curAABB = cAABB::get_ID_of_AABB_I_Might_Be_In(testPoints[i]->Position);

			//if AABB doesnt exist, check next point
			if (g_mapAABBs_World.find(curAABB) == g_mapAABBs_World.end())
				break;

			//Determine if inside a model by getting the direction (normalized distance) vector to closest point on triangle
			pPhysics->GetClosestAABBTriangle(testPoints[i]->Position, g_mapAABBs_World[curAABB], closestPoint, closestTriangle);

			glm::vec3 distance = testPoints[i]->Position - closestPoint;
			float fDist = glm::length(distance);
			if (fDist < 5.0f) {
				// then dot product with the triangle normal
				// if its negative, the point is inside
				float dirNormDot = glm::dot(glm::normalize(distance), closestTriangle.normal);
				if (dirNormDot < 0.0f) {
					//save collision info fo use with multi triangle collisions.
					//need collision point position, closest point, and triangle normal
					//glm::vec3 tmpArray[3] = { testPoints[i].Position, closestPoint, closestTriangle.normal };
					vec_Collisions.push_back(std::make_tuple(testPoints[i], closestPoint, closestTriangle));
				}
			}
		}

		//multi collision resolution
		for (size_t i = 0; i < vec_Collisions.size(); ++i) {

#pragma region Pre_final_collisions
			//var order: point position, closest point, and triangle normal
			//printf("Test Point %i collided at %f, %f, %f\n", i, curObj->GetPosition().x, curObj->GetPosition().y, curObj->GetPosition().z);
			//move back necessary distance
			//curObj->Move(std::get<1>(vec_Collisions[i]) - std::get<0>(vec_Collisions[i])->Position);

			//// Change to alter based on collided triangle normal?
			//// normals can be positive or negative, -1.0 -> 1.0
			//// mirror values in their respective direction ie: 0.2 -> 0.8
			//// absolute the negative values
			//// than multiply existing velocity by recalculated normal.
			//glm::vec3 curVel = curObj->GetVelocity();
			//glm::vec3 InvertedNormal;
			//InvertedNormal.x = (std::get<2>(vec_Collisions[i]).normal.x > 0) ? 1.0f - std::get<2>(vec_Collisions[i]).normal.x : 1.0f + std::get<2>(vec_Collisions[i]).normal.x;
			//InvertedNormal.y = (std::get<2>(vec_Collisions[i]).normal.y > 0) ? 1.0f - std::get<2>(vec_Collisions[i]).normal.y : 1.0f + std::get<2>(vec_Collisions[i]).normal.y;
			//InvertedNormal.z = (std::get<2>(vec_Collisions[i]).normal.z > 0) ? 1.0f - std::get<2>(vec_Collisions[i]).normal.z : 1.0f + std::get<2>(vec_Collisions[i]).normal.z;
			//printf("Normal x: %f, y: %f, z: %f\n", std::get<2>(vec_Collisions[i]).normal.x, std::get<2>(vec_Collisions[i]).normal.y, std::get<2>(vec_Collisions[i]).normal.z);
			//printf("Inverted x: %f, y: %f, z: %f\n", InvertedNormal.x, InvertedNormal.y, InvertedNormal.z);
			//glm::vec3 newVel = InvertedNormal * curVel;
			//curObj->SetVelocity(newVel);

			////TODO
			////Add ship mesh triangles within x radius of collision point flash
			////via debug renderer (not a singleton).
			////debug renderer at collision point!
			//if (isDebugDisplay) {
			//	if (isDebugCollision) {
			//		std::get<0>(vec_Collisions[i])->Collided = 0.0f;

			//		pDebugRenderer->addPointPointSize(std::get<0>(vec_Collisions[i])->Position, glm::vec3(1.0f, 0.0f, 0.0f), 15.0f, 5.0f);
			//		pDebugRenderer->addLine(std::get<2>(vec_Collisions[i]).verts[0], std::get<2>(vec_Collisions[i]).verts[1], glm::vec3(1.0f, 0.0f, 0.0f), 5.0f);
			//		pDebugRenderer->addLine(std::get<2>(vec_Collisions[i]).verts[1], std::get<2>(vec_Collisions[i]).verts[2], glm::vec3(1.0f, 0.0f, 0.0f), 5.0f);
			//		pDebugRenderer->addLine(std::get<2>(vec_Collisions[i]).verts[2], std::get<2>(vec_Collisions[i]).verts[0], glm::vec3(1.0f, 0.0f, 0.0f), 5.0f);
			//	}
			//}
#pragma endregion

#pragma region Q3_X-Wing_Collision
			if (curObj->friendlyName == "Luke Skywalker") {
				//Grab the shot
				cComplexObject* theShot = g_vec_pComplexObjects[1]; //always
				theShot->SetPosition(curObj->GetPosition());
				theShot->SetVelocity(glm::normalize(curObj->GetVelocity()) * 60.0f);
				theShot->SetOrientation(curObj->getQOrientation());

				glm::vec3 newvel = curObj->GetVelocity() * -1.0f;
				curObj->SetVelocity(newvel);
				glm::quat newQuat = glm::quatLookAt(glm::normalize(-newvel), glm::vec3(0.0f, 1.0f, 0.0f));
				curObj->SetOrientation(newQuat);

				turned_around = true;
				break;
			}
#pragma endregion

#pragma region Bolt_Collision
			else if (curObj->friendlyName == "Imperial Tears") {

				glm::vec3 ImpactPoint = (std::get<1>(vec_Collisions[i]));

				// Impact drawing:
				iGameObject* theImpact = ObjectFactory.CreateMediatedGameObject("model", "Sphere", "ImpactSphere");
				((cModelObject*)theImpact)->positionXYZ = ImpactPoint;
				((cModelObject*)theImpact)->scale = 3.0f;
				((cModelObject*)theImpact)->v_textureNames = std::vector<std::string>{ "Red" };
				((cModelObject*)theImpact)->v_texureBlendRatio = std::vector<float>{ 1.0f };

				curObj->SetVelocity(glm::vec3(0.0f, 0.0f, 0.0f));
				curObj->SetPosition(glm::vec3(100000.0f, 100000.0f, 100000.0f));

				//deflector shields check
				if (glm::distance(ImpactPoint, portPoint) < deflectorShieldGeneratorGlobesRadius) {
					// Port hit!
					portHP = (portHP == 0) ? 0 : portHP - 25;
					std::string newTitle = "Port Side Deflector Generator Struck: " + std::to_string(portHP) + "% HP";
					glfwSetWindowTitle(_window, newTitle.c_str());
				}
				else if (glm::distance(ImpactPoint, starboardPoint) < deflectorShieldGeneratorGlobesRadius) {
					// Starboard hit!
					starboardHP = (starboardHP == 0) ? 0 : starboardHP - 25;
					std::string newTitle = "Starboard Side Deflector Generator Struck: " + std::to_string(starboardHP) + "% HP";
					glfwSetWindowTitle(_window, newTitle.c_str());
				}

				if (portHP == 0 && starboardHP == 0) {
					glfwSetWindowTitle(_window, "You've destroyed the Star Destroyer!");
					printf("\n\nYou've destroyed the Star Destroyer!\n\n");
					blow_up = true;
					cModelObject* xplodeyBoi = pFindObjectByFriendlyName("Imperial Star Destroyer");
					xplodeyBoi->v_textureNames = std::vector<std::string>{ "Fire" };
					xplodeyBoi->alphaTransparency = 0.6f;
					g_mapAABBs_World.clear(); //CANT COLLIDE IF YOU'RE DEAD
				}

				break;
			}
#pragma endregion
		}

		vec_Collisions.clear();
	}
}

void cPhysicsManager::CalcAABBsForMeshModel(cModelObject* theModel) {
	//Calculate world matrix/mesh
	theModel->recalculateWorldMatrix();
	glm::mat4 matWorld = theModel->matWorld;
	cMesh theMesh;
	if (mpMesh.find(theModel->meshName) != mpMesh.end()) {
		pPhysics->CalculateTransformedMesh(*mpMesh[theModel->meshName], matWorld, theMesh);
	}

	//find bounding box
	//iterate through a vector of test points relative to position to gather most extreme points.
	//this will take a LONG time.
	for (size_t i = 0; i < vec_BoundingBox_Test_Points.size(); i++) {
		vec_BoundingBox_ClosestPoints[i] += theModel->getPosition();
		pPhysics->GetClosestPointToPoint(vec_BoundingBox_Test_Points[i], theMesh, vec_BoundingBox_ClosestPoints[i]);
	}

	//iterate through the returned points to find the most extreme x y z values
	//save the min corner and max corner
	glm::vec3 minXYZ = theModel->getPosition();
	glm::vec3 maxXYZ = theModel->getPosition();
	for (glm::vec3 extPoint : vec_BoundingBox_ClosestPoints) {
		if (extPoint.x < minXYZ.x)
			minXYZ.x = extPoint.x;
		else if (extPoint.x > maxXYZ.x)
			maxXYZ.x = extPoint.x;

		if (extPoint.y < minXYZ.y)
			minXYZ.y = extPoint.y;
		else if (extPoint.y > maxXYZ.y)
			maxXYZ.y = extPoint.y;

		if (extPoint.z < minXYZ.z)
			minXYZ.z = extPoint.z;
		else if (extPoint.z > maxXYZ.z)
			maxXYZ.z = extPoint.z;
	}

	//subdivide by a factor - passed into function ?
	int iX = minXYZ.x;
	int iY = minXYZ.y;
	int iZ = minXYZ.z;

	//round negative numbers down before TRUNCATING
	if (iX < 0) iX -= 100;
	if (iY < 0) iY -= 100;
	if (iZ < 0) iZ -= 100;

	//TRUNCATE TO NEAREST 100 
	iX /= 100;
	iY /= 100;
	iZ /= 100;
	minXYZ.x = iX * 100;
	minXYZ.y = iY * 100;
	minXYZ.z = iZ * 100;

	//just using 100 for now
	int AABBsize = 50;
	//create aabbs
	for (float x = minXYZ.x; x <= maxXYZ.x; x += AABBsize * 2) {
		for (float y = minXYZ.y; y <= maxXYZ.y; y += AABBsize * 2) {
			for (float z = minXYZ.z; z <= maxXYZ.z; z += AABBsize * 2) {
				glm::vec3 AABBmin;
				AABBmin.x = x;
				AABBmin.y = y;
				AABBmin.z = z;
				cAABB* pAABB = new cAABB(AABBmin, (float)AABBsize);

				unsigned long long ID = pAABB->getID();

				this->g_mapAABBs_World[ID] = pAABB;
			}
		}
	}

	//parse triangles into aabbs
	for (std::vector<sPlyTriangle>::iterator itTri = theMesh.vecTriangles.begin();
		itTri != theMesh.vecTriangles.end(); itTri++) {
		glm::vec3 v1;
		v1.x = theMesh.vecVertices[itTri->vert_index_1].x;
		v1.y = theMesh.vecVertices[itTri->vert_index_1].y;
		v1.z = theMesh.vecVertices[itTri->vert_index_1].z;
		glm::vec3 v2;
		v2.x = theMesh.vecVertices[itTri->vert_index_2].x;
		v2.y = theMesh.vecVertices[itTri->vert_index_2].y;
		v2.z = theMesh.vecVertices[itTri->vert_index_2].z;
		glm::vec3 v3;
		v3.x = theMesh.vecVertices[itTri->vert_index_3].x;
		v3.y = theMesh.vecVertices[itTri->vert_index_3].y;
		v3.z = theMesh.vecVertices[itTri->vert_index_3].z;

		//if negative shift them

		/****************************************************
		Want to use format AABBTriangle instead of sPlyTriangle to avoid needing mesh based lookup in future.
		Make it not assign sub triangles and be unique.
		Option:
			Keep current set assignment (enforces unique and has no subtriangles)
			Convert set of sPlyTriangles to set of AABB Triangles in all AABB's after this loop
			then erase previous (now unnecessary) set?
		Option 2:
			Add a cMesh* to the sPlyTriangle..... (avoiding)
		****************************************************/

		AssignTriangle(itTri, v1, v2, v3, AABBsize * 0.5);
	}

	//remove empty aabbs
	//figure out who to delete
	std::vector<unsigned long long> deleteQueue;

	for (std::map<unsigned long long, cAABB*>::iterator AABBit = g_mapAABBs_World.begin();
		AABBit != g_mapAABBs_World.end(); ++AABBit) {
		if (AABBit->second->set_pTriangles.empty()
			&& AABBit->second->vec_pAABBTriangles.empty())
			deleteQueue.push_back(AABBit->first);
	}
	//delete
	for (unsigned long long ID : deleteQueue) {
		if (g_mapAABBs_World.find(ID) != g_mapAABBs_World.end())
			g_mapAABBs_World.erase(ID);
	}

	//convert still existing aabb's to the more useful triangle format
	for (std::pair<unsigned long long, cAABB*> mapAABB : g_mapAABBs_World) {
		for (sPlyTriangle* plyTri : mapAABB.second->set_pTriangles) {
			sPlyVertexXYZ_N_UV v1 = theMesh.vecVertices[plyTri->vert_index_1];
			sPlyVertexXYZ_N_UV v2 = theMesh.vecVertices[plyTri->vert_index_2];
			sPlyVertexXYZ_N_UV v3 = theMesh.vecVertices[plyTri->vert_index_3];
			AABBTriangle aabbTri;	

			aabbTri.vertsPos[0].x = v2.x;
			aabbTri.vertsPos[0].y = v1.y;
			aabbTri.vertsPos[0].z = v1.z;
			aabbTri.vertsNorm[0].x = v1.nx;
			aabbTri.vertsNorm[0].y = v1.ny;
			aabbTri.vertsNorm[0].z = v1.nz;
			aabbTri.vertsNorm[0] = glm::normalize(aabbTri.vertsNorm[0]);

			aabbTri.vertsPos[1].x = v2.x;
			aabbTri.vertsPos[1].y = v2.y;
			aabbTri.vertsPos[1].z = v2.z;
			aabbTri.vertsNorm[1].x = v2.nx;
			aabbTri.vertsNorm[1].y = v2.ny;
			aabbTri.vertsNorm[1].z = v2.nz;
			aabbTri.vertsNorm[1] = glm::normalize(aabbTri.vertsNorm[1]);

			aabbTri.vertsPos[2].x = v3.x;
			aabbTri.vertsPos[2].y = v3.y;
			aabbTri.vertsPos[2].z = v3.z;
			aabbTri.vertsNorm[2].x = v3.nx;
			aabbTri.vertsNorm[2].y = v3.ny;
			aabbTri.vertsNorm[2].z = v3.nz;
			aabbTri.vertsNorm[1] = glm::normalize(aabbTri.vertsNorm[1]);

			mapAABB.second->vec_pAABBTriangles.push_back(aabbTri);
		}
		mapAABB.second->set_pTriangles.clear();
	}
}

void cPhysicsManager::AssignTriangle(std::vector<sPlyTriangle>::iterator itTri, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, float maxLength) {
	// Is the length of ANY side longer than the AABB (longer than a half the AABB)
	// Slice the triangle. 
	bool sliceTriangle = false;
	if (glm::distance(v1, v2) > maxLength
		|| glm::distance(v2, v3) > maxLength
		|| glm::distance(v3, v1) > maxLength)
		sliceTriangle = true;

	if (sliceTriangle) {
		glm::vec3 SlicedV1;
		SlicedV1 = v1 + v2;
		SlicedV1 *= 0.5;
		glm::vec3 SlicedV2;
		SlicedV2 = v2 + v3;
		SlicedV2 *= 0.5;
		glm::vec3 SlicedV3;
		SlicedV3 = v3 + v1;
		SlicedV3 *= 0.5;

		AssignTriangle(itTri, SlicedV1, SlicedV2, SlicedV3, maxLength);
	}

	//assign vertices
	// Get the ID of the AABB that this vertex is inside of 
	unsigned long long ID_AABB_V1 = cAABB::get_ID_of_AABB_I_Might_Be_In(v1);
	unsigned long long ID_AABB_V2 = cAABB::get_ID_of_AABB_I_Might_Be_In(v2);
	unsigned long long ID_AABB_V3 = cAABB::get_ID_of_AABB_I_Might_Be_In(v3);

	// Add this triangle to that box
	std::map<unsigned long long, cAABB* >::iterator itAABB = g_mapAABBs_World.find(ID_AABB_V1);
	if (itAABB != g_mapAABBs_World.end())
		g_mapAABBs_World[ID_AABB_V1]->set_pTriangles.emplace(&(*itTri));

	itAABB = g_mapAABBs_World.find(ID_AABB_V2);
	if (itAABB != g_mapAABBs_World.end())
		g_mapAABBs_World[ID_AABB_V2]->set_pTriangles.emplace(&(*itTri));

	itAABB = g_mapAABBs_World.find(ID_AABB_V3);
	if (itAABB != g_mapAABBs_World.end())
		g_mapAABBs_World[ID_AABB_V3]->set_pTriangles.emplace(&(*itTri));
}

void cPhysicsManager::CalculateTransformedMesh(cMesh& originalMesh, glm::mat4 matWorld, cMesh& mesh_transformedInWorld) {
	pPhysics->CalculateTransformedMesh(originalMesh, matWorld, mesh_transformedInWorld);
}

//not right
BoundingBox cPhysicsManager::CalculateBoundingBox(cModelObject* theModel) {
	//Calculate world matrix/mesh
	theModel->recalculateWorldMatrix();
	glm::mat4 matWorld = theModel->matWorld;
	cMesh theMesh;
	if (mpMesh.find(theModel->meshName) != mpMesh.end()) {
		pPhysics->CalculateTransformedMesh(*mpMesh[theModel->meshName], matWorld, theMesh);
	}

	//find bounding box
	//iterate through a vector of test points relative to position to gather most extreme points.
	//this will take a LONG time.
	for (size_t i = 0; i < vec_BoundingBox_Test_Points.size(); i++) {
		vec_BoundingBox_ClosestPoints[i] += theModel->getPosition();
		pPhysics->GetClosestPointToPoint(vec_BoundingBox_Test_Points[i], theMesh, vec_BoundingBox_ClosestPoints[i]);
	}

	//iterate through the returned points to find the most extreme x y z values
	//save the min corner and max corner
	glm::vec3 minXYZ = theModel->getPosition();
	glm::vec3 maxXYZ = theModel->getPosition();
	for (glm::vec3 extPoint : vec_BoundingBox_ClosestPoints) {
		if (extPoint.x < minXYZ.x)
			minXYZ.x = extPoint.x;
		else if (extPoint.x > maxXYZ.x)
			maxXYZ.x = extPoint.x;

		if (extPoint.y < minXYZ.y)
			minXYZ.y = extPoint.y;
		else if (extPoint.y > maxXYZ.y)
			maxXYZ.y = extPoint.y;

		if (extPoint.z < minXYZ.z)
			minXYZ.z = extPoint.z;
		else if (extPoint.z > maxXYZ.z)
			maxXYZ.z = extPoint.z;
	}
	BoundingBox BB;
	BB.maxCorner = maxXYZ;
	BB.minCorner = minXYZ;
	BB.orientation = theModel->getQOrientation();
	return BB;
}

void cPhysicsManager::DrawAABBs() {
	//TO DRAW
	//place wireframe cube scaled too aabb half length at aabb center
	for (std::pair<unsigned long long, cAABB*> prAABB : g_mapAABBs_World) {
		std::string name;
		std::stringstream strstream;
		strstream << prAABB.second->getID();
		strstream >> name;
		iGameObject* DebugCube = ObjectFactory.CreateMediatedGameObject("model", "Cube", name);
		((cModelObject*)DebugCube)->positionXYZ = prAABB.second->GetCenter();
		((cModelObject*)DebugCube)->scale = prAABB.second->GetHalfLength();
		((cModelObject*)DebugCube)->debugColour = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
		((cModelObject*)DebugCube)->isWireframe = true;
		((cModelObject*)DebugCube)->isVisible = true;
	}
}

int cPhysicsManager::TrianglesInCurrentAABB(glm::vec3 pos) {
	unsigned long long ID = cAABB::calcID(pos);

	if (g_mapAABBs_World.find(ID) != g_mapAABBs_World.end()) {
		std::string name;
		std::stringstream strstream;
		strstream << ID;
		strstream >> name;

		currAABB = name;
		/*pFindObjectByFriendlyName(currAABB)->isVisible = true;
		if (ID != prevAABB && g_mapAABBs_World.find(prevAABB) != g_mapAABBs_World.end()) {
			strstream.clear();
			strstream << prevAABB;
			name = "";
			strstream >> name;
			pFindObjectByFriendlyName(name)->isVisible = false;
		}*/
		prevAABB = ID;
		return g_mapAABBs_World[ID]->vec_pAABBTriangles.size();
	}
	/*else if (prevAABB != 0 && g_mapAABBs_World.find(prevAABB) != g_mapAABBs_World.end()) {
		std::string name;
		std::stringstream strstream;
		strstream << prevAABB;
		name = "";
		strstream >> name;
		pFindObjectByFriendlyName(name)->isVisible = false;
	}*/
	return -1;
}

std::string cPhysicsManager::CurrentAABB(glm::vec3 pos) {
	unsigned long long ID = cAABB::get_ID_of_AABB_I_Might_Be_In(pos);
	std::stringstream strstream;
	std::string aabbID;
	strstream << ID;
	strstream >> aabbID;
	return aabbID;
}

cAABB* cPhysicsManager::GetCurrentAABB() {
	if (currAABB == "")
		return NULL;
	else {
		std::stringstream strstream;
		unsigned long long ID;
		strstream << currAABB;
		strstream >> ID;
		return g_mapAABBs_World[ID];
	}
}

cAABB* cPhysicsManager::GetPointAABB(glm::vec3 pos) {
	unsigned long long ID = cAABB::get_ID_of_AABB_I_Might_Be_In(pos);
	if(g_mapAABBs_World.find(ID) != g_mapAABBs_World.end())
		return g_mapAABBs_World[ID];
	return nullptr;
}

unsigned long long cPhysicsManager::GetPreviousAABB() {
	return this->prevAABB;
}

void cPhysicsManager::DrawTestPoints(cComplexObject* Parent) {
	std::vector<collisionPoint*> vec_Coll_Pts = Parent->GetCollisionPoints();

	for (size_t i = 0; i < vec_Coll_Pts.size(); i++) {
		iGameObject* DebugCube = ObjectFactory.CreateMediatedGameObject("model", "Cube", "ColPnt");
		((cModelObject*)DebugCube)->positionXYZ = Parent->GetPosition() + vec_Coll_Pts[i]->RelativePos;
		((cModelObject*)DebugCube)->scale = 0.05f;
		((cModelObject*)DebugCube)->debugColour = glm::vec4(1.0f, 0.0f, 0.5f, 1.0f);
		((cModelObject*)DebugCube)->isWireframe = true;
		((cModelObject*)DebugCube)->isVisible = true;
		((cModelObject*)DebugCube)->inverseMass = 1.0f;

		Parent->AddModel(((cModelObject*)DebugCube));
	}
}

void cPhysicsManager::FINALXWingPath() {
#pragma region Q2
	//clear old path first
	std::vector<cModelObject*> deletionQueue;
	for (unsigned int index = 0; index != g_vec_pGameObjects.size(); index++) {
		if (g_vec_pGameObjects[index]->friendlyName == "PathingSphere")
			deletionQueue.push_back(g_vec_pGameObjects[index]);		
	}

	//quick and dirty, should change to iterators
	if (!deletionQueue.empty()) {
		for (unsigned int i = 0; i < deletionQueue.size(); i++) {
			for (unsigned int j = 0; j != g_vec_pGameObjects.size(); j++) {
				if (deletionQueue[i] == g_vec_pGameObjects[j]) {
					g_vec_pGameObjects.erase(g_vec_pGameObjects.begin() + (j));
					break;
				}
			}
		}
		deletionQueue.clear();
	}

	//pick a point 3000 away from origin
	//if inside star destroyer pick new one
	std::random_device rd{};
	std::mt19937 mt{ rd() };
	// 5 is mean, 2 is variance
	// from ref docs, this generates between 1 and 9 as floats, which should work for my purposes
	std::normal_distribution<float> n{ 5, 2 };
	std::uniform_int_distribution<int> u{ 0, 1 };

	bool generating = true;
	do {
		startingPoint.x = u(mt) == 0 ? n(mt) : n(mt) * -1.0f;
		startingPoint.y = u(mt) == 0 ? n(mt) : n(mt) * -1.0f;
		startingPoint.z = u(mt) == 0 ? n(mt) : n(mt) * -1.0f;
		startingPoint = glm::normalize(startingPoint) * 3000.0f;

		//this should never get hit as far as I know, with the 1.0 scaling Im using.
		if (TrianglesInCurrentAABB(startingPoint) != 0) {
			//we're in an aabb, meaning too close to the star destroyer
			cPhysics::sPhysicsTriangle closestTriangle;
			glm::vec3 closestPoint;

			//find AABB point is in
			unsigned long long curAABB = cAABB::get_ID_of_AABB_I_Might_Be_In(startingPoint);

			//if AABB doesnt exist, check next point
			if (g_mapAABBs_World.find(curAABB) == g_mapAABBs_World.end())
				break;

			//Determine if inside a model by getting the direction (normalized distance) vector to closest point on triangle
			pPhysics->GetClosestAABBTriangle(startingPoint, g_mapAABBs_World[curAABB], closestPoint, closestTriangle);

			glm::vec3 distance = startingPoint - closestPoint;
			float fDist = glm::length(distance);
			float dirNormDot = glm::dot(glm::normalize(distance), closestTriangle.normal);
			if (dirNormDot < 0.0f) {
				printf("Point Generated Inside Star Destroyer, selecting new point...\n");
			}
			else generating = false;
		}
		else generating = false;
	} while (generating);

	//pick a point in the opposite world space 'octant' (quadrant but 8?)
	//if inside star destroyer pick new one
	generating = true;
	do {
		endingPoint.x = startingPoint.x < 0.0f ? n(mt) : n(mt) * -1.0f;
		endingPoint.y = startingPoint.x < 0.0f ? n(mt) : n(mt) * -1.0f;
		endingPoint.z = startingPoint.x < 0.0f ? n(mt) : n(mt) * -1.0f;
		endingPoint = glm::normalize(endingPoint) * 3000.0f;

		//this should never get hit as far as I know, with the 1.0 scaling Im using.
		if (TrianglesInCurrentAABB(endingPoint) != 0) {
			//we're in an aabb, meaning too close to the star destroyer
			cPhysics::sPhysicsTriangle closestTriangle;
			glm::vec3 closestPoint;

			//find AABB point is in
			unsigned long long curAABB = cAABB::get_ID_of_AABB_I_Might_Be_In(endingPoint);

			//if AABB doesnt exist, check next point
			if (g_mapAABBs_World.find(curAABB) == g_mapAABBs_World.end())
				break;

			//Determine if inside a model by getting the direction (normalized distance) vector to closest point on triangle
			pPhysics->GetClosestAABBTriangle(endingPoint, g_mapAABBs_World[curAABB], closestPoint, closestTriangle);

			glm::vec3 distance = endingPoint - closestPoint;
			float fDist = glm::length(distance);
			float dirNormDot = glm::dot(glm::normalize(distance), closestTriangle.normal);
			if (dirNormDot < 0.0f) {
				printf("Point Generated Inside Star Destroyer, selecting new point...\n");
			}
			else generating = false;
		}
		else generating = false;
	} while (generating);

	//draw path with spheres of varying sizes / color

	//Path drawing:
	iGameObject* thePath = ObjectFactory.CreateMediatedGameObject("model", "Sphere", "PathingSphere");
	((cModelObject*)thePath)->positionXYZ = startingPoint;
	((cModelObject*)thePath)->scale = 10.0f;
	((cModelObject*)thePath)->debugColour = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
	((cModelObject*)thePath)->isWireframe = true;

	thePath = ObjectFactory.CreateMediatedGameObject("model", "Sphere", "PathingSphere");
	((cModelObject*)thePath)->positionXYZ = endingPoint;
	((cModelObject*)thePath)->scale = 10.0f;
	((cModelObject*)thePath)->debugColour = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	((cModelObject*)thePath)->isWireframe = true;

	direction = glm::normalize(endingPoint - startingPoint);
	glm::vec3 drawPos = startingPoint;
	bool draw = true;
	while (draw) {
		drawPos += direction * 20.0f;
		thePath = ObjectFactory.CreateMediatedGameObject("model", "Sphere", "PathingSphere");
		((cModelObject*)thePath)->positionXYZ = drawPos;
		((cModelObject*)thePath)->scale = 0.25f;
		((cModelObject*)thePath)->debugColour = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		((cModelObject*)thePath)->isWireframe = true;
		float distance = glm::distance(endingPoint, drawPos);
		if (distance < 30.0f)
			draw = false;
	}
#pragma endregion

#pragma region Q3_Orientation/Initial Velocity
	g_vec_pComplexObjects[0]->SetPosition(startingPoint);
	g_vec_pComplexObjects[0]->SetOrientation(glm::quatLookAt(-direction, glm::vec3(0.0f, 1.0f, 0.0f)));
	g_vec_pComplexObjects[0]->SetVelocity(direction * 80.0f);

	FreeCamera.SetTarget(g_vec_pComplexObjects[0]);
	isObjCtrl = false;
	useFreeCam = true;

	turned_around = false;
#pragma endregion

}

#pragma region MEDIATOR SETUP
void cPhysicsManager::setMediatorPointer(iMediatorInterface* pMediator) {
	this->pMediator = pMediator;
	return;
}
#pragma endregion

#pragma region MEDIATOR COMMUNICATIONS
sNVData cPhysicsManager::RecieveMessage(sNVData message) {
	sNVData response;

	response.status = "OK";
	return response;
}



sData cPhysicsManager::RecieveMessage(sData& data) {
	sData response;
	response.setResult(OK);

	return response;
}
#pragma endregion