#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <vector>
#include <ctime>
#include <glut.h>
#include <cmath>
#include <SDL.h>
#include <SDL_mixer.h>

// =================================  CONFIGURATIONS ================================= //
int WIDTH = glutGet(GLUT_SCREEN_WIDTH);
int HEIGHT = glutGet(GLUT_SCREEN_HEIGHT);

char title[] = "Minion Mania";

// 3D Projection Options
GLdouble fovy = 45.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 500;

// Background Textures
GLuint daytex;
GLuint nighttex;

// Model Variables
Model_3DS model_minion;
Model_3DS model_finishLine;
Model_3DS model_bridge;
Model_3DS model_banana;
Model_3DS model_sandbags;
Model_3DS model_barrier;
Model_3DS model_tree;
Model_3DS model_portal;
Model_3DS model_coin;
Model_3DS model_logs;
Model_3DS model_lamp;

// Textures
GLTexture tex_ground;

// Sounds
Mix_Music* background1Sound;
Mix_Music* background2Sound;
Mix_Chunk* coinSound;
Mix_Chunk* bananaSound;
Mix_Chunk* logSound;
Mix_Chunk* barrierSound;
Mix_Chunk* winSound;
Mix_Chunk* loseSound;

GLfloat sunLightPosition[] = { 0.0f, 10.0f, 0.0f, 1.0f }; // Sun position
GLfloat sunAmbient[] = { 0.1f, 0.1f, 0.1f, 1.0f };		// Ambient light
GLfloat sunDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };		// Diffuse light
GLfloat sunSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };		// Specular light

GLfloat flashlightPosition[] = { 0.0f, 2.0f, 0.0f, 1.0f }; // Initial position of the flashlight
GLfloat flashlightAmbient[] = { 0.1f, 0.1f, 0.1f, 1.0f };	 // Ambient light for flashlight
GLfloat flashlightDiffuse[] = { 1.0f, 1.0f, 0.8f, 1.0f };	 // Diffuse light for flashlight
GLfloat flashlightSpecular[] = { 1.0f, 1.0f, 0.8f, 1.0f };

// Assets Loading Function
void LoadAssets()
{
	// Loading Model files
	model_minion.Load("Models/minion/minion.3ds");
	model_finishLine.Load("Models/gate/gate.3ds");
	model_bridge.Load("Models/bridge/bridge.3ds");
	model_banana.Load("Models/banana/banana.3ds");
	model_sandbags.Load("Models/sandbags/sandbags.3ds");
	model_barrier.Load("Models/barriers/barrier.3ds");
	model_tree.Load("Models/tree/Tree1.3ds");
	model_portal.Load("Models/portal/portalTrial1.3ds");
	model_coin.Load("Models/coin/coin.3ds");
	model_logs.Load("Models/logs/logs.3ds");
	model_lamp.Load("Models/lamp/lamp.3ds");

	// Loading texture files
	tex_ground.Load("Textures/ground.bmp");
	loadBMP(&daytex, "Textures/blu-sky-3.bmp", true);
	loadBMP(&nighttex, "Textures/night-sky.bmp", true);
}

// Lighting Configuration Function
void InitLightSource()
{
	// Enable Lighting for this OpenGL Program
	glEnable(GL_LIGHTING);

	// Enable Light Source number 0
	// OpengL has 8 light sources
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1); // Enable the minion's light
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
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

// Material Configuration Function
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

void InitSound() {
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		printf_s("SDL could not initialize! SDL_Error: %s", Mix_GetError());
		exit(-1);
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		printf_s("SDL_mixer could not initialize! SDL_mixer Error: %s", Mix_GetError());
		exit(-1);
	}

	// Load all sounds
	coinSound = Mix_LoadWAV("sounds/coin.wav");
	background1Sound = Mix_LoadMUS("sounds/background1.mp3");
	bananaSound = Mix_LoadWAV("sounds/minion_yay.mp3");

	if (coinSound == nullptr || background1Sound == nullptr || bananaSound == nullptr) {
		printf_s("Failed to load sound effect! SDL_mixer Error: %s",Mix_GetError());
		exit(-1);
	}

}

void CleanUp() {
	Mix_FreeChunk(coinSound);
	Mix_CloseAudio();
	SDL_Quit();
}

// =================================  CAMERA CONFIG  ================================= //
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
	void operator+=(float value)
	{
		x += value;
		y += value;
		z += value;
	}
};

Vector Eye(2.4, 8, 66);
Vector At(0, 8, 0);
Vector Up(0, 1, 0);

int cameraZoom = 0;

// =================================  GAME VARIABLES  ================================= //
float SPEED = 0.0015f;
float SPEED2 = 0.005f;

// Store the start time
std::clock_t startTime = std::clock();
float elapsedTime = 0.0f;

// Minion Variables
bool isThirdPerson = true;
float minionPositionX = 2.4f;
float minionPositionZ = 60.0f;
float minionPositionY = 10.2f;

//level 2
float minionPositionX2 = 0.0f;
float minionPositionZ2 = 77.0f;
float minionPositionY2 = 1.3;

// Bridge Motion Variables
float minionStartY = 10.2f;
float minionEndY = 11.5f;

