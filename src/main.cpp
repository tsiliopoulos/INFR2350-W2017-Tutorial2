// Core Libraries
#include <iostream>
#include <string>
#include <math.h>
#include <map> // for std::map
#include <memory> // for std::shared_ptr

// 3rd Party Libraries
#include <GLEW\glew.h>
#include <GLUT\glut.h>
#include <TTK\OBJMesh.h>
#include <TTK\Camera.h>
#include <IL/il.h> // for ilInit()
#include <glm\vec3.hpp>

// User Libraries
#include "Shader.h"
#include "ShaderProgram.h"
#include "GameObject.h"
#include "FrameBufferObject.h"

// Defines and Core variables
#define FRAMES_PER_SECOND 60
const int FRAME_DELAY = 1000 / FRAMES_PER_SECOND; // Milliseconds per frame

int windowWidth = 800;
int windowHeight = 600;

glm::vec3 mousePosition; // x,y,0
glm::vec3 mousePositionFlipped; // x, height - y, 0

// A few conversions to know
const float degToRad = 3.14159f / 180.0f;
const float radToDeg = 180.0f / 3.14159f;

float deltaTime = 0.0f; // amount of time since last update (set every frame in timer callback)

glm::vec3 position;
float movementSpeed = 5.0f;
glm::vec4 lightPos;

// Cameras
TTK::Camera playerCamera; // the camera you move around with wasd + mouse
TTK::Camera renderCamera; // fixed used to render the scene to an fbo

// Asset databases
std::map<std::string, std::shared_ptr<TTK::MeshBase>> meshes;
std::map<std::string, std::shared_ptr<GameObject>> gameobjects;

// Materials
std::shared_ptr<Material> defaultMaterial;
std::shared_ptr<Material> invertPostProcessMaterial;
std::shared_ptr<Material> unlitTextureMaterial;

FrameBufferObject fbo;

enum GameMode
{
	DRAW_SCENE,
	FBO_DEMO,
	POST_PROCESS_DEMO,
};

GameMode currentMode = DRAW_SCENE;

void initializeShaders()
{
	std::string shaderPath = "../../Assets/Shaders/";

	// Load shaders

	Shader v_default, v_passThrough;
	v_default.loadShaderFromFile(shaderPath + "default_v.glsl", GL_VERTEX_SHADER);
	v_passThrough.loadShaderFromFile(shaderPath + "passThrough_v.glsl", GL_VERTEX_SHADER);

	Shader f_default, f_invertFilter, f_unlitTexture;
	f_default.loadShaderFromFile(shaderPath + "default_f.glsl", GL_FRAGMENT_SHADER);
	f_invertFilter.loadShaderFromFile(shaderPath + "invertFilter_f.glsl", GL_FRAGMENT_SHADER);
	f_unlitTexture.loadShaderFromFile(shaderPath + "unlitTexture_f.glsl", GL_FRAGMENT_SHADER);

	// Default material that all objects use
	defaultMaterial = std::make_shared<Material>();
	defaultMaterial->shader->attachShader(v_default);
	defaultMaterial->shader->attachShader(f_default);
	defaultMaterial->shader->linkProgram();

	// Unlit texture
	unlitTextureMaterial = std::make_shared<Material>();
	unlitTextureMaterial->shader->attachShader(v_passThrough);
	unlitTextureMaterial->shader->attachShader(f_unlitTexture);
	unlitTextureMaterial->shader->linkProgram();

	// Simple invert post process filter
	invertPostProcessMaterial = std::make_shared<Material>();
	invertPostProcessMaterial->shader->attachShader(v_passThrough);
	invertPostProcessMaterial->shader->attachShader(f_invertFilter);
	invertPostProcessMaterial->shader->linkProgram();
}

