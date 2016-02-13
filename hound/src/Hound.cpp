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
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Core/CoreEvents.h>

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
}

// ----------------------------------------------------------------------------
void Hound::Start()
{/*
	CreateScene();
	CreatePlayer();
	CreateCamera();*/

	// create scene
	/*
	scene_ = new Scene(context_);
	scene_->CreateComponent<Octree>();
	scene_->CreateComponent<DebugRenderer>();
    scene_->CreateComponent<PhysicsWorld>();*/

	ResourceCache* cache = GetSubsystem<ResourceCache>();
/*
	// level
	Node* level = scene_->CreateChild("Level");
	StaticModel* levelStaticModel = level->CreateComponent<StaticModel>();
	level->SetRotation(Quaternion(90, Vector3::LEFT));
	level->SetPosition(Vector3(0, -5, 0));
	Model* levelModel = cache->GetResource<Model>("Models/ramps-level.mdl");
	levelStaticModel->SetModel(levelModel);
	levelStaticModel->SetMaterial(cache->GetResource<Material>("Materials/DefaultMaterial.xml"));*/
	CreateScene();

	// create player
	/*
	playerNode_ = scene_->CreateChild("Player");
	playerNode_->SetScale(Vector3(0.122842, 0.122842, 0.122842));
	StaticModel* model = playerNode_->CreateComponent<StaticModel>();
	model->SetModel(cache->GetResource<Model>("Models/player.mdl"));
	model->SetMaterial(cache->GetResource<Material>("Materials/Material.xml"));

	RigidBody* body = playerNode_->CreateComponent<RigidBody>();
	body->SetMass(1);
	CollisionShape* shape = playerNode_->CreateComponent<CollisionShape>();
	shape->SetCapsule(9.36, 15.32);

	playerController_ = new PlayerController(context_);
	playerController_->SetNodeToControl(playerNode_);
	playerController_->SetMaxSpeed(7);
	playerController_->SetAccelerationSmoothness(0.1);
	playerController_->SetRotateSmoothness(0.05);*/
	CreatePlayer();

	// create a light source
	/*
	Node* lightNode = scene_->CreateChild("Light");
	lightNode->SetPosition(Vector3(10, 10, -50));
	lightNode->LookAt(Vector3(0, 0, 0));
	Light* light = lightNode->CreateComponent<Light>();
	light->SetLightType(LIGHT_DIRECTIONAL);
	light->SetColor(Color(255, 255, 255));*/

	CreateCamera();

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
#include <iostream>
#include <Urho3D/Graphics/DebugRenderer.h>
void Hound::CreateScene()
{
	ResourceCache* cache = GetSubsystem<ResourceCache>();

	// load scene, delete XML file after use
	scene_ = new Scene(context_);
	XMLFile* sceneFile = cache->GetResource<XMLFile>("Scenes/Ramps.xml");
	if(sceneFile)
	{
		scene_->LoadXML(sceneFile->GetRoot());
		sceneFile->ReleaseRef();
	}

	// issue #1193 - gravity is not exported correctly from editor
	scene_->GetComponent<PhysicsWorld>()->SetGravity(Vector3(0, -9.81, 0));

	/*RigidBody* body = scene_->GetChild("Player")->GetComponent<RigidBody>();
	body->SetGravityOverride(Vector3(0, -9.81, 0));*/
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
	viewport->SetDrawDebug(true);
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