bool isJumping = false;
float jumpVelocity = 0.0f;
const float gravity = -0.00025f;
const float gravity2 = -0.009f;
const float desiredJumpHeight = 2.2f;
const float desiredJumpHeight2 = 1.8f;
float jumpOffset = 0.0f;
float jumpForce = sqrt(2 * -gravity * desiredJumpHeight);
float jumpForce2 = sqrt(2 * -gravity2 * desiredJumpHeight2);

bool isRebounding = false;
float reboundVelocity = 0.0f;
float reboundDistance = 0.0f;
const float reboundInitialVelocity = 0.4f;
const float reboundDeceleration = -0.008f;

float dayNightTransition = 1.0f; // 1.0 = full day, 0.0 = full night
float initialZ = 60.0f;			 // Starting Z position
float transitionEndZ = -80.0f;	 // End Z position (portal position)
float transitionDuration = 30.0f;
float glitchUpdateInterval = 0.05f; // How often the glitch position updates
float lastGlitchUpdate = 0.0f;
float glitchIntensity = 0.3f; // Maximum distance of glitch displacement

// Banana Variables
float bananaPositionZ = 0.0f;

// Bridge Variables
float bridgePositionZ = -10.0f;

// Timer Variables
float remainingTime = 30.0f;
float start = remainingTime;

// for the obstacle collision in level 1 - glitch effect
bool isGlitching = false;
float glitchDuration = 1.0f;
float glitchStartTime = 0.0f;
float glitchdeceleration = SPEED * 1.5;

int score = 0;
bool firstLevel = true;
bool gameLoseLevelOne = false;
bool gameLose = false;
bool gameWin = false;
bool doneReset = false;
// =================================  STRUCTS LOGIC  ================================= //
struct Banana
{
	float x, y, z;
};
std::vector<Banana> bananas;
float yOffsetBanana;
struct Coin
{
	float x, y, z;
};
std::vector<Coin> coins;

struct Obstacle
{
	float x, y, z;
};
struct Log
{
	float x, y, z;
};
std::vector<Log> logs;
std::vector<Obstacle> obstacles;
std::vector<Obstacle> sandbags;

struct Tree
{
	float x, y, z;
};
std::vector<Tree> trees;

struct Portal
{
	float x, y, z;
};

Portal portal = { 3.0f, 8.0f, -80.0f };

float xPositions[] = { 0.3f, 1.3f, 2.4f, 3.6f, 4.6f, 5.8f };
int xCount = 6;
int LaneIndex = 2;
float xPositions2[] = { -3.67f, 0, 3.67 };
int xCount2 = 3;
int LaneIndex2 = 1;

void SpawnCoins(int count)
{
	coins.clear();
	float y = 10.8f;
	float zStart = 50.0f;

	for (int i = 0; i < count; ++i)
	{
		float x = xPositions[rand() % xCount];
		float z = zStart - i * 10.0f;
		coins.push_back({ x, y, z });
	}
}
void SpawnBananas(int count)
{
	bananas.clear();
	float y = 1.0f;
	float zStart = 73.0f;

	for (int i = 0; i < count; ++i)
	{
		float x = xPositions2[rand() % xCount2];
		float z = zStart - i * 10.0f;
		bananas.push_back({ x, y, z });
	}
}

void SpawnObstacles(int count)
{
	obstacles.clear();
	float y[] = { 10.0f, 10.5f, 10.8f, 10.8f, 10.7f };
	float zStart = 45.0f;

	for (int i = 0; i < count; ++i)
	{
		float x = xPositions[rand() % xCount];
		float z = zStart - i * 20.0f;
		obstacles.push_back({ x, y[i], z });
	}
}
void SpawnSandbags(int count)
{
	sandbags.clear();
	float y = 0.2f;
	float zStart = 68.0f;

	for (int i = 0; i < count; ++i)
	{
		float x = xPositions2[rand() % xCount2];
		float x2 = xPositions2[rand() % xCount2];
		float z = zStart - i * 10.0f;
		if (x != x2)
		{
			sandbags.push_back({ x2, y, z });
		}
		sandbags.push_back({ x, y, z });
	}
}

void SpawnLogs(int count)
{
	logs.clear();
	float y = 10.2f;
	float zStart = 65.0f;

	logs.push_back({ 4.6, 10.7, -5 });
	logs.push_back({ 2.4, 10.2, 35 });
}
void InitializeForest()
{
	trees.clear();
	float y = 0.0f;
	float roadWidth = 20.0f;
	float treeSpacing = 10.0f;
	float zStart = -50.0f;
	int numTreesPerSide = 15;
	int numRandomTrees = 4;

	// Seed the random number generator
	srand(static_cast<unsigned>(time(0)));

	// Place trees along the road
	for (int i = 0; i < numTreesPerSide; ++i)
	{
		float z = zStart + i * treeSpacing;
		trees.push_back({ -roadWidth / 2 - 1, y, z }); // Left side of the road
		trees.push_back({ roadWidth / 2 + 1, y, z });	 // Right side of the road
	}

	// Place additional random trees around the road
	for (int i = 0; i < numRandomTrees; ++i)
	{
		float x;
		if (rand() % 2 == 0)
		{
			// Place tree before the road buffer
			x = static_cast<float>(rand() % static_cast<int>(roadWidth * 2)) - roadWidth * 3;
		}
		else
		{
			// Place tree after the road buffer
			x = static_cast<float>(rand() % static_cast<int>(roadWidth * 2)) + roadWidth;
		}

		float z = static_cast<float>(rand() % 200) - 100; // Random z position within a range
		trees.push_back({ x, y, z });
	}
}
void UpdateGlitchEffect()
{
	if (isGlitching)
	{
		float currentTime = remainingTime;
		if (currentTime - lastGlitchUpdate >= glitchUpdateInterval)
		{
			// Generate random offsets
			lastGlitchUpdate = currentTime;
		}
	}
}
// =================================  DEBUGGING  ================================= //

