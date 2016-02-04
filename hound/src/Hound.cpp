#include "hound/Hound.h"
#include "hound/PlayerController.h"
#include "hound/CameraController.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/LuaScript/LuaScript.h>
#include <Urho3D/LuaScript/LuaFile.h>
#include <Urho3D/LuaScript/LuaScriptInstance.h>
#include <Urho3D/LuaScript/LuaFunction.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/Log.h>

using namespace Urho3D;

// ----------------------------------------------------------------------------
Hound::Hound(Context* context) :
	Application(context)
{
}

// ----------------------------------------------------------------------------
void Hound::Setup()
{
	// called before engine initialization

	engineParameters_["WindowTitle"] = "Hound";
	engineParameters_["FullScreen"]  = false;
	engineParameters_["Headless"]    = false;
}

// ----------------------------------------------------------------------------
void Hound::Start()
{
	// called after engine initialization

	context_->RegisterSubsystem(new LuaScript(context_));

	CreateScene();
	CreatePlayer();
	CreateCamera();

	SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(Hound, HandleKeyDown));
}

// ----------------------------------------------------------------------------
void Hound::Stop()
{
	cameraController_.Reset();
	cameraNode_.Reset();

	playerController_.Reset();
	playerNode_.Reset();

	scene_.Reset();
}

// ----------------------------------------------------------------------------
void Hound::CreateScene()
{
	ResourceCache* cache = GetSubsystem<ResourceCache>();

	// load scene, delete XML file after use
	scene_ = new Scene(context_);
	XMLFile* sceneFile = cache->GetResource<XMLFile>("Scenes/Ramps.xml");
	if(sceneFile)
		scene_->LoadXML(sceneFile->GetRoot());
	sceneFile->ReleaseRef();
}

// ----------------------------------------------------------------------------
void Hound::CreatePlayer()
{
	playerNode_ = scene_->GetChild("Player");
	if(!playerNode_)
	{
		URHO3D_LOGERROR("Scene doesn't have the node \"Player\"! Can't set up player controller");
		return;
	}

	playerController_ = new PlayerController(context_);
	playerController_->SetNodeToControl(playerNode_);
	playerController_->SetMaxSpeed(7);
	playerController_->SetAccelerationSmoothness(0.1);
	playerController_->SetRotateSmoothness(0.05);
}

// ----------------------------------------------------------------------------
void Hound::CreateCamera()
{
	if(!playerNode_)
		URHO3D_LOGERROR("Camera has no player node to follow!");

	cameraNode_ = scene_->CreateChild("Camera");
	Camera* camera = cameraNode_->CreateComponent<Camera>();
	camera->SetFarClip(300.0f);
	cameraNode_->SetPosition(Vector3(0.0f, 5.0f, -20.0f));

	Viewport* viewport = new Viewport(context_, scene_, camera);
	GetSubsystem<Renderer>()->SetViewport(0, viewport);

	cameraController_ = new CameraController(context_);
	cameraController_->SetNodeToControl(cameraNode_);
	cameraController_->SetNodeToFollow(playerNode_);
	cameraController_->SetMouseSensitivity(0.2);
	cameraController_->SetYOffset(0.7);
	cameraController_->SetMinDistance(3.0);
	cameraController_->SetMaxDistance(10.0);
	cameraController_->SetRotateSmoothness(0.033);
	cameraController_->SetZoomSmoothness(0.083);
}

// ----------------------------------------------------------------------------
void Hound::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
	using namespace KeyDown;
	(void)eventType;

	// Check for pressing ESC
	int key = eventData[P_KEY].GetInt();
	if(key == KEY_ESC)
		engine_->Exit();
}

// ----------------------------------------------------------------------------
URHO3D_DEFINE_APPLICATION_MAIN(Hound)
