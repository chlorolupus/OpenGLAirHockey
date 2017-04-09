/* Link the libraries code-wise. */
//assimp sample code from:https://raw.githubusercontent.com/assimp/assimp/master/samples/SimpleTexturedOpenGL/SimpleTexturedOpenGL/src/model_loading.cpp
#ifdef _MSC_VER
#	pragma comment(lib, "OpenGL32.lib")
#	pragma comment(lib, "freeglut.lib")
#	pragma comment(lib, "glfw3.lib")
#	pragma comment(lib, "glfw3dll.lib")
#	pragma comment(lib, "glew32.lib")
#	pragma comment(lib, "glew32s.lib")

#	pragma comment(lib, "SDL.lib")
#	pragma comment(lib, "SDLmain.lib")
#	pragma comment(lib, "SDL_image.lib")
#	pragma comment(lib, "assimp-vc140-mt.lib")
#	pragma comment(lib, "DevIL.lib")
#	pragma comment(lib, "ILU.lib")
#	pragma comment(lib, "ILUT.lib")



#endif //_MSC_VER
#include <algorithm>
#include <string>
#include <cmath>

#include <map>
#include <IL/il.h>
#include <SDL/SDL.h>
#include <GL/glew.h>
//#include <SDL/SDL_opengl.h>
#include <SDL/SDL_image.h>
#include <vector>
#include <GL/gl.h>
//#include <GL/glu.h>

#include <GL/freeglut.h>
/*
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include "assimp/LogStream.hpp"
*/
//#include <GL/glut.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>


//#include "mesh.h"

#include <fstream>
#include <glm/gtc/matrix_transform.hpp>

#define PI 3.141592653589793
	
unsigned Textures[10];
unsigned BoxList(0);					//Added!
int screenwidth[] = { 640, 800, 1024 };
int screenheight[] = { 480, 600, 768 };
int screenMode = 0; //0 for 640*480, 1 FOR 800*600, 2 FOR 1024*768
bool fullScreen = false;
bool lightMenuActive = false;
int lightColor[] = { 1,1,1 };
/* These will define the player's position and view angle. */
double X(0.0), Y(0.0), Z(2.3);
double ViewAngleHor(0.0), ViewAngleVer(0.0);

GLUquadric* DiscObj;
GLuint vertexbuffer;

int slices = 12;
float radius(.075f), halfLength(.025f);
int height = 5;
//Mesh* m_pMesh;


float holeWidth = 0.4f;
float holeHeight = 0.01666f;;


float tableX = 1.0f;
float tableZ = 2.0f;	
float enemyHoleZ = -(tableZ - holeHeight);
float playerHoleZ = tableZ - holeHeight;

float holeLeft = -holeWidth;
float holeRight = holeWidth;

float tableWidth = 1.f;
float tableTopFront = 2.0f;
float tableTopBack = -2.0f;
float tableTop = -0.35f;
float tableHorMid = 0.0f;
float tableBot = -0.5f;

//Ball. mallet and player parameter
double ballX(0.0f), ballY(-0.3f), ballZ(0.0f); // BallY = Mallets Y, BallX clamp at -1 - 1, ballZ clamp at -2 - 2, goal at 1.8 
double mallet1X(0.0f), mallet1Y(-0.3f), mallet1Z(1.0f);
double mallet2X(0.0f), mallet2Y(-0.3f), mallet2Z(-1.0f);
double malletSpeed(0.005f);
float ballRadius = radius;
float malletRadius = radius;
float ballForce = 0.0025f;
float ballVX = 0.0f;
float ballVZ = 0.0f;



float MaxMovement = 0.6f;
float period = 1.f / 128.f * PI;
float currentPeriod = 0.f;

//Code from: http://www.glprogramming.com/red/chapter05.html#name6
//Light param
GLfloat GL_LIGHT0_AMBIENT[] = { 0.5f,0.5f,0.5f,1.f };
GLfloat GL_LIGHT0_DIFFUSE[] = { 1.f,1.f,1.f,1.f };
GLfloat GL_LIGHT0_SPECULAR[] = { 1.f,1.f,1.f,1.f };
GLfloat GL_LIGHT0_POSITION[] = { 1.f,1.f,0.f }; 

//Mat Param
GLfloat no_mat[] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat mat_ambient[] = { 0.7, 0.7, 0.7, 1.0 };
GLfloat mat_ambient_color[] = { 0.8, 0.8, 0.2, 1.0 };
GLfloat mat_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat no_shininess[] = { 0.0 };
GLfloat low_shininess[] = { 5.0 };
GLfloat high_shininess[] = { 100.0 };
GLfloat mat_emission[] = { 0.3, 0.2, 0.2, 0.0 };

/*
 * DegreeToRadian
 *	Converts a specified amount of degrees to radians.
 */
 // check the ball go into which hole
 // 0 = none, 1 = enemy hole, 2 = player hole
int ballInHole(float x, float  y) {
	if (x > holeLeft && x < holeRight && y > -tableZ && y <enemyHoleZ) {
		return 1;
	}
	else if (x > holeLeft && x < holeRight && y > playerHoleZ && y < tableZ) {
		return 2;
	}
	return 0;
}
//Ball collision code
float ballDistance(float x1, float y1, float x2, float y2) {
	float dx = x2 - x1;
	float dy = y2 - y1;
	return glm::pow(dx,2) + glm::pow(dy,2);
};

void collisionWithMallet(float x1, float y1, float x2, float y2) {
	if (ballDistance(x1, y1, x2, y2) < malletRadius + ballRadius) {
		float angle = atan2(x2 - x1, y2 - y1) * PI;
		ballVX = ballForce * sin(angle);
		ballVZ = -ballForce * cos(angle);
	}
}