// =================================  RENDERS  ================================= //
void RenderGround()
{
	glDisable(GL_LIGHTING); // Disable lighting

	glColor3f(0.6, 0.6, 0.6); // Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D); // Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]); // Bind the ground texture

	if (!firstLevel)
	{
		glPushMatrix();
		glTranslatef(0.0f, 0.0f, bridgePositionZ);
		glScalef(4.0f, 4.0f, 4.0f);
		glRotatef(90, 0, 1, 0);
		glBegin(GL_QUADS);
		glNormal3f(0, 1, 0);	 // Set quad normal direction.
		glTexCoord2f(0, 0);		 // Set tex coordinates ( Using (0,0) -> (10,10) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
		glVertex3f(-20, 0, -40); // Increase the range to cover more of the xz plane
		glTexCoord2f(10, 0);
		glVertex3f(20, 0, -40);
		glTexCoord2f(10, 10);
		glVertex3f(40, 0, 40);
		glTexCoord2f(0, 10);
		glVertex3f(-40, 0, 40);
		glEnd();
		glPopMatrix();
	}

	glEnable(GL_LIGHTING); // Enable lighting again for other entities coming through the pipeline.
	glColor3f(1, 1, 1);	   // Set material back to white instead of grey used for the ground texture.
}

void RenderTrees()
{
	for (const auto& tree : trees)
	{
		glPushMatrix();
		glTranslatef(tree.x, tree.y, tree.z);
		glScalef(0.7, 0.7, 0.7);
		model_tree.Draw();
		glPopMatrix();
	}
}

float CalculateMinionHeight()
{
	float static currEyeY = 0;

	// Define z-ranges for different phases
	float ascentStartZ = 60.0f;
	double ascentEndZ = -3.0f;
	double waitStartZ = ascentEndZ;
	float waitEndZ = -45.0f;
	float descentStartZ = waitEndZ;
	float descentEndZ = -73.0f;

	if (minionPositionZ > ascentStartZ)
	{
		return minionStartY; // No change in height before the initial delay
	}

	if (minionPositionZ <= ascentStartZ && minionPositionZ > ascentEndZ)
	{
		// Ascent phase
		float t = (ascentStartZ - minionPositionZ) / (ascentStartZ - ascentEndZ);
		currEyeY = 12 + t * (minionEndY - minionStartY);
		if (!isThirdPerson)
		{
			Eye.y = currEyeY - 1.2;
		}
		else
		{
			Eye.y = currEyeY;
		}
		return minionStartY + t * (minionEndY - minionStartY);
	}

	if (minionPositionZ <= waitStartZ && minionPositionZ > waitEndZ)
	{
		// Wait phase
		if (!isThirdPerson)
		{
			Eye.y = currEyeY - 1.2;
		}
		else
		{
			Eye.y = currEyeY;
		}
		return minionEndY;
	}

	if (minionPositionZ <= descentStartZ && minionPositionZ > descentEndZ)
	{
		// Descent phase
		float t = (descentStartZ - minionPositionZ) / (descentStartZ - descentEndZ);
		if (!isThirdPerson)
		{
			Eye.y = (currEyeY - t * (minionEndY - minionStartY)) - 1.2;
		}
		else
		{
			Eye.y = (currEyeY - t * (minionEndY - minionStartY));
		}
		return minionEndY - t * (minionEndY - minionStartY);
	}

	return minionStartY;
}

void CalculateMinionPosition()
{
	// Update the z-position
	if (firstLevel)
	{
		if (!isGlitching) {
			minionPositionZ = minionPositionZ - SPEED * elapsedTime;
		}
		else {
			minionPositionZ = minionPositionZ - SPEED * elapsedTime + glitchdeceleration*elapsedTime;
		}

		minionPositionY = CalculateMinionHeight();
	}
	else
	{
		minionPositionZ2 = minionPositionZ2 - SPEED2 * elapsedTime;
		minionPositionY2 = 1.3f;
		if (!isThirdPerson)
		{
			Eye.y = 2.5;
		}
	}

	// Update the y-position based on the elapsed time
}

