#include "hound/PlayerController.h"
#include "hound/CameraController.h"

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/StringUtils.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Math/Matrix2.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Graphics/Animation.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationState.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Skeleton.h>
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
	scene_(scene)
{
	input_ = GetSubsystem<Input>();

	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(PlayerController, HandleUpdate));
	SubscribeToEvent(E_CAMERA_ROTATED, URHO3D_HANDLER(PlayerController, HandleCameraRotated));
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

#define READ_NODE(nodeName, setFunction) do {                               \
		String name = root.GetChild(nodeName).GetAttribute("name");         \
		if(name.Length() == 0)                                              \
			URHO3D_LOGERROR("[CameraController] Failed to read XML "        \
			                "attribute <" nodeName " name=\"...\" />");     \
		Node* node = scene_->GetChild(name);                                \
		if(!node)                                                           \
			URHO3D_LOGERRORF("[CameraController] Couldn't find node \"%s\" "\
			                 "in scene", name.CString());                   \
		else                                                                \
			setFunction(node);                                              \
	} while(0)

#define READ_DOUBLE_WARN_ZERO(name, setFunction) do {                       \
		double value = root.GetChild(name).GetDouble("value");              \
		if(value == 0)                                                      \
			URHO3D_LOGWARNING("[CameraController] " name " is 0. Change "   \
			                  "with <" name " value=\"...\" />");           \
		setFunction(value);                                                 \
	} while(0)

#define READ_DOUBLE_ERROR_ZERO(name, setFunction) do {                      \
		double value = root.GetChild(name).GetDouble("value");              \
		if(value == 0)                                                      \
			URHO3D_LOGERROR("[CameraController] " name " is 0. Change with" \
			                " <" name " value=\"...\" />");                 \
		else                                                                \
			setFunction(value);                                             \
	} while(0)

	// find player node in scene and store it
	READ_NODE("PlayerNode", SetNodeToControl);

	// read config values
	READ_DOUBLE_WARN_ZERO("WalkSpeed", SetWalkSpeed);
	READ_DOUBLE_WARN_ZERO("TrotSpeed", SetTrotSpeed);
	READ_DOUBLE_WARN_ZERO("RunSpeed", SetRunSpeed);
	READ_DOUBLE_ERROR_ZERO("AccelerationSmoothness", SetAccelerationSmoothness);
	READ_DOUBLE_ERROR_ZERO("RotationSmoothness", SetRotationSmoothness);

	// read spine bones, these are deformed procedurally when the player angle
	// changes
	spineBoneController_.spineBones_.Clear();
	spineBoneController_.animState_.Reset();
	for(;;)
	{
		break;
		if(!playerNode_)
			break;
		XMLElement spineBones = root.GetChild("SpineBones").GetChild("Bone");
		if(!spineBones)
		{
			URHO3D_LOGWARNING("[PlayerController] No spine bones were "
				"specified. You can specify spine bones to deform when the "
				"player turns with:\n"
				"<SpineBones>\n"
				"	<Bone name=\"...\" factor=\"...\" maxAngle=\"...\" />\n"
				"	...\n"
				"</SpineBones>");
			break;
		}

		for(; spineBones; spineBones = spineBones.GetNext("Bone"))
		{
			String name = spineBones.GetAttribute("name");
			Node* node = playerNode_->GetChild(name, true);
			if(!node)
			{
				URHO3D_LOGERRORF("[PlayerController] Couldn't find bone "
				                 "\"%s\"", name.CString());
				continue;
			}

			// set spine bone attributes
			SpineBone spineBone;
			String factorStr = spineBones.GetAttribute("factor");
			String maxAngleStr = spineBones.GetAttribute("maxAngle");
			spineBone.factor_ = ToDouble(factorStr);
			spineBone.maxAngle_ = ToDouble(maxAngleStr);
			// handle default values
			if(factorStr.Length() == 0)
				spineBone.factor_ = 1.0;
			if(maxAngleStr.Length() == 0)
				spineBone.maxAngle_ = 180;

			spineBoneController_.spineBones_.Push(spineBone);
		}
		break;
	}
}

