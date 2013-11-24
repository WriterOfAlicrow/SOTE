#include "Player.h"


#ifdef MULTIPLE_PLAYERS
std::string activePlayerName;
std::map<std::string, Player*> playerList;
#endif

Player::Player(std::string name, osg::Vec3 position) : Entity(name, position)
{
	modelNode->setUpdateCallback(new PlayerNodeCallback(this));
	equipWeapon(new Weapon());
	equipedWeapon->setPosition(position);
}

void Player::setPosition(osg::Vec3 newPosition) {
	transformNode->setPosition(newPosition);
	equipedWeapon->setPosition(newPosition);
}

void Player::processMovementControls(osg::Vec3 controlVector)
{

	osg::Vec3 movementVector = controlVector * maxSpeed * getDeltaTime();

	this->setPosition(getPosition() + movementVector);
	/*
	osg::Vec3 worldMovement = cameraToWorldTranslation(controlVector);
	if ( worldMovement.length() > 1.0f ) worldMovement.normalize();

	double controlAngle =  atan( worldMovement.y()/worldMovement.x());
	if ( worldMovement.x() < 0 )
	{
		controlAngle += pi;
	}
	static float threshold = pi/12.0;
	float angleDistance = (controlAngle + pi/2) - this->getHeading();
	if (abs(angleDistance) > pi)
	{
		if (angleDistance > 0 ) angleDistance -= 2*pi;
		else angleDistance += 2*pi;
	}
	if (worldMovement != osg::Vec3(0,0,0))
	{
		this->setHeading(controlAngle + pi/2);
	}
	this->controller->setWalkDirection(osgbCollision::asBtVector3(worldMovement) * maxSpeed * getDeltaTime() );
	// TODO: should turn around until it reaches the right point, instead of instantly changing.
	*/
}

void Player::checkForInput()
{
	static int moveUpKey = 'w';
	static int moveDownKey = 's';
	static int moveLeftKey = 'a';
	static int moveRightKey = 'd';

	osg::Vec3 movementVector = osg::Vec3(0,0,0);
	if (keh->keyState[moveUpKey]) movementVector += osg::Vec3(0, 1, 0);
	if (keh->keyState[moveDownKey]) movementVector += osg::Vec3(0, -1, 0);
	if (keh->keyState[moveLeftKey]) movementVector += osg::Vec3(-1, 0, 0);
	if (keh->keyState[moveRightKey]) movementVector += osg::Vec3(1, 0, 0);
	movementVector.normalize();
	processMovementControls(movementVector);
}

void PlayerNodeCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
#ifdef MULTIPLE_PLAYERS
	if (player->isActivePlayer())
#endif
	{
		player->checkForInput();
	}

	traverse(node, nv);	// need to call this so scene graph traversal continues.
}

#ifdef MULTIPLE_PLAYERS

bool Player::isActivePlayer() {
	return ( activePlayerName == this->name);
}

Player* getActivePlayer() {
	return playerList[activePlayerName];
}

void setActivePlayer(std::string newActivePlayerName) {
	activePlayerName = newActivePlayerName;
}
void setActivePlayer(Player *player) {
	activePlayerName = player->name;
}

void addNewPlayer(std::string playerName, osg::Vec3 position) {
	playerList[playerName] = new Player(playerName, position);
}

#endif