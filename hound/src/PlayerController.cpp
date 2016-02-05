#include "hound/PlayerController.h"
#include "hound/CameraController.h"

#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Math/Matrix2.h>

using namespace Urho3D;

// ----------------------------------------------------------------------------
PlayerController::PlayerController(Context* context) :
	Object(context),
	maxSpeed_(1),
	accelerationSmoothness_(1),
	rotateSmoothness_(1),
	cameraAngle_(0),
	actualAngle_(0)
{
	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(PlayerController, HandleUpdate));
	SubscribeToEvent(E_CAMERA_ROTATED, URHO3D_HANDLER(PlayerController, HandleCameraRotated));
}

#include <iostream>
// ----------------------------------------------------------------------------
void PlayerController::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	if(!node_)
		return;

	using namespace Update;
	(void)eventType;

	double timeStep = eventData[P_TIMESTEP].GetDouble();

	// get input direction
	Urho3D::Input* input = this->GetSubsystem<Input>();
	Vector2 targetDirection(0, 0);
	if(input->GetKeyDown(KEY_W)) targetDirection.y_ += 1;
	if(input->GetKeyDown(KEY_S)) targetDirection.y_ -= 1;
	if(input->GetKeyDown(KEY_A)) targetDirection.x_ -= 1;
	if(input->GetKeyDown(KEY_D)) targetDirection.x_ += 1;
	if(targetDirection.x_ != 0 || targetDirection.y_ != 0)
		targetDirection = targetDirection.Normalized() * maxSpeed_;

	// rotate input direction by camera angle using a 2D rotation matrix
	targetDirection = Matrix2(Cos(cameraAngle_), -Sin(cameraAngle_),
							  Sin(cameraAngle_), Cos(cameraAngle_)) * targetDirection;

	// apply the direction
	actualDirection_ += (targetDirection - actualDirection_) * timeStep / accelerationSmoothness_;
	node_->SetPosition(node_->GetPosition() + Vector3(actualDirection_.x_, 0, actualDirection_.y_) * timeStep);

	// apply rotation
	float dotProduct = actualDirection_.y_;  // with Vector2(0, 1)
	float determinant = actualDirection_.x_; // with Vector2(0, 1)
	float targetAngle = Atan2(determinant, dotProduct);
	if(actualAngle_ - targetAngle > 180)
		actualAngle_ -= 360;
	if(actualAngle_ - targetAngle < -180)
		actualAngle_ += 360;
	actualAngle_ += (targetAngle - actualAngle_) * timeStep / rotateSmoothness_;
	node_->SetRotation(Quaternion(actualAngle_, Vector3::UP));
}

// ----------------------------------------------------------------------------
void PlayerController::HandleCameraRotated(StringHash eventType, VariantMap& eventData)
{
	using namespace CameraRotated;
	(void)eventType;

	cameraAngle_ = -eventData[P_ANGLE].GetDouble();
}
