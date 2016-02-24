#include "hound/PlayerController.h"
#include "hound/CameraController.h"

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Math/Matrix2.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationState.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/ResourceEvents.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Resource/XMLElement.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>

using namespace Urho3D;

// ----------------------------------------------------------------------------
PlayerController::PlayerController(Context* context, Scene* scene) :
	Object(context),
	scene_(scene),
	walkSpeed_(1),
	accelerationSmoothness_(1),
	rotateSmoothness_(1),
	cameraAngle_(0),
	actualAngle_(0)
{
	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(PlayerController, HandleUpdate));
	SubscribeToEvent(E_CAMERA_ROTATED, URHO3D_HANDLER(PlayerController, HandleCameraRotated));
	SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(PlayerController, HandlePostRenderUpdate));
	SubscribeToEvent(E_FILECHANGED, URHO3D_HANDLER(PlayerController, HandleFileChanged));
}

// ----------------------------------------------------------------------------
#include <iostream>
void PlayerController::LoadXML(XMLFile* xml)
{
	if(!xml || !scene_)
		return;

	// store file name so we can reload when it's edited
	configResourceName_ = xml->GetName();

	XMLElement root = xml->GetRoot();

#define CONFIG_NODE(nodeName, setFunction) do {                             \
		String name = root.GetChild(nodeName).GetAttribute("name");         \
		if(name.Length() == 0)                                              \
			URHO3D_LOGERROR("[CameraController] Failed to read XML "        \
			                "attribute <" nodeName " name=\"...\" />");     \
		Node* node = scene_->GetChild(name);                                \
		if(!node)                                                           \
			URHO3D_LOGERRORF("[CameraController] Couldn't find node \"%s\" "\
			                 "in scene", name.CString());                   \
		else                                                                \
			this->setFunction(node);                                        \
	} while(0)

#define CONFIG_DOUBLE_WARN_ZERO(name, setFunction) do {                     \
		double value = root.GetChild(name).GetDouble("value");              \
		if(value == 0)                                                      \
			URHO3D_LOGWARNING("[CameraController] " name " is 0. Change "   \
			                  "with <" name " value=\"...\" />");           \
		this->setFunction(value);                                           \
	} while(0)

#define CONFIG_DOUBLE_ERROR_ZERO(name, setFunction) do {                    \
		double value = root.GetChild(name).GetDouble("value");              \
		if(value == 0)                                                      \
			URHO3D_LOGERROR("[CameraController] " name " is 0. Change with" \
			                " <" name " value=\"...\" />");                 \
		else                                                                \
			this->setFunction(value);                                       \
	} while(0)

	// find player node in scene and store it
	CONFIG_NODE("PlayerNode", SetNodeToControl);

	// read config values
	CONFIG_DOUBLE_WARN_ZERO("WalkSpeed", SetWalkSpeed);
	CONFIG_DOUBLE_WARN_ZERO("TrotSpeed", SetTrotSpeed);
	CONFIG_DOUBLE_WARN_ZERO("RunSpeed", SetRunSpeed);
	CONFIG_DOUBLE_ERROR_ZERO("AccelerationSmoothness", SetAccelerationSmoothness);
	CONFIG_DOUBLE_ERROR_ZERO("RotationSmoothness", SetRotateSmoothness);
}

// ----------------------------------------------------------------------------
void PlayerController::SetNodeToControl(Node* node)
{
	node_ = SharedPtr<Node>(node);

	if(node_)
		node_->SetRotation(Quaternion(actualAngle_, Vector3::UP));

	AnimatedModel* model = node_->GetComponent<AnimatedModel>();
	if(model && model->GetNumAnimationStates())
	{
		AnimationState* state = model->GetAnimationStates()[0];
		state->SetWeight(1);
	}
}

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
		targetDirection = targetDirection.Normalized() * walkSpeed_;

	// rotate input direction by camera angle using a 2D rotation matrix
	targetDirection = Matrix2(Cos(cameraAngle_), -Sin(cameraAngle_),
							  Sin(cameraAngle_), Cos(cameraAngle_)) * targetDirection;

	// apply the direction
	actualDirection_ += (targetDirection - actualDirection_) * timeStep / accelerationSmoothness_;
	//node_->SetPosition(node_->GetPosition() + Vector3(actualDirection_.x_, 0, actualDirection_.y_) * timeStep);
	node_->GetComponent<RigidBody>()->SetLinearVelocity(
		Vector3(actualDirection_.x_,
				node_->GetComponent<RigidBody>()->GetLinearVelocity().y_,
				actualDirection_.y_));

	// apply rotation
	float dotProduct = actualDirection_.y_;  // with Vector2(0, 1)
	float determinant = actualDirection_.x_; // with Vector2(0, 1)
	float targetAngle = Atan2(determinant, dotProduct);
    // always approach target angle with shortest angle (max 180°) to avoid
    // flipping
	if(actualAngle_ - targetAngle > 180)
		actualAngle_ -= 360;
	if(actualAngle_ - targetAngle < -180)
		actualAngle_ += 360;
	actualAngle_ += (targetAngle - actualAngle_) * timeStep / rotateSmoothness_;

	node_->SetRotation(Quaternion(actualAngle_, Vector3::UP));

	// update animation
	AnimatedModel* model = node_->GetComponent<AnimatedModel>();
	if(model->GetNumAnimationStates())
	{
		AnimationState* state = model->GetAnimationStates()[0];
		state->AddTime(timeStep);
	}
}

// ----------------------------------------------------------------------------
void PlayerController::HandleCameraRotated(StringHash eventType, VariantMap& eventData)
{
	using namespace CameraRotated;
	(void)eventType;

	cameraAngle_ = -eventData[P_ANGLE].GetDouble();
}

// ----------------------------------------------------------------------------
void PlayerController::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
	(void)eventType;
	(void)eventData;

	DebugRenderer* renderer = node_->GetParentComponent<DebugRenderer>();
	if(!renderer)
		return;

	contacts_.Seek(0);
	while(!contacts_.IsEof())
	{
		Vector3 position = contacts_.ReadVector3();
		Vector3 normal = contacts_.ReadVector3();
		float distance = contacts_.ReadFloat();
		float impulse = contacts_.ReadFloat();
		(void)impulse;

		renderer->AddCircle(position, normal, distance, Color(255, 0, 0));
	}
}

// ----------------------------------------------------------------------------
void PlayerController::HandleFileChanged(StringHash eventType, VariantMap& eventData)
{
	using namespace FileChanged;
	(void)eventType;

	if(configResourceName_ == eventData[P_FILENAME].GetString())
	{
		URHO3D_LOGINFO("[PlayerController] Reloading config");
		LoadXML(GetSubsystem<ResourceCache>()->GetResource<XMLFile>(
			configResourceName_
		));
	}
}
