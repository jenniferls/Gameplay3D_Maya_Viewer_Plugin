#include "maya_includes.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <queue>

#include "ComLib.h"
#include "MessageTypes.h"

MCallbackIdArray callbackIdArray;
static MCallbackId meshAddedCallbackID;

enum NODE_TYPE { TRANSFORM, MESH };
//MTimer gTimer;
double timeElapsed = 0;

// keep track of created meshes to maintain them
std::queue<MObject> newMeshes;

ComLib g_comlib("MayaComLib", 200, ComLib::PRODUCER);

//Function declarations
EXPORT MStatus initializePlugin(MObject obj);
EXPORT MStatus uninitializePlugin(MObject obj);
MStatus registerAllCallbacks();
MStatus checkScene();
MIntArray getlocalIndex(MIntArray& getVerts, MIntArray& getTris);
void getVertexData(std::vector<VertexMessage>& getVertices, MFnMesh& mesh);
void getTransformData(TransformMessage& getTransform, MObject& node);
void getMaterialData(MaterialMessage& getMaterial, MFnMesh& mesh);
void recursiveTransformUpdate(MFnDagNode& transform);
void nodeAdded(MObject& node, void* clientData);
void nodeRemoved(MObject& node, void* clientData);
void nodeRenamed(MObject& node, const MString& oldName, void* clientData);
void meshAttributeChanged(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void* x);
void matColorAttributeChanged(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& otherPlug, void* x);
void matAttributeChanged(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void* x);
void newTextureChange(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& otherPlug, void* x);
void newMeshAttributeChanged(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& otherPlug, void* x);
void timerCallback(float elapsedTime, float lastTime, void* clientData);
void topologyChanged(MObject& node, void* clientData);
void viewportChanged(const MString& str, void* clientData);

/*
 * Plugin entry point
 * For remote control of maya
 * open command port: commandPort -nr -name ":1234"
 * close command port: commandPort -cl -name ":1234"
 * send command: see loadPlugin.py and unloadPlugin.py
 */
EXPORT MStatus initializePlugin(MObject obj) {

	MStatus res = MS::kSuccess;

	MFnPlugin myPlugin(obj, "level editor", "1.0", "Any", &res);

	if (MFAIL(res)) {
		CHECK_MSTATUS(res);
		return res;
	}

	// redirect cout to cerr, so that when we do cout goes to cerr
	// in the maya output window (not the scripting output!)
	std::cout.set_rdbuf(MStreamUtils::stdOutStream().rdbuf());
	std::cerr.set_rdbuf(MStreamUtils::stdErrorStream().rdbuf());
	cout << "Viewer plugin loaded ===========================" << endl;

	res = registerAllCallbacks(); //Register all callbacks and check if successful
	res = checkScene(); //Check the scene for already existing meshes

	// a handy timer, courtesy of Maya
	//gTimer.clear();
	//gTimer.beginTimer();

	return res;
}


EXPORT MStatus uninitializePlugin(MObject obj) {
	MFnPlugin plugin(obj);

	cout << "Plugin unloaded =========================" << endl;

	MMessage::removeCallbacks(callbackIdArray);

	return MS::kSuccess;
}

MStatus registerAllCallbacks() {
	MStatus status = MS::kSuccess;

	callbackIdArray.append(MDGMessage::addNodeAddedCallback(nodeAdded, "dependNode", NULL, &status));
	callbackIdArray.append(MDGMessage::addNodeRemovedCallback(nodeRemoved, "dependNode", NULL, &status));
	callbackIdArray.append(MUiMessage::add3dViewPostRenderMsgCallback("modelPanel1", viewportChanged, NULL, &status));
	callbackIdArray.append(MUiMessage::add3dViewPostRenderMsgCallback("modelPanel2", viewportChanged, NULL, &status));
	callbackIdArray.append(MUiMessage::add3dViewPostRenderMsgCallback("modelPanel3", viewportChanged, NULL, &status));
	callbackIdArray.append(MUiMessage::add3dViewPostRenderMsgCallback("modelPanel4", viewportChanged, NULL, &status));
	callbackIdArray.append(MTimerMessage::addTimerCallback(5.0, timerCallback, NULL, &status));

	callbackIdArray.append(MNodeMessage::addAttributeChangedCallback(MObject(), newTextureChange, NULL, &status));

	cout << status << endl;
	return status;
}

