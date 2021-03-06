#include "PhysicsContactListener.h"

#include "Enemy.h"
#include "Projectile.h"

PhysicsContactListener::PhysicsContactListener()
{

}

PhysicsContactListener::~PhysicsContactListener()
{

}

void PhysicsContactListener::BeginContact(b2Contact* contact)
{
	//handle begin event
	PhysicsUserData *dataA = (PhysicsUserData*) contact->GetFixtureA()->GetBody()->GetUserData();
	PhysicsUserData *dataB = (PhysicsUserData*) contact->GetFixtureB()->GetBody()->GetUserData();

	if(!dataA || !dataB)
	{
		logError("Physics object with no userdata.");
		return;
	}

	if(dataA->ownerType == "Projectile" && (dataB->ownerType == "Enemy" || dataB->ownerType == "Player") )
	{
		//((Fighter*)dataB->owner)->takeDamages(((Projectile*)dataA->owner)->getDamages());
		((Fighter*)dataB->owner)->onCollision((Projectile*)dataA->owner);
		((Projectile*)dataA->owner)->onCollision((Fighter*)dataB->owner);
	}
	else if((dataA->ownerType == "Enemy" || dataA->ownerType == "Player") && dataB->ownerType == "Projectile")
	{
		//((Fighter*)dataA->owner)->takeDamages(((Projectile*)dataB->owner)->getDamages());
		((Fighter*)dataA->owner)->onCollision(((Projectile*)dataB->owner));
		((Projectile*)dataB->owner)->onCollision((Fighter*)dataA->owner);
	}
}
