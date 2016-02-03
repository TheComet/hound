#include "hound/PlayerController.h"
#include "hound/CameraController.h"

#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Input/Input.h>
#include "Urho3D/Scene/Node.h"
#include "Urho3D/Core/CoreEvents.h"

#include <iostream>

using namespace Urho3D;

// ----------------------------------------------------------------------------
PlayerController::PlayerController(Context* context) :
	Object(context)
{
	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(PlayerController, HandleUpdate));
	SubscribeToEvent(E_CAMERA_ROTATED, URHO3D_HANDLER(PlayerController, HandleCameraRotated));

}

// ----------------------------------------------------------------------------
void PlayerController::SetNodeToControl(Node* node)
{
	node_ = SharedPtr<Node>(node);
}

// ----------------------------------------------------------------------------
void PlayerController::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	if(!node_)
		return;

	using namespace Update;
	(void)eventType;

	double timeStep = eventData[P_TIMESTEP].GetDouble();

	Urho3D::Input* input = this->GetSubsystem<Input>();
	Vector2 controlStickDirection(0, 0);
	if(input->GetKeyDown(KEY_W)) controlStickDirection.y_ += 1;
	if(input->GetKeyDown(KEY_S)) controlStickDirection.y_ -= 1;
	if(input->GetKeyDown(KEY_A)) controlStickDirection.x_ -= 1;
	if(input->GetKeyDown(KEY_D)) controlStickDirection.x_ += 1;

	if(controlStickDirection.x_ == 0 && controlStickDirection.y_ == 0)
	{
		delta_.x_ = 0;
		delta_.z_ = 0;
	} else
	{
		const Vector2& n = controlStickDirection.Normalized();
		delta_.x_ = n.x_;
		delta_.z_ = n.y_;
	}

	node_->SetPosition(node_->GetPosition() + delta_ * timeStep);
}

// ----------------------------------------------------------------------------
void PlayerController::HandleCameraRotated(StringHash eventType, VariantMap& eventData)
{
	using namespace CameraRotated;
	(void)eventType;

	double angle = eventData[P_ANGLE].GetDouble();
	std::cout << angle << std::endl;
}
