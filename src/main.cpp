#include <iostream>

#ifdef _WIN32
	#include <windows.h>	/// Windows does some crazy shit if you don't include this file.
#endif

#include <GL/glew.h>

#include "globals.h"
#include "Level.h"
#include "Player.h"

#ifdef TOP_DOWN_2D_VIEW
	#include "TwoDimensionalCameraManipulator.h"
#else
	#include "ThirdPersonCameraManipulator.h"
#endif
#include "AngelScriptEngine.h"
#include "AngelScriptConsole.h"
#include "TemporaryText.h"

#include "GameObjectData.h"


//#include <osgViewer/config/SingleWindow>
/// could not find the config directory in the openSUSE repo, so I copied the essentials here instead.
namespace osgViewer
{
class SingleWindow : public ViewConfig
{
public:
    SingleWindow():_x(0),_y(0),_width(-1),_height(-1),_screenNum(0),_windowDecoration(true),_overrideRedirect(false) {}
    SingleWindow(int x, int y, int width, int height, unsigned int screenNum=0):_x(x),_y(y),_width(width),_height(height),_screenNum(screenNum),_windowDecoration(true),_overrideRedirect(false) {}
    SingleWindow(const SingleWindow& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY):ViewConfig(rhs,copyop),_x(rhs._x),_y(rhs._y),_width(rhs._width),_height(rhs._height),_screenNum(rhs._screenNum),_windowDecoration(rhs._windowDecoration), _overrideRedirect(rhs._overrideRedirect) {}

    META_Object(osgViewer,SingleWindow);

    virtual void configure(osgViewer::View& view) const;

	void setX(int x) { _x = x; }
	int getX() const { return _x; }

	void setY(int y) { _y = y; }
    int getY() const { return _y; }

	void setWidth(int w) { _width = w; }
	int getWidth() const { return _width; }

	void setHeight(int h) { _height = h; }
	int getHeight() const { return _height; }

	void setScreenNum(unsigned int sn) { _screenNum = sn; }
	unsigned int getScreenNum() const { return _screenNum; }

	void setWindowDecoration(bool wd) { _windowDecoration = wd; }
	bool getWindowDecoration() const { return _windowDecoration; }

	void setOverrideRedirect(bool override) { _overrideRedirect = override; }
	bool getOverrideRedirect() const { return _overrideRedirect; }

protected:
    int _x, _y, _width, _height;
    unsigned int _screenNum;
    bool _windowDecoration;
    bool _overrideRedirect;
};

}



#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/WriteFile>
#include <osgViewer/ViewerEventHandlers>

#include "CEGUIStuff.h"


#include "Editor/Editor.h"

#include <stdio.h>
#ifdef _WIN32
    #include <direct.h>
#else
    #include <unistd.h>
#endif


#include "osgwTools/FindNamedNode.h"


#include "Light.h"


using namespace std;

/// Variables declared extern in globals.h
osg::Group* root;
unsigned int windowWidth, windowHeight;
double deltaTime;


Level* level;


osgAnimation::BasicAnimationManager* animationManager;

void logError(std::string errorMessage)
{
	std::cout << "Error: " << errorMessage << std::endl;
}
void logWarning(std::string warning)
{
	std::cout << "Warning: " << warning << std::endl;
}

double getDeltaTime()
{
	return deltaTime;
}

double sind(double x) {
	return sin(x*pi/180);
}
double cosd(double x) {
	return cos(x*pi/180);
}
double tand(double x) {
	return tan(x*pi/180);
}
double asind(double x) {
	return asin(x) * 180/pi;
}
double acosd(double x) {
	return acos(x) * 180/pi;
}
double atand(double x) {
	return atan(x) * 180/pi;
}

float getDistance(osg::Vec3 a, osg::Vec3 b)
{
	return sqrt(pow(a.x() - b.x(), 2) + pow(a.y() - b.y(), 2) + pow(a.z() - b.z(), 2));
}

