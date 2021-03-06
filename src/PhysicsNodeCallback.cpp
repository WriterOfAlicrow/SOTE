#include "PhysicsNodeCallback.h"

#include "ImprovedMatrixTransform.h"


#ifdef USE_BOX2D_PHYSICS

Box2DPhysicsNodeCallback::Box2DPhysicsNodeCallback(b2Body *physicsBody, osg::Vec3 box2DToOsgAdjustment)
{
	//this->transformNode = transformNode;
	this->physicsBody = physicsBody;
	this->box2DToOsgAdjustment = box2DToOsgAdjustment;
}

void Box2DPhysicsNodeCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
	//std::cout << "PhysicsNodeCallback begin." << std::endl;
	osg::PositionAttitudeTransform *transformNode = dynamic_cast<osg::PositionAttitudeTransform *> (node);
	transformNode->setPosition(osg::Vec3(physicsBody->GetPosition().x, physicsBody->GetPosition().y, transformNode->getPosition().z() ) + box2DToOsgAdjustment);

	traverse(node, nv);	// need to call this so scene graph traversal continues.
	//std::cout << "PhysicsNodeCallback finished." << std::endl;
}

#else

BulletPhysicsNodeCallback::BulletPhysicsNodeCallback(btCollisionObject *physicsBody, osg::Vec3 physicsToOsgAdjustment)
{
	_physicsBody = physicsBody;
	_physicsToOsgAdjustment = osg::Matrix::translate(physicsToOsgAdjustment);
}

void BulletPhysicsNodeCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
	osg::Matrix mat = osgbCollision::asOsgMatrix(_physicsBody->getWorldTransform());

	ImprovedMatrixTransform* transformNode = dynamic_cast<ImprovedMatrixTransform*>(node);

	mat.preMult(_physicsToOsgAdjustment);
	mat.preMultScale(transformNode->getScale());
	transformNode->setMatrix(mat);

	traverse(node, nv);
}

#endif