bool CheckCollision(const Vector& minionPos, const Obstacle& obstacle)
{
	if (!isGlitching)
	{
		float collisionThreshold = 0.5f;
		return (
			fabs(minionPos.y - obstacle.y) <= 0.07 * minionPos.y &&
			fabs(minionPos.z - obstacle.z) < collisionThreshold &&
			fabs(minionPos.x - obstacle.x) < collisionThreshold);
	}
	return false;
}
bool CheckSandbagCollision(const Vector& minionPos, const Obstacle& sandbag)
{
	if (!isGlitching)
	{
		float collisionThreshold = 0.5f;
		return (
			fabs(minionPos.y - sandbag.y) < 1.15 &&
			fabs(minionPos.z - sandbag.z) < collisionThreshold &&
			fabs(minionPos.x - sandbag.x) < collisionThreshold);
	}
	return false;
}

void CheckPortalCollision()
{

	float collisionThreshold = 2.0f;
	bool didCollide = (fabs(minionPositionZ - portal.z - 1) < collisionThreshold);
	if (didCollide)
	{
		if (score >= 10)
		{
			firstLevel = false;
		}
		else
		{
			gameLoseLevelOne = true;
		}
	}
}
void CheckFinishLineCollision()
{

	if (minionPositionZ2 <= -45.0f)
	{
		if (score >= 18)
		{
			gameWin = true;
		}
		else
		{
			gameLose = true;
		}
	}
}

void HandleCollision()
{
	if (!isGlitching)
	{
		if (firstLevel)
		{
			isGlitching = true;
			glitchStartTime = remainingTime;
		}
	}
}

void HandleSandbagRebound()
{
	if (isRebounding)
	{
		// Update rebound motion
		reboundDistance += reboundVelocity;
		reboundVelocity += reboundDeceleration;

		// Apply rebound movement
		minionPositionZ2 += reboundVelocity;
		Eye.z += reboundVelocity;

		// Check if rebound is complete
		if (reboundVelocity <= 0)
		{
			isRebounding = false;
			reboundVelocity = 0.0f;
			reboundDistance = 0.0f;
			remainingTime -= 2.0f;
		}

		if (remainingTime <= 0) {
			gameLose = true;
		}
	}
}

bool CheckLogCollision(const Vector& minionPos, const Vector& obstacle)
{
	float collisionThreshold = 0.9f;
	return (fabs(minionPos.x - obstacle.x) < collisionThreshold &&
		fabs(minionPos.y - obstacle.y) < 0.07 * minionPos.y &&
		fabs(minionPos.z - obstacle.z) < collisionThreshold);
}

void BananaCollision()
{
	if (!isGlitching)
	{

		for (auto it = bananas.begin(); it != bananas.end();)
		{
			bool isWithinXRange = it->x == minionPositionX2;
			bool isWithinZRange = (it->z >= minionPositionZ2 - 0.4f && it->z <= minionPositionZ2 + 0.4f);
			bool isWithinYRange = (it->y + yOffsetBanana >= minionPositionY2 - 0.5f && it->y + yOffsetBanana <= minionPositionY2 + 0.5f);

			if (isWithinXRange && isWithinZRange && isWithinYRange)
			{
				it = bananas.erase(it);
				Mix_PlayChannel(-1, bananaSound, 0);
				score++;
			}
			else
			{
				++it;
			}
		}
	}
}
void CoinCollision()
{
	if (!isGlitching)
	{
		float minionBodyHeight = 2.5f;

		for (auto it = coins.begin(); it != coins.end();)
		{
			bool isWithinXRange = it->x == minionPositionX;
			bool isWithinZRange = (it->z >= minionPositionZ - 0.5f && it->z <= minionPositionZ + 0.5f);
			bool isWithinYRange = (it->y >= minionPositionY - minionBodyHeight && it->y <= minionPositionY);

			if (isWithinXRange && isWithinZRange && isWithinYRange)
			{
				it = coins.erase(it);
				Mix_PlayChannel(-1, coinSound, 0);
				score++;
			}
			else
			{
				++it;
			}
		}
	}
}

void RenderMinion()
{
	CalculateMinionPosition();
	if (isJumping)
	{
		jumpOffset += jumpVelocity;
		jumpVelocity += gravity;
		if (jumpOffset <= 0)
		{
			jumpOffset = 0;
			isJumping = false;
			jumpVelocity = 0.0f;
		}
		minionPositionY += jumpOffset + gravity;
		if (!isThirdPerson)
		{
			Eye.y += jumpOffset;
		}
	}

	// obstacle collision
	for (const auto& obstacle : obstacles)
	{
		if (CheckCollision(Vector(minionPositionX, minionPositionY, minionPositionZ), obstacle))
		{
			HandleCollision();
			break;
		}
	}

	if (CheckLogCollision(Vector(minionPositionX, minionPositionY, minionPositionZ), Vector(1.3, 10, 35)) ||
		CheckLogCollision(Vector(minionPositionX, minionPositionY, minionPositionZ), Vector(3.6, 12, -5)))
	{
		HandleCollision();
	}

	if (isGlitching) {
		float elapsedTime = glitchStartTime - remainingTime;
		if (elapsedTime >= glitchDuration) {
			isGlitching = false;
		}
		UpdateGlitchEffect();
	}

	// Single draw call for the minion with or without glitch effect
	glPushMatrix();
	glTranslatef(minionPositionX, minionPositionY, minionPositionZ );
	glScalef(0.20, 0.20, 0.20);
	glRotatef(180, 0, 1, 0);

	if (isGlitching) {
		// Save current color state
		float currentColor[4];
		glGetFloatv(GL_CURRENT_COLOR, currentColor);

		// Apply glitch color only to the minion
		float glitchColor = (float)rand() / RAND_MAX;
		glColor3f(1.0f, glitchColor, glitchColor);

		model_minion.Draw();

		// Restore previous color state
		glColor4fv(currentColor);
	}
	else {
		model_minion.Draw();
	}

	glPopMatrix();
}


