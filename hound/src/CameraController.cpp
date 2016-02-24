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
	scene_(scene),
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

#define CONFIG_DOUBLE(name, setFunction)                                    \
		this->setFunction(root.GetChild(name).GetDouble("value"))

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

	// find camera node and follow node in scene and store it
	CONFIG_NODE("CameraNode", SetNodeToControl);
	CONFIG_NODE("FollowNode", SetNodeToFollow);

	// read config values
	CONFIG_DOUBLE("YOffset", SetYOffset);
	CONFIG_DOUBLE_WARN_ZERO("MouseSensitivity", SetMouseSensitivity);
	CONFIG_DOUBLE_WARN_ZERO("MinDistance", SetMinDistance);
	CONFIG_DOUBLE_WARN_ZERO("MaxDistance", SetMaxDistance);
	CONFIG_DOUBLE_ERROR_ZERO("RotationSmoothness", SetRotationSmoothness);
	CONFIG_DOUBLE_ERROR_ZERO("ZoomSmoothness", SetZoomSmoothness);
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

// ----------------------------------------------------------------------------
void CameraController::HandleFileChanged(StringHash eventType, VariantMap& eventData)
{
	using namespace FileChanged;
	(void)eventType;

	if(configResourceName_ == eventData[P_FILENAME].GetString())
	{
		URHO3D_LOGINFO("[CameraController] Reloading config");
		LoadXML(GetSubsystem<ResourceCache>()->GetResource<XMLFile>(
			configResourceName_
		));
	}
}
