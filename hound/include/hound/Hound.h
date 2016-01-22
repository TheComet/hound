#include <Urho3D/Engine/Application.h>

using namespace Urho3D;

namespace Urho3D {
	class Node;
	class Scene;
}

class Hound : public Application
{
public:
    Hound(Context* context);
    virtual void Setup() override;
    virtual void Start() override;
    virtual void Stop() override;

private:

	void CreateScene();

    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
	void SetupViewports();

	SharedPtr<Scene> scene_;
	SharedPtr<Node> cameraNode_;
};