void RenderMinionSecond()
{
	CalculateMinionPosition();
	if (isJumping)
	{
		jumpOffset += jumpVelocity;
		jumpVelocity += gravity2;

		if (jumpOffset <= 0)
		{
			jumpOffset = 0;
			isJumping = false;
			jumpVelocity = 0.0f;
		}

		minionPositionY2 += jumpOffset + gravity2;
		if (!isThirdPerson)
		{
			Eye.y += jumpOffset;
		}
	}

	// Handle rebound physics
	HandleSandbagRebound();

	// obstacle collision
	for (const auto& sandbag : sandbags)
	{
		if (CheckSandbagCollision(Vector(minionPositionX2, minionPositionY2, minionPositionZ2), sandbag))
		{
			HandleCollision();
			// Instead of instant position change, initiate rebound
			if (!isRebounding)
			{
				isRebounding = true;
				reboundVelocity = reboundInitialVelocity;
				reboundDistance = 0.0f;
			}
			break;
		}
	}

	// Setup minion's light source (LIGHT1)
	glEnable(GL_LIGHT1);

	// Position the light slightly in front of and above the minion's hands
	GLfloat lightPos[] = { minionPositionX2, minionPositionY2 + 0.5f, minionPositionZ2 - 1.0f, 1.0f };
	glLightfv(GL_LIGHT1, GL_POSITION, lightPos);

	// Set light direction to point forward
	GLfloat lightDir[] = { 0.0f, -0.5f, -1.0f };
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, lightDir);

	// Configure light properties
	GLfloat lightAmb[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	GLfloat lightDiff[] = { 1.0f, 1.0f, 0.8f, 1.0f };
	GLfloat lightSpec[] = { 1.0f, 1.0f, 0.8f, 1.0f };

	glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmb);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiff);
	glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpec);

	// Set spotlight parameters
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0f);	 // Light cone angle
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 5.0f); // Light focus sharpness
	glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.5f);
	glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.01f);
	glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.009f);

	glPushMatrix();
	glTranslatef(minionPositionX2, minionPositionY2, minionPositionZ2);
	glScalef(0.40, 0.40, 0.40);
	glRotatef(180, 0, 1, 0);
	model_minion.Draw();
	glPopMatrix();
}

void RenderFinishLine()
{
	glPushMatrix();
	glTranslatef(0.0f, 0.5f, -45.0f);
	glScalef(3.0f, 3.0f, 3.0f);
	model_finishLine.Draw();
	glPopMatrix();
}

void UpdateLighting()
{
	if (firstLevel)
	{
		// Calculate transition based on remaining time
		dayNightTransition = remainingTime / transitionDuration;

		// Clamp values between 0 and 1
		if (dayNightTransition > 1.0f)
			dayNightTransition = 1.0f;
		if (dayNightTransition < 0.0f)
			dayNightTransition = 0.0f;

		// Update light properties based on time of day
		GLfloat sunIntensity = dayNightTransition;

		GLfloat ambientR = 0.1f + (0.2f * dayNightTransition);
		GLfloat ambientG = 0.1f + (0.2f * dayNightTransition);
		GLfloat ambientB = 0.2f + (0.3f * dayNightTransition);

		GLfloat diffuseR = 1.0f * sunIntensity;
		GLfloat diffuseG = (0.7f + 0.3f * sunIntensity) * sunIntensity;
		GLfloat diffuseB = (0.4f + 0.6f * sunIntensity) * sunIntensity;

		GLfloat ambient[] = { ambientR, ambientG, ambientB, 1.0f };
		GLfloat diffuse[] = { diffuseR, diffuseG, diffuseB, 1.0f };
		GLfloat specular[] = { sunIntensity, sunIntensity, sunIntensity, 1.0f };

		glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

		glClearColor(0.2f * dayNightTransition,
			0.3f * dayNightTransition,
			0.5f * dayNightTransition, 1.0f);
	}
	else
	{
		// Night time settings for level 2
		glDisable(GL_LIGHT0);
		glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
	}
}
void RenderSky()
{
	glPushMatrix();
	GLUquadricObj* qobj;
	qobj = gluNewQuadric();
	glTranslated(50, 0, 0);
	glRotated(90, 1, 0, 1);

	if (firstLevel)
	{
		// Blend between day and night textures based on transition
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Render day sky with fading alpha
		glBindTexture(GL_TEXTURE_2D, daytex);
		gluQuadricTexture(qobj, true);
		gluQuadricNormals(qobj, GL_SMOOTH);
		gluSphere(qobj, 100, 100, 100);
		glColor4f(1.0f, 1.0f, 1.0f, dayNightTransition);
		gluQuadricTexture(qobj, true);
		gluQuadricNormals(qobj, GL_SMOOTH);
		gluSphere(qobj, 100, 100, 100);

		//// Render night sky with increasing alpha
		glBindTexture(GL_TEXTURE_2D, nighttex);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f - dayNightTransition);
		gluQuadricTexture(qobj, true);
		gluSphere(qobj, 100, 100, 100);

		glDisable(GL_BLEND);
	}
	else
	{
		// Full night sky for level 2
		glBindTexture(GL_TEXTURE_2D, nighttex);
		gluQuadricTexture(qobj, true);
		gluQuadricNormals(qobj, GL_SMOOTH);
		gluSphere(qobj, 100, 100, 100);
	}

	gluDeleteQuadric(qobj);
	glPopMatrix();
}

