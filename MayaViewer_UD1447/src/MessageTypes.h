#pragma once

#define NAME_SIZE 64
#define PATH_SIZE 256

enum MessageType {
	NONE,
	MESH_ADDED,
	MESH_REMOVED,
	MESH_RENAMED,
	MESH_TRANSFORM_CHANGED,
	MESH_TOPOLOGY_CHANGED,
	CAMERA_ADDED,
	VIEW_CHANGED,
	MATERIAL_CHANGED,
	LIGHT_ADDED,
	LIGHT_REMOVED
};

enum CameraType {
	PERSPECTIVE_CAM,
	ORTHOGRAPHIC_CAM
};

struct MessageHeader {
	MessageType type = NONE;
};

struct MeshMessage {
	char name[NAME_SIZE] = "\0";
	char oldName[NAME_SIZE] = "\0";
	size_t vertexCount = 0;
};

struct VertexMessage {
	float pos[3] = { 0.0f };
	float normal[3] = { 0.0f };
	float uv[2] = { 0.0f };
};

struct TransformMessage {
	float transformationMatrix[16] = { 0.0f };
};

struct CameraMessage {
	CameraType type = PERSPECTIVE_CAM;
	char name[NAME_SIZE] = "\0";
	float transformationMatrix[16] = { 0.0f };
	float FoV = 0.0f;
	float aspectRatio = 0.0f;
	float farPlane = 0.0f;
	float nearPlane = 0.0f;
	float viewWidth = 0.0f;
};

struct MaterialMessage {
	char name[NAME_SIZE] = "\0";
	char oldName[NAME_SIZE] = "\0";
	float color[3] = { 0.0f };
	char diffuseTexPath[PATH_SIZE] = "\0";
	float specularPower = 0.0f;
};

struct LightMessage {

};