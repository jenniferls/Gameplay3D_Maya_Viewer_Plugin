#include "MayaViewer.h"

// Declare our game instance
MayaViewer game;

//constexpr int gModelCount = 0;
static bool gKeys[256] = {};
int gDeltaX;
int gDeltaY;
bool gMousePressed;

MayaViewer::MayaViewer() : _scene(NULL), _wireframe(false), _comLib("MayaComLib", 200, ComLib::CONSUMER) {

}

void MayaViewer::initialize() {
	DebugConsole::Init();
    // Load game scene from file
	_scene = Scene::create();

	//Create default camera
	Camera* cam = Camera::createPerspective(45.0f, getAspectRatio(), 1.0, 100.0f);
	Node* cameraNode = _scene->addNode("defaultCamera");
	cameraNode->setCamera(cam);
	_scene->setActiveCamera(cam);
	SAFE_RELEASE(cam);

	cameraNode->translate(0, 0, 10);
	cameraNode->rotateX(MATH_DEG_TO_RAD(0.f));

	_defaultLight = _scene->addNode("light");
	Light* light = Light::createPoint(Vector3(0.5f, 0.5f, 0.5f), 20);
	_defaultLight->setLight(light);
	SAFE_RELEASE(light);
	_defaultLight->translate(Vector3(0, 1, 5));
}

void MayaViewer::finalize() {
    SAFE_RELEASE(_scene);
}

void MayaViewer::update(float elapsedTime) {
	for (int i = 0; i < 20; i++) { //Worse framerate but potentially better response
		fetchMessage();
	}

	static float totalTime = 0;
	totalTime += elapsedTime;	

	Node* camnode = _scene->getActiveCamera()->getNode();
	if (gKeys[Keyboard::KEY_W])
		camnode->translateForward(0.5);
	if (gKeys[Keyboard::KEY_S])
		camnode->translateForward(-0.5);
	if (gKeys[Keyboard::KEY_A])
		camnode->translateLeft(0.5);
	if (gKeys[Keyboard::KEY_D])
		camnode->translateLeft(-0.5);

	if (gMousePressed) {
		camnode->rotate(camnode->getRightVectorWorld(), MATH_DEG_TO_RAD(gDeltaY / 10.0));
		camnode->rotate(camnode->getUpVectorWorld(), MATH_DEG_TO_RAD(gDeltaX / 5.0));
	}
}

void MayaViewer::render(float elapsedTime) {
    // Clear the color and depth buffers
    clear(CLEAR_COLOR_DEPTH, Vector4(0.25f, 0.35f, 0.58f, 1.0f), 1.0f, 0);

    // Visit all the nodes in the scene for drawing
    _scene->visit(this, &MayaViewer::drawScene);
}

bool MayaViewer::drawScene(Node* node) {
    // If the node visited contains a drawable object, draw it
    Drawable* drawable = node->getDrawable(); 
    if (drawable)
        drawable->draw(_wireframe);

    return true;
}