/// These are used by Player to figure out the right translation to move the player when you press a movement key. They should be moved to Player to prevent confusion (they don't technically do a proper conversion from camera to world coordinates, because they only account for rotation along the z axis).
osg::Vec3 cameraToWorldTranslation(float x, float y, float z)
{
	double camangle = ((BaseCameraManipulator*) getViewer()->getCameraManipulator())->getHeading();
	return osg::Vec3(x*cos(camangle) + y*cos(camangle+pi/2), x*sin(camangle) + y*sin(camangle+pi/2), z);
}
osg::Vec3 cameraToWorldTranslation(osg::Vec3 camTranslation)
{
	return cameraToWorldTranslation(camTranslation.x(), camTranslation.y(), camTranslation.z());
}


/// Visitor to return the world coordinates of a node.
/// It traverses from the starting node to the parent.
/// The first time it reaches a root node, it stores the world coordinates of the node it started from.  The world coordinates are found by concatenating all the matrix transforms found on the path from the start node to the root node.
class getWorldCoordOfNodeVisitor : public osg::NodeVisitor
{
public:
   getWorldCoordOfNodeVisitor():
      osg::NodeVisitor(NodeVisitor::TRAVERSE_PARENTS), done(false)
      {
         wcMatrix= new osg::Matrixd();
      }
      virtual void apply(osg::Node &node)
      {
         if (!done)
         {
            if ( 0 == node.getNumParents() ) // no parents
            {
               wcMatrix->set( osg::computeLocalToWorld(this->getNodePath()) );
               done = true;
            }
            traverse(node);
         }
      }
      osg::Matrixd* giveUpDaMat()
      {
         return wcMatrix;
      }
private:
   bool done;
   osg::Matrix* wcMatrix;
};

/// Given a valid node placed in a scene under a transform, return the world coordinates in an osg::Matrix.
/// Creates a visitor that will update a matrix representing world coordinates of the node, return this matrix.
/// (This could be a class member for something derived from node also.)
osg::Matrixd* getWorldCoordinates( osg::Node* node)
{
   getWorldCoordOfNodeVisitor* ncv = new getWorldCoordOfNodeVisitor();
   if (node && ncv)
   {
      node->accept(*ncv);
      return ncv->giveUpDaMat();
   }
   else
   {
      return NULL;
      logWarning("Got NULL world coordinates");
   }
}


void GameOverYouLose()
{
	std::cout << "You Lose." << std::endl;
	new TemporaryText("You Lose", getActivePlayer()->getWorldPosition(), 5.0);
	// TODO: should actually end the game.
}

bool safeToAddRemoveNodes;
std::deque<std::pair<osg::Node*,osg::Group*>> nodesToAdd;
std::deque<std::pair<osg::Node*,osg::Group*>> nodesToRemove;

void addToSceneGraph(osg::Node* node, osg::Group* parent)
{
	if(safeToAddRemoveNodes)
		parent->addChild(node);
	else
	{
		nodesToAdd.push_back(std::pair<osg::Node*,osg::Group*>(node, parent));
	}
}
void removeFromSceneGraph(osg::Node* node, osg::Group* parent)
{
	if(safeToAddRemoveNodes)
		parent->removeChild(node);
	else
	{
		nodesToRemove.push_back(std::pair<osg::Node*,osg::Group*>(node, parent));
	}
}

void addAndRemoveQueuedNodes()
{
	while(!nodesToAdd.empty())
	{
		std::pair<osg::Node*,osg::Group*> nodeAndParent = nodesToAdd.front();
		nodeAndParent.second->addChild(nodeAndParent.first);
		nodesToAdd.pop_front();
	}
	while(!nodesToRemove.empty())
	{
		std::pair<osg::Node*,osg::Group*> nodeAndParent = nodesToRemove.front();
		nodeAndParent.second->removeChild(nodeAndParent.first);
		nodesToRemove.pop_front();
	}
}

void runCleanup()
{
	removeExpiredObjects();
}

