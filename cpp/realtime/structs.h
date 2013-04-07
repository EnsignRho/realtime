//////////
//
// structs.h
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




// Generic data point structure to use
struct SDataPoint
{
	SDataPoint*		next;									// Pointer to the next entry in the link list
	float			value;									// The value of this data point
};

struct SBitmap
{
	HBITMAP			hbmp;									// the bitmap we create which holds the 
	BITMAPINFO		bmi;									// the bitmap's vital statistics (except actualWidth)
	int				actualWidth;							// Actual width for each bits row (x3 value rounded up to nearest dword)
	char*			bits;									// Bits of the DIBSection
	HDC				hdc;									// hdc of the frame bitmap
};

struct SMouse
{
	bool			isValid;								// Is this a valid set of data?
	int				x;										// Mouse X coordinate
	int				y;										// Mouse Y coordinate
	bool			leftButton;								// Left button is down?
	bool			rightButton;							// Right button is down?
};

// Data unique to the graph control
struct SGraph
{
	// For general graph coloring
	int			bar1Rgb;									// For alternating graph color bars, bar 1 color
	int			bar2Rgb;									// For alternating graph color bars, bar 2 color
	int			dataRgb;									// For data points, data point color
	int			dataPointThickness;							// For data points, how thick should the data point be (vertically)

	// For rendering the graduation scale
	int			textRgb;									// For graduation scale, color to use for font rendering
	char*		fontName;									// For graduation scale, name of font to use
	int			fontSize;									// For graduation scale, size of font to use
	HFONT		textHFont;									// Windows font
	int			integers;									// Number of integers to use in graduation scale, of the form STR(n, integers, decimals)
	int			decimals;									// Number of decimals to use in graduation scale, of the form STR(n, integers, decimals)

	// Overlay grid items
	int			gridVisible;								// Should the grid be visible?
	float		gridCenter;									// Where is the grid's "center line" drawn?
	float		gridSquareSize;								// How big should the squares be vertically (which then also determines their size horizontally)
	int			gridCenterRgb;								// What color should the "center line" be?
	int			gridLinesRgb;								// What color should the other grid lines be?
	float		gridAlpha;									// What alpha level should it be (0.0=transparent, 1.0=opaque)

	// Margins
	int			marginLeft;									// The left-side margin
	int			marginRight;								// The right-side margin

	// For data
	float		rangeUpper;									// Upper-limit on the range
	float		rangeLower;									// Lower-limit on the range
	int			sampleAverageCount;							// Number of points which should be averaged together before plotting
	SDataPoint* dataPoints;									// The data points to use for this rendering

	// For data range (high and low)
	int			rangeVisible;								// Should the range overlay for data point ranges be included?
	int			rangeRgb;									// What color?
	float		rangeAlpha;									// What alpha level? (0.0=transparent, 1.0=opaque)
	int			rangeAverageSamples;						// How many samples should be averaged to make this determination?
};

// Data unique to the gauge control
struct SGauge
{
	// For rendering the graduation scale
	int			textRgb;									// For graduation scale, color to use for font rendering
	char*		fontName;									// For graduation scale, name of font to use
	int			fontSize;									// For graduation scale, size of font to use
	HFONT		textHFont;									// Windows font
	HFONT		textHFontBold;								// Windows font for use in the gauge value area
	int			graduationIntegers;							// Number of integers to use in graduation scale, of the form STR(n, integers, decimals)
	int			graduationDecimals;							// Number of decimals to use in graduation scale, of the form STR(n, integers, decimals)

	// For general gauge color rendering
	int			lowRgb;										// Low rgb color to render
	int			midRgb;										// Mid rgb color to render
	int			highRgb;									// High rgb color to render
	float		scaleFactor;								// If they want the gauge to be smaller, then something like 0.9

	// For the highlighted value box
	int			highlightedIntegers;						// Number of integers to use in the highlighted value box, of the form STR(n, integers, decimals)
	int			highlightedDecimals;						// Number of decimals to use in the highlighted value box, of the form STR(n, integers, decimals)

	// Ranges to apply various colors
	bool		frameGauge;									// Should the gauge be framed with an outline of the color the needle is pointing at?
	float		rangeStart;									// Lowest value for graduation
	float		rangeEnd;									// Highest value for graduation
	float		rangePercentLow;							// Percentage to render in lowRgb color
	float		rangePercentMid;							// Percentage beyond low to render in midRgb color
	// Note: Everything above that is rendered in the highRgb color

	// Current gauge needle value
	bool		colorizeNeedle;								// Should the needle be colorized by the low/mid/high color it's pointing at?
	float		needleValue;								// Current value the needle's pointing at
	bool		snapNeedle;									// Should the needle snap to the displayed value rather than the actual value?
};

struct SMoverPos
{
	int				col;									// Column of the grid for this object
	int				row;									// Row of the grid for this object
	int				stepCount;								// Number of steps remaining to be drawn in the current animation

	// The coordinate location of where it's drawn, its rectangle
	float			x;										// X coordinate
	float			y;										// Y coordinate
	int				width;									// Width
	int				height;									// Height
	int				snapDirection;							// Is _DIRECTION_NORTH[SOUTH,WEST,EAST]

	// How far the x position should move each time
	float			xStep;									// X adjustment step per iteration coordinate
	float			yStep;									// Y adjustment step per iteration coordinate
};