void MayaViewer::fetchMessage() {
	size_t messageLength = _comLib.nextSize();
	if (messageLength > 0) {
		char* msg = new char[messageLength];
		if (_comLib.recv(msg, messageLength) == true) {
			MessageHeader* header = (MessageHeader*)msg; //Remember that this is just a pointer to msg! It is deleted when msg is deleted.
			if (header->type == MESH_ADDED) {
				MeshMessage* meshInfo = (MeshMessage*)(msg + sizeof(MessageHeader));
				std::vector<VertexMessage> vertices;
				vertices.resize(meshInfo->vertexCount);
				for (size_t i = 0; i < meshInfo->vertexCount; i++) {
					vertices[i] = *(VertexMessage*)(msg + sizeof(MessageHeader) + sizeof(MeshMessage) + (sizeof(VertexMessage) * i));
					//std::cout << "Positions: " << vertices[i].pos[0] << ", " << vertices[i].pos[1] << ", " << vertices[i].pos[2] << std::endl; //Debug
					//std::cout << "Normals: " << vertices[i].normal[0] << ", " << vertices[i].normal[1] << ", " << vertices[i].normal[2] << std::endl;
					//std::cout << "UVs: " << vertices[i].uv[0] << ", " << vertices[i].uv[1] << std::endl;
				}
				TransformMessage* transformInfo = (TransformMessage*)(msg + sizeof(MessageHeader) + sizeof(MeshMessage) + (sizeof(VertexMessage) * meshInfo->vertexCount));
				Matrix* matrix = new Matrix();
				matrix->set(transformInfo->transformationMatrix);
				Vector3 translation, scale;
				Quaternion rotationQuat;
				matrix->decompose(&scale, &rotationQuat, &translation);
				MaterialMessage* matInfo = (MaterialMessage*)(msg + sizeof(MessageHeader) + sizeof(MeshMessage) + (sizeof(VertexMessage) * meshInfo->vertexCount) + sizeof(TransformMessage));

				addNewModel(meshInfo->name, vertices, matInfo);
				updateTransform(translation, rotationQuat, scale, meshInfo->name);
				std::cout << "A mesh with the name " << meshInfo->name << " was added!" << std::endl; //Debug
				delete matrix;
			}
			else if (header->type == MESH_REMOVED) {
				MeshMessage* meshInfo = (MeshMessage*)(msg + sizeof(MessageHeader));
				removeModel(meshInfo->name);
				std::cout << "A mesh with the name " << meshInfo->name << " was removed!" << std::endl; //Debug
			}
			else if (header->type == MESH_RENAMED) {
				MeshMessage* meshInfo = (MeshMessage*)(msg + sizeof(MessageHeader));
				renameModel(meshInfo->oldName, meshInfo->name);
				std::cout << meshInfo->oldName << " was renamed to: " << meshInfo->name << std::endl; //Debug
			}
			else if(header->type == MESH_TRANSFORM_CHANGED){
				MeshMessage* meshInfo = (MeshMessage*)(msg + sizeof(MessageHeader));
				TransformMessage* transformInfo = (TransformMessage*)(msg + sizeof(MessageHeader) + sizeof(MeshMessage));
				Matrix* matrix = new Matrix();
				matrix->set(transformInfo->transformationMatrix);
				Vector3 translation, scale;
				Quaternion rotationQuat;
				matrix->decompose(&scale, &rotationQuat, &translation);

				updateTransform(translation, rotationQuat, scale, meshInfo->name);

				//std::cout << "A mesh with the name " << meshInfo->name << " was transformed!" << std::endl; //Debug
				delete matrix;
			}
			else if (header->type == CAMERA_ADDED) {
				CameraMessage* camInfo = (CameraMessage*)(msg + sizeof(MessageHeader));
				//Create camera
				if (camInfo->type == PERSPECTIVE_CAM) {
					Camera* camera = Camera::createPerspective(MATH_RAD_TO_DEG(camInfo->FoV), getAspectRatio(), camInfo->nearPlane, camInfo->farPlane);
					Node* cameraNode = _scene->addNode(camInfo->name);
					cameraNode->setCamera(camera);
					SAFE_RELEASE(camera);
					//std::cout << "perspective camera created!" << std::endl; //Debug
				}
				else {
					Camera* camera = Camera::createOrthographic(camInfo->viewWidth, camInfo->viewWidth / getAspectRatio(), getAspectRatio(), camInfo->nearPlane, camInfo->farPlane);
					Node* cameraNode = _scene->addNode(camInfo->name);
					cameraNode->setCamera(camera);
					SAFE_RELEASE(camera);
					//std::cout << "orthographic camera created!" << std::endl; //Debug
				}
			}
			else if (header->type == VIEW_CHANGED) {
				CameraMessage* camInfo = (CameraMessage*)(msg + sizeof(MessageHeader));
				Node* camNode = _scene->findNode(camInfo->name);
				if (camNode) {
					if (camInfo->type == ORTHOGRAPHIC_CAM) { //To get the zoom working correctly, create a new camera
						Camera* camera = Camera::createOrthographic(camInfo->viewWidth, camInfo->viewWidth / getAspectRatio(), getAspectRatio(), camInfo->nearPlane, camInfo->farPlane);
						camNode->setCamera(camera);
						SAFE_RELEASE(camera);
					}

					_scene->setActiveCamera(camNode->getCamera());

					Matrix* matrix = new Matrix();
					matrix->set(camInfo->transformationMatrix);
					Vector3 translation, scale;
					Quaternion rotationQuat;
					matrix->decompose(&scale, &rotationQuat, &translation);

					camNode->setTranslation(translation);
					camNode->setRotation(rotationQuat);

					//std::cout << "Active camera is: " << camInfo->name << std::endl; //Debug
					delete matrix;
				}
			}
			else if (header->type == MATERIAL_CHANGED) {
				MeshMessage* meshInfo = (MeshMessage*)(msg + sizeof(MessageHeader));
				MaterialMessage* matInfo = (MaterialMessage*)(msg + sizeof(MessageHeader) + sizeof(MeshMessage));

				changeMaterial(meshInfo->name, matInfo);
			}
			else if (header->type == MESH_TOPOLOGY_CHANGED) {
				MeshMessage* meshInfo = (MeshMessage*)(msg + sizeof(MessageHeader));
				std::vector<VertexMessage> vertices;
				vertices.resize(meshInfo->vertexCount);
				for (size_t i = 0; i < meshInfo->vertexCount; i++) {
					vertices[i] = *(VertexMessage*)(msg + sizeof(MessageHeader) + sizeof(MeshMessage) + (sizeof(VertexMessage) * i));
				}
				updateModel(meshInfo->name, vertices);
			}

			delete msg; //Header and message gets deleted
		}
	}
}

