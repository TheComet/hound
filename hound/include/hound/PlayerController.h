#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H

#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Variant.h>
#include <Urho3D/Math/StringHash.h>
#include <Urho3D/IO/VectorBuffer.h>

namespace Urho3D {
	class AnimationState;
	class Context;
	class Node;
	class Scene;
	class XMLFile;
}

class PlayerController : public Urho3D::Object
{
	URHO3D_OBJECT(PlayerController, Urho3D::Object)

public:

	/*!
	 * @brief Constructs new player controller.
	 * @param context Urho3D context object.
	 */
	PlayerController(Urho3D::Context* context, Urho3D::Scene* scene);

	void LoadXML(Urho3D::XMLFile* xml);

	/*!
	 * @brief Gives the controller a node to manipulate.
	 * @param node The node to control.
	 */
	void SetNodeToControl(Urho3D::Node* node);

	void SetWalkSpeed(double speed) { walkSpeed_ = speed; }
	void SetTrotSpeed(double speed) { trotSpeed_ = speed; }
	void SetRunSpeed(double speed) { runSpeed_ = speed; }

	void SetAccelerationSmoothness(double smoothness)
			{ accelerationSmoothness_ = smoothness; }

	void SetRotateSmoothness(double smoothness)
			{ rotateSmoothness_ = smoothness; }

private:
	void HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
	void HandleCameraRotated(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
	void HandlePostRenderUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
	void HandleFileChanged(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

	Urho3D::SharedPtr<Urho3D::Scene> scene_;
	Urho3D::SharedPtr<Urho3D::Node> node_;
	Urho3D::SharedPtr<Urho3D::AnimationState> walkAnimation_;
	Urho3D::SharedPtr<Urho3D::AnimationState> duckAnimation_;

	double walkSpeed_;
	double trotSpeed_;
	double runSpeed_;
	double accelerationSmoothness_;
	double rotateSmoothness_;
	double cameraAngle_;
	double actualAngle_;

	Urho3D::Vector2 actualDirection_;
	Urho3D::VectorBuffer contacts_;

	Urho3D::String configResourceName_;
};

#endif // PLAYER_CONTROLLER_H