void updateBall() {
	float newX = ballX + ballVX;
	float newZ = ballZ + ballVZ;

	// collision with wall
	if (newX - ballRadius > tableX) {
		ballVX = -abs(ballVX);
	}
	if (newX + ballRadius < -tableX) {
		ballVX = abs(ballVX);
	}
	if (newZ - ballRadius > tableZ) {
		ballVZ = -abs(ballVZ);
	}
	if (newZ + ballRadius < -tableZ) {
		ballVZ = abs(ballVZ);
	}

	// collision with mallet
	collisionWithMallet(ballX, ballZ, mallet1X, mallet1Z);
	collisionWithMallet(ballX, ballZ, mallet2X, mallet2Z);


	ballX = newX;
	ballZ = newZ;
}

inline double DegreeToRadian(double degrees)
{
	return (degrees / 180.f * PI);
}


/*
 * GrabTexObjFromFile
 *	This function will use SDL to load the specified image, create an OpenGL
 *	texture object from it and return the texture object number.
 */
GLuint GrabTexObjFromFile(const std::string& fileName)
{
	/* Use SDL_image to load the PNG image. */
	SDL_Surface *Image = IMG_Load(fileName.c_str());

	/* Image doesn't exist or failed loading? Return 0. */
	if (!Image)
		return 0;

	unsigned Object(0);

	/* Generate one texture (we're creating only one). */
	glGenTextures(1, &Object);

	/* Set that texture as current. */
	glBindTexture(GL_TEXTURE_2D, Object);

	/* You can use these values to specify mipmaps if you want to, such as 'GL_LINEAR_MIPMAP_LINEAR'. */
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/* We're setting textures to be repeated here. */
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //NEW!
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //NEW!

	/* Create the actual texture object. */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Image->w, Image->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, Image->pixels);

	/* Free the surface, we are finished with it. */
	SDL_FreeSurface(Image);

	return Object;
}

/*
 *	CompileLists
 *		Compiles the display lists used by our application.
 */
void CompileLists()
{
	/* Let's generate a display list for a box. */
	BoxList = glGenLists(1);
	glNewList(BoxList, GL_COMPILE);

	/*
	 * Render everything as you usually would, without texture binding. We're rendering the box from the
	 * '3D Objects' tutorial here.
	 */
	glBegin(GL_QUADS);
	/* Front */
	glTexCoord2d(0, 0); glVertex3d(400, 125, 0.4);
	glTexCoord2d(1, 0); glVertex3d(750, 125, 0.4);
	glTexCoord2d(1, 1); glVertex3d(750, 475, 0.4);
	glTexCoord2d(0, 1); glVertex3d(400, 475, 0.4);

	/* Left side */
	glTexCoord2d(0, 0); glVertex3d(400, 125, -0.4);
	glTexCoord2d(1, 0); glVertex3d(400, 125, 0.4);
	glTexCoord2d(1, 1); glVertex3d(400, 475, 0.4);
	glTexCoord2d(0, 1); glVertex3d(400, 475, -0.4);

	/* Back */
	glTexCoord2d(0, 0); glVertex3d(750, 125, -0.4);
	glTexCoord2d(1, 0); glVertex3d(400, 125, -0.4);
	glTexCoord2d(1, 1); glVertex3d(400, 475, -0.4);
	glTexCoord2d(0, 1); glVertex3d(750, 475, -0.4);

	/* Right side */
	glTexCoord2d(0, 0); glVertex3d(750, 125, 0.4);
	glTexCoord2d(1, 0); glVertex3d(750, 125, -0.4);
	glTexCoord2d(1, 1); glVertex3d(750, 475, -0.4);
	glTexCoord2d(0, 1); glVertex3d(750, 475, 0.4);

	/* Top */
	glTexCoord2d(0, 0); glVertex3d(400, 125, -0.4);
	glTexCoord2d(1, 0); glVertex3d(750, 125, -0.4);
	glTexCoord2d(1, 1); glVertex3d(750, 125, 0.4);
	glTexCoord2d(0, 1); glVertex3d(400, 125, 0.4);

	/* Bottom */
	glTexCoord2d(0, 0); glVertex3d(400, 475, -0.4);
	glTexCoord2d(1, 0); glVertex3d(750, 475, -0.4);
	glTexCoord2d(1, 1); glVertex3d(750, 475, 0.4);
	glTexCoord2d(0, 1); glVertex3d(400, 475, 0.4);
	glEnd();
	glEndList();
}

