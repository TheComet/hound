#include "hound/CameraController.h"

#include <Urho3D/Scene/Node.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/ResourceEvents.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Resource/XMLElement.h>
#include <Urho3D/Scene/Scene.h>

using namespace Urho3D;

// ----------------------------------------------------------------------------
CameraController::CameraController(Context* context, Scene* scene) :
	Object(context),
	scene_(scene)
{
	SubscribeToEvent(E_MOUSEMOVE, URHO3D_HANDLER(CameraController, HandleMouseMove));
	SubscribeToEvent(E_MOUSEWHEEL, URHO3D_HANDLER(CameraController, HandleMouseWheel));
	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(CameraController, HandleUpdate));
	SubscribeToEvent(E_FILECHANGED, URHO3D_HANDLER(CameraController, HandleFileChanged));
}

// ----------------------------------------------------------------------------
void CameraController::LoadXML(XMLFile* xml)
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

#define READ_DOUBLE(name, setFunction)                                      \
		setFunction(root.GetChild(name).GetDouble("value"))

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

	// find camera node and follow node in scene and store it
	READ_NODE("CameraNode", SetNodeToControl);
	READ_NODE("FollowNode", SetNodeToFollow);

	// read config values
	READ_DOUBLE_WARN_ZERO("MinDistance", SetMinDistance);
	READ_DOUBLE_WARN_ZERO("MaxDistance", SetMaxDistance);
	READ_DOUBLE_WARN_ZERO("MouseMoveSensitivity", SetMouseMoveSensitivity);
	READ_DOUBLE_WARN_ZERO("MouseZoomSensitivity", SetMouseZoomSensitivity);
	READ_DOUBLE_ERROR_ZERO("RotationSmoothness", SetRotationSmoothness);
	READ_DOUBLE_ERROR_ZERO("ZoomSmoothness", SetZoomSmoothness);
	READ_DOUBLE("YOffset", SetYOffset);

	// update current zoom to the max zoom
	targetDistance_ = config_.maxDistance_;
}

// ----------------------------------------------------------------------------
void CameraController::HandleMouseMove(StringHash eventType, VariantMap& eventData)
{
	using namespace MouseMove;
	(void)eventType;

	ApplyMouseMove(eventData[P_DX].GetInt(),
	               eventData[P_DY].GetInt());
}

// ----------------------------------------------------------------------------
void CameraController::ApplyMouseMove(int dx, int dy)
{
	targetAngleX_ += dy * config_.mouseMoveSensitivity_;
	targetAngleY_ += dx * config_.mouseMoveSensitivity_;
	dx = Clamp(dx, -89, 89);
}

// ----------------------------------------------------------------------------
void CameraController::HandleMouseWheel(StringHash eventType, VariantMap& eventData)
{
	using namespace MouseWheel;
	(void)eventType;

	ApplyMouseWheel(eventData[P_WHEEL].GetInt());
}

// ----------------------------------------------------------------------------
void CameraController::ApplyMouseWheel(int dz)
{
    // Mouse controls target distance
	targetDistance_ -= dz * config_.mouseZoomSensitivity_;
	targetDistance_ = Clamp(targetDistance_,
	                        config_.minDistance_,
	                        config_.maxDistance_);
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
	actualAngleX_ += (targetAngleX_ - actualAngleX_) * timeStep /
			config_.rotationSmoothness_;
	actualAngleY_ += (targetAngleY_ - actualAngleY_) * timeStep /
			config_.rotationSmoothness_;
	actualDistance_ += (targetDistance_ - actualDistance_) * timeStep /
			config_.zoomSmoothness_;

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
	cameraNode_->LookAt(followNode_->GetPosition() +
			Vector3(0, config_.yOffset_, 0));

	// camera rotated, send an event
	Urho3D::VariantMap map;
	map[CameraRotated::P_ANGLE] = actualAngleY_;
	SendEvent(E_CAMERA_ROTATED, map);
}

// ----------------------------------------------------------------------------
void CameraController::HandleFileChanged(StringHash eventType, VariantMap& eventData)
{
	using namespace FileChanged;
	(void)eventType;

	if(configResourceName_ == eventData[P_RESOURCENAME].GetString())
	{
		URHO3D_LOGINFO("[CameraController] Reloading config");
		LoadXML(GetSubsystem<ResourceCache>()->GetResource<XMLFile>(
			configResourceName_
		));
	}
}
