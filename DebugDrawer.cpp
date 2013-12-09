#include "DebugDrawer.h"
#include "globals.h"

DebugDrawer::DebugDrawer()
{
	_group = new osg::Group();
	//_group->setName("Box2D Debug Drawing");

	_geode = new osg::Geode();
	//_geode->setName("Box2D Debug Drawing");
//	_geode->setDataVariance(osg::Object::DYNAMIC);
	/// BUG: Data variance should be set to dynamic, but that seems to drastically increase seg faults in stl_construct. Without setting dynamic data variance, program still seg faults in stl_construct if too many physics objects are created, but otherwise seems to perform fine.

	//_group->addChild(_geode.get());
	//_group->addChild(_geode);

	_lineGeometry = new osg::Geometry();
//	_lineGeometry->setDataVariance(osg::Object::DYNAMIC);
	//_lineGeometry->setUseDisplayList(false);
	//_lineGeometry->setUseVertexBufferObjects(false);
	//_geode->addDrawable(_lineGeometry.get());
	_geode->addDrawable(_lineGeometry);

	_lineVertices = new osg::Vec3Array();
	_lineGeometry->setVertexArray(_lineVertices);
	_lineColors = new osg::Vec4Array();
	//_lineGeometry->setColorArray(_lineColors);
	//_lineGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	//root->addChild(_group.get());
	root->addChild(_geode);

	active = false;
}

DebugDrawer::~DebugDrawer()
{
	//dtor
}

void DebugDrawer::beginDraw()
{
	if(_lineVertices->size() > 0)
	{
		_lineGeometry->removePrimitiveSet(0);
		_lineVertices->clear();
		_lineColors->clear();
	}

	active = true;
}

void DebugDrawer::endDraw()
{
	if(_lineVertices->size())
		_lineGeometry->addPrimitiveSet(new osg::DrawArrays(GL_LINES, 0, _lineVertices->size()));

	active = false;
}

/// Draw a closed polygon provided in CCW order.
void DebugDrawer::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	for (int i = 0; i < vertexCount; ++i)
	{
		if (i == vertexCount - 1)
			this->DrawSegment(vertices[i], vertices[0], color);	// last point connects to the first one.
		else
			this->DrawSegment(vertices[i], vertices[i+1], color);
	}
}

/// Draw a solid closed polygon provided in CCW order.
void DebugDrawer::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	//TODO:
	getDebugDisplayer()->addText("DebugDrawer DrawSolidPolygon called\n");
}

/// Draw a circle.
void DebugDrawer::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
{
	//TODO:
	getDebugDisplayer()->addText("DebugDrawer DrawCircle called\n");
}

/// Draw a solid circle.
void DebugDrawer::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
{
	//TODO:
	getDebugDisplayer()->addText("DebugDrawer DrawSolidCircle called\n");
}

/// Draw a line segment.
void DebugDrawer::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
	//TODO:
	//getDebugDisplayer()->addText("DebugDrawer DrawSegment called\n");

	_lineVertices->push_back(b2Vec2ToOsgVec3(p1, drawZCoordinate));
	_lineVertices->push_back(b2Vec2ToOsgVec3(p2, drawZCoordinate));

	osg::Vec4 osgColor = b2ColorToOsgVec4(color);
	_lineColors->push_back(osgColor);
	_lineColors->push_back(osgColor);
}

/// Draw a transform. Choose your own length scale.
/// @param xf a transform.
void DebugDrawer::DrawTransform(const b2Transform& xf)
{
	//TODO:
	getDebugDisplayer()->addText("DebugDrawer DrawTransform called\n");
}
