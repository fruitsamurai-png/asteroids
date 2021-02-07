/******************************************************************************/
/*!
\file		GameState_Asteroids.cpp
\author 	DigiPen
\par    	email: digipen\@digipen.edu
\date   	January 01, 20xx
\brief

Copyright (C) 20xx DigiPen Institute of Technology.
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
const float			SHIP_SIZE				= 16.0f;		// ship size
const float			SHIP_ACCEL_FORWARD		= 60.0f;		// ship forward acceleration (in m/s^2)
const float			SHIP_ACCEL_BACKWARD		= 60.0f;		// ship backward acceleration (in m/s^2)
const float			SHIP_ROT_SPEED			= (2.0f * PI);	// ship rotation speed (degree/second)

const float			BULLET_SPEED			= 10.0f;		// bullet speed (m/s)
const unsigned int ASTEROID_NUM = 4;
const float			ASTEROID_SIZE			= 64.0f;
const float			ASTEROID_BASE			= 10.0f;
const float			ASTEROID_SPEED			= 20.0f;

const float			BULLET_SIZE				= 10.0f;
extern float		g_dt;
extern double		g_appTime;
bool				input=false;
// -----------------------------------------------------------------------------
enum TYPE
{
	// list of game object types
	TYPE_SHIP = 0, 
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
	//AEGfxTexture*		pTex;
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

	//void				(*pfUpdate)(void);
	//void				(*pfDraw)(void);
};

/******************************************************************************/
/*!
	Static Variables
*/
/******************************************************************************/

// list of original object
static GameObj				sGameObjList[GAME_OBJ_NUM_MAX];				// Each element in this array represents a unique game object (shape)
static unsigned long		sGameObjNum;								// The number of defined game objects
AEVec2						accCurr;

// list of object instances
static GameObjInst			sGameObjInstList[GAME_OBJ_INST_NUM_MAX];	// Each element in this array represents a unique game object instance (sprite)
static unsigned long		sGameObjInstNum;							// The number of used game object instances

// pointer to the ship object
static GameObjInst *		spShip;										// Pointer to the "Ship" game object instance

//pointer to the init asteroid object
static GameObjInst*			asteroid[ASTEROID_NUM]; 

static GameObjInst*			bullet;

// number of ship available (lives 0 = game over)
static long					sShipLives;									// The number of lives left

// the score = number of asteroid destroyed
static unsigned long		sScore;										// Current score
// ---------------------------------------------------------------------------

// functions to create/destroy a game object instance
GameObjInst *		gameObjInstCreate (unsigned long type, float scale, 
											   AEVec2 * pPos, AEVec2 * pVel, float dir);
