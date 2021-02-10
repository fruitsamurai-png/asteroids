/******************************************************************************/
/*!
	\file		GameState_Asteroids.cpp
	\author 	Chng Nai Wei Keith
	\par    	email: n.chng\@digipen.edu
	\date   	10/2/21
	\brief		This is the main game state file where it runs the game asteroids.
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#include "main.h"
#include <iostream>
/******************************************************************************/
/*!
	Defines
*/
/******************************************************************************/
const unsigned int	GAME_OBJ_NUM_MAX		= 32;			//The total number of different objects (Shapes)
const unsigned int	GAME_OBJ_INST_NUM_MAX	= 2048;			//The total number of different game object instances


const unsigned int	SHIP_INITIAL_NUM		= 3;			// initial number of ship lives
const float			SHIP_SIZE				= 30.0f;		// ship size
const float			SHIP_ACCEL_FORWARD		= 60.0f;		// ship forward acceleration (in m/s^2)
const float			SHIP_ACCEL_BACKWARD		= 60.0f;		// ship backward acceleration (in m/s^2)
const float			SHIP_ROT_SPEED			= (2.0f * PI);	// ship rotation speed (degree/second)

const unsigned int	ASTEROID_NUM			= 4;			//number of initial asteroid to spawn
const float			ASTEROID_SIZE			= 64.0f;		//size multiplier for randomised asteroid spawn
const float			ASTEROID_BASE			= 20.0f;		//base value for asteroid size and speed
const float			ASTEROID_SPEED			= 32.0f;		//speed multiplier for randomised asteroid spawn

const float			BULLET_SIZE				= 15.0f;		//scale of bullet 
const float			BULLET_SPEED			= 10.0f;		// bullet speed (m/s)


extern float		g_dt;									//delta time called from main.cpp

//booleans
static bool			gameover =false;						//condition if lives less than 0 or if it hits the high score of 5000
static bool			onValueChange = true;					//condition to print the score or live updates on the console
// -----------------------------------------------------------------------------
enum TYPE
{
	// list of game object types
	TYPE_BG=0,
	TYPE_SHIP, 
	TYPE_BULLET,
	TYPE_ASTEROID,

	TYPE_NUM
};

// -----------------------------------------------------------------------------
// object flag definition

const unsigned long FLAG_ACTIVE				= 0x00000001;

/******************************************************************************/
/*!
	Struct/Class Definitions
*/
/******************************************************************************/

//Game object structure
struct GameObj
{
	unsigned long		type;		// object type
	AEGfxVertexList *	pMesh;		// This will hold the triangles which will form the shape of the object
	AEGfxTexture*		pTex;		// This will hold the textures to lay on the mesh
};

// ---------------------------------------------------------------------------

//Game object instance structure
struct GameObjInst
{
	GameObj *			pObject;	// pointer to the 'original' shape
	unsigned long		flag;		// bit flag or-ed together
	float				scale;		// scaling value of the object instance
	AEVec2				posCurr;	// object current position
	AEVec2				velCurr;	// object current velocity
	float				dirCurr;	// object current direction
	AABB				boundingBox;// object bouding box that encapsulates the object
	AEMtx33				transform;	// object transformation matrix: Each frame, 
									// calculate the object instance's transformation matrix and save it here
};

/******************************************************************************/
/*!
	Static Variables
*/
/******************************************************************************/

// list of original object
static GameObj				sGameObjList[GAME_OBJ_NUM_MAX];				// Each element in this array represents a unique game object (shape)
static unsigned long		sGameObjNum;								// The number of defined game objects

// list of object instances
static GameObjInst			sGameObjInstList[GAME_OBJ_INST_NUM_MAX];	// Each element in this array represents a unique game object instance (sprite)
static unsigned long		sGameObjInstNum;							// The number of used game object instances

// pointer to the ship object
static GameObjInst *		spShip;										// Pointer to the "Ship" game object instance

//pointer to the init asteroid object
static GameObjInst*			asteroid[ASTEROID_NUM];						// Pointer array to the initial "Asteroid" game object instance


// number of ship available (lives 0 = game over)
static long					sShipLives;									// The number of lives left

// the score = number of asteroid destroyed
static unsigned long		sScore;										// Current score
s8							Text;										//Text to display on screen


// ---------------------------------------------------------------------------