void RenderBridge()
{
	glPushMatrix();
	glTranslatef(70.0f, 0.0f, bridgePositionZ);
	glScalef(4.0f, 4.0f, 4.0f);
	glRotatef(90, 0, 1, 0);
	model_bridge.Draw();
	glPopMatrix();
}

void RenderCoins()
{
	float static coinAnimationTime = 0.0f;
	coinAnimationTime += 0.5f;

	coins.erase(
		std::remove_if(coins.begin(), coins.end(), [](const Coin& coin)
			{ return coin.z > Eye.z; }),
		coins.end());

	for (const auto& coin : coins)
	{

		glPushMatrix();

		glTranslatef(coin.x, 10.85, coin.z);
		glRotatef(coinAnimationTime, 0.0f, 1.0f, 0.0f);
		glScalef(0.2f, 0.2f, 0.2f);
		model_coin.Draw();
		glPopMatrix();
	}
}

void RenderBananas()
{
	float static bananaAnimationTime = 0.0f;
	bananaAnimationTime += 0.15f;

	// Remove bananas that the camera has passed
	bananas.erase(
		std::remove_if(bananas.begin(), bananas.end(), [](const Banana& banana)
			{ return banana.z > Eye.z; }),
		bananas.end());

	for (const auto& banana : bananas)
	{

		glPushMatrix();
		yOffsetBanana = sin(bananaAnimationTime) * 0.2f;
		glTranslatef(banana.x, banana.y + yOffsetBanana, banana.z);
		glRotatef(90, 0, 1, 0);
		glScalef(0.6f, 0.6f, 0.6f);
		model_banana.Draw();
		glPopMatrix();
	}
}

void RenderObstacles()
{
	// Remove obstacles that the camera has passed
	obstacles.erase(
		std::remove_if(obstacles.begin(), obstacles.end(), [](const Obstacle& obstacle)
			{ return obstacle.z > Eye.z; }),
		obstacles.end());
	for (const auto& obstacle : obstacles)
	{
		glPushMatrix();
		glTranslatef(obstacle.x, obstacle.y, obstacle.z);
		glRotatef(180, 0, 1, 0);
		glScalef(0.2f, 0.2f, 0.2f);
		model_barrier.Draw();
		glPopMatrix();
	}
}

void RenderSandbags()
{
	sandbags.erase(
		std::remove_if(sandbags.begin(), sandbags.end(), [](const Obstacle& sandbag)
			{ return sandbag.z > Eye.z; }),
		sandbags.end());
	for (const auto& sandbag : sandbags)
	{
		glPushMatrix();
		glTranslatef(sandbag.x, sandbag.y, sandbag.z);
		glRotatef(180, 0, 1, 0);
		glScalef(1.0f, 1.0f, 1.0f);
		model_sandbags.Draw();
		glPopMatrix();
	}
}

void RenderLogs()
{
	glPushMatrix();
	glTranslatef(1.3f, 10.0f, 35.0f);
	glRotatef(150, 0, 1, 0);
	glScalef(0.5f, 0.5f, 0.5f);
	model_logs.Draw();
	glPopMatrix();
}

void RenderPortal()
{
	glPushMatrix();
	glTranslatef(portal.x, portal.y - 5, portal.z - 1);
	glScalef(9.0f, 9.0f, 3.0f);
	model_portal.Draw();
	glPopMatrix();
}

void RenderLamp()
{
	// Setup lantern's light source (LIGHT2)
	glEnable(GL_LIGHT2);

	// Position the light at the lantern's location
	GLfloat lightPos[] = { minionPositionX2 - 1.5f, minionPositionY2 - 0.7f, minionPositionZ2 - 0.07f, 1.0f };
	glLightfv(GL_LIGHT2, GL_POSITION, lightPos);

	// Purple-tinted light colors
	GLfloat lightAmb[] = { 0.2f, 0.2f, 0.2f, 1.0f };	// Dim white ambient
	GLfloat lightDiff[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // Bright white diffuse
	GLfloat lightSpec[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // Pure white specular

	glLightfv(GL_LIGHT2, GL_AMBIENT, lightAmb);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, lightDiff);
	glLightfv(GL_LIGHT2, GL_SPECULAR, lightSpec);

	// Make it an omnidirectional light with smooth falloff
	glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 1.0f);
	glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.3f);
	glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.05f);

	// Render the lantern model
	glPushMatrix();
	glTranslatef(minionPositionX2 - 1.0, minionPositionY2 - 0.7, minionPositionZ2 - 0.07);
	glScalef(2.5f, 2.5f, 2.5f);
	model_lamp.Draw();
	glPopMatrix();
}

