#include "globals.h"
#include "Level.h"



#include "Player.h"
#include "ControlledObject.h"
#include "Controller.h"
#include "Door.h"
#include "DangerZone.h"
#include "Light.h"

#include "PhysicsData.h"
#include "GameObjectData.h"

#include "ThirdPersonCameraManipulator.h"

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>

#include "osgbCollision/CollisionShapes.h"


#include "yaml-cpp/yaml.h"




void myTickCallback(btDynamicsWorld* world, btScalar timeStep)
{
	for(auto kv : getPlayers())
		kv.second->setObjectToInteractWith(NULL);	/// reset each Player's stored interable object, since we're going to regenerate it.

	int numManifolds = world->getDispatcher()->getNumManifolds();
	for (int i=0;i<numManifolds;i++)
	{
		btPersistentManifold* contactManifold =  world->getDispatcher()->getManifoldByIndexInternal(i);
		const btCollisionObject* obA = contactManifold->getBody0();
		const btCollisionObject* obB = contactManifold->getBody1();


		PhysicsUserData* dataA = (PhysicsUserData*) obA->getUserPointer();
		PhysicsUserData* dataB = (PhysicsUserData*) obB->getUserPointer();

		if(!dataA || !dataB)
		{
			logError("Physics object with no userdata.");
			continue;
		}

		/*if(dataA->ownerType == "Projectile" && (dataB->ownerType == "Fighter" || dataB->ownerType == "Player") )
		{
			((Fighter*)dataB->owner)->onCollision((Projectile*)dataA->owner);
			((Projectile*)dataA->owner)->onCollision((Fighter*)dataB->owner);
		}
		else if((dataA->ownerType == "Fighter" || dataA->ownerType == "Player") && dataB->ownerType == "Projectile")
		{
			((Fighter*)dataA->owner)->onCollision((Projectile*)dataB->owner);
			((Projectile*)dataB->owner)->onCollision((Fighter*)dataA->owner);
		}*/
		if(dataA->ownerType == "DangerZone" && (dataB->ownerType == "Fighter" || dataB->ownerType == "Player") )
		{
			((Fighter*)dataB->owner)->onCollision((DangerZone*)dataA->owner);
			//((DangerZone*)dataA->owner)->onCollision((Fighter*)dataB->owner);
		}
		else if((dataA->ownerType == "Fighter" || dataA->ownerType == "Player") && dataB->ownerType == "DangerZone")
		{
			((Fighter*)dataA->owner)->onCollision((DangerZone*)dataB->owner);
			//((DangerZone*)dataB->owner)->onCollision((Fighter*)dataA->owner);
		}
		else if(dataA->ownerType == "Player" && dataB->ownerType == "Controller")
		{
			((Player*)dataA->owner)->onCollision((Controller*)dataB->owner);
			//((GameObject*)dataB->owner)->onCollision((GameObject*)dataA->owner);
		}
		else if(dataA->ownerType == "Controller" && dataB->ownerType == "Player")
		{
			((Player*)dataB->owner)->onCollision((Controller*)dataA->owner);
			//((GameObject*)dataB->owner)->onCollision((GameObject*)dataA->owner);
		}
		else if(dataA->ownerType != "Level" && dataB->ownerType != "Level")		// Neither body is part of the Level geometry
		{
			((GameObject*)dataA->owner)->onCollision((GameObject*)dataB->owner);
			((GameObject*)dataB->owner)->onCollision((GameObject*)dataA->owner);
		}

		int numContacts = contactManifold->getNumContacts();
		for (int j=0;j<numContacts;j++)
		{
			btManifoldPoint& pt = contactManifold->getContactPoint(j);
			if (pt.getDistance()<0.f)
			{
				const btVector3& ptA = pt.getPositionWorldOnA();
				const btVector3& ptB = pt.getPositionWorldOnB();
				const btVector3& normalOnB = pt.m_normalWorldOnB;


			}
		}
	}
}


Level::Level(std::string filename) : _cameraManipulator(NULL)
{
	setCurrentLevel(this);

	_cameraManipulator = new ThirdPersonCameraManipulator();
	getViewer()->setCameraManipulator(_cameraManipulator);

	_levelRoot = new osg::Group();
	addToSceneGraph(_levelRoot);

#ifdef USE_BOX2D_PHYSICS
	gravity = b2Vec2(0.0, 0.0);
	physicsWorld = new b2World(gravity);
	debugDrawer = new Box2DDebugDrawer();
	uint32 flags = 0;
	flags += b2Draw::e_shapeBit;
	flags += b2Draw::e_jointBit;
	flags += b2Draw::e_aabbBit;
	//flags += b2Draw::e_centerOfMassBit;
	debugDrawer->SetFlags(flags);
	physicsWorld->SetDebugDraw(debugDrawer);
	physicsWorld->SetContactListener(new PhysicsContactListener());
#else
	// Bullet world setup
	btDefaultCollisionConfiguration *collisionConfiguration = new btDefaultCollisionConfiguration();
	btBroadphaseInterface *broadphase = new btAxisSweep3(btVector3(-1000, -1000, -1000), btVector3(1000, 1000, 1000));
	broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
	btCollisionDispatcher *dispatcher = new btCollisionDispatcher(collisionConfiguration);
	btSequentialImpulseConstraintSolver *solver = new btSequentialImpulseConstraintSolver();
	_physicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
	_physicsWorld->setGravity(btVector3(0, 0, -9.81));
	_debugDrawer = new osgbCollision::GLDebugDrawer();
	_physicsWorld->setDebugDrawer(_debugDrawer);
	_debugDrawer->setDebugMode(btIDebugDraw::DBG_MAX_DEBUG_DRAW_MODE & ~btIDebugDraw::DBG_DrawNormals);
	_debugDrawer->setEnabled(false);
	addToSceneGraph(_debugDrawer->getSceneGraph(), _levelRoot);
	_physicsWorld->setInternalTickCallback(myTickCallback);
#endif

	_filename = filename;
	loadFromFile(filename);
}