void MayaViewer::keyEvent(Keyboard::KeyEvent evt, int key) {
    if (evt == Keyboard::KEY_PRESS) {
		gKeys[key] = true;
        switch (key) {
        case Keyboard::KEY_ESCAPE:
            exit();
            break;
		};
    }
	else if (evt == Keyboard::KEY_RELEASE){
		gKeys[key] = false;
	}
}

bool MayaViewer::mouseEvent(Mouse::MouseEvent evt, int x, int y, int wheelDelta) {
	static int lastX = 0;
	static int lastY = 0;
	gDeltaX = lastX - x;
	gDeltaY = lastY - y;
	lastX = x;
	lastY = y;
	gMousePressed =
		(evt == Mouse::MouseEvent::MOUSE_PRESS_LEFT_BUTTON) ? true :
		(evt == Mouse::MouseEvent::MOUSE_RELEASE_LEFT_BUTTON) ? false : gMousePressed;

	return true;
}

void MayaViewer::addNewModel(const char* modelName, std::vector<VertexMessage>& vertices, MaterialMessage* matInfo) {
	Mesh* mesh2 = createMesh(vertices);
	Model* tempModel = Model::create(mesh2);

	if (strcmp(matInfo->diffuseTexPath, "") != 0) {
		//Textured
		Material* tempMaterial;
		if (matInfo->specularPower > 0.0f) {
			tempMaterial = tempModel->setMaterial("res/shaders/textured.vert", "res/shaders/textured.frag", "POINT_LIGHT_COUNT 1;SPECULAR");
		}
		else {
			tempMaterial = tempModel->setMaterial("res/shaders/textured.vert", "res/shaders/textured.frag", "POINT_LIGHT_COUNT 1");
		}
		Texture::Sampler* tempSampler = tempMaterial->getParameter("u_diffuseTexture")->setValue(matInfo->diffuseTexPath, true);

		tempMaterial->setParameterAutoBinding("u_worldViewProjectionMatrix", "WORLD_VIEW_PROJECTION_MATRIX");
		tempMaterial->setParameterAutoBinding("u_inverseTransposeWorldViewMatrix", "INVERSE_TRANSPOSE_WORLD_VIEW_MATRIX");
		tempMaterial->getParameter("u_ambientColor")->setValue(Vector3(0.1f, 0.1f, 0.2f));
		tempMaterial->getParameter("u_pointLightColor[0]")->setValue(_defaultLight->getLight()->getColor());
		tempMaterial->getParameter("u_pointLightPosition[0]")->bindValue(_defaultLight, &Node::getTranslationWorld);
		tempMaterial->getParameter("u_pointLightRangeInverse[0]")->bindValue(_defaultLight->getLight(), &Light::getRangeInverse);

		tempMaterial->getParameter("u_specularExponent")->setValue(matInfo->specularPower);
		tempMaterial->setParameterAutoBinding("u_cameraPosition", "CAMERA_WORLD_POSITION");

		tempSampler->setFilterMode(Texture::LINEAR_MIPMAP_LINEAR, Texture::LINEAR);

		tempMaterial->getStateBlock()->setCullFace(true);
		tempMaterial->getStateBlock()->setDepthTest(true);
		tempMaterial->getStateBlock()->setDepthWrite(true);

		tempMaterial->getStateBlock()->setBlend(true);
		tempMaterial->getStateBlock()->setBlendSrc(RenderState::BLEND_SRC_ALPHA);
		tempMaterial->getStateBlock()->setBlendDst(RenderState::BLEND_ONE_MINUS_SRC_ALPHA);
	}
	else {
		//Colored
		Material* tempMaterial;
		if (matInfo->specularPower > 0.0f) {
			tempMaterial = tempModel->setMaterial("res/shaders/colored.vert", "res/shaders/colored.frag", "POINT_LIGHT_COUNT 1;SPECULAR");
		}
		else {
			tempMaterial = tempModel->setMaterial("res/shaders/colored.vert", "res/shaders/colored.frag", "POINT_LIGHT_COUNT 1");
		}
		
		tempMaterial->setParameterAutoBinding("u_worldViewProjectionMatrix", "WORLD_VIEW_PROJECTION_MATRIX");
		tempMaterial->setParameterAutoBinding("u_inverseTransposeWorldViewMatrix", "INVERSE_TRANSPOSE_WORLD_VIEW_MATRIX");
		tempMaterial->getParameter("u_diffuseColor")->setValue(Vector4(matInfo->color[0], matInfo->color[1], matInfo->color[2], 1.0f));
		tempMaterial->getParameter("u_ambientColor")->setValue(Vector3(0.1f, 0.1f, 0.2f));
		tempMaterial->getParameter("u_pointLightColor[0]")->setValue(_defaultLight->getLight()->getColor());
		tempMaterial->getParameter("u_pointLightPosition[0]")->bindValue(_defaultLight, &Node::getTranslationWorld);
		tempMaterial->getParameter("u_pointLightRangeInverse[0]")->bindValue(_defaultLight->getLight(), &Light::getRangeInverse);

		tempMaterial->getParameter("u_specularExponent")->setValue(matInfo->specularPower);
		tempMaterial->setParameterAutoBinding("u_cameraPosition", "CAMERA_WORLD_POSITION");

		tempMaterial->getStateBlock()->setCullFace(true);
		tempMaterial->getStateBlock()->setDepthTest(true);
		tempMaterial->getStateBlock()->setDepthWrite(true);
	}

	Node* node = _scene->addNode(modelName);
	node->setDrawable(tempModel);
	SAFE_RELEASE(tempModel);
	_modelCount += 1;
	//_materialCount += 1;
	_modelnames.push_back(modelName);
	//_materialNames.push_back(matInfo->name);
}

