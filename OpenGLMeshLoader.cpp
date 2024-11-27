#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <glut.h>
#include <vector>
#include <cstdlib> 
#include <ctime>   

int WIDTH = 1280;
int HEIGHT = 720;

GLuint tex;
char title[] = "Minion Mania";

// 3D Projection Options
GLdouble fovy = 45.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 500;
float xPositions[] = {0.6f, 1.6f, 2.4f, 3.3f, 4.1f, 5.0f }; 
int xCount = 6;
float minionX = 2.4f;      
int LaneIndex = 2; 

struct Banana {
	float x, y, z;
};
std::vector<Banana> bananas;

void SpawnBananas(int count) {
	bananas.clear(); 
	float y = 11.0f; 
	float zStart = 55.0f; 

	for (int i = 0; i < count; ++i) {
		float x = xPositions[rand() % xCount]; 
		float z = zStart - i * 10.0f;          
		bananas.push_back({ x, y, z });
	}
}


class Vector
{
public:
	GLdouble x, y, z;
	Vector() {}
	Vector(GLdouble _x, GLdouble _y, GLdouble _z) : x(_x), y(_y), z(_z) {}
	//================================================================================================//
	// Operator Overloading; In C++ you can override the behavior of operators for you class objects. //
	// Here we are overloading the += operator to add a given value to all vector coordinates.        //
	//================================================================================================//
	void operator +=(float value)
	{
		x += value;
		y += value;
		z += value;
	}
};

Vector Eye(2.5, 12, 64);
Vector At(0, 0, 0);
Vector Up(0, 1, 0);

int cameraZoom = 0;

// Model Variables
Model_3DS model_minion;
Model_3DS model_finishLine;
Model_3DS model_bridge;
Model_3DS model_banana;
Model_3DS model_sandbags;
// Textures
GLTexture tex_ground;

bool isThirdPerson = true;
float bridgePositionZ = 0.0f;
float bridgeSpeed = 0.006f;
float bananaPositionZ = 0.0f;



//=======================================================================
// Lighting Configuration Function
//=======================================================================
void InitLightSource()
{
	// Enable Lighting for this OpenGL Program
	glEnable(GL_LIGHTING);

	// Enable Light Source number 0
	// OpengL has 8 light sources
	glEnable(GL_LIGHT0);

	// Define Light source 0 ambient light
	GLfloat ambient[] = { 0.1f, 0.1f, 0.1, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	// Define Light source 0 diffuse light
	GLfloat diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	// Define Light source 0 Specular light
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	// Finally, define light source 0 position in World Space
	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

//=======================================================================
// Material Configuration Function
//======================================================================
void InitMaterial()
{
	// Enable Material Tracking
	glEnable(GL_COLOR_MATERIAL);

	// Sich will be assigneet Material Properties whd by glColor
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	// Set Material's Specular Color
	// Will be applied to all objects
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);

	// Set Material's Shine value (0->128)
	GLfloat shininess[] = { 96.0f };
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}

//=======================================================================
// OpengGL Configuration Function
//=======================================================================
void myInit(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	gluPerspective(fovy, aspectRatio, zNear, zFar);
	//*//
	// fovy:			Angle between the bottom and top of the projectors, in degrees.			 //
	// aspectRatio:		Ratio of width to height of the clipping plane.							 //
	// zNear and zFar:	Specify the front and back clipping planes distances from camera.		 //
	//*//

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
	//*//
	// EYE (ex, ey, ez): defines the location of the camera.									 //
	// AT (ax, ay, az):	 denotes the direction where the camera is aiming at.					 //
	// UP (ux, uy, uz):  denotes the upward orientation of the camera.							 //
	//*//

	InitLightSource();

	InitMaterial();
	
	srand(static_cast<unsigned>(time(0)));
	SpawnBananas(10); 
	

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_NORMALIZE);
}

//=======================================================================
// Render Ground Function
//=======================================================================
void RenderGround()
{
	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-20, 0, -20);
	glTexCoord2f(5, 0);
	glVertex3f(20, 0, -20);
	glTexCoord2f(5, 5);
	glVertex3f(20, 0, 20);
	glTexCoord2f(0, 5);
	glVertex3f(-20, 0, 20);
	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

void RenderPlayer() {
	glPushMatrix();
	glTranslatef(minionX, 11.0, 60.0);
	glScalef(0.20, 0.20, 0.20);
	glRotatef(180, 0, 1, 0);
	model_minion.Draw();
	glPopMatrix();
}

void RenderSky() {
	glPushMatrix();

	GLUquadricObj* qobj;
	qobj = gluNewQuadric();
	glTranslated(50, 0, 0);
	glRotated(90, 1, 0, 1);
	glBindTexture(GL_TEXTURE_2D, tex);
	gluQuadricTexture(qobj, true);
	gluQuadricNormals(qobj, GL_SMOOTH);
	gluSphere(qobj, 100, 100, 100);
	gluDeleteQuadric(qobj);


	glPopMatrix();

}