void DrawPlayert1Mallet()
{
	static float BallTexWidth(0.f);
	static float BallTexHeight(0.f);

	//Bottom disc
	glBindTexture(GL_TEXTURE_2D, Textures[7]);
	glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &BallTexWidth);
	glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &BallTexHeight);

	glBindTexture(GL_TEXTURE_2D, Textures[7]);

	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2d(0.5, 0.5);
	glVertex3d(0.0, halfLength, 0.0);
	for (int i = 0; i < slices; i++)
	{
		float theta = (float)i / slices* 2.0 * PI;
		float nextTheta = (float)(i + 1) / slices* 2.0* PI;
		glTexCoord2d(0.5 + 0.5* glm::cos(theta), 0.5 + 0.5* glm::sin(theta));
		glVertex3d(radius * glm::cos(theta), halfLength, radius * glm::sin(theta));
		glTexCoord2d(0.5 + 0.5* glm::cos(nextTheta), 0.5 + 0.5* glm::sin(nextTheta));
		glVertex3d(radius * glm::cos(nextTheta), halfLength, radius * glm::sin(nextTheta));

	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, Textures[7]);

	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2d(0.5, 0.5);
	glVertex3d(0.0, -halfLength, 0.0);
	for (int i = 0; i < slices; i++)
	{
		float theta = (float)i / slices* 2.0 * PI;
		float nextTheta = (float)(i + 1) / slices* 2.0* PI;
		glTexCoord2d(0.5 + 0.5* glm::cos(theta), 0.5 + 0.5* glm::sin(theta));
		glVertex3d(radius * glm::cos(theta), -halfLength, radius * glm::sin(theta));
		glTexCoord2d(0.5 + 0.5* glm::cos(nextTheta), 0.5 + 0.5* glm::sin(nextTheta));
		glVertex3d(radius * glm::cos(nextTheta), -halfLength, radius * glm::sin(nextTheta));

	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, Textures[7]);
	for (int i = 0; i < slices; i++)
	{
		float theta = (float)i * 2.0 * PI / slices;
		float nextTheta = (float)(i + 1)* 2.0* PI / slices;
		glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2d(0., 0.1);
		glVertex3d(radius*glm::cos(theta), halfLength, radius*glm::sin(theta));
		glTexCoord2d(0., 0.0);
		glVertex3d(radius*glm::cos(theta), -halfLength, radius*glm::sin(theta));
		glTexCoord2d(0.1, 0.01);
		glVertex3d(radius*glm::cos(nextTheta), halfLength, radius*glm::sin(nextTheta));
		glTexCoord2d(0.0, 0.0);
		glVertex3d(radius*glm::cos(nextTheta), -halfLength, radius*glm::sin(nextTheta));
		glEnd();
	}

	//glDisable(GL_COLOR_MATERIAL);
}

void DrawPlayert2Mallet()
{
	static float BallTexWidth(0.f);
	static float BallTexHeight(0.f);

	//Bottom disc
	glBindTexture(GL_TEXTURE_2D, Textures[8]);
	glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &BallTexWidth);
	glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &BallTexHeight);

	glBindTexture(GL_TEXTURE_2D, Textures[8]);

	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2d(0.5, 0.5);
	glVertex3d(0.0, halfLength, 0.0);
	for (int i = 0; i < slices; i++)
	{
		float theta = (float)i / slices* 2.0 * PI;
		float nextTheta = (float)(i + 1) / slices* 2.0* PI;
		glTexCoord2d(0.5 + 0.5* glm::cos(theta), 0.5 + 0.5* glm::sin(theta));
		glVertex3d(radius * glm::cos(theta), halfLength, radius * glm::sin(theta));
		glTexCoord2d(0.5 + 0.5* glm::cos(nextTheta), 0.5 + 0.5* glm::sin(nextTheta));
		glVertex3d(radius * glm::cos(nextTheta), halfLength, radius * glm::sin(nextTheta));

	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, Textures[8]);

	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2d(0.5, 0.5);
	glVertex3d(0.0, -halfLength, 0.0);
	for (int i = 0; i < slices; i++)
	{
		float theta = (float)i / slices* 2.0 * PI;
		float nextTheta = (float)(i + 1) / slices* 2.0* PI;
		glTexCoord2d(0.5 + 0.5* glm::cos(theta), 0.5 + 0.5* glm::sin(theta));
		glVertex3d(radius * glm::cos(theta), -halfLength, radius * glm::sin(theta));
		glTexCoord2d(0.5 + 0.5* glm::cos(nextTheta), 0.5 + 0.5* glm::sin(nextTheta));
		glVertex3d(radius * glm::cos(nextTheta), -halfLength, radius * glm::sin(nextTheta));

	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, Textures[8]);
	for (int i = 0; i < slices; i++)
	{
		float theta = (float)i * 2.0 * PI / slices;
		float nextTheta = (float)(i + 1)* 2.0* PI / slices;
		glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2d(0., 0.1);
		glVertex3d(radius*glm::cos(theta), halfLength, radius*glm::sin(theta));
		glTexCoord2d(0., 0.0);
		glVertex3d(radius*glm::cos(theta), -halfLength, radius*glm::sin(theta));
		glTexCoord2d(0.1, 0.01);
		glVertex3d(radius*glm::cos(nextTheta), halfLength, radius*glm::sin(nextTheta));
		glTexCoord2d(0.0, 0.0);
		glVertex3d(radius*glm::cos(nextTheta), -halfLength, radius*glm::sin(nextTheta));
		glEnd();
	}

	//glDisable(GL_COLOR_MATERIAL);
}