void					gameObjInstDestroy(GameObjInst * pInst);


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

	// =====================
	// create the ship shape
	// =====================

	pObj		= sGameObjList + sGameObjNum++;
	pObj->type	= TYPE_SHIP;

	AEGfxMeshStart();
	AEGfxTriAdd(
		-0.5f,  0.5f, 0xFFFF0000, 1.0f, 1.0f,
		-0.5f, -0.5f, 0xFFFF0000, 1.0f, 0.0f,
		 0.5f,  0.0f, 0xFFFFFFFF, 0.0f, 1.0f );

	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG(pObj->pMesh, "fail to create object!!");
	//pObj->pTex = AEGfxTextureLoad("../Resources/Art/ship.png");
	//AE_ASSERT_MESG(pObj->pTex, "fail to create object!!");

	// =======================
	// create the bullet shape
	// =======================
	pObj = sGameObjList + sGameObjNum++;
	pObj->type = TYPE_BULLET;

	AEGfxMeshStart();
	AEGfxTriAdd(
		 -0.5f, 0.25f, 0xFFFF0000, 0.0f, 0.0f,//top left
		-0.5f, -0.25f, 0xFFFF0000, 0.0f, 0.0f,//bottom left
		0.5f, 0.25f, 0xFFFFFFFF, 0.0f, 0.0f);//top right
	AEGfxTriAdd(
		0.5f,0.25f, 0xFFFF0000, 0.0f, 0.0f,//top right
		-0.5f, -0.25f, 0xFFFF0000, 0.0f, 0.0f,//bottom left
		0.5f, -0.25f, 0xFFFFFFFF, 0.0f, 0.0f);//bottom right

	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG(pObj->pMesh, "fail to create object!!");
	


	// =========================
	// create the asteroid shape
	// =========================
	pObj = sGameObjList + sGameObjNum++;
	pObj->type = TYPE_ASTEROID;

	AEGfxMeshStart();
	AEGfxTriAdd(
		0.5f, 0.5f, 0xFFFF0000, 0.0f, 0.0f,
		-0.5f, 0.5f, 0xFFFF0000, 0.0f, 0.0f,
		-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
	AEGfxTriAdd(
		-0.5f, -0.5f, 0xFFFF0000, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFFFF0000, 0.0f, 0.0f,
		0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);

	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG(pObj->pMesh, "fail to create object!!");
	
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

	// create the main ship
	spShip = gameObjInstCreate(TYPE_SHIP, SHIP_SIZE, nullptr, nullptr, 0.0f);
	AE_ASSERT(spShip);	
	// CREATE THE INITIAL ASTEROIDS INSTANCES USING THE "gameObjInstCreate" FUNCTION
	for (unsigned int i = 0; i < ASTEROID_NUM; i++)
	{
			vel.y = ASTEROID_SPEED * AERandFloat();
			vel.x = ASTEROID_SPEED * AERandFloat();
			AEVec2Set(&pos, 0.5f * AEGetWindowWidth() * AERandFloat(), (0.5f * (AEGetWindowHeight() * AERandFloat())));
			dir = AERandFloat() * 2 * PI;
			size = (ASTEROID_SIZE * AERandFloat()+ ASTEROID_BASE);
			asteroid[i] = gameObjInstCreate(TYPE_ASTEROID, size, &pos, &vel, dir);
			AE_ASSERT(asteroid[i]);
	}
	// reset the score and the number of ships
	sScore      = 0;
	sShipLives  = SHIP_INITIAL_NUM;
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


	if (AEInputCheckCurr(AEVK_UP) || AEInputCheckCurr(AEVK_W))
	{
		// Find the velocity according to the acceleration
		AEVec2Set(&accCurr, cosf(spShip->dirCurr)*SHIP_ACCEL_FORWARD*g_dt, sinf(spShip->dirCurr) * SHIP_ACCEL_FORWARD * g_dt);
		AEVec2ScaleAdd(&spShip->velCurr, &accCurr, &spShip->velCurr, g_dt);

		// Limit your speed over here
		AEVec2Scale(&spShip->velCurr, &spShip->velCurr, 0.99);
	}
	if (AEInputCheckCurr(AEVK_DOWN)|| AEInputCheckCurr(AEVK_S))
	{
		// Find the velocity according to the decceleration
		AEVec2Set(&accCurr, -cosf(spShip->dirCurr) * SHIP_ACCEL_BACKWARD * g_dt, -sinf(spShip->dirCurr) * SHIP_ACCEL_BACKWARD * g_dt);
		AEVec2ScaleAdd(&spShip->velCurr, &accCurr, &spShip->velCurr, g_dt);

		// Limit your speed over here
		AEVec2Scale(&spShip->velCurr, &spShip->velCurr, 0.99);
	}

	if (AEInputCheckCurr(AEVK_LEFT) || AEInputCheckCurr(AEVK_A))
	{
		spShip->dirCurr += SHIP_ROT_SPEED * (float)(AEFrameRateControllerGetFrameTime ());
		spShip->dirCurr =  AEWrap(spShip->dirCurr, -PI, PI);
	}
	
	if (AEInputCheckCurr(AEVK_RIGHT) || AEInputCheckCurr(AEVK_D))
	{
		spShip->dirCurr -= SHIP_ROT_SPEED * (float)(AEFrameRateControllerGetFrameTime ());
		spShip->dirCurr =  AEWrap(spShip->dirCurr, -PI, PI);
		
	}

	// Shoot a bullet if space is triggered (Create a new object instance)
	AEVec2 angle;
	if (AEInputCheckTriggered(AEVK_SPACE))
	{
		// Get the bullet's direction according to the ship's direction
			AEVec2Set(&angle, cosf(spShip->dirCurr)*BULLET_SPEED, sinf(spShip->dirCurr) * BULLET_SPEED);

		// Set the velocity
		// Create an instance
			bullet= gameObjInstCreate(TYPE_BULLET, BULLET_SIZE, &spShip->posCurr, &angle, spShip->dirCurr);
	}

	// ======================================================
	// update physics of all active game object instances
	//	-- Positions are updated here with the computed velocity
	//  -- Get the bounding rectangle of every active instance:
	//		boundingRect_min = -BOUNDING_RECT_SIZE * instance->scale + instance->pos
	//		boundingRect_max = BOUNDING_RECT_SIZE * instance->scale + instance->pos
	// ======================================================
	AEVec2 added;
	for (unsigned long i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		GameObjInst* pInst = sGameObjInstList + i;
		// skip non-active object
		if ((pInst->flag & FLAG_ACTIVE) == 0)
			continue;
		if (pInst->pObject->type == TYPE_SHIP)
		{
			AEVec2Add(&pInst->posCurr, &pInst->posCurr, &pInst->velCurr);
		}
		if (pInst->pObject->type == TYPE_ASTEROID)
		{
			AEVec2Set(&added, -cosf(pInst->dirCurr), sinf(pInst->dirCurr));
			AEVec2Add(&pInst->posCurr, &pInst->posCurr, &added);//YOU MAY NEED TO CHANGE/REPLACE THIS LINE
		}
		if (pInst->pObject->type == TYPE_BULLET)
		{
			AEVec2Set(&angle, cosf(pInst->dirCurr) * BULLET_SPEED, sinf(pInst->dirCurr) * BULLET_SPEED);
			AEVec2Add(&pInst->posCurr, &pInst->posCurr, &angle);
		}
		AEVec2Set(&pInst->boundingBox.min, -0.5 * pInst->scale, -0.5 * pInst->scale);
		AEVec2Add(&pInst->boundingBox.min, &pInst->posCurr, &pInst->boundingBox.min);
		AEVec2Set(&pInst->boundingBox.max, -0.5 * pInst->scale, -0.5 * pInst->scale);
		AEVec2Add(&pInst->boundingBox.max, &pInst->posCurr, &pInst->boundingBox.max);
	}

	// ===================================
	// update active game object instances
	// Example:
	//		-- Wrap specific object instances around the world (Needed for the assignment)
	//		-- Removing the bullets as they go out of bounds (Needed for the assignment)
	//		-- If you have a homing missile for example, compute its new orientation 
	//			(Homing missiles are not required for the Asteroids project)
	//		-- Update a particle effect (Not required for the Asteroids project)
	// ===================================
	for (unsigned long i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		GameObjInst * pInst = sGameObjInstList + i;

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
			pInst->posCurr.x= AEWrap(pInst->posCurr.x, AEGfxGetWinMinX()-ASTEROID_BASE,
														AEGfxGetWinMaxX() + ASTEROID_BASE);
			pInst->posCurr.y = AEWrap(pInst->posCurr.y, AEGfxGetWinMinY()-ASTEROID_BASE,
														AEGfxGetWinMaxY()-ASTEROID_BASE);
		}

		// remove bullets that go out of bounds
		if (pInst->pObject->type == TYPE_BULLET)
		{
			if ((pInst->boundingBox.max.x> AEGfxGetWinMaxX() && pInst->boundingBox.max.y > AEGfxGetWinMaxY())||
			(pInst->boundingBox.min.x < AEGfxGetWinMinX() && pInst->boundingBox.min.y < AEGfxGetWinMinY()))
			{
				gameObjInstDestroy(pInst);
			}
		}
	}

	for (unsigned long i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		GameObjInst* pInst = sGameObjInstList + i;

		if ((pInst->flag & FLAG_ACTIVE) == 0)
			continue;
		if (pInst->pObject->type == TYPE_ASTEROID)
		{
			for (unsigned long j = i + 1; j < GAME_OBJ_INST_NUM_MAX; j++)
			{
				GameObjInst* pInst1 = sGameObjInstList + j;

				if (((pInst1->flag & FLAG_ACTIVE) == 0))
				continue;
				if (pInst1->pObject->type == TYPE_ASTEROID)
					continue;

				if (pInst1->pObject->type == TYPE_SHIP)
				{
					if (CollisionIntersection_RectRect(pInst->boundingBox, pInst->velCurr,
													pInst1->boundingBox, pInst1->velCurr))
					{
						sShipLives--;
						AEVec2Zero(&pInst1->posCurr);
					}
				}
				else if (pInst1->pObject->type == TYPE_BULLET)
				{
					if (CollisionIntersection_RectRect(pInst->boundingBox, pInst->velCurr,
						pInst1->boundingBox, pInst1->velCurr))
					{
						sScore += 100;
						gameObjInstDestroy(pInst);

					}
				}

			}
		}
	}
	// ====================
	// check for collision
	// ====================
	
	/*
	for each object instance: oi1
		if oi1 is not active
			skip

		if oi1 is an asteroid
			for each object instance oi2
				if(oi2 is not active or oi2 is an asteroid)
					skip

				if(oi2 is the ship)
					Check for collision between ship and asteroids (Rectangle - Rectangle)
					Update game behavior accordingly
					Update "Object instances array"
				else
				if(oi2 is a bullet)
					Check for collision between ship and asteroids (Rectangle - Rectangle)
					Update game behavior accordingly
					Update "Object instances array"
	*/



	// =====================================
	// calculate the matrix for all objects
	// =====================================

	for (unsigned long i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		GameObjInst * pInst = sGameObjInstList + i;
		AEMtx33		 trans, rot, scale;
		//AEMtx33 result;
		/*UNREFERENCED_PARAMETER(trans);
		UNREFERENCED_PARAMETER(rot);
		UNREFERENCED_PARAMETER(scale);*/

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
		AEMtx33Concat(&pInst->transform,&rot, &scale);
		AEMtx33Concat(&(pInst->transform), &trans, &pInst->transform);
	}
}