MStatus checkScene() {
	MStatus status = MS::kSuccess;
	//Iterate meshes
	MItDag meshIt(MItDag::kDepthFirst, MFn::kMesh, &status); //Mesh iterator
	if (status == MS::kSuccess) {
		while (!meshIt.isDone()) {
			MFnMesh mesh = meshIt.item(); //Get the mesh from the iterator
			if (!mesh.isIntermediateObject()) { //Intermediate objects are often temporary and not drawn in the scene
				newMeshes.push(mesh.object());
				callbackIdArray.append(MPolyMessage::addPolyTopologyChangedCallback(mesh.object(), topologyChanged, NULL));
				callbackIdArray.append(MNodeMessage::addNameChangedCallback(mesh.object(), nodeRenamed, NULL, &status));
				callbackIdArray.append(MNodeMessage::addAttributeChangedCallback(mesh.parent(0), meshAttributeChanged, NULL, &status)); //Transform changes
				callbackIdArray.append(MNodeMessage::addAttributeChangedCallback(mesh.object(), meshAttributeChanged, NULL, &status)); //Vertex changes
				callbackIdArray.append(MNodeMessage::addAttributeChangedCallback(mesh.object(), matAttributeChanged, (void*)mesh.name().asChar(), &status));

				//Gather information
				MessageType type = MESH_ADDED;
				std::vector<VertexMessage> vertices;
				getVertexData(vertices, mesh);
				MeshMessage meshInfo;
				memcpy(&meshInfo.name, mesh.name().asChar(), NAME_SIZE);
				meshInfo.vertexCount = vertices.size();
				TransformMessage transformInfo;
				getTransformData(transformInfo, mesh.parent(0)); //Send in the parent (shape node)
				MaterialMessage matInfo;
				getMaterialData(matInfo, mesh);

				//Create and send message
				size_t msgSize = sizeof(MessageType) + sizeof(MeshMessage) + (sizeof(VertexMessage) * vertices.size()) + sizeof(TransformMessage) + sizeof(MaterialMessage);
				void* msg = new char[msgSize];

				memcpy(msg, &type, sizeof(MessageType));
				memcpy((char*)msg + sizeof(MessageType), &meshInfo, sizeof(meshInfo));
				memcpy((char*)msg + sizeof(MessageType) + sizeof(MeshMessage), vertices.data(), (sizeof(VertexMessage) * vertices.size()));
				memcpy((char*)msg + sizeof(MessageType) + sizeof(MeshMessage) + (sizeof(VertexMessage) * vertices.size()), &transformInfo, sizeof(TransformMessage));
				memcpy((char*)msg + sizeof(MessageType) + sizeof(MeshMessage) + (sizeof(VertexMessage) * vertices.size()) + sizeof(TransformMessage), &matInfo, sizeof(MaterialMessage));
				g_comlib.send(msg, msgSize);

				delete msg;
			}
			meshIt.next();
		}
	}
	else {
		cout << "ERROR: Could not create mesh iterator" << endl;
	}

	//Iterate cameras
	MItDag camIt(MItDag::kDepthFirst, MFn::kCamera, &status);
	if (status == MS::kSuccess) {
		while (!camIt.isDone()) {
			MFnCamera camera = camIt.item();

			//Gather information
			MessageType type = CAMERA_ADDED;
			CameraMessage camInfo;
			memcpy(&camInfo.name, camera.name().asChar(), NAME_SIZE);
			camInfo.type = camera.isOrtho() ? ORTHOGRAPHIC_CAM : PERSPECTIVE_CAM;
			camInfo.viewWidth = camera.orthoWidth();
			camInfo.farPlane = camera.farClippingPlane();
			camInfo.nearPlane = camera.nearClippingPlane();
			camInfo.FoV = camera.horizontalFieldOfView();
			camInfo.aspectRatio = camera.aspectRatio();

			//Create and send message
			size_t msgSize = sizeof(MessageType) + sizeof(CameraMessage);
			void* msg = new char[msgSize];

			memcpy(msg, &type, sizeof(MessageType));
			memcpy((char*)msg + sizeof(MessageType), &camInfo, sizeof(camInfo));
			g_comlib.send(msg, msgSize);

			delete msg;

			camIt.next();
		}
	}

	////Iterate materials
	//MItDependencyNodes matIt(MFn::kLambert, &status); //All material types are derived from lambert
	//if (status == MS::kSuccess) {
	//	while (!matIt.isDone()) {
	//		MFnLambertShader lambert(matIt.item());
	//		cout << "Shader in scene: " << lambert.name() << endl; //Debug

	//		matIt.next();
	//	}
	//}

	return status;
}

/*
 * how Maya calls this method when a node is added.
 * new POLY mesh: kPolyXXX, kTransform, kMesh
 * new MATERIAL : kBlinn, kShadingEngine, kMaterialInfo
 * new LIGHT    : kTransform, [kPointLight, kDirLight, kAmbientLight]
 * new JOINT    : kJoint
 */
void nodeAdded(MObject& node, void* clientData) {
	MString nodeName;
	MDagPath dagPath;
	MFnDagNode dagNodeFn(node);
	dagNodeFn.getPath(dagPath);
	MStatus status = MS::kSuccess;

	if (!node.isNull()) {
		switch (node.apiType()) {
		case MFn::kMesh: {
			meshAddedCallbackID = MNodeMessage::addAttributeChangedCallback(node, newMeshAttributeChanged, NULL); //Create a callback to check for when the object is usable
			break;
		}
		}
	}
}