void DrawDisc()
{
	static float BallTexWidth(0.f);
	static float BallTexHeight(0.f);
		
	//Bottom disc
		glBindTexture(GL_TEXTURE_2D, Textures[6]);
		glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &BallTexWidth);
		glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &BallTexHeight);

		glBindTexture(GL_TEXTURE_2D, Textures[6]);
		
		glBegin(GL_TRIANGLE_FAN);
		glTexCoord2d(0.5, 0.5);
		glVertex3d(0.0, halfLength, 0.0);
		for (int i = 0; i < slices; i++)
		{
			float theta = (float)i /slices* 2.0 * PI;
			float nextTheta = (float)(i + 1) / slices* 2.0* PI;
			glTexCoord2d(0.5 + 0.5* glm::cos(theta), 0.5 + 0.5* glm::sin(theta));
			glVertex3d(radius * glm::cos(theta),halfLength, radius * glm::sin(theta));
			glTexCoord2d(0.5 + 0.5* glm::cos(nextTheta), 0.5 + 0.5* glm::sin(nextTheta));
			glVertex3d(radius * glm::cos(nextTheta), halfLength,radius * glm::sin(nextTheta));

		}
		glEnd();
		glBindTexture(GL_TEXTURE_2D, Textures[6]);

		glBegin(GL_TRIANGLE_FAN);
		glTexCoord2d(0.5, 0.5);
		glVertex3d(0.0, -halfLength, 0.0);
		for (int i = 0; i < slices; i++)
		{
			float theta = (float)i / slices* 2.0 * PI;
			float nextTheta = (float)(i + 1) / slices* 2.0* PI;
			glTexCoord2d(0.5 + 0.5* glm::cos(theta), 0.5 + 0.5* glm::sin(theta));
			glVertex3d(radius * glm::cos(theta), -halfLength, radius * glm::sin(theta));
			glTexCoord2d(0.5 + 0.5* glm::cos(nextTheta), 0.5 + 0.5* glm::sin(nextTheta));
			glVertex3d(radius * glm::cos(nextTheta), -halfLength, radius * glm::sin(nextTheta));

		}
		glEnd();
		glBindTexture(GL_TEXTURE_2D, Textures[5]);
		for (int i = 0; i < slices; i++)
		{
			float theta = (float)i * 2.0 * PI / slices;
			float nextTheta = (float)(i + 1)* 2.0* PI / slices;
			glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2d(0., 0.53);
			glVertex3d(radius*glm::cos(theta), halfLength, radius*glm::sin(theta));
			glTexCoord2d(0., 0.5);
			glVertex3d(radius*glm::cos(theta), -halfLength, radius*glm::sin(theta));
			glTexCoord2d(0.084, 0.53);
			glVertex3d(radius*glm::cos(nextTheta), halfLength, radius*glm::sin(nextTheta));
			glTexCoord2d(0.084, 0.5);
			glVertex3d(radius*glm::cos(nextTheta), -halfLength, radius*glm::sin(nextTheta));
			glEnd();
		}
		
		//glDisable(GL_COLOR_MATERIAL);
}

void DrawTable() 
{
	static float TableTexWidth(0.f);
	static float TableTexHeight(0.f);

	glBindTexture(GL_TEXTURE_2D, Textures[9]);
	glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &TableTexWidth);
	glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &TableTexHeight);

	glBindTexture(GL_TEXTURE_2D, Textures[9]);

	glBegin(GL_QUADS);
	glTexCoord2d(1.0, 1.0);
	glVertex3d(-tableX,tableTop,-tableZ);
	glTexCoord2d(0.0, 1.0);
	glVertex3d(tableX ,tableTop, -tableZ);
	glTexCoord2d(0.0, 0.0);
	glVertex3d(tableX, tableTop, tableHorMid);
	glTexCoord2d(1.0, 0.0);
	glVertex3d(-tableX, tableTop, tableHorMid);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2d(1.0, 1.0);
	glVertex3d(-tableX, tableTop, tableZ);
	glTexCoord2d(0.0, 1.0);
	glVertex3d(tableX, tableTop, tableZ);
	glTexCoord2d(0.0, 0.0);
	glVertex3d(tableX, tableTop, tableHorMid);
	glTexCoord2d(1.0, 0.0);
	glVertex3d(-tableX, tableTop, tableHorMid);
	glEnd();
}

/*
 * DrawRoom
 *	This will render the entire scene (in other words, draw the room).
 */