// ----------------------------------------------------------------------------
void PlayerController::SetNodeToControl(Node* node)
{
	playerNode_ = SharedPtr<Node>(node);

	if(playerNode_)
		playerNode_->SetRotation(Quaternion(actualAngle_, Vector3::UP));

	AnimatedModel* model = playerNode_->GetComponent<AnimatedModel>();
	if(model && model->GetNumAnimationStates())
	{
		AnimationState* state = model->GetAnimationStates()[0];
		state->SetWeight(0);
	}
}

// ----------------------------------------------------------------------------
void PlayerController::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	if(!playerNode_)
		return;

	using namespace Update;
	(void)eventType;

	double timeStep = eventData[P_TIMESTEP].GetDouble();
	UpdatePlayerPosition(timeStep);
	UpdatePlayerAngle(timeStep);
	UpdatePlayerAnimation(timeStep);
}

// ----------------------------------------------------------------------------
void PlayerController::UpdatePlayerPosition(double timeStep)
{
	double speed = config_.walkSpeed_;

	// get input direction
	Vector2 targetDirection(0, 0);
	if(input_->GetKeyDown(KEY_W)) targetDirection.y_ += 1;
	if(input_->GetKeyDown(KEY_S)) targetDirection.y_ -= 1;
	if(input_->GetKeyDown(KEY_A)) targetDirection.x_ -= 1;
	if(input_->GetKeyDown(KEY_D)) targetDirection.x_ += 1;
	if(targetDirection.x_ != 0 || targetDirection.y_ != 0)
		targetDirection = targetDirection.Normalized() * speed;

	// rotate input direction by camera angle using a 2D rotation matrix
	targetDirection = Matrix2(Cos(cameraAngle_), -Sin(cameraAngle_),
							  Sin(cameraAngle_), Cos(cameraAngle_)) * targetDirection;

	// smoothly approach target direction
	actualDirection_ += (targetDirection - actualDirection_) * timeStep /
			config_.accelerationSmoothness_;

	// update player position using target direction
	//node_->SetPosition(node_->GetPosition() + Vector3(actualDirection_.x_, 0, actualDirection_.y_) * timeStep);
	playerNode_->GetComponent<RigidBody>()->SetLinearVelocity(
		Vector3(actualDirection_.x_,
				playerNode_->GetComponent<RigidBody>()->GetLinearVelocity().y_,
				actualDirection_.y_));
}

// ----------------------------------------------------------------------------
void PlayerController::UpdatePlayerAngle(double timeStep)
{
	// From the actual direction vector, calculate the Y angle the player
	// should be facing. We only do this if the player is actually moving.
	// If the player is not moving, we instead let the target angle approach
	// the actual angle to smoothly stop rotating the player.
	double speed = actualDirection_.Length();
	if(speed > config_.walkSpeed_ * 0.1)
	{
		float dotProduct = actualDirection_.y_;  // with Vector2(0, 1)
		float determinant = actualDirection_.x_; // with Vector2(0, 1)
		targetAngle_ = Atan2(determinant, dotProduct);
	}
	else
	{
		targetAngle_ += (actualAngle_ - targetAngle_) * timeStep /
				config_.rotationSmoothness_;
	}

	// always approach target angle with shortest angle (max 180°) to avoid
	// flipping
	if(actualAngle_ - targetAngle_ > 180)
		actualAngle_ -= 360;
	if(actualAngle_ - targetAngle_ < -180)
		actualAngle_ += 360;
	double delta = targetAngle_ - actualAngle_;
	actualAngle_ += delta * timeStep * speed / config_.rotationSmoothness_;

	// apply actual angle to player angle
	playerNode_->SetRotation(Quaternion(actualAngle_, Vector3::UP));
}

// ----------------------------------------------------------------------------
void PlayerController::UpdatePlayerAnimation(double timeStep)
{
	AnimatedModel* model = playerNode_->GetComponent<AnimatedModel>();
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
void PlayerController::HandleFileChanged(StringHash eventType, VariantMap& eventData)
{
	using namespace FileChanged;
	(void)eventType;

	if(configResourceName_ == eventData[P_RESOURCENAME].GetString())
	{
		URHO3D_LOGINFO("[PlayerController] Reloading config");
		LoadXML(GetSubsystem<ResourceCache>()->GetResource<XMLFile>(
			configResourceName_
		));
	}
}
