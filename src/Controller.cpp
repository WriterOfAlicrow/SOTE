#include "Controller.h"

#include "ControlledObject.h"

#ifndef USE_BOX2D_PHYSICS
	#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#endif // USE_BOX2D_PHYSICS
#include "PhysicsData.h"
#include "PhysicsNodeCallback.h"

#include "AngelScriptEngine.h"

#include "Level.h"

#include "GameObjectData.h"

#include "Door.h"


Controller::Controller() : Controller(root)
{
}

Controller::Controller(osg::Group* parentNode) : GameObject(parentNode)
{
	_objectType = "Controller";

	registerController();


	osg::Vec3 position;

	physicsToModelAdjustment = osg::Vec3(0, 0, 0);
	btSphereShape* shape = new btSphereShape(_radius/2);

	btTransform transform = btTransform();
	transform.setIdentity();
	transform.setOrigin(osgbCollision::asBtVector3(position + physicsToModelAdjustment));

	if(_physicsBody)
	{
#ifdef USE_BOX2D_PHYSICS
		getCurrentLevel()->getPhysicsWorld()->DestroyBody(physicsBody);
#else
		getCurrentLevel()->getBulletWorld()->removeCollisionObject(_physicsBody);
		delete _physicsBody;
#endif
	}

	_physicsBody = new btPairCachingGhostObject();
	_physicsBody->setWorldTransform(transform);
	_physicsBody->setCollisionShape(shape);
	getCurrentLevel()->getBulletWorld()->addCollisionObject(_physicsBody, btBroadphaseProxy::SensorTrigger, btBroadphaseProxy::AllFilter);
	_physicsBody->setCollisionFlags(_physicsBody->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);

	_transformNode->setUpdateCallback(new BulletPhysicsNodeCallback(_physicsBody, -physicsToModelAdjustment));

	PhysicsUserData *userData = new PhysicsUserData;
	userData->owner = this;
	userData->ownerType = "Controller";
#ifdef USE_BOX2D_PHYSICS
	physicsBody->SetUserData(userData);
#else
	_physicsBody->setUserPointer(userData);
#endif
}

Controller::Controller(GameObjectData* dataObj, osg::Group* parentNode) : Controller(parentNode)
{
	load(dataObj);
/*
	_objectType = "Controller";

	registerController();

	load(dataObj);

	//osg::Vec3 position;

	physicsToModelAdjustment = osg::Vec3(0, 0, 0);
	btSphereShape* shape = new btSphereShape(_radius/2);

	btTransform transform = btTransform();
	transform.setIdentity();
	transform.setOrigin(osgbCollision::asBtVector3(getWorldPosition() + physicsToModelAdjustment));

	if(_physicsBody)
	{
#ifdef USE_BOX2D_PHYSICS
		getCurrentLevel()->getPhysicsWorld()->DestroyBody(physicsBody);
#else
		getCurrentLevel()->getBulletWorld()->removeCollisionObject(_physicsBody);
		delete _physicsBody;
#endif
	}

	_physicsBody = new btPairCachingGhostObject();
	_physicsBody->setWorldTransform(transform);
	_physicsBody->setCollisionShape(shape);
	getCurrentLevel()->getBulletWorld()->addCollisionObject(_physicsBody, btBroadphaseProxy::SensorTrigger, btBroadphaseProxy::AllFilter);
	_physicsBody->setCollisionFlags(_physicsBody->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
	_transformNode->setUpdateCallback(new BulletPhysicsNodeCallback(_physicsBody, -physicsToModelAdjustment));

	PhysicsUserData *userData = new PhysicsUserData;
	userData->owner = this;
	userData->ownerType = "Controller";
#ifdef USE_BOX2D_PHYSICS
	physicsBody->SetUserData(userData);
#else
	_physicsBody->setUserPointer(userData);
#endif
*/
}

Controller::~Controller()
{
	for(ControlledObject* object: _controlled)
		markForRemoval(object, object->_objectType);
}


void Controller::addControlledObject(ControlledObject* object)
{
	_controlled.push_back(object);
}

void Controller::onPlayerInteraction()
{
	std::cout << "Player interacted." << std::endl;
	if(_functions["onInteraction"])
		getScriptEngine()->runFunction(_functions["onInteraction"], "object", this);
}

void Controller::sendMessage(std::string& message)
{
	std::cout << message << std::endl;
	for(ControlledObject* object : _controlled)
		object->receiveMessage(message);
}




GameObjectData* Controller::save()
{
	GameObjectData* dataObj = new GameObjectData();

	saveSaveableVariables(dataObj);
	saveGameObjectVariables(dataObj);
	saveControllerVariables(dataObj);

	return dataObj;
}
void Controller::saveControllerVariables(GameObjectData* dataObj)
{

	//for(auto child : _controlled)
	//	dataObj->addChild(child);
	dataObj->addData("controlled", _controlled);
	dataObj->addData("radius", _radius);
}

void Controller::load(GameObjectData* dataObj)
{
	loadSaveableVariables(dataObj);
	loadGameObjectVariables(dataObj);
	loadControllerVariables(dataObj);
}
void Controller::loadControllerVariables(GameObjectData* dataObj)
{
	_radius = dataObj->getFloat("radius");

	//for(auto child : dataObj->getChildren())
	for(auto child : dataObj->getObjectList("controlled"))
	{
		if(child->getType() == "ControlledObject")
			_controlled.push_back(new ControlledObject(child, root));
		if(child->getType() == "Door")
			_controlled.push_back(new Door(child, root));
		else
			logWarning("No frickin' clue what this non-ControlledObject object is doing as a child of a Controller");

	}

}







namespace AngelScriptWrapperFunctions
{
Controller* ControllerFactoryFunction()
{
	return new Controller();
}
void ControllerDestructor(void *memory)
{
	// Uninitialize the memory by calling the object destructor
	((Controller*)memory)->~Controller();
}

}

using namespace AngelScriptWrapperFunctions;


void registerController()
{
	static bool registered = false;
	if(registered)
		return;

	registerGameObject();

	getScriptEngine()->registerObjectType("Controller", sizeof(Controller), asOBJ_REF | asOBJ_NOCOUNT );
	getScriptEngine()->registerFactoryFunction("Controller", "Controller@ f()", asFUNCTION(ControllerFactoryFunction));

	getScriptEngine()->registerObjectMethod("Controller", "void sendMessage(const string &in)", asMETHOD(Controller, sendMessage), asCALL_THISCALL);

	registered = true;
}