void DrawRoom()
{
	/* You also could do this at front by using the SDL surface's values or in an array. */
	static float WallTexWidth(0.f);
	static float WallTexHeight(0.f);

	static float FloorTexWidth(0.f);
	static float FloorTexHeight(0.f);

	static bool Once(false);

	/* Perform this check only once. */
	if (!Once)
	{
		/* Bind the wall texture. */
		glBindTexture(GL_TEXTURE_2D, Textures[0]);

		/* Retrieve the width and height of the current texture (can also be done up front with SDL and saved somewhere). */
		glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &WallTexWidth);
		glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &WallTexHeight);

		/* Bind the floor texture. */
		glBindTexture(GL_TEXTURE_2D, Textures[1]);

		/* Retrieve the width and height of the current texture (can also be done up front with SDL and saved somewhere). */
		glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &FloorTexWidth);
		glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &FloorTexHeight);

		Once = true;
	}

	glPushMatrix();

	/* Move the world and rotate the view. */
	glRotated(ViewAngleVer, 1, 0, 0);
	glRotated(ViewAngleHor, 0, 1, 0);

	glTranslated(-X, -Y, -Z);
	/*
	glMaterialfv(GL_FRONT, GL_AMBIENT, no_mat);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, no_mat);
	glMaterialfv(GL_FRONT, GL_SHININESS, no_shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);
	*/

	/* Set the coordinate system. */
	DrawTable();
	//Ball coordinate
	glPushMatrix();
	glTranslated(ballX, ballY, ballZ);
	DrawDisc();
	//glScaled(0.5, 0.2, 0.5);
	glPopMatrix();

	//Player 1 Mallet coordinate
	glPushMatrix();
	glTranslated(mallet1X, mallet1Y, mallet1Z);
	DrawPlayert1Mallet();
	//glScaled(0.5, 0.2, 0.5);
	glPopMatrix();

	//Player 2 Mallet coordinate
	glPushMatrix();
	glTranslated(mallet2X, mallet2Y, mallet2Z);
	DrawPlayert2Mallet();
	//glScaled(0.5, 0.2, 0.5);
	glPopMatrix();

	/* Draw walls. */
	glOrtho(0, 800, 600, 0, -1, 1);
	glBindTexture(GL_TEXTURE_2D, Textures[0]);

	glBegin(GL_QUADS);
	/* Wall in front of you when the app starts. */
	glTexCoord2f(0, 0);
	glVertex3d(-200, 0, 4.0);

	glTexCoord2f(1200.f / WallTexWidth, 0);
	glVertex3d(1000, 0, 4.0);

	glTexCoord2f(1200.f / WallTexWidth, 400.f / WallTexHeight);
	glVertex3d(1000, 500, 4.0);

	glTexCoord2f(0, 400.f / WallTexHeight);
	glVertex3d(-200, 500, 4.0);

	/* Wall left of you. */
	glTexCoord2f(0, 0);
	glVertex3d(-200, 0, -4.0);

	glTexCoord2f(1200.f / WallTexWidth, 0);
	glVertex3d(-200, 0, 4.0);

	glTexCoord2f(1200.f / WallTexWidth, 400.f / WallTexHeight);
	glVertex3d(-200, 500, 4.0);

	glTexCoord2f(0, 400.f / WallTexHeight);
	glVertex3d(-200, 500, -4.0);

	/* Wall right of you. */
	glTexCoord2f(0, 0);
	glVertex3d(1000, 0, 4.0);

	glTexCoord2f(1200.f / WallTexWidth, 0);
	glVertex3d(1000, 0, -4.0);

	glTexCoord2f(1200.f / WallTexWidth, 400.f / WallTexHeight);
	glVertex3d(1000, 500, -4.0);

	glTexCoord2f(0, 400.f / WallTexHeight);
	glVertex3d(1000, 500, 4.0);

	/* Wall behind you (you won't be able to see this just yet, but you will later). */
	glTexCoord2f(0, 0);
	glVertex3d(1000, 0, -4.0);

	glTexCoord2f(1200.f / WallTexWidth, 0);
	glVertex3d(-200, 0, -4.0);

	glTexCoord2f(1200.f / WallTexWidth, 400.f / WallTexHeight);
	glVertex3d(-200, 500, -4.0);

	glTexCoord2f(0, 400.f / WallTexHeight);
	glVertex3d(1000, 500, -4.0);
	glEnd();

	/* Draw the floor and the ceiling, this is done separatly because glBindTexture isn't allowed inside glBegin. */
	glBindTexture(GL_TEXTURE_2D, Textures[1]);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex3d(-200, 500, 4.0);

	glTexCoord2f(1200.f / FloorTexWidth, 0);
	glVertex3d(1000, 500, 4.0);

	glTexCoord2f(1200.f / FloorTexWidth, (8.f / 2.f * 600.f) / FloorTexHeight);
	glVertex3d(1000, 500, -4.0);

	glTexCoord2f(0, (8.f / 2.f * 600.f) / FloorTexHeight);
	glVertex3d(-200, 500, -4.0);

	/* Ceiling. */
	glTexCoord2f(0, 0);
	glVertex3d(-200, 0, 4.0);

	glTexCoord2f(1200.f / FloorTexWidth, 0);
	glVertex3d(1000, 0, 4.0);

	glTexCoord2f(1200.f / FloorTexWidth, (8.f / 2.f * 600.f) / FloorTexHeight);
	glVertex3d(1000, 0, -4.0);

	glTexCoord2f(0, (8.f / 2.f * 600.f) / FloorTexHeight);
	glVertex3d(-200, 0, -4.0);
	glEnd();

	/* Now we're going to render some boxes using display lists. */
	glPushMatrix();
	/* Let's make it a bit smaller... */
	glScaled(0.5, 0.4, 0.5);

	/* Can't bind textures while generating a display list, but we can give it texture coordinates and bind it now. */
	glBindTexture(GL_TEXTURE_2D, Textures[2]);

	/*
	 * Because display lists have preset coordinates, we'll need to translate it to move it around. Note that we're
	 * moving the small version of the cube around, not the big version (because we scaled *before* translating).
	 */
	glTranslated(-700, 750, 6);

	/*
	 * Let's draw a whole lot of boxes. Note that because we're not pushing and popping matrices, translations
	 * and changes will 'accumulate' and add to the previous translation.
	 */
	for (short i(0); i < 12; ++i)
	{
		glTranslated(350, 0, 0);

		/* These make sure that every once in a while, a new row is started. */
		if (i == 5)		glTranslated(-1575, -350, 0);
		if (i == 9)		glTranslated(-1225, -350, 0);

		/*
		 * glCallList is all that is really needed to execute the display list. Remember to try the 'K' button
		 * to turn on wireframe mode, with these extra polygons, it looks pretty neat!
		 */
		glCallList(BoxList);
	}

	glPopMatrix();
	
	glPopMatrix();
}


