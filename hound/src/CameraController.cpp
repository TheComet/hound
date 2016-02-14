#include "hound/CameraController.h"

#include <Urho3D/Scene/Node.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Input/InputEvents.h>

using namespace Urho3D;

// ----------------------------------------------------------------------------
CameraController::CameraController(Context* context) :
	Object(context),
	maxDistance_(10.0),
	minDistance_(1.0),
	rotateSmoothness_(1.0),
	zoomSmoothness_(1.0),
	mouseSensitivity_(1.0),
	yOffset_(0.0),
	actualAngleX_(0),
	targetAngleX_(0),
	actualAngleY_(0),
	targetAngleY_(0),
	actualDistance_(10.0),
	targetDistance_(10.0)
{
	SubscribeToEvent(E_MOUSEMOVE, URHO3D_HANDLER(CameraController, HandleMouseMove));
	SubscribeToEvent(E_MOUSEWHEEL, URHO3D_HANDLER(CameraController, HandleMouseWheel));
	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(CameraController, HandleUpdate));
}

// ----------------------------------------------------------------------------
void CameraController::HandleMouseMove(StringHash eventType, VariantMap& eventData)
{
	using namespace MouseMove;
	(void)eventType;

	int dx = eventData[P_DX].GetInt();
	int dy = eventData[P_DY].GetInt();

	targetAngleX_ += dy;
	targetAngleY_ += dx;

	// clamp X angle
	if(targetAngleX_ >= 89)
		targetAngleX_ = 89;
	if(targetAngleX_ <= -89)
		targetAngleX_ = -89;
}

// ----------------------------------------------------------------------------
void CameraController::HandleMouseWheel(StringHash eventType, VariantMap& eventData)
{
	using namespace MouseWheel;
	(void)eventType;

	int dz = eventData[P_WHEEL].GetInt();

    // Mouse controls target distance
	targetDistance_ -= dz;
	if(targetDistance_ > maxDistance_) targetDistance_ = maxDistance_;
	if(targetDistance_ < minDistance_) targetDistance_ = minDistance_;
}

// ----------------------------------------------------------------------------
void CameraController::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	if(!cameraNode_ || !followNode_)
		return;

	using namespace Update;
	(void)eventType;

	double timeStep = eventData[P_TIMESTEP].GetDouble();

	// smooth rotations and distance changes
	actualAngleX_ += (targetAngleX_ - actualAngleX_) * timeStep / rotateSmoothness_;
	actualAngleY_ += (targetAngleY_ - actualAngleY_) * timeStep / rotateSmoothness_;
	actualDistance_ += (targetDistance_ - actualDistance_) * timeStep / zoomSmoothness_;

	// rotate camera
	Vector3 cameraPosition(0, 0, -actualDistance_);
	cameraPosition = Matrix3(1, 0, 0,
							 0, Cos(actualAngleX_), -Sin(actualAngleX_),
							 0, Sin(actualAngleX_), Cos(actualAngleX_)) * cameraPosition;
	cameraPosition = Matrix3(Cos(actualAngleY_), 0, Sin(actualAngleY_),
							 0, 1, 0,
							 -Sin(actualAngleY_), 0, Cos(actualAngleY_)) * cameraPosition;
	cameraNode_->SetPosition(followNode_->GetPosition() + cameraPosition);

	// look at follow node
	cameraNode_->LookAt(followNode_->GetPosition() + Vector3(0, yOffset_, 0));

	// camera rotated, send an event
	Urho3D::VariantMap map;
	map[CameraRotated::P_ANGLE] = actualAngleY_;
	SendEvent(E_CAMERA_ROTATED, map);
}
