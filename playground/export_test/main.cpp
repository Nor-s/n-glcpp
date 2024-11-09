#include "assimp/Exporter.hpp"
#include "assimp/scene.h"
#include "assimp/mesh.h"
#include "assimp/anim.h"
#include "assimp/matrix4x4.h"
#include "assimp/vector3.h"
#include <iostream>

int main()
{
	// Create a new Assimp scene
	aiScene* scene = new aiScene();
	scene->mRootNode = new aiNode();

	// Create a mesh for the box
	aiMesh* mesh = new aiMesh();
	mesh->mMaterialIndex = 0;
	mesh->mNumVertices = 8;
	mesh->mVertices = new aiVector3D[8];

	// Define vertices for a simple box
	mesh->mVertices[0] = aiVector3D(-0.5f, -0.5f, 0.5f);
	mesh->mVertices[1] = aiVector3D(0.5f, -0.5f, 0.5f);
	mesh->mVertices[2] = aiVector3D(0.5f, 0.5f, 0.5f);
	mesh->mVertices[3] = aiVector3D(-0.5f, 0.5f, 0.5f);
	mesh->mVertices[4] = aiVector3D(-0.5f, -0.5f, -0.5f);
	mesh->mVertices[5] = aiVector3D(0.5f, -0.5f, -0.5f);
	mesh->mVertices[6] = aiVector3D(0.5f, 0.5f, -0.5f);
	mesh->mVertices[7] = aiVector3D(-0.5f, 0.5f, -0.5f);

	// Create faces for the box (12 triangles)
	mesh->mNumFaces = 12;
	mesh->mFaces = new aiFace[12];
	unsigned int indices[36] = {
		0, 1, 2, 2, 3, 0,	 // Front
		1, 5, 6, 6, 2, 1,	 // Right
		7, 6, 5, 5, 4, 7,	 // Back
		4, 0, 3, 3, 7, 4,	 // Left
		4, 5, 1, 1, 0, 4,	 // Bottom
		3, 2, 6, 6, 7, 3	 // Top
	};

	for (unsigned int i = 0; i < 12; ++i)
	{
		mesh->mFaces[i].mNumIndices = 3;
		mesh->mFaces[i].mIndices = new unsigned int[3];
		for (unsigned int j = 0; j < 3; ++j)
		{
			mesh->mFaces[i].mIndices[j] = indices[i * 3 + j];
		}
	}

	// Assign the mesh to the scene
	scene->mMeshes = new aiMesh*[1];
	scene->mMeshes[0] = mesh;
	scene->mNumMeshes = 1;

	// Create a node for the mesh and link it to the scene's root node
	aiNode* meshNode = new aiNode();
	meshNode->mName = "Box";	// Name the node
	meshNode->mMeshes = new unsigned int[1];
	meshNode->mMeshes[0] = 0;	 // The first and only mesh
	meshNode->mNumMeshes = 1;
	scene->mRootNode->addChildren(1, &meshNode);

	// Create a simple material for the mesh
	aiMaterial* material = new aiMaterial();
	scene->mMaterials = new aiMaterial*[1];
	scene->mMaterials[0] = material;
	scene->mNumMaterials = 1;

	// Create an animation for the box
	aiAnimation* animation = new aiAnimation();
	animation->mName = "BoxRotation";
	animation->mDuration = 10.0;	// 10 seconds
	animation->mTicksPerSecond = 1.0;
	animation->mNumChannels = 1;
	animation->mChannels = new aiNodeAnim*[1];

	// Create a node animation channel for the box's rotation and translation
	aiNodeAnim* channel = new aiNodeAnim();
	channel->mNodeName = meshNode->mName;

	const unsigned int numKeys = 10 * 24;	 // 10 seconds * 24 FPS
	channel->mNumRotationKeys = 2;
	channel->mRotationKeys = new aiQuatKey[2];
	channel->mNumPositionKeys = 2;
	channel->mPositionKeys = new aiVectorKey[2];

	int idx = 0;
	for (unsigned int i = 0; i < numKeys; ++i)
	{
		if (!(i == 0 || i == numKeys - 1))
		{
			continue;
		}
		double time = static_cast<double>(i) / 24.0;	// time
		aiQuaternion rotation;
		rotation.w = 1.;
		rotation.x = 0.;
		rotation.y = 0.;
		rotation.z = 0.;
		channel->mRotationKeys[idx].mValue = rotation;
		channel->mRotationKeys[idx].mTime = time;
		channel->mPositionKeys[idx].mTime = time;
		channel->mPositionKeys[idx].mValue = aiVector3D(time * 0.5, 0, 0);
		idx++;
		std::cout << "time: " << time << std::endl;
	}

	animation->mChannels[0] = channel;
	scene->mAnimations = new aiAnimation*[1];
	scene->mAnimations[0] = animation;
	scene->mNumAnimations = 1;

	// Use Assimp's exporter to write the scene to an FBX file
	Assimp::Exporter exporter;
	if (exporter.Export(scene, "fbx", "rotating_box.fbx") == AI_SUCCESS)
	{
		std::cout << "FBX animation exported successfully." << std::endl;
	}
	else
	{
		std::cerr << "Error exporting FBX animation: " << exporter.GetErrorString() << std::endl;
	}

	// Clean up the memory (Assimp usually handles this for you after export)
	delete scene;

	return 0;
}