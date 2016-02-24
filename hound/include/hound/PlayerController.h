#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H

#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Variant.h>
#include <Urho3D/Math/StringHash.h>
#include <Urho3D/IO/VectorBuffer.h>

namespace Urho3D {
	class AnimationState;
	class Context;
	class DebugRenderer;
	class Input;
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

	void SetWalkSpeed(double speed) { config_.walkSpeed_ = speed; }
	void SetTrotSpeed(double speed) { config_.trotSpeed_ = speed; }
	void SetRunSpeed(double speed) { config_.runSpeed_ = speed; }

	void SetAccelerationSmoothness(double smoothness)
			{ config_.accelerationSmoothness_ = smoothness; }

	void SetRotateSmoothness(double smoothness)
			{ config_.rotateSmoothness_ = smoothness; }

private:
	void HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
	void HandleCameraRotated(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
	void HandleFileChanged(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

	void UpdatePlayerPosition(double timeStep);
	void UpdatePlayerAngle(double timeStep);
	void UpdatePlayerAnimation(double timeStep);

	struct {
		double walkSpeed_ = 0;
		double trotSpeed_ = 0;
		double runSpeed_ = 0;
		double accelerationSmoothness_ = 1;
		double rotateSmoothness_ = 1;
	} config_;

	double cameraAngle_ = 0;
	double actualAngle_ = 0;
	double targetAngle_ = 0;

	Urho3D::SharedPtr<Urho3D::Scene> scene_;
	Urho3D::SharedPtr<Urho3D::Input> input_;
	Urho3D::SharedPtr<Urho3D::Node> node_;
	Urho3D::SharedPtr<Urho3D::AnimationState> walkAnimation_;
	Urho3D::SharedPtr<Urho3D::AnimationState> duckAnimation_;

	Urho3D::Vector2 actualDirection_;
	Urho3D::VectorBuffer contacts_;

	Urho3D::String configResourceName_;
};

#endif // PLAYER_CONTROLLER_H
