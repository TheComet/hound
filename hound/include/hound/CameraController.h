#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include <Urho3D/Core/Object.h>

namespace Urho3D {
	class Context;
	class Node;
	class Scene;
	class XMLFile;
}

URHO3D_EVENT(E_CAMERA_ROTATED, CameraRotated)
{
	URHO3D_PARAM(P_ANGLE, Angle); // double
}

class CameraController : public Urho3D::Object
{
	URHO3D_OBJECT(CameraController, Urho3D::Object)
public:

	/*!
	 * @brief Cosntructs a camera controller.
	 * @param context The Urho3D context object.
	 */
	CameraController(Urho3D::Context* context, Urho3D::Scene* scene);

	void LoadXML(Urho3D::XMLFile* xml);

	/*!
	 * @brief Give the controller a node to manipulate as a camera.
	 * @param node The node to control.
	 */
	void SetNodeToControl(Urho3D::Node* node)
			{ cameraNode_ = Urho3D::SharedPtr<Urho3D::Node>(node); }

	/*!
	 * @brief Give the controller a target node to follow around.
	 * @param node The node to follow.
	 */
	void SetNodeToFollow(Urho3D::Node* node)
			{ followNode_ = Urho3D::SharedPtr<Urho3D::Node>(node); }

	void SetMaxDistance(double distance) { config_.maxDistance_ = distance; }
	void SetMinDistance(double distance) { config_.minDistance_ = distance; }

	void SetRotationSmoothness(double smoothness)
			{ config_.rotationSmoothness_ = smoothness; }
	void SetZoomSmoothness(double smoothness)
			{ config_.zoomSmoothness_ = smoothness; }

	void SetMouseMoveSensitivity(double sensitivity)
			{ config_.mouseMoveSensitivity_ = sensitivity; }
	void SetMouseZoomSensitivity(double sensitivity)
			{ config_.mouseZoomSensitivity_ = sensitivity; }

	void SetYOffset(double yOffset)
			{ config_.yOffset_ = yOffset; }

private:
	void HandleMouseMove(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
	void HandleMouseWheel(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
	void ApplyMouseWheel(int dz);
	void HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
	void HandleFileChanged(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

	Urho3D::SharedPtr<Urho3D::Scene> scene_;
	Urho3D::SharedPtr<Urho3D::Node> cameraNode_;
	Urho3D::SharedPtr<Urho3D::Node> followNode_;

	Urho3D::String configResourceName_;

	struct
	{
		double maxDistance_ = 5;
		double minDistance_ = 1.5;
		double rotationSmoothness_ = 1;
		double zoomSmoothness_ = 1;
		double mouseMoveSensitivity_ = 1;
		double mouseZoomSensitivity_ = 1;
		double yOffset_ = 0;
	} config_;

	double actualAngleX_ = 0;
	double targetAngleX_ = 0;
	double actualAngleY_ = 0;
	double targetAngleY_ = 0;
	double actualDistance_ = 0;
	double targetDistance_ = 0;
};

#endif // CAMERA_CONTROLLER_H
