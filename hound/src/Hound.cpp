#include "hound/Hound.h"
#include "hound/PlayerController.h"
#include "hound/CameraController.h"

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Animation.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/AnimationState.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/LuaScript/LuaScript.h>
#include <Urho3D/LuaScript/LuaFile.h>
#include <Urho3D/LuaScript/LuaScriptInstance.h>
#include <Urho3D/LuaScript/LuaFunction.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/UI/Text.h>

#include <iostream>

using namespace Urho3D;

// ----------------------------------------------------------------------------
Hound::Hound(Context* context) :
	Application(context),
	drawDebugGeometry_(false)
{
}

// ----------------------------------------------------------------------------
void Hound::Setup()
{
	// called before engine initialization

	engineParameters_["WindowTitle"] = "Hound";
	engineParameters_["FullScreen"]  = false;
	engineParameters_["Headless"]    = false;
	engineParameters_["Multisample"] = 2;
}

// ----------------------------------------------------------------------------
#include <chrono>
#include <thread>
void Hound::Start()
{
	// configure resource cache
	cache_ = GetSubsystem<ResourceCache>();
	cache_->SetAutoReloadResources(true);

	CreateScene();
	CreatePlayer();
	CreateCamera();
    CreateUI();

	SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(Hound, HandleKeyDown));
	SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(Hound, HandlePostRenderUpdate));
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
	// load scene, delete XML file after use
	scene_ = new Scene(context_);
	XMLFile* xmlScene = cache_->GetResource<XMLFile>("Scenes/Ramps.xml");
	if(xmlScene)
		scene_->LoadXML(xmlScene->GetRoot());

	// issue #1193 - gravity is not exported correctly from editor
	scene_->GetComponent<PhysicsWorld>()->SetGravity(Vector3(0, -9.81, 0));
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

	playerController_ = new PlayerController(context_, scene_.Get());
	playerController_->LoadXML(cache_->GetResource<XMLFile>("Config/PlayerController.xml"));
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
	viewport->SetDrawDebug(true);
	GetSubsystem<Renderer>()->SetViewport(0, viewport);

	cameraController_ = new CameraController(context_, scene_.Get());
	cameraController_->LoadXML(cache_->GetResource<XMLFile>("Config/CameraController.xml"));
}

// ----------------------------------------------------------------------------
void Hound::CreateUI()
{
	UI* ui = GetSubsystem<UI>();
	UIElement* root = ui->GetRoot();

	XMLFile* xmlDefaultStyle = cache_->GetResource<XMLFile>("UI/DefaultStyle.xml");
	root->SetDefaultStyle(xmlDefaultStyle);

	Window* window = new Window(context_);
	window->SetMinWidth(384);
	window->SetMinHeight(100);
	window->SetPosition(8, 8);
	window->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
	window->SetName("Window");

	UIElement* titleBar = new UIElement(context_);
	titleBar->SetMinSize(0, 24);
	titleBar->SetVerticalAlignment(VA_TOP);
	titleBar->SetLayoutMode(LM_HORIZONTAL);

	Text* windowTitle = new Text(context_);
	windowTitle->SetName("WindowTitle");
	windowTitle->SetText("This is a test!");

	Button* button = new Button(context_);
	button->SetName("TestButton");

	Text* buttonText = new Text(context_);
	buttonText->SetText("button");

	root->AddChild(window);
	window->AddChild(button);
	window->AddChild(titleBar);
	titleBar->AddChild(windowTitle);
	button->AddChild(buttonText);

	window->SetStyleAuto();
	button->SetStyleAuto();
	windowTitle->SetStyleAuto();
	buttonText->SetStyleAuto();
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

	// Toggle debug geometry
	if(key == KEY_P)
		drawDebugGeometry_ = !drawDebugGeometry_;
}

// ----------------------------------------------------------------------------
void Hound::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
	(void)eventType;
	(void)eventData;
	if(!drawDebugGeometry_)
		return;

	PhysicsWorld* phy = scene_->GetComponent<PhysicsWorld>();
	if(!phy)
		return;
	phy->DrawDebugGeometry(true);
}

// ----------------------------------------------------------------------------
URHO3D_DEFINE_APPLICATION_MAIN(Hound)