void nodeRemoved(MObject& node, void* clientData) {
	if (node.apiType() == MFn::kMesh) {
		MFnDagNode dagNode(node);
		MFnMesh mesh(node);
		cout << dagNode.name() << " was removed!" << endl;
		cout << endl;

		void* msg = new char[sizeof(MessageType) + sizeof(MeshMessage)];
		MessageType type = MESH_REMOVED;
		MeshMessage meshInfo;
		memcpy(&meshInfo.name, dagNode.name().asChar(), NAME_SIZE);
		meshInfo.vertexCount = mesh.numVertices();

		memcpy(msg, &type, sizeof(MessageType));
		memcpy((char*)msg + sizeof(MessageType), &meshInfo, sizeof(meshInfo));
		g_comlib.send(msg, sizeof(MessageType) + sizeof(MeshMessage)); //We can't do sizeof(msg) since it's a void*
		delete msg;
	}
	else if (node.apiType() == MFn::kPointLight) {

	}
	//else {
	//	//cout << "A node was removed!" << endl;
	//	//cout << endl;
	//}
}

void nodeRenamed(MObject& node, const MString& oldName, void* clientData) {
	MString nodeName;
	MDagPath dagPath;
	MFnDagNode dagNodeFn(node);
	dagNodeFn.getPath(dagPath);

	if (!node.isNull()) {
		switch (node.apiType()) {
		case MFn::kMesh: {
			MFnMesh mesh(node);
			if (!mesh.isIntermediateObject()) {
				cout << "A node name was changed!" << endl;
				cout << "Old name: " << oldName << endl;
				cout << "New name: " << dagNodeFn.name() << endl;
				cout << "DAG Path: " << dagPath.fullPathName() << endl;
				cout << endl;

				MaterialMessage tempMsg;
				getMaterialData(tempMsg, mesh); //Temporary, not very great solution

				void* msg = new char[sizeof(MessageType) + sizeof(MeshMessage)];
				MessageType type = MESH_RENAMED;
				MeshMessage meshInfo;
				memcpy(&meshInfo.oldName, oldName.asChar(), NAME_SIZE);
				memcpy(&meshInfo.name, dagNodeFn.name().asChar(), NAME_SIZE);
				meshInfo.vertexCount = mesh.numVertices();

				memcpy(msg, &type, sizeof(MessageType));
				memcpy((char*)msg + sizeof(MessageType), &meshInfo, sizeof(meshInfo));
				g_comlib.send(msg, sizeof(MessageType) + sizeof(MeshMessage)); //We can't do sizeof(msg) since it's a void*
				delete msg;
			}
			break;
		}
		}
	}
}

MIntArray getlocalIndex(MIntArray& getVerts, MIntArray& getTri) { //Converts from object-relative index to face-relative index
	MIntArray localIndex;
	size_t gv, gt;
	if (getTri.length() == 3) { //Only handle triangles
		for (gt = 0; gt < getTri.length(); gt++) {
			for (gv = 0; gv < getVerts.length(); gv++) {
				if (getTri[gt] == getVerts[gv]) {
					localIndex.append(gv);
					break;
				}
			}
			// if nothing was added, add default "no match"
			if (localIndex.length() == gt) {
				localIndex.append(-1);
			}
		}
	}
	else {
		cout << "ERROR in call to getLocalIndex function!" << endl;
	}

	return localIndex;
}

void getVertexData(std::vector<VertexMessage>& getVertices, MFnMesh& mesh) {
	//Gather data from mesh
	MPointArray pointArray;
	mesh.getPoints(pointArray);
	MFloatVectorArray normalArray;
	mesh.getNormals(normalArray);
	MStringArray uvSetNames;
	mesh.getUVSetNames(uvSetNames);
	MFloatArray uArray;
	MFloatArray vArray;
	mesh.getUVs(uArray, vArray, &uvSetNames[0]);

	// Optimization //
	MIntArray triCount;
	MIntArray triVerts;
	mesh.getTriangles(triCount, triVerts);
	getVertices.resize(triVerts.length()); //Resizing the vector beforehand to avoid pushing back elements
	size_t counter = 0;
	//////////////////

	MItMeshPolygon polyIt(mesh.object()); //Iterator
	for (; !polyIt.isDone(); polyIt.next()) { //Goes through every polygon in the mesh
		//Points for this polygon
		MIntArray polyVerts;
		polyIt.getVertices(polyVerts);

		int triCount;
		polyIt.numTriangles(triCount); //Triangles in this polygon
		while (triCount--) { //Becomes false when it becomes negative
			MPointArray arr;
			MIntArray localIndex;
			MIntArray triVerts;

			polyIt.getTriangle(triCount, arr, triVerts);
			VertexMessage tempVertex[3];
			for (int i = 0; i < 3; i++) {
				tempVertex[i].pos[0] = pointArray[triVerts[i]][0];
				tempVertex[i].pos[1] = pointArray[triVerts[i]][1];
				tempVertex[i].pos[2] = pointArray[triVerts[i]][2];
			}

			localIndex = getlocalIndex(polyVerts, triVerts);
			// Get UVs for each vertex in the polygon
			int uvID[3];
			for (int polyVtx = 0; polyVtx < 3; polyVtx++) {
				polyIt.getUVIndex(localIndex[polyVtx], uvID[polyVtx], &uvSetNames[0]);
			}

			for (int i = 0; i < 3; i++) {
				tempVertex[i].normal[0] = normalArray[polyIt.normalIndex(localIndex[i])][0];
				tempVertex[i].normal[1] = normalArray[polyIt.normalIndex(localIndex[i])][1];
				tempVertex[i].normal[2] = normalArray[polyIt.normalIndex(localIndex[i])][2];

				tempVertex[i].uv[0] = uArray[uvID[i]];
				tempVertex[i].uv[1] = vArray[uvID[i]];
			}

			getVertices[counter] = tempVertex[0];
			getVertices[counter + 1] = tempVertex[1];
			getVertices[counter + 2] = tempVertex[2];

			counter += 3;
		}
	}
}

