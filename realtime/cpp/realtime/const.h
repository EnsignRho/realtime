//////////
//
// const.h
//
//////
// Version 1.00
// Copyright (c) 2013 by Rick C. Hodgin
//////
// Last update:
//		Mar.04.2013
//////
// Change log:
//		Mar.04.2013 - Initial creation
//////
//
// This software is released as Liberty Software under a Repeat License, as governed
// by the Public Benefit License v1.0 or later (PBL).
//
// You are free to use, copy, modify and share this software. However, it can only
// be released under the PBL version indicated, and every project must include a copy
// of the pbl.txt document for its version as is at http://www.libsf.org/licenses/.
//
// For additional information about this project, or to view the license, see:
//
// http://www.libsf.org/
// http://www.libsf.org/licenses/
// https://github.com/RickCHodgin/resourcex
//
// Thank you. And may The Lord bless you richly as you lift up your life, your
// talents, your gifts, your praise, unto Him. In Jesus' name I pray. Amen.
//
//////////




//////////
// Constants
//////
	const float _PI6					= 0.5235987756f;
	const float _PI3					= 1.0471975512f;
	const float	_PI2					= 1.5707963268f;
	const float	_2PI_3					= 2.0943951024f;
	const float	_PI						= 3.1415926536f;
	const float	_4PI_3					= 4.1887902048f;
	const float _3PI_2					= 4.7123889804f;
	const float	_2PI					= 6.2831853072f;

	const float _OUTER_TICK				= 0.72f;
	const float _INNER_TICK_SMALL		= 0.67f;
	const float _INNER_TICK_MEDIUM		= 0.62f;
	const float _INNER_TICK_BIG			= 0.58f;
	const float _INNER_RADIUS_GRAY		= 0.40f;
	const float _INNER_RADIUS			= 0.35f;
	const float _INNER_RADIUS_FRAME1	= 0.05f;
	const float _INNER_RADIUS_FRAME2	= 0.35f;
	const float _OUTER_RADIUS			= 0.75f;
	const float _OUTER_RADIUS_FRAME1	= 0.71f;
	const float _OUTER_RADIUS_FRAME2	= 0.66f;
	const float _TEXT_RADIUS			= 0.90f;
	const float _NEEDLE_RADIUS			= 0.70f;
	const float _INNER_OSCILLATION		= 5.0f;
	const float _OUTER_OSCILLATION		= 1.0f;

	const int	_TYPE_GAUGE				= 0;
	const int	_TYPE_GRAPH				= 1;
	const int	_TYPE_MOVER				= 2;
	const int	_TYPE_MAX				= 2;

	const int	_LOW_RANGE				= 0;
	const int	_MID_RANGE				= 1;
	const int	_HIGH_RANGE				= 2;

	// Used in mover to indicate the snap direction
	const int	_DIRECTION_NORTH		= 0;
	const int	_DIRECTION_SOUTH		= 1;
	const int	_DIRECTION_WEST			= 2;
	const int	_DIRECTION_EAST			= 3;
	const int	_DIRECTION_DROP			= 4;

	// Used to indicate the identified components within a
	const float	_MIDDLE_WIDTH			= 0.60f;
	const float	_MIDDLE_HEIGHT			= 0.40f;
	const float	_EDGE_WIDTH				= 0.20f;
	const float	_EDGE_HEIGHT			= 0.30f;