void MayaViewer::removeModel(const char* modelName) {
	Node* node = _scene->findNode(modelName);
	if (node) {
		_scene->removeNode(node);
		auto it = std::find(_modelnames.begin(), _modelnames.end(), modelName);
		if (it != _modelnames.end()) {
			swap(*it, _modelnames.back());
			_modelnames.pop_back();
		}
		_modelCount -= 1;
	}
}

void MayaViewer::renameModel(const char* oldName, const char* newName) {
	Node* node = _scene->findNode(oldName);
	if (node) {
		node->setId(newName);
		auto it = std::find(_modelnames.begin(), _modelnames.end(), oldName);
		if (it != _modelnames.end()) {
			swap(*it, _modelnames.back());
			_modelnames.pop_back();
			_modelnames.push_back(newName);
		}
	}
}

void MayaViewer::updateModel(const char* modelName, std::vector<VertexMessage>& vertices) {
	Node* node = _scene->findNode(modelName);
	if (node) {
		Model* oldModel = (Model*)node->getDrawable();
		Material* material = oldModel->getMaterial();

		Mesh* mesh = createMesh(vertices);
		Model* newModel = Model::create(mesh);
		newModel->setMaterial(material);

		node->setDrawable(newModel);
		SAFE_RELEASE(newModel);
	}
}

void MayaViewer::renameMaterial(const char* oldName, const char* newName) {
	Node* node = _scene->findNode(oldName);
	if (node) {
		node->setId(newName);
		auto it = std::find(_materialNames.begin(), _materialNames.end(), oldName);
		if (it != _materialNames.end()) {
			swap(*it, _materialNames.back());
			_materialNames.pop_back();
			_materialNames.push_back(newName);
		}
	}
}