void getTransformData(TransformMessage& getTransform, MObject& node) {
	MFnTransform transform(node);
	MDagPath dagPath;
	transform.getPath(dagPath);
	MMatrix objToWorldMatrix = dagPath.inclusiveMatrix();
	float matrixValues[4][4];
	objToWorldMatrix.get(matrixValues);

	getTransform.transformationMatrix[0] = matrixValues[0][0];
	getTransform.transformationMatrix[1] = matrixValues[0][1];
	getTransform.transformationMatrix[2] = matrixValues[0][2];
	getTransform.transformationMatrix[3] = matrixValues[0][3];
	getTransform.transformationMatrix[4] = matrixValues[1][0];
	getTransform.transformationMatrix[5] = matrixValues[1][1];
	getTransform.transformationMatrix[6] = matrixValues[1][2];
	getTransform.transformationMatrix[7] = matrixValues[1][3];
	getTransform.transformationMatrix[8] = matrixValues[2][0];
	getTransform.transformationMatrix[9] = matrixValues[2][1];
	getTransform.transformationMatrix[10] = matrixValues[2][2];
	getTransform.transformationMatrix[11] = matrixValues[2][3];
	getTransform.transformationMatrix[12] = matrixValues[3][0];
	getTransform.transformationMatrix[13] = matrixValues[3][1];
	getTransform.transformationMatrix[14] = matrixValues[3][2];
	getTransform.transformationMatrix[15] = matrixValues[3][3];
}

void getMaterialData(MaterialMessage& getMaterial, MFnMesh& mesh) {
	MStatus status = MS::kSuccess;
	MObjectArray shaders;
	MIntArray shaderIndices;
	mesh.getConnectedShaders(0, shaders, shaderIndices);

	MFnDependencyNode shadingGroup(shaders[0]);
	MPlug shaderPlug = shadingGroup.findPlug("surfaceShader");

	MPlugArray shaderConnections;
	shaderPlug.connectedTo(shaderConnections, true, false);
	MObject shaderNode = shaderConnections[0].node();
	MFnDependencyNode shaderDepNode(shaderNode);

	callbackIdArray.append(MNodeMessage::addNameChangedCallback(shaderNode, nodeRenamed, NULL, &status));
	callbackIdArray.append(MNodeMessage::addAttributeChangedCallback(shaderNode, matColorAttributeChanged, (void*)mesh.name().asChar(), &status));

	if (shaderNode.hasFn(MFn::kPhong)) {
		MPlug cosinePowerPlug = shaderDepNode.findPlug("cosinePower");
		cosinePowerPlug.getValue(getMaterial.specularPower);
	}

	MPlug colorPlug = shaderDepNode.findPlug("color");
	MObject colorData;
	colorPlug.getValue(colorData);
	MFnNumericData colorValues(colorData);
	float r, g, b;
	colorValues.getData(r, g, b);

	MPlugArray colorPlugArr;
	colorPlug.connectedTo(colorPlugArr, true, false);

	if (colorPlugArr.length() > 0) {
		MObject texPathNode(colorPlugArr[0].node());
		MPlug texPathPlug = MFnDependencyNode(texPathNode).findPlug("fileTextureName", &status);
		if (status == MS::kSuccess) {
			MString textureName;
			texPathPlug.getValue(textureName);

			memcpy(&getMaterial.diffuseTexPath, textureName.asChar(), NAME_SIZE);

			//cout << "In getMaterialData: " << textureName << endl; //Debug
		}
	}

	memcpy(&getMaterial.name, &shaderDepNode.name(), NAME_SIZE);
	getMaterial.color[0] = r;
	getMaterial.color[1] = g;
	getMaterial.color[2] = b;
}