void changeFullScreen()
{
	fullScreen = !fullScreen;
	if (fullScreen)
	{
		SDL_SetVideoMode(screenwidth[screenMode], screenheight[screenMode], 32, SDL_OPENGL | SDL_FULLSCREEN);
		glViewport(0, 0, screenwidth[screenMode], screenheight[screenMode]);
		CompileLists();
		//Rebuild objects after SDL mode change to rebuild scene
		glShadeModel(GL_SMOOTH);
		glClearColor(0, 0, 0, 1);

		glViewport(0, 0, screenwidth[screenMode], screenheight[screenMode]);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		gluPerspective(80.0, 800.0 / 600.0, 0.1, 100.0);

		/* We now switch to the modelview matrix. */
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glEnable(GL_DEPTH_TEST);

		glDepthFunc(GL_LEQUAL);

		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

		/* Enable 2D texturing. */
		glEnable(GL_TEXTURE_2D);

		/* Set up alpha blending. */
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		//glEnable(GL_LIGHTING);
		//glEnable(GL_LIGHT0);
		
		
		glColor4d(1, 1, 1, 1);
		
		Textures[0] = GrabTexObjFromFile("Data/Wall.png");
		Textures[1] = GrabTexObjFromFile("Data/Floor.png");
		Textures[2] = GrabTexObjFromFile("Data/Box.png");
		Textures[3] = GrabTexObjFromFile("Data/MalletTexture.png");
		Textures[4] = GrabTexObjFromFile("Data/MalletTexturebake.png");
		Textures[5] = GrabTexObjFromFile("Data/BallTex.png");
		Textures[6] = GrabTexObjFromFile("Data/BallTexTopBot.png");
		Textures[7] = GrabTexObjFromFile("Data/Player1MalletTopBot.png");
		Textures[8] = GrabTexObjFromFile("Data/Player2MalletTopBot.png");
		Textures[9] = GrabTexObjFromFile("Data/tabletop.png");
	}
	else
	{
		SDL_SetVideoMode(screenwidth[screenMode], screenheight[screenMode], 32, SDL_OPENGL);
		glViewport(0, 0, screenwidth[screenMode], screenheight[screenMode]);
		CompileLists();
		//Rebuild objects after SDL mode change to rebuild scene
		glShadeModel(GL_SMOOTH);
		glClearColor(0, 0, 0, 1);

		glViewport(0, 0, screenwidth[screenMode], screenheight[screenMode]);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		gluPerspective(80.0, 800.0 / 600.0, 0.1, 100.0);

		/* We now switch to the modelview matrix. */
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glEnable(GL_DEPTH_TEST);

		glDepthFunc(GL_LEQUAL);

		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

		/* Enable 2D texturing. */
		glEnable(GL_TEXTURE_2D);

		/* Set up alpha blending. */
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		/*
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		*/

		glColor4d(1, 1, 1, 1);
		
		Textures[0] = GrabTexObjFromFile("Data/Wall.png");
		Textures[1] = GrabTexObjFromFile("Data/Floor.png");
		Textures[2] = GrabTexObjFromFile("Data/Box.png");
		Textures[3] = GrabTexObjFromFile("Data/MalletTexture.png");
		Textures[4] = GrabTexObjFromFile("Data/MalletTexturebake.png");
		Textures[5] = GrabTexObjFromFile("Data/BallTex.png");
		Textures[6] = GrabTexObjFromFile("Data/BallTexTopBot.png");
		Textures[7] = GrabTexObjFromFile("Data/Player1MalletTopBot.png");
		Textures[8] = GrabTexObjFromFile("Data/Player2MalletTopBot.png");
		Textures[9] = GrabTexObjFromFile("Data/tabletop.png");
		
	}
}

void changeResolution()
{
	screenMode++;
	if (screenMode < 3) {
		if (fullScreen)
		{	
			SDL_SetVideoMode(screenwidth[screenMode], screenheight[screenMode], 32, SDL_OPENGL | SDL_FULLSCREEN);
			glViewport(0, 0, screenwidth[screenMode], screenheight[screenMode]);
			CompileLists();
			//Rebuild objects after SDL mode change to rebuild scene
			glShadeModel(GL_SMOOTH);
			glClearColor(0, 0, 0, 1);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();

			gluPerspective(80.0, 800.0 / 600.0, 0.1, 100.0);

			/* We now switch to the modelview matrix. */
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			glEnable(GL_DEPTH_TEST);

			glDepthFunc(GL_LEQUAL);

			glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

			/* Enable 2D texturing. */
			glEnable(GL_TEXTURE_2D);

			/* Set up alpha blending. */
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			
			//glEnable(GL_LIGHTING);
			//glEnable(GL_LIGHT0);

			
			glColor4d(1, 1, 1, 1);
			
			Textures[0] = GrabTexObjFromFile("Data/Wall.png");
			Textures[1] = GrabTexObjFromFile("Data/Floor.png");
			Textures[2] = GrabTexObjFromFile("Data/Box.png");
			Textures[3] = GrabTexObjFromFile("Data/MalletTexture.png");
			Textures[4] = GrabTexObjFromFile("Data/MalletTexturebake.png");
			Textures[5] = GrabTexObjFromFile("Data/BallTex.png");
			Textures[6] = GrabTexObjFromFile("Data/BallTexTopBot.png");
			Textures[7] = GrabTexObjFromFile("Data/Player1MalletTopBot.png");
			Textures[8] = GrabTexObjFromFile("Data/Player2MalletTopBot.png");
			Textures[9] = GrabTexObjFromFile("Data/tabletop.png");
		}
		else
		{
			
			SDL_SetVideoMode(screenwidth[screenMode], screenheight[screenMode], 32, SDL_OPENGL);
			glViewport(0, 0, screenwidth[screenMode], screenheight[screenMode]);
		}
	}

	else
	{
		screenMode = 0;
		if (fullScreen)
		{
			SDL_SetVideoMode(screenwidth[screenMode], screenheight[screenMode], 32, SDL_OPENGL | SDL_FULLSCREEN);
			glViewport(0, 0, screenwidth[screenMode], screenheight[screenMode]);

			CompileLists();
		//	glEnable(GL_LIGHTING);
		//	glEnable(GL_LIGHT0);
	
			//Rebuild objects after SDL mode change to rebuild scene
			glShadeModel(GL_SMOOTH);
			glClearColor(0, 0, 0, 1);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();

			gluPerspective(80.0, 800.0 / 600.0, 0.1, 100.0);

			/* We now switch to the modelview matrix. */
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			glEnable(GL_DEPTH_TEST);

			glDepthFunc(GL_LEQUAL);

			glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

			/* Enable 2D texturing. */
			glEnable(GL_TEXTURE_2D);

			/* Set up alpha blending. */
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glColor4d(1, 1, 1, 1);
			
			Textures[0] = GrabTexObjFromFile("Data/Wall.png");
			Textures[1] = GrabTexObjFromFile("Data/Floor.png");
			Textures[2] = GrabTexObjFromFile("Data/Box.png");
			Textures[3] = GrabTexObjFromFile("Data/MalletTexture.png");
			Textures[4] = GrabTexObjFromFile("Data/MalletTexturebake.png");
			Textures[5] = GrabTexObjFromFile("Data/BallTex.png");
			Textures[6] = GrabTexObjFromFile("Data/BallTexTopBot.png");
			Textures[7] = GrabTexObjFromFile("Data/Player1MalletTopBot.png");
			Textures[8] = GrabTexObjFromFile("Data/Player2MalletTopBot.png");
			Textures[9] = GrabTexObjFromFile("Data/tabletop.png");

		}
		else
		{
			SDL_SetVideoMode(screenwidth[screenMode], screenheight[screenMode], 32, SDL_OPENGL);
			glViewport(0, 0, screenwidth[screenMode], screenheight[screenMode]);
			DrawRoom();
		}
	}
}