void MayaViewer::changeMaterial(const char* modelName, MaterialMessage* matInfo) {
	Node* node = _scene->findNode(modelName);
	if (node) {
		Model* model = (Model*)node->getDrawable();
		if (strcmp(matInfo->diffuseTexPath, "") != 0) {
			//Textured
			Material* tempMaterial;
			if (matInfo->specularPower > 0.0f) {
				tempMaterial = model->setMaterial("res/shaders/textured.vert", "res/shaders/textured.frag", "POINT_LIGHT_COUNT 1;SPECULAR");
			}
			else {
				tempMaterial = model->setMaterial("res/shaders/textured.vert", "res/shaders/textured.frag", "POINT_LIGHT_COUNT 1");
			}
			Texture::Sampler* tempSampler = tempMaterial->getParameter("u_diffuseTexture")->setValue(matInfo->diffuseTexPath, true);

			tempMaterial->setParameterAutoBinding("u_worldViewProjectionMatrix", "WORLD_VIEW_PROJECTION_MATRIX");
			tempMaterial->setParameterAutoBinding("u_inverseTransposeWorldViewMatrix", "INVERSE_TRANSPOSE_WORLD_VIEW_MATRIX");
			tempMaterial->getParameter("u_ambientColor")->setValue(Vector3(0.1f, 0.1f, 0.2f));
			tempMaterial->getParameter("u_pointLightColor[0]")->setValue(_defaultLight->getLight()->getColor());
			tempMaterial->getParameter("u_pointLightPosition[0]")->bindValue(_defaultLight, &Node::getTranslationWorld);
			tempMaterial->getParameter("u_pointLightRangeInverse[0]")->bindValue(_defaultLight->getLight(), &Light::getRangeInverse);

			tempMaterial->getParameter("u_specularExponent")->setValue(matInfo->specularPower);
			tempMaterial->setParameterAutoBinding("u_cameraPosition", "CAMERA_WORLD_POSITION");

			tempSampler->setFilterMode(Texture::LINEAR_MIPMAP_LINEAR, Texture::LINEAR);

			tempMaterial->getStateBlock()->setCullFace(true);
			tempMaterial->getStateBlock()->setDepthTest(true);
			tempMaterial->getStateBlock()->setDepthWrite(true);

			tempMaterial->getStateBlock()->setBlend(true);
			tempMaterial->getStateBlock()->setBlendSrc(RenderState::BLEND_SRC_ALPHA);
			tempMaterial->getStateBlock()->setBlendDst(RenderState::BLEND_ONE_MINUS_SRC_ALPHA);
		}
		else {
			//Colored
			Material* tempMaterial;
			if (matInfo->specularPower > 0.0f) {
				tempMaterial = model->setMaterial("res/shaders/colored.vert", "res/shaders/colored.frag", "POINT_LIGHT_COUNT 1;SPECULAR");
			}
			else {
				tempMaterial = model->setMaterial("res/shaders/colored.vert", "res/shaders/colored.frag", "POINT_LIGHT_COUNT 1");
			}
			tempMaterial->setParameterAutoBinding("u_worldViewProjectionMatrix", "WORLD_VIEW_PROJECTION_MATRIX");
			tempMaterial->setParameterAutoBinding("u_inverseTransposeWorldViewMatrix", "INVERSE_TRANSPOSE_WORLD_VIEW_MATRIX");
			tempMaterial->getParameter("u_diffuseColor")->setValue(Vector4(matInfo->color[0], matInfo->color[1], matInfo->color[2], 1.0f));
			tempMaterial->getParameter("u_ambientColor")->setValue(Vector3(0.1f, 0.1f, 0.2f));
			tempMaterial->getParameter("u_pointLightColor[0]")->setValue(_defaultLight->getLight()->getColor());
			tempMaterial->getParameter("u_pointLightPosition[0]")->bindValue(_defaultLight, &Node::getTranslationWorld);
			tempMaterial->getParameter("u_pointLightRangeInverse[0]")->bindValue(_defaultLight->getLight(), &Light::getRangeInverse);

			tempMaterial->getParameter("u_specularExponent")->setValue(matInfo->specularPower);
			tempMaterial->setParameterAutoBinding("u_cameraPosition", "CAMERA_WORLD_POSITION");

			tempMaterial->getStateBlock()->setCullFace(true);
			tempMaterial->getStateBlock()->setDepthTest(true);
			tempMaterial->getStateBlock()->setDepthWrite(true);
		}

		node->setDrawable(model);
	}
}

void MayaViewer::touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex) {
    switch (evt) {
    case Touch::TOUCH_PRESS:
        _wireframe = !_wireframe;
        break;
    case Touch::TOUCH_RELEASE:
        break;
    case Touch::TOUCH_MOVE:
        break;
    };
}

Mesh* MayaViewer::createMesh(std::vector<VertexMessage>& vertices) {
	size_t vertexCount = vertices.size();

	VertexFormat::Element elements[] = {
		VertexFormat::Element(VertexFormat::POSITION, 3),
		VertexFormat::Element(VertexFormat::NORMAL, 3),
		VertexFormat::Element(VertexFormat::TEXCOORD0, 2)
	};
	Mesh* mesh = Mesh::createMesh(VertexFormat(elements, 3), vertexCount, false);
	if (mesh == NULL) {
		GP_ERROR("Failed to create mesh.");
		return NULL;
	}
	mesh->setVertexData(vertices.data(), 0, vertexCount);
	return mesh;
}

void MayaViewer::updateTransform(Vector3 translation, Quaternion rotation, Vector3 scale, const char* nodeName) {
	Node* node = _scene->findNode(nodeName);
	if (node) {
		node->setScale(scale);
		node->setTranslation(translation);
		node->setRotation(rotation);
	}
}