void recursiveTransformUpdate(MFnDagNode& transform) {
	MFnDagNode childNode = transform.child(0);

	MessageType type = MESH_TRANSFORM_CHANGED;
	MeshMessage meshInfo;
	memcpy(&meshInfo.name, childNode.name().asChar(), NAME_SIZE);
	TransformMessage transformInfo;
	getTransformData(transformInfo, transform.object());

	//Create and send message
	size_t msgSize = sizeof(MessageType) + sizeof(MeshMessage) + sizeof(TransformMessage);
	void* msg = new char[msgSize];

	memcpy(msg, &type, sizeof(MessageType));
	memcpy((char*)msg + sizeof(MessageType), &meshInfo, sizeof(meshInfo));
	memcpy((char*)msg + sizeof(MessageType) + sizeof(MeshMessage), &transformInfo, sizeof(TransformMessage));
	g_comlib.send(msg, msgSize); //We can't do sizeof(msg) since it's a void*

	delete msg;

	for (unsigned int i = 0; i < transform.childCount(); i++) {
		if (transform.child(i).apiType() == MFn::Type::kTransform) {
			MFnDagNode childNode(transform.child(i));
			recursiveTransformUpdate(childNode);
		}
	}
}

void newMeshAttributeChanged(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& otherPlug, void* x) {
	if (plug.node().apiType() == MFn::kMesh) {
		MStatus status = MS::kSuccess;
		MFnMesh mesh(plug.node(), &status);
		if (status == MS::kSuccess) {
			//Push the mesh and add callbacks
			newMeshes.push(mesh.object());
			callbackIdArray.append(MPolyMessage::addPolyTopologyChangedCallback(mesh.object(), topologyChanged, NULL));
			callbackIdArray.append(MNodeMessage::addNameChangedCallback(mesh.object(), nodeRenamed, NULL));
			callbackIdArray.append(MNodeMessage::addAttributeChangedCallback(mesh.parent(0), meshAttributeChanged, NULL, &status)); //Transform changes
			callbackIdArray.append(MNodeMessage::addAttributeChangedCallback(mesh.object(), meshAttributeChanged, NULL, &status)); //Vertex changes
			callbackIdArray.append(MNodeMessage::addAttributeChangedCallback(mesh.object(), matAttributeChanged, (void*)mesh.name().asChar(), &status)); //Material chnages

			MessageType type = MESH_ADDED;
			std::vector<VertexMessage> vertices;
			getVertexData(vertices, mesh);
			MeshMessage meshInfo;
			memcpy(&meshInfo.name, mesh.name().asChar(), NAME_SIZE);
			meshInfo.vertexCount = vertices.size();
			TransformMessage transformInfo;
			getTransformData(transformInfo, mesh.parent(0));
			MaterialMessage matInfo;
			getMaterialData(matInfo, mesh);

			//Create and send message
			size_t msgSize = sizeof(MessageType) + sizeof(MeshMessage) + (sizeof(VertexMessage) * vertices.size()) + sizeof(TransformMessage) + sizeof(MaterialMessage);
			void* msg = new char[msgSize];

			memcpy(msg, &type, sizeof(MessageType));
			memcpy((char*)msg + sizeof(MessageType), &meshInfo, sizeof(meshInfo));
			memcpy((char*)msg + sizeof(MessageType) + sizeof(MeshMessage), vertices.data(), (sizeof(VertexMessage) * vertices.size()));
			memcpy((char*)msg + sizeof(MessageType) + sizeof(MeshMessage) + (sizeof(VertexMessage) * vertices.size()), &transformInfo, sizeof(TransformMessage));
			memcpy((char*)msg + sizeof(MessageType) + sizeof(MeshMessage) + (sizeof(VertexMessage) * vertices.size()) + sizeof(TransformMessage), &matInfo, sizeof(MaterialMessage));
			g_comlib.send(msg, msgSize);

			delete msg;

			MMessage::removeCallback(meshAddedCallbackID);
		}
		//else {
		//	cout << "Mesh not ready yet." << endl; //Debug
		//}
	}
}

