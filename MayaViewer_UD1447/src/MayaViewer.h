#ifndef MayaViewer_H_
#define MayaViewer_H_

#include <vector>

#include "gameplay.h"
#include "ComLib.h"
#include "DebugConsole.h"
#include "MessageTypes.h"

using namespace gameplay;

/**
 * Main game class.
 */
class MayaViewer: public Game
{
public:

    MayaViewer();

	void keyEvent(Keyboard::KeyEvent evt, int key) override;
	
    void touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex) override;

	bool mouseEvent(Mouse::MouseEvent evt, int x, int y, int wheelDelta) override;

	void addNewModel(const char* modelName, std::vector<VertexMessage>& vertices, MaterialMessage* matInfo);
	void removeModel(const char* modelName);
	void renameModel(const char* oldName, const char* newName);
	void updateModel(const char* modelName, std::vector<VertexMessage>& vertices);
	void renameMaterial(const char* oldName, const char* newName);
	void changeMaterial(const char* modelName, MaterialMessage* matInfo);

protected:

    void initialize() override; //On startup

    void finalize() override; //On shutdown

    void update(float elapsedTime) override; //Once per frame

    void render(float elapsedTime) override; //Once per frame

private:

	ComLib _comLib;

    bool drawScene(Node* node); //Draws the scene each frame
	void fetchMessage();

	Mesh* createMesh(std::vector<VertexMessage>& vertices);
	Material* createMaterial();

	void updateTransform(Vector3 translation, Quaternion rotation, Vector3 scale, const char* nodeName);

	//std::vector<Model*> _models;
	//std::vector <Material*> _mats;
	//std::vector<Texture::Sampler*> _samplers;

	size_t _modelCount;
	size_t _materialCount;
	std::vector<std::string> _modelnames;
	std::vector<std::string> _materialNames;

	Node* _defaultLight;
    Scene* _scene;
    bool _wireframe;
};

#endif
