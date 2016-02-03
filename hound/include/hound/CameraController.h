#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include <Urho3D/Core/Object.h>

namespace Urho3D {
	class Context;
	class Node;
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
	CameraController(Urho3D::Context* context);

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

	void SetMaxDistance(double distance) { maxDistance_ = distance; }
	void SetMinDistance(double distance) { minDistance_ = distance; }

	void SetRotateSmoothness(double smoothness)
			{ rotateSmoothness_ = smoothness; }
	void SetZoomSmoothness(double smoothness)
			{ zoomSmoothness_ = smoothness; }

	void SetMouseSensitivity(double sensitivity)
			{ mouseSensitivity_ = sensitivity; }

	void SetYOffset(double yOffset)
			{ yOffset_ = yOffset; }

private:
	void HandleMouseMove(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
	void HandleMouseWheel(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
	void HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

	Urho3D::SharedPtr<Urho3D::Node> cameraNode_;
	Urho3D::SharedPtr<Urho3D::Node> followNode_;

	double maxDistance_;
	double minDistance_;
	double rotateSmoothness_;
	double zoomSmoothness_;
	double mouseSensitivity_;
	double yOffset_;

	double actualAngleX_;
	double targetAngleX_;
	double actualAngleY_;
	double targetAngleY_;
	double actualDistance_;
	double targetDistance_;
};

#endif // CAMERA_CONTROLLER_H