void meshAttributeChanged(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void* x) {
	if (plug.node().apiType() == MFn::kMesh) {
		if (msg & MNodeMessage::kAttributeSet) {
			MStatus status = MS::kSuccess;
			MPlug vtx = MFnDependencyNode(plug.node()).findPlug("pnts", &status); //Check if vertex
			if (status == MS::kSuccess) {
				MDagPath dagPath;
				MFnDagNode dagNode(plug.node());
				dagNode.getPath(dagPath);

				unsigned int index = plug.logicalIndex();
				if (index != -1) {
					MFnMesh mesh(plug.node());
					MPoint pnt;
					mesh.getPoint(index, pnt);

					//Gather information
					MessageType type = MESH_TOPOLOGY_CHANGED;
					std::vector<VertexMessage> vertices;
					getVertexData(vertices, mesh);
					MeshMessage meshInfo;
					memcpy(&meshInfo.name, mesh.name().asChar(), NAME_SIZE);
					meshInfo.vertexCount = vertices.size();

					//Create and send message
					size_t msgSize = sizeof(MessageType) + sizeof(MeshMessage) + (sizeof(VertexMessage) * vertices.size());
					void* msg = new char[msgSize];

					memcpy(msg, &type, sizeof(MessageType));
					memcpy((char*)msg + sizeof(MessageType), &meshInfo, sizeof(meshInfo));
					memcpy((char*)msg + sizeof(MessageType) + sizeof(MeshMessage), vertices.data(), (sizeof(VertexMessage) * vertices.size()));
					g_comlib.send(msg, msgSize);

					delete msg;
				}
			}
		}
	}

	if (plug.node().apiType() == MFn::kTransform) {
		MFnDagNode parentNode(plug.node());

		recursiveTransformUpdate(parentNode);

		//cout << "The transform node " << plug.name() << " has changed!" << endl;
		//cout << endl;
	}
}

void matAttributeChanged(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &otherPlug, void* x) {
	if (msg & MNodeMessage::kConnectionMade) { //This could be improved
		MStatus status = MS::kSuccess;

		MObjectArray shaders;
		MIntArray shaderIndices;
		MFnMesh mesh(plug.node());
		mesh.getConnectedShaders(0, shaders, shaderIndices);

		MFnDependencyNode shadingGroup(shaders[0]);
		MPlug shaderPlug = shadingGroup.findPlug("surfaceShader");

		MPlugArray shaderConnections;
		shaderPlug.connectedTo(shaderConnections, true, false);
		MObject shaderNode = shaderConnections[0].node();
		MFnDependencyNode shaderDepNode(shaderNode);

		callbackIdArray.append(MNodeMessage::addAttributeChangedCallback(shaderNode, matColorAttributeChanged, (void*)mesh.name().asChar(), &status));

		//Gather information
		MessageType type = MATERIAL_CHANGED;
		MeshMessage meshInfo;
		memcpy(&meshInfo.name, mesh.name().asChar(), NAME_SIZE);
		MaterialMessage matInfo;

		if (shaderNode.hasFn(MFn::kPhong)) {
			MPlug cosinePowerPlug = shaderDepNode.findPlug("cosinePower");
			cosinePowerPlug.getValue(matInfo.specularPower);
		}

		MPlug colorPlug = shaderDepNode.findPlug("color");
		MObject colorData;
		colorPlug.getValue(colorData);
		MFnNumericData colorValues(colorData);
		float r, g, b;
		colorValues.getData(r, g, b);

		MPlugArray colorPlugArr;
		colorPlug.connectedTo(colorPlugArr, true, false);
		if (colorPlugArr.length() > 0) {
			MObject texPathNode(colorPlugArr[0].node());
			MPlug texPathPlug = MFnDependencyNode(texPathNode).findPlug("fileTextureName", &status);
			if (status == MS::kSuccess) {
				MString textureName;
				texPathPlug.getValue(textureName);

				memcpy(&matInfo.diffuseTexPath, textureName.asChar(), NAME_SIZE);

				//cout << textureName << endl; //Debug
			}
		}

		memcpy(&matInfo.name, &shaderDepNode.name(), NAME_SIZE);
		matInfo.color[0] = r;
		matInfo.color[1] = g;
		matInfo.color[2] = b;

		//Create and send message
		size_t msgSize = sizeof(MessageType) + sizeof(MeshMessage) + sizeof(MaterialMessage);
		void* msg = new char[msgSize];

		memcpy(msg, &type, sizeof(MessageType));
		memcpy((char*)msg + sizeof(MessageType), &meshInfo, sizeof(meshInfo));
		memcpy((char*)msg + sizeof(MessageType) + sizeof(MeshMessage), &matInfo, sizeof(matInfo));
		g_comlib.send(msg, msgSize);

		delete msg;

		//cout << "connection made for: " << plug.name() << ",  " << plug.partialName() << endl; //Debug
	}
}

