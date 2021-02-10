/******************************************************************************/
/*!
\file		GameState_Asteroids.h
\author 	Keith Chng
\par    	email: n.chng\@digipen.edu
\date   	10/2/21
\brief		This is the main game state file where it runs the game asteroids. 

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#ifndef CS230_GAME_STATE_PLAY_H_
#define CS230_GAME_STATE_PLAY_H_

// ---------------------------------------------------------------------------

void GameStateAsteroidsLoad(void);
void GameStateAsteroidsInit(void);
void GameStateAsteroidsUpdate(void);
void GameStateAsteroidsDraw(void);
void GameStateAsteroidsFree(void);
void GameStateAsteroidsUnload(void);

// ---------------------------------------------------------------------------

#endif // CS230_GAME_STATE_PLAY_H_