/******************************************************************************/
/*!
	
*/
/******************************************************************************/
void GameStateAsteroidsDraw(void)
{
	char strBuffer[1024];
	
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
			AEGfxTextureSet(NULL,0,0);
	// draw all object instances in the list
	for (unsigned long i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		GameObjInst * pInst = sGameObjInstList + i;
		// skip non-active object
		if ((pInst->flag & FLAG_ACTIVE)==0)
			continue;
		
			AEGfxSetTransform(pInst->transform.m);
		// Set the current object instance's transform matrix using "AEGfxSetTransform"
		// Draw the shape used by the current object instance using "AEGfxMeshDraw"
			AEGfxSetTintColor(1.0f, 1.0f, 1.0f, 1.0f);
			AEGfxMeshDraw(pInst->pObject->pMesh, AE_GFX_MDM_TRIANGLES);
			AEGfxSetTransparency(1.0f);
	}

	//You can replace this condition/variable by your own data.
	//The idea is to display any of these variables/strings whenever a change in their value happens
	static bool onValueChange = true;
	if(onValueChange)
	{
		sprintf_s(strBuffer, "Score: %d", sScore);
		//AEGfxPrint(10, 10, (u32)-1, strBuffer);
		printf("%s \n", strBuffer);

		sprintf_s(strBuffer, "Ship Left: %d", sShipLives >= 0 ? sShipLives : 0);
		//AEGfxPrint(600, 10, (u32)-1, strBuffer);
		printf("%s \n", strBuffer);

		// display the game over message
		if (sShipLives < 0)
		{
			//AEGfxPrint(280, 260, 0xFFFFFFFF, "       GAME OVER       ");
			printf("       GAME OVER       \n");
		}

		onValueChange = false;
	}
}

/******************************************************************************/
/*!
	
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
}

/******************************************************************************/
/*!
	
*/
/******************************************************************************/
void GameStateAsteroidsUnload(void)
{
	// free all mesh data (shapes) of each object using "AEGfxTriFree"

	for (unsigned long i = 0; i < GAME_OBJ_NUM_MAX; i++)
	{
		GameObjInst* pInst = sGameObjInstList + i;
		if (pInst->pObject->pMesh != nullptr)
		{
			AEGfxMeshFree(pInst->pObject->pMesh);
		}
	}
}

/******************************************************************************/
/*!
	
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