void ResetLevel()
{
	Eye = Vector(0, 4, 86);
	At = Vector(0, 2, 0);
	Up = Vector(0, 1, 0);
	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
	remainingTime = start;
	elapsedTime = 0.0f;
	doneReset = true;
}

void RenderTimer()
{
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, WIDTH, 0, HEIGHT);
	glMatrixMode(GL_MODELVIEW);
	glColor3f(0.0f, 0.0f, 0.0f);			 // Set the text color to black
	glRasterPos2i(WIDTH - 106, HEIGHT - 25); // Position the text
	char timerText[50];
	sprintf_s(timerText, "Time: %.1f s", remainingTime);
	for (char* c = timerText; *c != '\0'; c++)
	{
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
	}

	char scoreText[50];
	sprintf_s(scoreText, "Score: %d  | ", score);
	glRasterPos2i(WIDTH - 200, HEIGHT - 25);
	for (char* c = scoreText; *c != '\0'; c++)
	{
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
	}

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void RenderTimer2()
{
	glDisable(GL_LIGHTING); // Disable lighting

	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, WIDTH, 0, HEIGHT);
	glMatrixMode(GL_MODELVIEW);
	glColor3f(1.0f, 1.0f, 1.0f);			 // Set the text color to white
	glRasterPos2i(WIDTH - 700, HEIGHT - 25); // Position the text
	char timerText[50];
	sprintf_s(timerText, "Time: %.1f s", remainingTime);
	for (char* c = timerText; *c != '\0'; c++)
	{
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
	}

	char scoreText[50];
	sprintf_s(scoreText, "Score: %d  | ", score);
	glRasterPos2i(WIDTH - 800, HEIGHT - 25);
	for (char* c = scoreText; *c != '\0'; c++)
	{
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
	}

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glEnable(GL_LIGHTING);
}

void RenderText(float x, float y, const char* text)
{
	glDisable(GL_TEXTURE_2D); // Disable texturing
	glPushMatrix();
	glLoadIdentity();

	// Position the text
	glRasterPos2f(x, y);

	// Render each character
	for (const char* c = text; *c != '\0'; c++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c); // Using a larger font
	}

	glPopMatrix();
	glEnable(GL_TEXTURE_2D); // Re-enable texturing
}

void RenderGameOverScreen()
{
	glDisable(GL_LIGHTING);	  // Disable lighting
	glDisable(GL_DEPTH_TEST); // Disable depth testing for 2D rendering

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, WIDTH, 0, HEIGHT);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// Clear the screen to white
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float xCenter = WIDTH / 2.0f - 50.0f;
	float yCenter = HEIGHT / 2.0f;

	glColor3f(1.0f, 0.0f, 0.0f); // Red color for text
	if (gameLoseLevelOne)
	{
		RenderText(xCenter - 200, yCenter, "Game Over! Try again! Collect more coins to advance to level 2");
	}
	else if (gameLose)
	{
		RenderText(xCenter, yCenter, "Game Over!");
	}
	else
	{
		glColor3f(0.0f, 1.0f, 0.0f); // Red color for text
		RenderText(xCenter, yCenter, "Game Win!");
	}
	char scoreText[50];
	sprintf_s(scoreText, "Your score is %d", score);
	RenderText(xCenter-10, yCenter-50, scoreText);
	// Restore matrices
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	// Re-enable states
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

void MoveCamera()
{
	glLoadIdentity(); // Clear Model_View Matrix
	if (firstLevel)
	{
		Eye.z -= SPEED * elapsedTime;
		if (isGlitching) {
			Eye.z += glitchdeceleration * elapsedTime;
		}
		At.z -= SPEED * elapsedTime;

		gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z); // Setup Camera with modified parameters
	}
	else
	{

		Eye.z -= SPEED2 * elapsedTime;
		At.z -= SPEED2 * elapsedTime;

		gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
	}
}

void UpdateCamera()
{
	if (!isThirdPerson)
	{
		if (firstLevel)
		{
			Eye.x = minionPositionX;
			At = Vector(0, -2, At.z);
		}
		else
		{
			Eye.x = minionPositionX2;
			At = Vector(0, -2, At.z);
		}
	}

	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
}

void UpdateFlashlightPosition()
{
	// Assuming minionPositionX2, minionPositionY2, and minionPositionZ2 are the minion's coordinates
	flashlightPosition[0] = minionPositionX2;		 // X position based on minion
	flashlightPosition[1] = minionPositionY2 + 1.0f; // Slightly above the minion's height
	flashlightPosition[2] = minionPositionZ2;		 // Z position based on minion
}

