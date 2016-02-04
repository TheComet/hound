#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H

#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Variant.h>
#include <Urho3D/Math/StringHash.h>

namespace Urho3D {
	class Context;
	class Node;
}

class PlayerController : public Urho3D::Object
{
	URHO3D_OBJECT(PlayerController, Urho3D::Object)

public:

	/*!
	 * @brief Constructs new player controller.
	 * @param context Urho3D context object.
	 */
	PlayerController(Urho3D::Context* context);

	/*!
	 * @brief Gives the controller a node to manipulate.
	 * @param node The node to control.
	 */
	void SetNodeToControl(Urho3D::Node* node)
			{ node_ = Urho3D::SharedPtr<Urho3D::Node>(node); }

	void SetMaxSpeed(double speed) { maxSpeed_ = speed; }

	void SetAccelerationSmoothness(double smoothness)
			{ accelerationSmoothness_ = smoothness; }
			
	void SetRotateSmoothness(double smoothness)
			{ rotateSmoothness_ = smoothness; }

private:
	void HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
	void HandleCameraRotated(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

	Urho3D::SharedPtr<Urho3D::Node> node_;

	double maxSpeed_;
	double accelerationSmoothness_;
	double rotateSmoothness_;
	double cameraAngle_;
	double actualAngle_;

	Urho3D::Vector2 actualDirection_;
};

#endif // PLAYER_CONTROLLER_H
