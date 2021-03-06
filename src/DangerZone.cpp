#include "DangerZone.h"

#include "Level.h"
#include "OwnerUpdateCallback.h"

#ifndef USE_BOX2D_PHYSICS
	#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#endif // USE_BOX2D_PHYSICS
#include "PhysicsData.h"
#include "PhysicsNodeCallback.h"



DangerZone::DangerZone(osg::Group* parentNode) : Attachment(parentNode), _team("Wild")
{
	_objectType = "DangerZone";
	//registerDangerZone();

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
	userData->ownerType = _objectType;
#ifdef USE_BOX2D_PHYSICS
	physicsBody->SetUserData(userData);
#else
	_physicsBody->setUserPointer(userData);
#endif
}

DangerZone::DangerZone(GameObjectData* dataObj, osg::Group* parentNode) : DangerZone(parentNode)
{
	load(dataObj);

	//osg::Vec3 position;

	physicsToModelAdjustment = osg::Vec3(0, 0, 0);
	btSphereShape* shape = new btSphereShape(_radius/2);
	shape->setLocalScaling(osgbCollision::asBtVector3(getScale()));

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
	userData->ownerType = _objectType;
#ifdef USE_BOX2D_PHYSICS
	physicsBody->SetUserData(userData);
#else
	_physicsBody->setUserPointer(userData);
#endif
}

DangerZone::~DangerZone()
{
	//dtor
}


Damages DangerZone::getDamages()
{
	return _damages;
}
std::string DangerZone::getTeam()
{
	return _team;
}


GameObjectData* DangerZone::save()
{
	GameObjectData* dataObj = new GameObjectData();

	saveSaveableVariables(dataObj);
	saveGameObjectVariables(dataObj);
	saveDangerZoneVariables(dataObj);

	return dataObj;
}

void DangerZone::load(GameObjectData* dataObj)
{
	loadSaveableVariables(dataObj);
	loadGameObjectVariables(dataObj);
	loadDangerZoneVariables(dataObj);
}


void DangerZone::saveDangerZoneVariables(GameObjectData* dataObj)
{
	std::vector<GameObjectData*> damageListData;
	for(Damage damage : _damages)
	{
		damageListData.push_back(damage.save());
	}
	dataObj->addData("damages", damageListData);

	dataObj->addData("radius", _radius);
}

void DangerZone::loadDangerZoneVariables(GameObjectData* dataObj)
{
	_radius = dataObj->getFloat("radius");

	for(GameObjectData* damageData : dataObj->getObjectList("damages"))
	{
		Damage dam;
		dam.load(damageData);
		_damages.push_back(dam);
	}
}