Level::~Level()
{
}

GameObjectData* Level::save()
{
	GameObjectData* dataObj = new GameObjectData();
	saveSaveableVariables(dataObj);

	std::vector<GameObject*> objectVector;

	for(GameObject* object : _objects)
		objectVector.push_back(object);

	dataObj->addData("children", objectVector);

	return dataObj;
}


void Level::saveAsFile(std::string filename)
{
	YAML::Emitter emitter;
	this->save()->toYAML(emitter);
	std::ofstream fout(filename);
	fout << emitter.c_str();
}
std::string Level::saveAsString()
{
	YAML::Emitter emitter;
	this->save()->toYAML(emitter);
	return emitter.c_str();
}

void Level::load(GameObjectData* dataObj)
{
	loadSaveableVariables(dataObj);

	for(auto child : dataObj->getObjectList("children"))
		addObject(GameObject::create(child, _levelRoot));
}
void Level::loadFromString(std::string text)
{
	clear();

	GameObjectData* dataObj = new GameObjectData(YAML::Load(text));

	load(dataObj);
}
void Level::loadFromFile(std::string filename)
{
	clear();

	_filename = filename;

	GameObjectData* dataObj = new GameObjectData(YAML::LoadFile(filename));

	load(dataObj);
}

void Level::reload()
{
	loadFromFile(_filename);
}
void Level::clear()
{
	for(auto obj : _objects)
		markForRemoval(obj, obj->_objectType);

	_playerNames.clear();
}

void Level::addObject(GameObject* obj)
{
	_objects.push_back(obj);
	if(dynamic_cast<Player*>(obj))
		addPlayer(((Player*) obj)->getName(), (Player*) obj);
}
void Level::removeObject(GameObject* obj)
{
	_objects.remove(obj);
}


Player* Level::getClosestPlayer(osg::Vec3 position)
{
	Player* closest;
	float shortestDistance = FLT_MAX;

	for(auto kv : _players)
	{
		Player* p = kv.second;	// Get the Player from the key-value pair.
		if(getDistance(position, p->getWorldPosition()) < shortestDistance)
		{
			closest = p;
			shortestDistance = getDistance(position, p->getWorldPosition());
		}
	}

	return closest;
}

Player* Level::getActivePlayer()
{
	/// In case we haven't set an active player yet, make the first player active.
	if(!_players.count(_activePlayerName))
		setActivePlayer(_playerNames.front());
	if(!_players.count(_activePlayerName))
		logError("No players found.");

	return _players[_activePlayerName];
}

void Level::setActivePlayer(std::string newActivePlayerName)
{
	_activePlayerName = newActivePlayerName;
}

void Level::addPlayer(std::string playerName, Player* thePlayer)
{
	_players[playerName] = thePlayer;
	_playerNames.push_back(playerName);
}

void Level::switchToNextPlayer()
{
	std::string nextPlayerName = _activePlayerName;
	for(auto it = _playerNames.begin(); it != _playerNames.end(); ++it)
	{
		if(*it == _activePlayerName)
		{
			++it;	/// Okay, it's ugly, but this seems the simplest way to access the next name in the list.
			if(it == _playerNames.end())
				nextPlayerName = *_playerNames.begin();	/// the node with the current player name is the last in the list, so we switch to the first player name
			else
				nextPlayerName = *it;	/// we'll switch to the next name in the list.
			break;
		}
	}
	setActivePlayer(nextPlayerName);

}
void Level::switchToPreviousPlayer()
{
	std::string previousPlayerName = _activePlayerName;
	/// Here we do the same thing as for switchToNextPlayer(), except with a reverse iterator (so we start with the last name and move towards the first one)
	for(auto it = _playerNames.rbegin(); it != _playerNames.rend(); ++it)
	{
		if(*it == _activePlayerName)
		{
			++it;
			if(it == _playerNames.rend())
				previousPlayerName = *_playerNames.rbegin();
			else
				previousPlayerName = *it;
			break;
		}
	}
	setActivePlayer(previousPlayerName);
}

std::list<std::string> Level::getPlayerNames()
{
	return _playerNames;
}

std::unordered_map<std::string, Player*> Level::getPlayers()
{
	return _players;
}

std::string Level::getFilename()
{
	return _filename;
}

void Level::setCameraManipulator(BaseCameraManipulator* cameraManipulator)
{
	_cameraManipulator = cameraManipulator;
}

osg::Group* Level::getRootNode()
{
	return _levelRoot;
}



Level* currentLevel;

Level* getCurrentLevel() {
	return currentLevel;
}
void setCurrentLevel(Level* newLevel) {
	currentLevel = newLevel;
}

