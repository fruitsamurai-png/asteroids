/******************************************************************************/
/*!
\file		GameStateList.h
\author 	Keith Chng
\par    	email: n.chng\@digipen.edu
\date   	10/2/21
\brief

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#ifndef CS230_GAME_STATE_LIST_H_
#define CS230_GAME_STATE_LIST_H_

// ---------------------------------------------------------------------------
// game state list

enum
{
	// list of all game states 
	GS_ASTEROIDS = 0, 
	
	// special game state. Do not change
	GS_RESTART,
	GS_QUIT, 
	GS_NONE
};

// ---------------------------------------------------------------------------

#endif // CS230_GAME_STATE_LIST_H_