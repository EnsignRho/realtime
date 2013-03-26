//////////
//
// globals.h
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




SWindow*			gsRootWind				= NULL;				// Link-list of windows this DLL is maintaining
ATOM				gnAtom					= NULL;				// Used one time to register the class
bool				glTestMode				= false;			// When enabled, all functions simply immediately return
int					gnNextUniqueId			= 0x08101969;		// Begin at Rick C. Hodgin's birthday
CRITICAL_SECTION	gcsUniqueId;								// Used for synchronous access to the unique id
int					gnDrawSnaps				= 0;				// Draw the snaps around objects?  0=no, 1=highlighted, 2=all