void ChangeLightParam()
{
	if(lightMenuActive)
	{	
		lightColor[1] = 0;
		lightColor[2] = 0;
	}
}
int main(int argc, char **argv)
{// Objloader from http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/

	
	/* Initialize SDL and set up a window. */
	SDL_Init(SDL_INIT_VIDEO);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_WM_SetCaption("OpenGL - Display Lists", 0);
	SDL_WM_GrabInput(SDL_GRAB_ON);

	SDL_ShowCursor(SDL_DISABLE);

	SDL_SetVideoMode(screenwidth[screenMode], screenheight[screenMode], 32, SDL_OPENGL);

	CompileLists();

	/* Basic OpenGL initialization, handled in 'The Screen'. */
	glShadeModel(GL_SMOOTH);
	glClearColor(0, 0, 0, 1);

	glViewport(0, 0, screenwidth[screenMode], screenheight[screenMode]);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(80.0, 800.0 / 600.0, 0.1, 100.0);

	/* We now switch to the modelview matrix. */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	//glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);
	
	glLightfv(GL_LIGHT0, GL_AMBIENT, GL_LIGHT0_AMBIENT);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, GL_LIGHT0_DIFFUSE);
	glLightfv(GL_LIGHT0, GL_SPECULAR, GL_LIGHT0_SPECULAR);
	glLightfv(GL_LIGHT0, GL_POSITION, GL_LIGHT0_POSITION);
	

	glEnable(GL_DEPTH_TEST);

	glDepthFunc(GL_LEQUAL);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	/* Enable 2D texturing. */
	glEnable(GL_TEXTURE_2D);

	/* Set up alpha blending. */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4d(1, 1, 1, 1);

	Textures[0] = GrabTexObjFromFile("Data/Wall.png");
	Textures[1] = GrabTexObjFromFile("Data/Floor.png");
	Textures[2] = GrabTexObjFromFile("Data/Box.png");
	Textures[3] = GrabTexObjFromFile("Data/MalletTexture.png");
	Textures[4] = GrabTexObjFromFile("Data/MalletTexturebake.png");
	Textures[5] = GrabTexObjFromFile("Data/BallTex.png");
	Textures[6] = GrabTexObjFromFile("Data/BallTexTopBot.png");
	Textures[7] = GrabTexObjFromFile("Data/Player1MalletTopBot.png");
	Textures[8] = GrabTexObjFromFile("Data/Player2MalletTopBot.png");
	Textures[9] = GrabTexObjFromFile("Data/tabletop.png");
	//Added!


	//Replaced this with a loop that immediately checks the entire array.
	//sizeof(Textures) is the size of the entire array in bytes (unsigned int = 4 bytes)
	//so sizeof(Textures) would give 3 * 4 = 12 bytes, divide this by 4 bytes and you
	//have 3.
	for (unsigned i(0); i < sizeof(Textures) / sizeof(unsigned); ++i)
	{
		if (Textures[i] == 0)
		{
#ifdef _WIN32
			MessageBoxA(0, "Something went seriously wrong!", "Fatal Error!", MB_OK | MB_ICONERROR);
#endif //_WIN32

			return 1;
		}
	}

	/* Compile the display lists. */
	CompileLists();

	SDL_Event event;

	int RelX(0), RelY(0);
	int MovementDelay(SDL_GetTicks());
	DiscObj = gluNewQuadric();
	bool Wireframe(false);
	bool Keys[8] =
	{
		false, /* Up arrow down? */
		false, /* Down arrow down? */
		false, /* Left arrow down? */
		false, /* Right arrow down? */
		false, /* w down? */
		false, /* a down? */
		false, /* s Left arrow down? */
		false  /* d  Right arrow down? */
	};
	Uint32 dwStartTime = SDL_GetTicks();
	Uint32 dwEndTime = 0;
	/* Application loop. */
	for (;;)
	{
		dwEndTime = SDL_GetTicks();


		/* Handle events with SDL. */
			if(SDL_PollEvent(&event))
			{	
				if (event.type == SDL_QUIT)
					break;

				/* Mouse events? */
				else if (event.type == SDL_MOUSEMOTION)
				{
					/* Get the relative mouse movement of the mouse (based on CurMouseCoord - PrevMouseCoord). */
					SDL_GetRelativeMouseState(&RelX, &RelY);

					ViewAngleHor += RelX / 4;
					ViewAngleVer += RelY / 4;

					/* Prevent the horizontal angle from going over 360 degrees or below 0 degrees. */
					if (ViewAngleHor >= 360.0)		ViewAngleHor = 0.0;
					else if (ViewAngleHor < 0.0)		ViewAngleHor = 360.0;

					/* Prevent the vertical view from moving too far (comment this out to get a funny effect). */
					if (ViewAngleVer > 60.0)			ViewAngleVer = 60.0; /* 60 degrees is when you're looking down. */
					else if (ViewAngleVer < -60.0)	ViewAngleVer = -60.0; /* This is when you're looking up. */

					/* This delay might seem strange, but it helps smoothing out the mouse if you're experiencing jittering. */
					SDL_Delay(5);
				}

				else if (event.type == SDL_KEYDOWN)
				{
					if (event.key.keysym.sym == SDLK_ESCAPE)
						break;

					if (event.key.keysym.sym == SDLK_k)
						glPolygonMode(GL_FRONT_AND_BACK, ((Wireframe = !Wireframe) ? GL_LINE : GL_FILL));

					if (event.key.keysym.sym == SDLK_UP)		Keys[0] = true;
					if (event.key.keysym.sym == SDLK_DOWN)		Keys[1] = true;
					if (event.key.keysym.sym == SDLK_LEFT)		Keys[2] = true;
					if (event.key.keysym.sym == SDLK_RIGHT)		Keys[3] = true;
					if (event.key.keysym.sym == SDLK_w)			Keys[4] = true;
					if (event.key.keysym.sym == SDLK_a)			Keys[5] = true;
					if (event.key.keysym.sym == SDLK_s)			Keys[6] = true;
					if (event.key.keysym.sym == SDLK_d)			Keys[7] = true;
					if (event.key.keysym.sym == SDLK_r) //Reset key
					{ 
						ballX = 0.0f;
						ballZ = 0.0f;
						ballVX = 0.0f;
						ballVZ = 0.0f;
					}
	//				if (event.key.keysym.sym == SDLK_F1)		ChangeLightParam();
					if (event.key.keysym.sym == SDLK_F10)		changeResolution();
					if (event.key.keysym.sym == SDLK_F11)		changeFullScreen();
				}

				else if (event.type == SDL_KEYUP)
				{
					if (event.key.keysym.sym == SDLK_UP)		Keys[0] = false;
					if (event.key.keysym.sym == SDLK_DOWN)		Keys[1] = false;
					if (event.key.keysym.sym == SDLK_LEFT)		Keys[2] = false;
					if (event.key.keysym.sym == SDLK_RIGHT)		Keys[3] = false;
					if (event.key.keysym.sym == SDLK_w)			Keys[4] = false;
					if (event.key.keysym.sym == SDLK_a)			Keys[5] = false;
					if (event.key.keysym.sym == SDLK_s)			Keys[6] = false;
					if (event.key.keysym.sym == SDLK_d)			Keys[7] = false;
				}
			}

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			

			glPushMatrix();
			glColor4f(1.f, 1.f, 1.f, 1.f);
			DrawRoom();
			glPopMatrix();

			/* Move if the keys are pressed, this is explained in the tutorial. */
			if (Keys[0]) //up
			{
				X -= cos(DegreeToRadian(ViewAngleHor + 90.0)) * 0.05;
				Z -= sin(DegreeToRadian(ViewAngleHor + 90.0)) * 0.05;
			}

			if (Keys[1]) //down
			{
				X += cos(DegreeToRadian(ViewAngleHor + 90.0)) * 0.05;
				Z += sin(DegreeToRadian(ViewAngleHor + 90.0)) * 0.05;
			}

			if (Keys[2]) //left
			{
				X += cos(DegreeToRadian(ViewAngleHor + 180.0)) * 0.05;
				Z += sin(DegreeToRadian(ViewAngleHor + 180.0)) * 0.05;
			}

			if (Keys[3]) //right
			{
				X -= cos(DegreeToRadian(ViewAngleHor + 180.0)) * 0.05;
				Z -= sin(DegreeToRadian(ViewAngleHor + 180.0)) * 0.05;
			}
			//Move Mallet 1	
			if (Keys[4]) //w
			{	
				if(mallet1Z > -tableZ)
				mallet1Z -= malletSpeed;
			}

			if (Keys[5]) //a
			{
				if (mallet1X > -tableX)
				mallet1X -= malletSpeed;
			}

			if (Keys[6]) //s
			{	
				if(mallet1Z < tableZ)
				mallet1Z += malletSpeed;
			}

			if (Keys[7]) //d
			{
				if(mallet1X < tableX)
				mallet1X += malletSpeed;
			}

			//line 1156 - 1162 got looped, dwEndTime and dwStartTime did get change, but not currentPeriod, period, and mallet2X

			updateBall();
			if ((dwEndTime - dwStartTime) % 16 == 0) 
			{
				currentPeriod += period;
			}

			mallet2X = 0 + sin(currentPeriod) * MaxMovement;

			//Make ball stuck at goal when enter
			if(ballInHole(ballX,ballZ) == 1 || ballInHole(ballX, ballZ) == 2)
			{
				ballVX = 0.0f;
				ballVZ = 0.0f;
			}

			/* Swap the display buffers. */
			SDL_GL_SwapBuffers();
	}
	/* Delete the created textures. */
	glDeleteTextures(6, Textures);		//Changed to 3.
	glDeleteLists(BoxList, 1);

	/* Clean up. */
	SDL_Quit();

	return 0;
}