struct SMoverObj
{
	SMoverObj*		next;									// One-way link list
	int				objectId;								// Unique ID assigned to this control
	int				visible;								// 0=no, others=yes

	// For normal and animation (real=where it really is, curr=where it is currently (if animating, then as it's moving), cand=where it will drop if the user drops at the current snap position
	SMoverPos		real;									// Where its real home is (were nothing being moved at the current time)
	SMoverPos		curr;									// Where the part is currently being drawn during animation from real to candidate position
	SMoverPos		cand;									// Where it is moving to, and where it will move to were the object to be dropped at its currently highlighted location

	// Snap coordinates surround the object
	SMoverPos		snapNorth;								// Above this object
	SMoverPos		snapSouth;								// Below this object
	SMoverPos		snapWest;								// To the left of this object
	SMoverPos		snapEast;								// To the right of this object
	SMoverPos		snapDrop;								// Snap to an item as it's being dropped on

	// Events this object responds to (by default, responds to no events, 0=no, others=yes)
	int				eventClick;								// Responds to left-click events
	int				eventRightClick;						// Responds to right-click events
	int				eventMouseMove;							// Responds to mouse move events
	int				eventMouseEnter;						// Responds to mouse enter events
	int				eventMouseLeave;						// Responds to mouse leave events
	int				eventDragStart;							// Responds to drag start events
	int				eventDragMove;							// Responds to drag move events (in lieu of mouseMove events while dragging)
	int				eventDragAbort;							// Responds to drag abort events (drag was not completed, dropped in some non-droppable area)
	int				eventDragDrop;							// Responds to drag drop events (dropped in a droppable area)
	int				eventHover;								// Responds to hover events

	// Settings for the object
	int				col;									// Column position
	int				row;									// Row position
	int				draggable;								// is this item draggable?  If 0, then is a fixed item
	int				acceptsDrops;							// Does this item accept drops made onto it?
	int				copiesOnDrop;							// When a drop is made, does the item copy?  If 0 then moves.
	int				callbackCode;							// If non-zero, then callback code to hwnd parent to tell them of the event signal

	// The bitmap being drawn there
	SBitmap			bmp;									// Bitmap data
	// If disposition objects are indicated, we'll use those at various times
	int				dispNormal;								// Normal rendering bitmap
	int				dispOver;								// When the mouse is over the object rendering bitmap
	int				dispDown;								// When the mouse is pressed down on the object rendering bitmap
	int				dispHover;								// When hovering over the object rendering bitmap
	int				dispDragging;							// When dragging the object rendering bitmap
};

struct SMover
{
	int				marginVertical;							// The vertical margin
	int				marginHorizontal;						// The horizontal margin

	// For animation
	SMoverPos		snap;									// The area we've snapped into
	SMoverPos		snapReal;								// The position we're snapping to formally
	bool			isAnimating;							// Whenever we are animating, set to true
	UINT_PTR		timer;									// The timer used for computing the animation locations while moving
	
	// Iterative parts used for the mover
	SMoverObj*		firstObject;							// A link list of mover object parts which can be moved (if they are set to be draggable), but are placed at a particular location
	SMoverObj*		dragObject;								// If something is being dragged, it is this object
	int				maxCol;									// For all objects, the maximum column
	int				maxRow;									// For all objects, the maximum row
	int				maxWidth;								// For all objects, the maximum width
	int				maxHeight;								// For all objects, the maximum height
};


// Structure for each defined window
struct SWindow
{
	SWindow*		next;									// Pointer to the next entry in the link list
	RECT			rcParent;								// Rectangular position of this window on the parent
	RECT			rcThis;									// Rectangular coordinate for this window
	int				type;									// 0=gauge, 1=graph
	HANDLE			threadHandle;							// Handle to this thread
	DWORD			threadId;								// ID number for the thread
	bool			threadBusy;								// For rendering, if busy, the latest redraw request is ignored
	int				threadCalls;							// Reset at each iteration, indicates how many times we were asked to re-render while already rendering

	// Mouse information
	SMouse			mouseCurrent;							// Current mouse position
	SMouse			mouseLast;								// Last mouse position, used to compare what's changed for signaling events
	SMouse			mouseDown;								// Where the mouse button went down

	// Handles
	HWND			hwndParent;								// the original thisForm.hwnd value
	HWND			hwnd;									// this actual window to render to
	HDC				hdc1;									// hdc of this window
	// Implied hbmp1, bmi1, etc., that is in the hdc1 device context

	// Bitmap stuff for rendering each frame
	SBitmap			bmpMain;								// Bitmap data

	// Bitmap stuff for background
	SBitmap			bmpBackground;							// Bitmap data

	// Gradient coloring
	int				ulRgb;									// Upper-left color
	int				urRgb;									// Upper-right color
	int				lrRgb;									// Lower-right color
	int				llRgb;									// Lower-left color

	// For subclassing, the old window procedure
	WNDPROC			oldWndProcAddress;

	// Data unique to the identity of each instance
	union
	{
		SGraph		graph;
		SGauge		gauge;
		SMover		mover;
	};
};

// Structure for accessing RGB data within the DIB section
struct SRGB
{
	unsigned char	blu;
	unsigned char	grn;
	unsigned char	red;
};