// Display Function
void Display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Update the camera view based on the current mode
	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);

	UpdateLighting();

	// Lighting setup
	GLfloat lightIntensity[] = { 0.7, 0.7, 0.7, 1.0f };
	GLfloat lightPosition[] = { 0.0f, 100.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);
	if (gameLoseLevelOne || gameLose || gameWin)
	{
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		RenderGameOverScreen();
	}
	else
	{
		if (firstLevel)
		{
			CoinCollision();
			RenderGround();
			RenderMinion();
			RenderSky();
			CheckPortalCollision();
			RenderPortal();
			RenderBridge();
			RenderCoins();
			RenderObstacles();
			RenderLogs();
			glPushMatrix();
			glTranslated(2.3, 1.0, -40);
			RenderLogs();
			RenderTimer();
			MoveCamera();
		}
		else
		{
			if (!doneReset)
			{
				ResetLevel();
			}
			CheckFinishLineCollision();
			RenderFinishLine();
			BananaCollision();
			RenderBananas();
			RenderSandbags();
			RenderGround();
			RenderMinionSecond();
			RenderSky();
			RenderLamp();
			RenderTrees();
			MoveCamera();
			RenderTimer2();
		}
	}

	glutSwapBuffers();
	glutPostRedisplay();
}

// Keyboard Function
void Keyboard(unsigned char button, int x, int y)
{
	switch (button)
	{
	case 27: // esc
		exit(0);
		break;

	default:
		break;
	}
}

void SpecialKeyboard(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		if (firstLevel)
		{
			if (!isJumping)
			{
				isJumping = true;
				jumpVelocity = jumpForce;
				jumpOffset = 0.0f;
			}
		}
		else
		{
			if (!isJumping)
			{
				isJumping = true;
				jumpVelocity = jumpForce2;
				jumpOffset = 0.0f;
			}
		}
		break;

	case GLUT_KEY_LEFT:
		if (firstLevel)
		{
			if (LaneIndex > 0)
			{
				LaneIndex--;
				minionPositionX = xPositions[LaneIndex];
				UpdateCamera();
			}
		}
		else
		{
			if (LaneIndex2 > 0)
			{
				LaneIndex2--;
				minionPositionX2 = xPositions2[LaneIndex2];
				UpdateCamera();
			}
		}
		break;

	case GLUT_KEY_RIGHT:
		if (firstLevel)
		{
			if (LaneIndex < xCount - 1)
			{
				LaneIndex++;
				minionPositionX = xPositions[LaneIndex];
				UpdateCamera();
			}
		}
		else
		{
			if (LaneIndex2 < xCount2 - 1)
			{
				LaneIndex2++;
				minionPositionX2 = xPositions2[LaneIndex2];
				UpdateCamera();
			}
		}
		break;

	default:
		break;
	}
}

void MouseFunc(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		isThirdPerson = !isThirdPerson;
		if (firstLevel) {
			if (isThirdPerson) {
				Eye = Vector(2.4, Eye.y, Eye.z + 6.4);
				At = Vector(0, 0, At.z + 3);
			}
			else {
				Eye = Vector(minionPositionX, Eye.y, Eye.z - 6.4);
				At = Vector(0, 0, At.z - 3); // look forward
			}
		}
		else {
			if (isThirdPerson) {
				Eye = Vector(0, Eye.y + 1.0, Eye.z + 9.5);
				At = Vector(0, 0, At.z);
			}
			else {
				Eye = Vector(minionPositionX2, Eye.y - 1.0, Eye.z - 9.5);
				At = Vector(0, 0, At.z); // look forward
			}
		}
		glLoadIdentity();
		gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
	}
}

// Motion Function
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

	glLoadIdentity(); // Clear Model_View Matrix

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z); // Setup Camera with modified paramters

	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

// Mouse Function
void Mouse(int button, int state, int x, int y)
{
	y = HEIGHT - y;

	if (state == GLUT_DOWN)
	{
		cameraZoom = y;
	}
}

// Reshape Function
void Reshape(int w, int h)
{
	if (h == 0)
	{
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

// OpengGL Configuration Function
void init(void)
{
	LoadAssets();
	InitSound();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT2);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	srand(static_cast<unsigned>(time(0)));
	SpawnCoins(12);
	SpawnBananas(12);
	SpawnObstacles(5);
	SpawnSandbags(10);
	SpawnLogs(2);
	InitializeForest();

	glShadeModel(GL_SMOOTH);

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

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_NORMALIZE);
}

void Render(int value)
{
	elapsedTime = (std::clock() - startTime) / (double)CLOCKS_PER_SEC;
	if (remainingTime > 0.0f)
	{
		remainingTime -= 0.016f;
	}
	Display();
	glutTimerFunc(16, Render, 0);
}

// Main Function
int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(WIDTH, HEIGHT);

	glutInitWindowPosition(80, 180);

	glutCreateWindow(title);

	glutDisplayFunc(Display);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeyboard);
	glutMouseFunc(MouseFunc);
	glutMotionFunc(Motion);
	glutReshapeFunc(Reshape);

	init();

	glutTimerFunc(16, Render, 0);

	Mix_VolumeMusic(30);
	Mix_PlayMusic(background1Sound, -1);

	glutMainLoop();
	return 0;
}