void RenderBridge() {
	glPushMatrix();
	glTranslatef(70.0f, 0.0f, bridgePositionZ - 10.0f);
	glScalef(4.0f, 4.0f, 4.0f);
	glRotatef(90, 0, 1, 0);
	model_bridge.Draw();
	glPopMatrix();
}


void RenderBananas() {
	for (const auto& banana : bananas) {
		glPushMatrix();
		glTranslatef(banana.x, banana.y, banana.z);
		glRotatef(90, 0, 1, 0);
		glScalef(0.3f, 0.3f, 0.3f);
		model_banana.Draw();
		glPopMatrix();
	}
}

void RenderSandbags() {
	glPushMatrix();
	glTranslatef(1.0f, 10.0f, 40.0f);
	glRotatef(90, 0, 1, 0);
	glScalef(1.0f, 1.0f, 1.0f);
	model_sandbags.Draw();
	glPopMatrix();
}

void UpdatePositions() {
	bridgePositionZ += bridgeSpeed;
	if (bridgePositionZ < -50.0f) {
		bridgePositionZ = 0.0f;
	}
}


//=======================================================================
// Display Function
//=======================================================================
void Display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Update the camera view based on the current mode
	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);

	// Lighting setup
	GLfloat lightIntensity[] = { 0.7, 0.7, 0.7, 1.0f };
	GLfloat lightPosition[] = { 0.0f, 100.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);

	// Render the scene
	RenderGround();
	RenderPlayer();
	RenderSky();
	RenderBridge();
	RenderBananas();
	UpdatePositions();


	glutSwapBuffers();
}
void Idle() {
    for (auto& banana : bananas) {
        banana.z += 0.005f;
    }
    glutPostRedisplay();
}

void UpdateCamera() {
	if (isThirdPerson) {
		Eye = Vector(minionX, 12, 64);
		At = Vector(0, 0, 0);
	}
	else {
		Eye = Vector(minionX, 12, 45);
		At = Vector(0, 2, -10);
	}

	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
}


//=======================================================================
// Keyboard Function
//=======================================================================
void Keyboard(unsigned char button, int x, int y) {
	switch (button) {
	case 27: // Escape
		exit(0);
		break;

	case 'w': // Toggle third-person view
		isThirdPerson = !isThirdPerson;
		UpdateCamera();
		break;

	case 'a': 
		if (LaneIndex > 0) {
			LaneIndex--;
			minionX = xPositions[LaneIndex];
		}
		break;

	case 'd': 
		if (LaneIndex < xCount - 1) {
			LaneIndex++;
			minionX = xPositions[LaneIndex];
		}
		break;

	default:
		break;
	}
	glutPostRedisplay();
}



//=======================================================================
// Motion Function
//=======================================================================
void Motion(int x, int y)
{
	y = HEIGHT - y;

	if (cameraZoom - y > 0)
	{
		Eye.x += -0.1;
		Eye.z += -0.1;
	}
	else
	{
		Eye.x += 0.1;
		Eye.z += 0.1;
	}

	cameraZoom = y;

	glLoadIdentity();	//Clear Model_View Matrix

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);	//Setup Camera with modified paramters

	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glutPostRedisplay();	//Re-draw scene 
}

//=======================================================================
// Mouse Function
//=======================================================================
void Mouse(int button, int state, int x, int y)
{
	y = HEIGHT - y;

	if (state == GLUT_DOWN)
	{
		cameraZoom = y;
	}
}

//=======================================================================
// Reshape Function
//=======================================================================
void Reshape(int w, int h)
{
	if (h == 0) {
		h = 1;
	}

	WIDTH = w;
	HEIGHT = h;

	// set the drawable region of the window
	glViewport(0, 0, w, h);

	// set up the projection matrix 
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (GLdouble)WIDTH / (GLdouble)HEIGHT, zNear, zFar);

	// go back to modelview matrix so we can move the objects about
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
}

//=======================================================================
// Assets Loading Function
//=======================================================================
void LoadAssets()
{
	// Loading Model files
	model_minion.Load("Models/minion/minion.3ds");
	model_finishLine.Load("Models/gate/gate.3ds");
	model_bridge.Load("Models/bridge/bridge.3ds");
	model_banana.Load("Models/banana/banana.3ds");
	//model_sandbags.Load("Models/sandbags/sandbags.3ds");

	// Loading texture files
	tex_ground.Load("Textures/ground.bmp");
	loadBMP(&tex, "Textures/blu-sky-3.bmp", true);

}

//=======================================================================
// Main Function
//=======================================================================
void main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(WIDTH, HEIGHT);

	glutInitWindowPosition(100, 150);

	glutCreateWindow(title);

	glutDisplayFunc(Display);
	glutKeyboardFunc(Keyboard);
	glutMotionFunc(Motion);
	glutMouseFunc(Mouse);
	glutReshapeFunc(Reshape);
	glutIdleFunc(Idle);

	myInit();
	LoadAssets();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);

	glutMainLoop();
}