void initializeScene()
{
	std::string meshPath = "../../Assets/Models/";
	
	std::shared_ptr<TTK::OBJMesh> floorMesh = std::make_shared<TTK::OBJMesh>();
	std::shared_ptr<TTK::OBJMesh> sphereMesh = std::make_shared<TTK::OBJMesh>();
	std::shared_ptr<TTK::OBJMesh> torusMesh = std::make_shared<TTK::OBJMesh>();

	floorMesh->loadMesh(meshPath + "floor.obj");
	sphereMesh->loadMesh(meshPath + "sphere.obj");
	torusMesh->loadMesh(meshPath + "torus.obj");

	// Note: looking up a mesh by it's string name is not the fastest thing,
	// you don't want to do this every frame, once in a while (like now) is fine.
	// If you need you need constant access to a mesh (i.e. you need it every frame),
	// store a reference to it so you don't need to look it up every time.
	meshes["floor"] = floorMesh;
	meshes["sphere"] = sphereMesh;
	meshes["torus"] = torusMesh;

	// Create objects
	gameobjects["floor"] = std::make_shared<GameObject>(glm::vec3(0.0f, 0.0f, 0.0f), floorMesh, defaultMaterial);
	gameobjects["sphere"] = std::make_shared<GameObject>(glm::vec3(0.0f, 5.0f, 0.0f), sphereMesh, defaultMaterial);
	gameobjects["torus"] = std::make_shared<GameObject>(glm::vec3(5.0f, 5.0f, 0.0f), torusMesh, defaultMaterial);

	// Set object properties
	gameobjects["sphere"]->colour = glm::vec4(1.0f);
	gameobjects["floor"]->colour = glm::vec4(0.2f, 0.1f, 0.2f, 1.0f);
	gameobjects["torus"]->colour = glm::vec4(0.1f, 0.2f, 0.2f, 1.0f);

	// Create a quad (probably want to put this in a class...)
	std::shared_ptr<TTK::MeshBase> quadMesh = std::make_shared<TTK::MeshBase>();
	meshes["quad"] = quadMesh;

	// Triangle 1
	quadMesh->vertices.push_back(glm::vec3(1.0f, 1.0f, 0.0f));
	quadMesh->textureCoordinates.push_back(glm::vec2(1.0f, 1.0f));

	quadMesh->vertices.push_back(glm::vec3(-1.0f, 1.0f, 0.0f));
	quadMesh->textureCoordinates.push_back(glm::vec2(0.0f, 1.0f));

	quadMesh->vertices.push_back(glm::vec3(-1.0, -1.0, 0.0f));
	quadMesh->textureCoordinates.push_back(glm::vec2(0.0f, 0.0f));

	// Triangle 2
	quadMesh->vertices.push_back(glm::vec3(1.0, 1.0, 0.0f));
	quadMesh->textureCoordinates.push_back(glm::vec2(1.0f, 1.0f));

	quadMesh->vertices.push_back(glm::vec3(-1.0, -1.0, 0.0f));
	quadMesh->textureCoordinates.push_back(glm::vec2(0.0f, 0.0f));

	quadMesh->vertices.push_back(glm::vec3(1.0, -1.0, 0.0f));
	quadMesh->textureCoordinates.push_back(glm::vec2(1.0f, 0.0f));

	quadMesh->createVBO();
}

void initializeFrameBufferObjects()
{
	/// CODE HERE ////////////////////////////////////////////////////////////
}

void updateScene()
{
	// Move light in simple circular path
	static float ang = 0.0f;

	ang += deltaTime;
	lightPos.x = cos(ang) * 15.0f;
	lightPos.y = 10.0f;
	lightPos.z = sin(ang) * 15.0f;
	lightPos.w = 1.0f;

	gameobjects["sphere"]->setPosition(lightPos);

	// Update all game objects
	for (auto itr = gameobjects.begin(); itr != gameobjects.end(); ++itr)
	{
		auto gameobject = itr->second;

		// Remember: root nodes are responsible for updating all of its children
		// So we need to make sure to only invoke update() for the root nodes.
		// Otherwise some objects would get updated twice in a frame!
		if (gameobject->isRoot())
			gameobject->update(deltaTime);
	}
}

void drawScene(TTK::Camera& cam)
{
	// Send light position to shader
	defaultMaterial->vec4Uniforms["u_lightPos"] = cam.viewMatrix * lightPos;

	for (auto itr = gameobjects.begin(); itr != gameobjects.end(); ++itr)
	{
		auto gameobject = itr->second;

		if (gameobject->isRoot())
			gameobject->draw(cam);
	}
}

// This is where we draw stuff
void DisplayCallbackFunction(void)
{
	// Update cameras (there's two now!)
	playerCamera.update();
	renderCamera.update();

	// Update all gameobjects
	updateScene();

	switch (currentMode)
	{
		case DRAW_SCENE: // press 1
		{
			/// CODE HERE ////////////////////////////////////////////////////////////

			// Clear back buffer
			glClearColor(0.8f, 0.8f, 0.8f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Just draw the scene to the back buffer
			drawScene(playerCamera);
		}
		break;

		case FBO_DEMO: // press 2
		{
			/// CODE HERE ////////////////////////////////////////////////////////////
		}
		break;

		case POST_PROCESS_DEMO: // press 3
		{
			/// CODE HERE ////////////////////////////////////////////////////////////
		}
		break;
	}

	/* Swap Buffers to Make it show up on screen */
	glutSwapBuffers();
}

/* function void KeyboardCallbackFunction(unsigned char, int,int)
* Description:
*   - this handles keyboard input when a button is pressed
*/
void KeyboardCallbackFunction(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27: // the escape key
	case 'q': // the 'q' key
	case 'Q':
		playerCamera.moveUp();
		break;
	case 'e':
	case 'E':
		playerCamera.moveDown();
	case 'W':
	case 'w':
		playerCamera.moveForward();
		break;
	case 'S':
	case 's':
		playerCamera.moveBackward();
		break;
	case 'A':
	case 'a':
		playerCamera.moveRight();
		break;
	case 'D':
	case 'd':
		playerCamera.moveLeft();
		break;
	}
}