void matColorAttributeChanged(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& otherPlug, void* x) {
	if (msg & MNodeMessage::kAttributeSet) {
		MFnDependencyNode shaderDepNode(plug.node());
		MStatus status = MS::kSuccess;

		//Gather information
		MessageType type = MATERIAL_CHANGED;
		MeshMessage meshInfo;
		memcpy(&meshInfo.name, (char*)x, NAME_SIZE);
		MaterialMessage matInfo;

		if (plug.node().hasFn(MFn::kPhong)) {
			MPlug cosinePowerPlug = shaderDepNode.findPlug("cosinePower");
			cosinePowerPlug.getValue(matInfo.specularPower);
		}

		MPlug colorPlug = shaderDepNode.findPlug("color");
		MObject colorData;
		colorPlug.getValue(colorData);
		MFnNumericData colorValues(colorData);
		float r, g, b;
		colorValues.getData(r, g, b);

		MPlugArray colorPlugArr;
		colorPlug.connectedTo(colorPlugArr, true, false);
		if (colorPlugArr.length() > 0) {
			MObject texPathNode(colorPlugArr[0].node());
			MPlug texPathPlug = MFnDependencyNode(texPathNode).findPlug("fileTextureName", &status);
			if (status == MS::kSuccess) {
				MString textureName;
				texPathPlug.getValue(textureName);

				memcpy(&matInfo.diffuseTexPath, textureName.asChar(), NAME_SIZE);

				//cout << "Inside matColorAttributeChanged " << textureName << endl; //Debug
			}
		}

		memcpy(&matInfo.name, &shaderDepNode.name(), NAME_SIZE);
		matInfo.color[0] = r;
		matInfo.color[1] = g;
		matInfo.color[2] = b;

		//Create and send message
		size_t msgSize = sizeof(MessageType) + sizeof(MeshMessage) + sizeof(MaterialMessage);
		void* msg = new char[msgSize];

		memcpy(msg, &type, sizeof(MessageType));
		memcpy((char*)msg + sizeof(MessageType), &meshInfo, sizeof(meshInfo));
		memcpy((char*)msg + sizeof(MessageType) + sizeof(MeshMessage), &matInfo, sizeof(matInfo));
		g_comlib.send(msg, msgSize);

		delete msg;

		//cout << "Material changed for mesh: " << meshInfo.name << endl; //Debug
	}
}

void newTextureChange(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& otherPlug, void* x) {
	MStatus status = MS::kSuccess;
	if (msg & MNodeMessage::kAttributeSet) {
		if (plug.node().apiType() == MFn::kFileTexture) {
			if (!plug.node().isNull()) {
				//cout << "Triggering plug: " << plug.name() << endl; //Debug

				MFnDependencyNode depNode(plug.node());
				MPlugArray plugArray;
				depNode.getConnections(plugArray);

				for (int i = 0; i < plugArray.length(); i++) {
					//cout << plugArray[i].name() << endl; //Debug
					MString textureName;

					MPlugArray connections;
					plugArray[i].connectedTo(connections, true, true);

					MObject shaderNode = connections[0].node();
					MFnLambertShader lambertShader(shaderNode);

					//cout << "Lambert shader name: " << lambertShader.name() << endl; //Debug

					float specularPower = 0.0f;

					if (shaderNode.hasFn(MFn::kPhong)) {
						MPlug cosinePowerPlug = lambertShader.findPlug("cosinePower");
						cosinePowerPlug.getValue(specularPower);
					}

					MPlug colorPlug = lambertShader.findPlug("color", &status);

					MPlugArray colorConnections;
					colorPlug.connectedTo(colorConnections, true, false);
					if (colorConnections.length() > 0) {
						MObject texPathNode(colorConnections[0].node());
						MPlug texPathPlug = MFnDependencyNode(texPathNode).findPlug("fileTextureName", &status);
						if (status == MS::kSuccess) {
							texPathPlug.getValue(textureName);

							//cout << "Texture name: " << textureName << endl; //Debug
						}
					}

					MPlugArray lambertPlugs;
					lambertShader.getConnections(lambertPlugs);

					if (lambertPlugs.length() > 1) {
						MPlug outColorPlug(lambertPlugs[1]); //OutColor
						//cout << "OutColorPlug name: " << outColorPlug.name() << ", " << plug.partialName() << endl; //Debug
						MPlugArray outColorPlugs;
						outColorPlug.connectedTo(outColorPlugs, true, true);

						MFnDependencyNode matInfoNode(outColorPlugs[0].node());
						MPlugArray matInfoPlugs;
						matInfoNode.getConnections(matInfoPlugs);

						for (int j = 0; j < matInfoPlugs.length(); j++) {
							MFnAttribute myAttr = matInfoPlugs[j].attribute();
							MString attributeName = myAttr.name();
							//cout << "Attribute name: " << attributeName << endl; //Debug
							if (attributeName == "dagSetMembers") {
								MPlug memberPlug = matInfoPlugs[j];
								MPlugArray memberPlugs;
								memberPlug.connectedTo(memberPlugs, true, true);
								for (int k = 0; k < memberPlugs.length(); k++) {
									MFnMesh mesh(memberPlugs[k].node(), &status);
									if (status == MS::kSuccess) {
										//cout << "Mesh name: " << mesh.name() << endl; //Debug

										MessageType type = MATERIAL_CHANGED;
										MeshMessage meshInfo;
										memcpy(&meshInfo.name, mesh.name().asChar(), NAME_SIZE);
										MaterialMessage matInfo;
										memcpy(&matInfo.diffuseTexPath, textureName.asChar(), NAME_SIZE);

										matInfo.color[0] = 1.0f;
										matInfo.color[1] = 1.0f;
										matInfo.color[2] = 1.0f;

										matInfo.specularPower = specularPower;

										//Create and send message
										size_t msgSize = sizeof(MessageType) + sizeof(MeshMessage) + sizeof(MaterialMessage);
										void* msg = new char[msgSize];

										memcpy(msg, &type, sizeof(MessageType));
										memcpy((char*)msg + sizeof(MessageType), &meshInfo, sizeof(meshInfo));
										memcpy((char*)msg + sizeof(MessageType) + sizeof(MeshMessage), &matInfo, sizeof(matInfo));
										g_comlib.send(msg, msgSize);

										delete msg;
									}
								}
							}
						}
					}
				}
				//cout << "file change" << endl; //Debug
			}
		}
	}
}

