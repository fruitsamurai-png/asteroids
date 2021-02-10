/******************************************************************************/
/*!
\file		Collision.h
\author 	Keith Chng
\par    	email: n.chng\@digipen.edu
\date   	10/2/21
\brief		Header file for AABB collision function to see if 2 instances/entity are colliding
			statically or dynamically

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#ifndef CS230_COLLISION_H_
#define CS230_COLLISION_H_

#include "AEEngine.h"

/**************************************************************************/
/*!
	Struct for the AABB collision box
	*/
/**************************************************************************/
struct AABB
{
	AEVec2	min;
	AEVec2	max;
};

bool CollisionIntersection_RectRect(const AABB & aabb1, const AEVec2 & vel1, 
									const AABB & aabb2, const AEVec2 & vel2);


#endif // CS230_COLLISION_H_