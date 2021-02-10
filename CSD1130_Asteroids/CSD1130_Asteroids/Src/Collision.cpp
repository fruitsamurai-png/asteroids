/******************************************************************************/
/*!
\file		Collision.cpp
\author 	Keith Chng
\par    	email: n.chng\@digipen.edu
\date   	10/2/21
\brief	AABB collision function to see if 2 instances/entity are colliding statically or dynamically

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
 */
/******************************************************************************/

#include "main.h"

/**************************************************************************/
/*!

	*/
/**************************************************************************/
bool CollisionIntersection_RectRect(const AABB & aabb1, const AEVec2 & vel1, 
									const AABB & aabb2, const AEVec2 & vel2)
{
	if (aabb1.max.x<aabb2.min.x || aabb1.min.x>aabb2.max.x ||
		aabb1.max.y<aabb2.min.y || aabb1.min.y>aabb2.max.y)//condition for static collision
	{
		return false;//if the second min point is more than the first max point or 
	}
	f32 tFirst = 0;
	f32 tLast = g_dt;
	AEVec2 vb;//resultant vector
	AEVec2 v1 = vel1;
	AEVec2 v2 = vel2;
	AEVec2Sub(&vb, &v2, &v1);
	if (vb.x < 0)//case 1 4 for x axis ,negative vel which means going left
	{
		if (aabb1.min.x > aabb2.max.x)return false;
		if (aabb1.max.x < aabb2.min.x)
		{
			tFirst = AEMax((aabb1.max.x - aabb2.min.x) / vb.x,tFirst);
		}
		if (aabb1.min.x < aabb2.max.x)
		{
			tLast = AEMin((aabb1.min.x - aabb2.max.x) / vb.x,tLast);
		}
	}
	if (vb.x > 0)//case 2 3 for x axis,positive vel which means going right
	{
		if (aabb1.max.x < aabb2.min.x)return false;
		if (aabb1.min.x > aabb2.max.x)//if the min hits the second obj max
		{
			tFirst = AEMax((aabb1.min.x - aabb2.max.x) / vb.x,tFirst);
		}
		if (aabb1.max.x > aabb2.min.x)//if the max hits the second obj min
		{
			tLast = AEMin((aabb1.max.x - aabb2.min.x) / vb.x,tLast);
		}
	}
	if (vb.y < 0)//case 1 4 for y axis,negative vel which means going down
	{
		if (aabb1.min.y > aabb2.max.y)return false;
		if (aabb1.max.y < aabb2.min.y)
		{
			tFirst = AEMax((aabb1.max.y - aabb2.min.y) / vb.y, tFirst);
		}
		if (aabb1.min.y < aabb2.max.y)
		{
			tLast = AEMin((aabb1.min.y - aabb2.max.y) / vb.y, tLast);
		}
	}
	if (vb.y > 0)//case 2 3 for y axis,positive vel which means going up
	{
		if (aabb1.max.y < aabb2.min.y)return false;
		if (aabb1.min.y > aabb2.max.y)
		{
			tFirst = AEMax((aabb1.min.y - aabb2.max.y) / vb.y, tFirst);
		}
		if (aabb1.max.y > aabb2.min.y)
		{
			tLast = AEMin((aabb1.max.y - aabb2.min.y) / vb.y, tLast);
		}
	}

	if (tFirst > tLast)return false;//
		
	return true;
}