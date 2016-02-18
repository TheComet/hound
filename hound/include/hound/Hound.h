#include <Urho3D/Engine/Application.h>

namespace Urho3D {
	class Node;
	class Scene;
}

class PlayerController;
class CameraController;

class Hound : public Urho3D::Application
{
public:
    Hound(Urho3D::Context* context);
    virtual void Setup() override;
    virtual void Start() override;
    virtual void Stop() override;

private:

	void CreateScene();
	void CreatePlayer();
	void CreateCamera();
    void CreateUI();

	void HandleKeyDown(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
	void HandlePostRenderUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

	Urho3D::SharedPtr<Urho3D::Scene> scene_;
	Urho3D::SharedPtr<Urho3D::Node> playerNode_;
	Urho3D::SharedPtr<Urho3D::Node> cameraNode_;
	Urho3D::SharedPtr<PlayerController> playerController_;
	Urho3D::SharedPtr<CameraController> cameraController_;

	bool drawDebugGeometry_;
};
