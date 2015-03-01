//////////
//
// realtime.h
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




// Note: To view properly, set tabs to 4 characters.
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>


//////////
// Use of builder
//////
	#include "\libsf\utils\common\cpp\common_types.h"
	#include "\libsf\utils\common\cpp\builder.h"
	#include "\libsf\utils\common\cpp\builder.cpp"


//////////
// Exporting or importing
//////
	#ifdef REALTIME_EXPORTS
		#define REALTIME_API __declspec(dllexport)
	#else
		#define REALTIME_API __declspec(dllimport)
	#endif


//////////
// Required include files
//////
	#include "const.h"						// Constants
	#include "structs.h"					// Structures
	#include "globals.h"					// Global variables
	#include "defs.h"						// Forward declarations


//////////
// Include an overlay.bmp file as a source file
//////
	#include "overlay.h"					// cgc_Overlay