void timerCallback(float elapsedTime, float lastTime, void* clientData) {
	timeElapsed += elapsedTime;
	//cout << "Elapsed time: " << timeElapsed << " seconds" << endl;
	//cout << endl;
}

void topologyChanged(MObject& node, void* clientData) {
	if (!node.isNull()) {
		MStatus status = MS::kSuccess;
		MDagPath dagPath;
		MFnMesh mesh(node, &status);
		mesh.getPath(dagPath);
		//cout << "Topology has chaged!" << endl; //Debug
		//cout << "DAG Path: " << mesh.fullPathName() << endl; //Debug

		//Gather information
		MessageType type = MESH_TOPOLOGY_CHANGED;
		std::vector<VertexMessage> vertices;
		getVertexData(vertices, mesh);
		MeshMessage meshInfo;
		memcpy(&meshInfo.name, mesh.name().asChar(), NAME_SIZE);
		meshInfo.vertexCount = vertices.size();

		//Create and send message
		size_t msgSize = sizeof(MessageType) + sizeof(MeshMessage) + (sizeof(VertexMessage) * vertices.size());
		void* msg = new char[msgSize];

		memcpy(msg, &type, sizeof(MessageType));
		memcpy((char*)msg + sizeof(MessageType), &meshInfo, sizeof(meshInfo));
		memcpy((char*)msg + sizeof(MessageType) + sizeof(MeshMessage), vertices.data(), (sizeof(VertexMessage) * vertices.size()));
		g_comlib.send(msg, msgSize);

		delete msg;
	}
}

void viewportChanged(const MString& panelName, void* clientData) {
	MStatus status;
	M3dView view;
	status = M3dView::getM3dViewFromModelPanel(panelName, view);

	if (status == MS::kSuccess) {
		MDagPath camPath;
		view.getCamera(camPath);
		MFnCamera camera(camPath);
		MMatrix camPos = camPath.inclusiveMatrix();

		//Gather information
		MessageType type = VIEW_CHANGED;
		CameraMessage camInfo;
		memcpy(&camInfo.name, camera.name().asChar(), NAME_SIZE);
		camInfo.type = camera.isOrtho(&status) ? ORTHOGRAPHIC_CAM : PERSPECTIVE_CAM;
		camInfo.viewWidth = camera.orthoWidth();
		camInfo.farPlane = camera.farClippingPlane();
		camInfo.nearPlane = camera.nearClippingPlane();
		camInfo.FoV = camera.horizontalFieldOfView();
		camInfo.aspectRatio = camera.aspectRatio();

		float matrixValues[4][4];
		camPos.get(matrixValues);

		camInfo.transformationMatrix[0] = matrixValues[0][0];
		camInfo.transformationMatrix[1] = matrixValues[0][1];
		camInfo.transformationMatrix[2] = matrixValues[0][2];
		camInfo.transformationMatrix[3] = matrixValues[0][3];
		camInfo.transformationMatrix[4] = matrixValues[1][0];
		camInfo.transformationMatrix[5] = matrixValues[1][1];
		camInfo.transformationMatrix[6] = matrixValues[1][2];
		camInfo.transformationMatrix[7] = matrixValues[1][3];
		camInfo.transformationMatrix[8] = matrixValues[2][0];
		camInfo.transformationMatrix[9] = matrixValues[2][1];
		camInfo.transformationMatrix[10] = matrixValues[2][2];
		camInfo.transformationMatrix[11] = matrixValues[2][3];
		camInfo.transformationMatrix[12] = matrixValues[3][0];
		camInfo.transformationMatrix[13] = matrixValues[3][1];
		camInfo.transformationMatrix[14] = matrixValues[3][2];
		camInfo.transformationMatrix[15] = matrixValues[3][3];

		//Create and send message
		size_t msgSize = sizeof(MessageType) + sizeof(CameraMessage);
		void* msg = new char[msgSize];

		memcpy(msg, &type, sizeof(MessageType));
		memcpy((char*)msg + sizeof(MessageType), &camInfo, sizeof(camInfo));
		g_comlib.send(msg, msgSize);

		delete msg;
	}
}
