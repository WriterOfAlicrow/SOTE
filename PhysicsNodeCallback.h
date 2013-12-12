#ifndef PHYSICSNODECALLBACK_H
#define PHYSICSNODECALLBACK_H

#include "Box2D/Box2D.h"
#include "globals.h"
#include <osg/PositionAttitudeTransform>

class PhysicsNodeCallback : public osg::NodeCallback
{
public:

	osg::PositionAttitudeTransform *transformNode;
	b2Body *physicsBody;
	osg::Vec3 box2DToOsgAdjustment;	//adjustment between the visual and physical components

	PhysicsNodeCallback(osg::PositionAttitudeTransform *transformNode, b2Body *physicsBody, osg::Vec3 box2DToOsgAdjustment);

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
};

#endif // PHYSICSNODECALLBACK_H