void writeOutSceneGraph()
{
	osgDB::writeNodeFile(*root, "output/sceneGraph.osg");
	level->saveAsFile("output/exportedScene.yaml");
}



osgViewer::Viewer* getViewer()
{
	static osgViewer::Viewer* viewer = new osgViewer::Viewer();
	return viewer;
}


bool isGamePaused()
{
	return !(getEditor()->getMode() == Editor::Play);
}


int main()
{
	root = new osg::Group();
	safeToAddRemoveNodes = true;

	char currentDirectory[FILENAME_MAX];
	getcwd(currentDirectory, sizeof(currentDirectory));
	std::cout << currentDirectory << std::endl;

	///  Set up asset search paths
	osgDB::FilePathList pathList = osgDB::getDataFilePathList();
	pathList.push_back(std::string(currentDirectory) + "/media/");
	pathList.push_back(std::string(currentDirectory) + "/");
	osgDB::setDataFilePathList(pathList);



	osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
	if (!wsi)
		osg::notify(osg::NOTICE) << "Error, no WindowSystemInterface available, cannot create windows." << std::endl;

	wsi->getScreenResolution(osg::GraphicsContext::ScreenIdentifier(0), windowWidth, windowHeight);

	getViewer()->setSceneData(root);
	getViewer()->addEventHandler(getMainEventHandler());
	getViewer()->apply(new osgViewer::SingleWindow(0, 0, windowWidth, windowHeight, 0));
#ifdef TOP_DOWN_2D_VIEW
	getViewer()->setCameraManipulator(new TwoDimensionalCameraManipulator());
	getViewer()->getCamera()->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	getViewer()->getCamera()->setViewMatrix(osg::Matrix::identity());
#else
	getViewer()->setCameraManipulator(new ThirdPersonCameraManipulator());
#endif

	getViewer()->setKeyEventSetsDone(0);	/// By default, osg exits if you press escape. We don't want that.

	osgViewer::StatsHandler* statsHandler = new osgViewer::StatsHandler();
	statsHandler->setKeyEventTogglesOnScreenStats( osgGA::GUIEventAdapter::KEY_F3);
	getViewer()->addEventHandler(statsHandler);
    //getViewer()->addEventHandler(new osgViewer::WindowSizeHandler());

    getViewer()->setRealizeOperation(new CEGUIInitOperation());	/// Must initialize CEGUI from the graphics thread
	getViewer()->setReleaseContextAtEndOfFrameHint(false);		/// Trying to fix CEGUI, see http://forum.openscenegraph.org/viewtopic.php?p=27747#27747
	getViewer()->realize();

	level = new Level("media/SampleLevel.yaml");

	getScriptEngine()->runFile("initialize.as");

	root->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::ON);


	addAndRemoveQueuedNodes();

	getViewer()->frame(0);

	Light::enableAllLights();


	osg::Timer_t frame_tick = osg::Timer::instance()->tick();

	double elapsedTime = 0.0;	/// Elapsed time, in seconds, that the game has been running for.



	getEditor()->setMode(Editor::Edit);

	addAndRemoveQueuedNodes();

	while(!getViewer()->done())
	{
		osg::Timer_t now_tick = osg::Timer::instance()->tick();
		deltaTime = osg::Timer::instance()->delta_s(frame_tick, now_tick);
		frame_tick = now_tick;

		elapsedTime = osg::Timer::instance()->time_s();

		if(!isGamePaused())
			getCurrentLevel()->updatePhysics(deltaTime);
		else
			getCurrentLevel()->updatePhysics(0);

		std::ostringstream hudStream;
		hudStream << std::endl;		/// I don't know why, but it seems we need to do this or CEGUI won't display.
		getDebugDisplayer()->addText(hudStream);

		removeExpiredObjects();

		safeToAddRemoveNodes = false;
		getViewer()->frame(elapsedTime);
		safeToAddRemoveNodes = true;
		addAndRemoveQueuedNodes();
	}

	return 0;
}