/* function void KeyboardUpCallbackFunction(unsigned char, int,int)
* Description:
*   - this handles keyboard input when a button is lifted
*/
void KeyboardUpCallbackFunction(unsigned char key, int x, int y)
{
	switch (key)
	{
		case '1':
			currentMode = DRAW_SCENE;
		break;

		case '2':
			currentMode = FBO_DEMO;
		break;

		case '3':
			currentMode = POST_PROCESS_DEMO;
		break;


	default:
		break;
	}
}

/* function TimerCallbackFunction(int value)
* Description:
*  - this is called many times per second
*  - this enables you to animate things
*  - no drawing, just changing the state
*  - changes the frame number and calls for a redisplay
*  - FRAME_DELAY is the number of milliseconds to wait before calling the timer again
*/
void TimerCallbackFunction(int value)
{
	// Calculate new deltaT for potential updates and physics calculations
	static int elapsedTimeAtLastTick = 0;
	int totalElapsedTime = glutGet(GLUT_ELAPSED_TIME);

	deltaTime = totalElapsedTime - elapsedTimeAtLastTick;
	deltaTime /= 1000.0f;
	elapsedTimeAtLastTick = totalElapsedTime;

	/* this call makes it actually show up on screen */
	glutPostRedisplay();
	/* this call gives it a proper frame delay to hit our target FPS */
	glutTimerFunc(FRAME_DELAY, TimerCallbackFunction, 0);
}

/* function WindowReshapeCallbackFunction()
* Description:
*  - this is called whenever the window is resized
*  - and sets up the projection matrix properly
*/
void WindowReshapeCallbackFunction(int w, int h)
{
	/* Update our Window Properties */
	windowWidth = w;
	windowHeight = h;

	playerCamera.winHeight = h;
	playerCamera.winWidth = w;

	renderCamera.winHeight = h;
	renderCamera.winWidth = w;
}


void MouseClickCallbackFunction(int button, int state, int x, int y)
{
	mousePosition.x = x;
	mousePosition.y = y;

	mousePositionFlipped = mousePosition;
	mousePositionFlipped.y = windowHeight - mousePosition.y;
}

void SpecialInputCallbackFunction(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		position.z += movementSpeed * deltaTime;
		break;
	case GLUT_KEY_DOWN:
		position.z -= movementSpeed * deltaTime;
		break;
	case GLUT_KEY_LEFT:
		position.x += movementSpeed * deltaTime;
		break;
	case GLUT_KEY_RIGHT:
		position.x -= movementSpeed * deltaTime;
		break;
	}
}

// Called when the mouse is clicked and moves
void MouseMotionCallbackFunction(int x, int y)
{
	if (mousePosition.length() > 0)
		playerCamera.processMouseMotion(x, y, mousePosition.x, mousePosition.y, deltaTime);

	mousePosition.x = x;
	mousePosition.y = y;

	mousePositionFlipped = mousePosition;
	mousePositionFlipped.y = windowHeight - mousePosition.y;
}

/* function main()
* Description:
*  - this is the main function
*  - does initialization and then calls glutMainLoop() to start the event handler
*/
int main(int argc, char **argv)
{
	// Memory Leak Detection
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	/* initialize the window and OpenGL properly */
	glutInit(&argc, argv);
	glutInitWindowSize(windowWidth, windowHeight);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutCreateWindow("Tutorial");

	/* set up our function callbacks */
	glutDisplayFunc(DisplayCallbackFunction);
	glutKeyboardFunc(KeyboardCallbackFunction);
	glutKeyboardUpFunc(KeyboardUpCallbackFunction);
	glutReshapeFunc(WindowReshapeCallbackFunction);
	glutMouseFunc(MouseClickCallbackFunction);
	glutMotionFunc(MouseMotionCallbackFunction);
	glutTimerFunc(1, TimerCallbackFunction, 0);
	glutSpecialFunc(SpecialInputCallbackFunction);

	// Init GLEW
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		std::cout << "TTK::InitializeTTK Error: GLEW failed to init" << std::endl;
	}
	printf("OpenGL version: %s, GLSL version: %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

	// Init IL
	ilInit();

	// Init GL
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// Initialize scene
	initializeShaders();
	initializeScene();
	initializeFrameBufferObjects();

	/* Start Game Loop */
	deltaTime = glutGet(GLUT_ELAPSED_TIME);
	deltaTime /= 1000.0f;

	glutMainLoop();

	return 0;
}