// functions to create/destroy a game object instance
GameObjInst *			gameObjInstCreate (unsigned long type, float scale, 
										   AEVec2 * pPos, AEVec2 * pVel, float dir);
void					gameObjInstDestroy(GameObjInst * pInst);

static void				GameStateAsteroidsCreate(void);//helper function to intialised the asteroid entity with randomised variables
/******************************************************************************/
/*!
	"Load" function of this state
*/
/******************************************************************************/
void GameStateAsteroidsLoad(void)
{
	// zero the game object array
	memset(sGameObjList, 0, sizeof(GameObj) * GAME_OBJ_NUM_MAX);
	// No game objects (shapes) at this point
	sGameObjNum = 0;

	// zero the game object instance array
	memset(sGameObjInstList, 0, sizeof(GameObjInst) * GAME_OBJ_INST_NUM_MAX);
	// No game object instances (sprites) at this point
	sGameObjInstNum = 0;

	// The ship object instance hasn't been created yet, so this "spShip" pointer is initialized to 0
	spShip = nullptr;
	// load/create the mesh data (game objects / Shapes)
	GameObj * pObj;

	//load the font texture
	Text = AEGfxCreateFont("../Resources/Fonts/Arial Italic.ttf",24);
	AE_ASSERT_MESG(Text, "fail to create object!!");
	// ======================================
	// create the Background mesh and texture
	// ======================================
	pObj = sGameObjList + sGameObjNum++;//move the struct pointer to have the mesh of the background
	pObj->type = TYPE_BG;
	AEGfxMeshStart();
	AEGfxTriAdd(
		-0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
		-0.5f, -0.5f, 0xFFFF0000, 1.0f, 0.0f,
		0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 1.0f);
	AEGfxTriAdd(
		0.5f, 0.5f, 0xFFFF0000, 0.0f, 1.0f,
		-0.5f, -0.5f, 0xFFFF0000, 1.0f, 0.0f,
		0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG(pObj->pMesh, "fail to create object!!");
	pObj->pTex = AEGfxTextureLoad("../Resources/Art/bg.png");
	AE_ASSERT_MESG(pObj->pTex, "fail to create object!!");

	// ======================================
	// create the ship shape mesh and texture
	// ======================================

	pObj		= sGameObjList + sGameObjNum++;//move the struct pointer to have the mesh of the ship
	pObj->type	= TYPE_SHIP;

	AEGfxMeshStart();
	AEGfxTriAdd(
		-0.5f,  0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
		-0.5f, -0.5f, 0xFFFF0000, 1.0f, 0.0f,
		 0.5f,  0.5f, 0xFFFFFFFF, 0.0f, 1.0f );
	AEGfxTriAdd(
		0.5f, 0.5f, 0xFFFF0000, 0.0f, 1.0f,
		-0.5f, -0.5f, 0xFFFF0000, 1.0f, 0.0f,
		0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG(pObj->pMesh, "fail to create object!!");
	pObj->pTex = AEGfxTextureLoad("../Resources/Art/ship.png");
	AE_ASSERT_MESG(pObj->pTex, "fail to create object!!");

	// ========================================
	// create the bullet shape mesh and texture
	// ========================================
	pObj = sGameObjList + sGameObjNum++;//move the struct pointer to have the mesh of the bullet
	pObj->type = TYPE_BULLET;

	AEGfxMeshStart();
	AEGfxTriAdd(
		 -0.5f, 0.25f, 0xFFFFFFFF, 1.0f, 1.0f,//top left
		-0.5f, -0.25f, 0xFFFF0000, 1.0f, 0.0f,//bottom left
		0.5f, 0.25f, 0xFFFFFFFF, 0.0f, 1.0f);//top right
	AEGfxTriAdd(
		0.5f,0.25f, 0xFFFF0000, 0.0f, 1.0f,//top right
		-0.5f, -0.25f, 0xFFFF0000, 1.0f, 0.0f,//bottom left
		0.5f, -0.25f, 0xFFFFFFFF, 0.0f, 0.0f);//bottom right

	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG(pObj->pMesh, "fail to create object!!");
	pObj->pTex = AEGfxTextureLoad("../Resources/Art/beam.png");
	AE_ASSERT_MESG(pObj->pTex, "fail to create object!!");


	// ==========================================
	// create the asteroid shape mesh and texture
	// ==========================================
	pObj = sGameObjList + sGameObjNum++;//move the struct pointer to have the mesh of the asteroid
	pObj->type = TYPE_ASTEROID;

	AEGfxMeshStart();
	AEGfxTriAdd(
		0.5f, 0.5f, 0xFFFF0000, 1.0f, 1.0f,
		-0.5f, 0.5f, 0xFFFF0000, 0.0f, 1.0f,
		-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	AEGfxTriAdd(
		-0.5f, -0.5f, 0xFFFF0000, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFFFF0000, 1.0f, 0.0f,
		0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 1.0f);

	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG(pObj->pMesh, "fail to create object!!");
	pObj->pTex = AEGfxTextureLoad("../Resources/Art/block.png");
	AE_ASSERT_MESG(pObj->pTex, "fail to create object!!");
	
}

/******************************************************************************/
/*!
	"Initialize" function of this state
*/
/******************************************************************************/
void GameStateAsteroidsInit(void)
{
	AEVec2 vel,pos;
	float dir,size;
	//initialise the background with the size of the windowwidth
	gameObjInstCreate(TYPE_BG, AEGetWindowWidth(), nullptr, nullptr, 0.0f);
	// create the main ship
	spShip = gameObjInstCreate(TYPE_SHIP, SHIP_SIZE, nullptr, nullptr, 0.0f);
	AE_ASSERT(spShip);	
	// CREATE THE INITIAL ASTEROIDS INSTANCES USING THE "gameObjInstCreate" FUNCTION
	for (unsigned int i = 0; i < ASTEROID_NUM; i++)//intialise 4 times
	{
			f32 polarity = AERandFloat() > 0.5f ? -1.0f : 1.0f;
			AEVec2Set(&vel, ASTEROID_SPEED * AERandFloat() + ASTEROID_BASE, ASTEROID_SPEED * AERandFloat() + ASTEROID_BASE);
			f32 pos1 = AERandFloat() > 0.5f ? AEGfxGetWinMaxX() + ASTEROID_BASE : AEGfxGetWinMinX() - ASTEROID_BASE;
			f32 pos2 = AERandFloat() > 0.5f ? AEGfxGetWinMaxY() + ASTEROID_BASE : AEGfxGetWinMinY() - ASTEROID_BASE;
			AEVec2Scale(&vel, &vel, polarity);
			AEVec2Set(&pos, pos1 *0.75f, pos2  * 0.75f);//times the pos with 0.75f so all 4 will appear on screen
			dir = 2*PI*AERandFloat();
			size = (ASTEROID_SIZE * AERandFloat()+ ASTEROID_BASE);
			asteroid[i] = gameObjInstCreate(TYPE_ASTEROID, size, &pos, &vel, dir);
			AE_ASSERT(asteroid[i]);
	}
	// reset the score and the number of ships,also reset the gameover boolean to false
	sScore      = 0;
	sShipLives  = SHIP_INITIAL_NUM;
	gameover = false;
}

/******************************************************************************/
/*!
	Score function to print onto the game screen and terminal
*/
/******************************************************************************/
static void GameStateAsteroidsScore(void)
{
	char strBuffer[1024];
	char ScoreLiveBuffer[1024];
	char GameBuffer[1024];
	char RestartBuffer[1024];

	//sprint the score and lives into string
	sprintf_s(ScoreLiveBuffer, "Score: %-80d Ship Left: %d", sScore, sShipLives);

	//display the score and lives on game screen
	AEGfxPrint(Text, ScoreLiveBuffer, -1.0f, 0.9f, 1.0f, 1.0f, 1.0f, 1.0f);
	if (gameover)
	{
		if (sShipLives <= 0)//if no lives
		{
			sprintf_s(GameBuffer, "       GAME OVER       \n");

		}
		if (sScore >= 5000)//if its hit max score
		{
			sprintf_s(GameBuffer, "       YOU ROCK       \n");
		}

		sprintf_s(RestartBuffer, "       PRESS R TO RESTART       \n");
		//display the restart string on screen
		AEGfxPrint(Text, GameBuffer, -0.3f, 0.1f, 1.1f, 1.0f, 1.0f, 1.0f);
		AEGfxPrint(Text, RestartBuffer, -0.5f, -0.1f, 1.1f, 1.0f, 1.0f, 1.0f);
		if (AEInputCheckTriggered(AEVK_R))
		{
			gGameStateNext = GS_RESTART;//restart the game state
		}
	}
	if (onValueChange)
	{
		sprintf_s(strBuffer, "Score: %d", sScore);
		printf("%s \n", strBuffer);

		sprintf_s(strBuffer, "Ship Left: %d", sShipLives >= 0 ? sShipLives : 0);
		printf("%s \n", strBuffer);

		// display the game over message
		if (sShipLives <= 0)
		{
			printf("       GAME OVER       \n");
		}
		// display the win game message
		if (sScore>=5000)
		{
			printf("       YOU ROCK       \n");
		}
		onValueChange = false;
	}
}
/******************************************************************************/
/*!
	Helper function to create randomised asteroid spawns
*/
/******************************************************************************/
static void GameStateAsteroidsCreate(void)
{
	AEVec2 vel, pos;
	f32 dir, size;
	f32 polarity = AERandFloat()> 0.5f ? -1.0f : 1.0f;//rand feature for polarity of the spawn
	f32 pos1 = AERandFloat() > 0.5f ? AEGfxGetWinMaxX()+ASTEROID_BASE : AEGfxGetWinMinX() - ASTEROID_BASE;//rand feature to decide if it will on the left or right of x axis
	f32 pos2 = AERandFloat() > 0.5f ? AEGfxGetWinMaxY() + ASTEROID_BASE : AEGfxGetWinMinY() - ASTEROID_BASE;//rand feature to decide if it will on the left or right of y axis
	f32 neg = AERandFloat() > 0.5f ? 0.0f : 1.0f;//rand feature to decide to negate the value 
	AEVec2Set(&pos, pos1*neg, pos2);//only times the rand1 for the x axis so it will not spawn on the centre of screen
	AEVec2Set(&vel, ASTEROID_SPEED * AERandFloat() + ASTEROID_BASE, ASTEROID_SPEED * AERandFloat() + ASTEROID_BASE);//rand the speed
	dir = 2*PI*AERandFloat();//rand the dirr
	
	AEVec2Scale(&pos, &pos, polarity);//times the pos and vel with polarity
	AEVec2Scale(&vel, &vel, polarity);
	if (sScore>= 1500)
	{
		AEVec2Scale(&vel, &vel,(1.5f));//increase the vel with each score increment 
	}
	if (sScore >= 3000)
	{
		AEVec2Scale(&vel, &vel, (1.5f));
	}
	if (sScore >= 4000)
	{
		AEVec2Scale(&vel, &vel, (1.5f));
	}
	size = (ASTEROID_SIZE * AERandFloat() + ASTEROID_BASE);//rand the size 
	gameObjInstCreate(TYPE_ASTEROID, size, &pos, &vel, dir);//initialise
}
/******************************************************************************/
/*!
	Input function to control the ship in the state
*/
/******************************************************************************/
static void GameStateAsteroidsInput(void)
{
	AEVec2	accCurr;//vector to have the resultant vector for the acceleration
	if (AEInputCheckCurr(AEVK_UP) || AEInputCheckCurr(AEVK_W))
	{
		// Find the velocity according to the acceleration
		AEVec2Set(&accCurr, cosf(spShip->dirCurr) * SHIP_ACCEL_FORWARD * g_dt, sinf(spShip->dirCurr) * SHIP_ACCEL_FORWARD * g_dt);
		AEVec2ScaleAdd(&spShip->velCurr, &accCurr, &spShip->velCurr, g_dt);

		// Limit your speed over here
		AEVec2Scale(&spShip->velCurr, &spShip->velCurr, 0.99f);
	}
	if (AEInputCheckCurr(AEVK_DOWN) || AEInputCheckCurr(AEVK_S))
	{
		// Find the velocity according to the decceleration
		AEVec2Set(&accCurr, -cosf(spShip->dirCurr) * SHIP_ACCEL_BACKWARD * g_dt, -sinf(spShip->dirCurr) * SHIP_ACCEL_BACKWARD * g_dt);
		AEVec2ScaleAdd(&spShip->velCurr, &accCurr, &spShip->velCurr, g_dt);

		// Limit your speed over here
		AEVec2Scale(&spShip->velCurr, &spShip->velCurr, 0.99f);
	}

	if (AEInputCheckCurr(AEVK_LEFT) || AEInputCheckCurr(AEVK_A))
	{
		spShip->dirCurr += SHIP_ROT_SPEED * (float)(AEFrameRateControllerGetFrameTime());//dir added by the rotation speed of the ship x delta time 
		spShip->dirCurr = AEWrap(spShip->dirCurr, -PI, PI);//wrap the dir so its always moving in 360 deg
	}

	if (AEInputCheckCurr(AEVK_RIGHT) || AEInputCheckCurr(AEVK_D))
	{
		spShip->dirCurr -= SHIP_ROT_SPEED * (float)(AEFrameRateControllerGetFrameTime());
		spShip->dirCurr = AEWrap(spShip->dirCurr, -PI, PI);

	}

	// Shoot a bullet if space is triggered (Create a new object instance)
	if (AEInputCheckTriggered(AEVK_SPACE))
	{
		AEVec2 angle;
		// Get the bullet's direction according to the ship's direction
		// Set the velocity
		AEVec2Set(&angle, cosf(spShip->dirCurr) * BULLET_SPEED, sinf(spShip->dirCurr) * BULLET_SPEED);

		// Create an instance
		gameObjInstCreate(TYPE_BULLET, BULLET_SIZE, &spShip->posCurr, &angle, spShip->dirCurr);
	}
	
}
/******************************************************************************/
/*!
	Physics function in the state
*/
/******************************************************************************/
static void GameStateAsteroidsPhysics(void)
{
	for (unsigned long i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		GameObjInst* pInst = sGameObjInstList + i;
		// skip non-active object
		if ((pInst->flag & FLAG_ACTIVE) == 0)
			continue;
		if (pInst->pObject->type == TYPE_SHIP)
		{
			AEVec2Add(&pInst->posCurr, &pInst->posCurr, &pInst->velCurr);//update the pos with the vel
		}
		if (pInst->pObject->type == TYPE_ASTEROID)
		{
			AEVec2 added;
			AEVec2Set(&added, cosf(pInst->dirCurr), sinf(pInst->dirCurr));//initial vector using the asteroid direction curr
			AEVec2ScaleAdd(&added, &pInst->velCurr, &added,g_dt);//add the vector with vel
			AEVec2Add(&pInst->posCurr, &pInst->posCurr, &added);//update the pos
		}
		if (pInst->pObject->type == TYPE_BULLET)
		{
			AEVec2 angle;
			AEVec2Set(&angle, cosf(pInst->dirCurr) * BULLET_SPEED, sinf(pInst->dirCurr) * BULLET_SPEED);//update speed based on angle scaled by the speed
			AEVec2Add(&pInst->posCurr, &pInst->posCurr, &angle);
		}
		//BOUNDING BOX FOR MIN AND MAX BOUNDARIES BELOW
		AEVec2Set(&pInst->boundingBox.min, -0.5f * pInst->scale, -0.5f * pInst->scale);//set the vector for the min bounding vector
		AEVec2Add(&pInst->boundingBox.min, &pInst->posCurr, &pInst->boundingBox.min);//add that to the pos of the instance
		AEVec2Set(&pInst->boundingBox.max, 0.5f * pInst->scale, 0.5f * pInst->scale);
		AEVec2Add(&pInst->boundingBox.max, &pInst->posCurr, &pInst->boundingBox.max);

	}

}
/******************************************************************************/
/*!
	Wrapping function in the state so that each entity will respond accordingly 
	when going through the screen of the game
*/
/******************************************************************************/
static void GameStateAsteroidsWrap(void)
{
	for (unsigned long i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		GameObjInst* pInst = sGameObjInstList + i;

		// skip non-active object
		if ((pInst->flag & FLAG_ACTIVE) == 0)
			continue;

		// check if the object is a ship
		if (pInst->pObject->type == TYPE_SHIP)
		{
			// warp the ship from one end of the screen to the other
			pInst->posCurr.x = AEWrap(pInst->posCurr.x, AEGfxGetWinMinX() - SHIP_SIZE,
				AEGfxGetWinMaxX() + SHIP_SIZE);
			pInst->posCurr.y = AEWrap(pInst->posCurr.y, AEGfxGetWinMinY() - SHIP_SIZE,
				AEGfxGetWinMaxY() + SHIP_SIZE);
		}

		// Wrap asteroids here
		if (pInst->pObject->type == TYPE_ASTEROID)
		{
			pInst->posCurr.x = AEWrap(pInst->posCurr.x, AEGfxGetWinMinX() - ASTEROID_SIZE,
				AEGfxGetWinMaxX() + ASTEROID_SIZE);
			pInst->posCurr.y = AEWrap(pInst->posCurr.y, AEGfxGetWinMinY() - ASTEROID_SIZE,
				AEGfxGetWinMaxY() + ASTEROID_SIZE);
		}

		// remove bullets that go out of bounds
		if (pInst->pObject->type == TYPE_BULLET)
		{
			if ((pInst->boundingBox.max.x > AEGfxGetWinMaxX() && pInst->boundingBox.max.y > AEGfxGetWinMaxY()) ||
				(pInst->boundingBox.min.x < AEGfxGetWinMinX() && pInst->boundingBox.min.y < AEGfxGetWinMinY()))
			{
				gameObjInstDestroy(pInst);
			}
		}
	}
}
/******************************************************************************/
/*!
	Collision function for the game state
*/
/******************************************************************************/
static void GameStateAsteroidsCollision(void)
{
	for (unsigned long i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)//for every instance
	{
		GameObjInst* pInst = sGameObjInstList + i;

		if ((pInst->flag & FLAG_ACTIVE) == 0)//if its a non-active instance,skip
			continue;
		if (pInst->pObject->type == TYPE_ASTEROID)//if its an asteroid,check every instance again
		{
			for (unsigned long j = 0; j < GAME_OBJ_INST_NUM_MAX; j++)
			{
				GameObjInst* pInst1 = sGameObjInstList + j;

				if (((pInst1->flag & FLAG_ACTIVE) == 0) || (pInst1->pObject->type == TYPE_ASTEROID))//skip if its another asteroid or non-active instance
					continue;

				if (pInst1->pObject->type == TYPE_SHIP)//if its a ship
				{
					if (CollisionIntersection_RectRect(pInst->boundingBox, pInst->velCurr,
						pInst1->boundingBox, pInst1->velCurr))//call the collide function
					{
						--sShipLives;
						AEVec2Zero(&pInst1->posCurr);//reset the velo and pos vectors of the ship
						AEVec2Zero(&pInst1->velCurr);
						onValueChange = true;//change the bool so that the value change will show on terminal
						gameObjInstDestroy(pInst);//destroy the asteroid
						if (sShipLives <= 0)//if its zero lives
						{
							gameover = true;//game over is true
						}
						GameStateAsteroidsCreate();//create another asteroid to replace the destroyed one
					}
				}
				if (pInst1->pObject->type == TYPE_BULLET)//if its a bullet
				{
					if (CollisionIntersection_RectRect(pInst->boundingBox, pInst->velCurr,
						pInst1->boundingBox, pInst1->velCurr))
					{
						sScore += 100;
						onValueChange = true;
						gameObjInstDestroy(pInst1);//destroy the bullet and asteroid
						gameObjInstDestroy(pInst);
						for (unsigned int k = 0; k < 2; k++)//create 2 instances of asteroid
						{
							GameStateAsteroidsCreate();
						}
						if (sScore >= 5000)gameover = true;//if it hits max score, gameover is true
					}
				}

			}
		}
	}

}
/******************************************************************************/
/*!
	Matrix function for the game state
*/
/******************************************************************************/
static void GameStateAsteroidsMatrix(void)
{
	for (unsigned long i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		GameObjInst* pInst = sGameObjInstList + i;
		AEMtx33		 trans, rot, scale;
	
		// skip non-active object
		if ((pInst->flag & FLAG_ACTIVE) == 0)
			continue;
		// Compute the scaling matrix
		AEMtx33Scale(&scale, pInst->scale, pInst->scale);
		// Compute the rotation matrix 
		AEMtx33Rot(&rot, pInst->dirCurr);
		// Compute the translation matrix
		AEMtx33Trans(&trans, pInst->posCurr.x, pInst->posCurr.y);
		// Concatenate the 3 matrix in the correct order in the object instance's "transform" matrix
		AEMtx33Concat(&pInst->transform, &rot, &scale);
		AEMtx33Concat(&(pInst->transform), &trans, &pInst->transform);
	}
}
/******************************************************************************/
/*!
	"Update" function of this state
*/
/******************************************************************************/
void GameStateAsteroidsUpdate(void)
{
		// =========================
		// update according to input
		// =========================
		if (gameover == false)//if its not game over
		{
			GameStateAsteroidsInput();
		}

		// ======================================================
		// update physics and wrapping of all active game object instances
		// ======================================================
		GameStateAsteroidsPhysics();
		GameStateAsteroidsWrap();

		// ====================
		// check for collision
		// ====================
		if (gameover == false)//if its not game over
		{
			GameStateAsteroidsCollision();
		}
		// =====================================
		// calculate the matrix for all objects
		// =====================================
		GameStateAsteroidsMatrix();
}

/******************************************************************************/
/*!
	"Draw" function of this state
*/
/******************************************************************************/
void GameStateAsteroidsDraw(void)
{

	AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);//set the render to have textures
	AEGfxSetBlendMode(AE_GFX_BM_BLEND);
	AEGfxSetTransparency(1.0f);//set to opaque
	AEGfxSetTintColor(1.0f, 1.0f, 1.0f, 1.0f);//no tint color
	// draw all object instances in the list
	for (unsigned long i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		GameObjInst * pInst = sGameObjInstList + i;
		// skip non-active object
		if ((pInst->flag & FLAG_ACTIVE)==0)
			continue;
		// Set the current object instance's Texture using "AEGfxTextureSet" 
			AEGfxTextureSet(pInst->pObject->pTex, 0, 0);
		// Set the current object instance's transform matrix using "AEGfxSetTransform"
			AEGfxSetTransform(pInst->transform.m);
		// Draw the shape used by the current object instance using "AEGfxMeshDraw"
			AEGfxMeshDraw(pInst->pObject->pMesh, AE_GFX_MDM_TRIANGLES);
	}
	
	//You can replace this condition/variable by your own data.
	//The idea is to display any of these variables/strings whenever a change in their value happens
	GameStateAsteroidsScore();
}

/******************************************************************************/
/*!
	"Free" function of this state
*/
/******************************************************************************/
void GameStateAsteroidsFree(void)
{
	// kill all object instances in the array using "gameObjInstDestroy"
	for (unsigned long i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		GameObjInst* pInst = sGameObjInstList + i;

		gameObjInstDestroy(pInst);	
	}
	gameover = false;//reset the gameover bool
	AEGfxDestroyFont(Text);//free the font
}

/******************************************************************************/
/*!
	"Unload" function of this state
*/
/******************************************************************************/
void GameStateAsteroidsUnload(void)
{
	// free all mesh data (shapes) of each object using "AEGfxMeshFree"

	for (unsigned long i = 0; i < GAME_OBJ_NUM_MAX; i++)
	{
		GameObj* pObj = sGameObjList + i;
		if (pObj->pMesh )
			AEGfxMeshFree(pObj->pMesh);
		if(pObj->pTex)
			AEGfxTextureUnload(pObj->pTex);
	}
}

/******************************************************************************/
/*!
	Function to initialise each entity in the state 
*/
/******************************************************************************/
GameObjInst * gameObjInstCreate(unsigned long type, 
							   float scale, 
							   AEVec2 * pPos, 
							   AEVec2 * pVel, 
							   float dir)
{
	AEVec2 zero;
	AEVec2Zero(&zero);

	AE_ASSERT_PARM(type < sGameObjNum);
	
	// loop through the object instance list to find a non-used object instance
	for (unsigned long i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		GameObjInst * pInst = sGameObjInstList + i;

		// check if current instance is not used
		if (pInst->flag == 0)
		{
			// it is not used => use it to create the new instance
			pInst->pObject	= sGameObjList + type;
			pInst->flag		= FLAG_ACTIVE;
			pInst->scale	= scale;
			pInst->posCurr	= pPos ? *pPos : zero;
			pInst->velCurr	= pVel ? *pVel : zero;
			pInst->dirCurr	= dir;
			
			// return the newly created instance
			return pInst;
		}
	}

	// cannot find empty slot => return 0
	return 0;
}

/******************************************************************************/
/*!
	Function to destroy each entity in the state 
*/
/******************************************************************************/
void gameObjInstDestroy(GameObjInst * pInst)
{
	// if instance is destroyed before, just return
	if (pInst->flag == 0)
		return;

	// zero out the flag
	pInst->flag = 0;
}