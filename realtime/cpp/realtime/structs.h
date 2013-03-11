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

	// Handles
	HWND			hwndParent;								// the original thisForm.hwnd value
	HWND			hwnd;									// this actual window to render to
	HDC				hdc;									// hdc of this window
	HDC				hdc2;									// hdc of the bitmap

	// Bitmap stuff
	HBITMAP			hbmp;									// the bitmap we create
	BITMAPINFO		bmi;									// the bitmap's vital statistics (except actualWidth)
	int				actualWidth;							// Actual width for each bits row (x3 value rounded up to nearest dword)
	char*			bits;									// Bits of the DIBSection

	// Gradient coloring
	int				ulRgb;									// Upper-left color
	int				urRgb;									// Upper-right color
	int				lrRgb;									// Lower-right color
	int				llRgb;									// Lower-left color

	// For subclassing, the old window procedure
	WNDPROC			oldWndProcAddress;

	//////////
	// Graphs -- For data in drawing the window
	//////
	union
	{
	//////////
	// GRAPH
	//////
		struct {
			// For general graph coloring
			int			bar1Rgb;							// For alternating graph color bars, bar 1 color
			int			bar2Rgb;							// For alternating graph color bars, bar 2 color
			int			dataRgb;							// For data points, data point color
			int			dataPointThickness;					// For data points, how thick should the data point be (vertically)

			// For rendering the graduation scale
			int			textRgb;							// For graduation scale, color to use for font rendering
			char*		fontName;							// For graduation scale, name of font to use
			int			fontSize;							// For graduation scale, size of font to use
			HFONT		textHFont;							// Windows font
			int			integers;							// Number of integers to use in graduation scale, of the form STR(n, integers, decimals)
			int			decimals;							// Number of decimals to use in graduation scale, of the form STR(n, integers, decimals)

			// Overlay grid items
			int			gridVisible;						// Should the grid be visible?
			float		gridCenter;							// Where is the grid's "center line" drawn?
			float		gridSquareSize;						// How big should the squares be vertically (which then also determines their size horizontally)
			int			gridCenterRgb;						// What color should the "center line" be?
			int			gridLinesRgb;						// What color should the other grid lines be?
			float		gridAlpha;							// What alpha level should it be (0.0=transparent, 1.0=opaque)

			// Margins
			int			marginLeft;							// The left-side margin
			int			marginRight;						// The right-side margin

			// For data
			float		rangeUpper;							// Upper-limit on the range
			float		rangeLower;							// Lower-limit on the range
			int			sampleAverageCount;					// Number of points which should be averaged together before plotting
			SDataPoint* dataPoints;							// The data points to use for this rendering

			// For data range (high and low)
			int			rangeVisible;						// Should the range overlay for data point ranges be included?
			int			rangeRgb;							// What color?
			float		rangeAlpha;							// What alpha level? (0.0=transparent, 1.0=opaque)
			int			rangeAverageSamples;				// How many samples should be averaged to make this determination?

		} graph ;

	//////////
	// GAUGE
	//////
		struct {
			// For rendering the graduation scale
			int			textRgb;							// For graduation scale, color to use for font rendering
			char*		fontName;							// For graduation scale, name of font to use
			int			fontSize;							// For graduation scale, size of font to use
			HFONT		textHFont;							// Windows font
			HFONT		textHFontBold;						// Windows font for use in the gauge value area
			int			graduationIntegers;					// Number of integers to use in graduation scale, of the form STR(n, integers, decimals)
			int			graduationDecimals;					// Number of decimals to use in graduation scale, of the form STR(n, integers, decimals)

			// For general gauge color rendering
			int			lowRgb;								// Low rgb color to render
			int			midRgb;								// Mid rgb color to render
			int			highRgb;							// High rgb color to render
			float		scaleFactor;						// If they want the gauge to be smaller, then something like 0.9

			// For the highlighted value box
			int			highlightedIntegers;				// Number of integers to use in the highlighted value box, of the form STR(n, integers, decimals)
			int			highlightedDecimals;				// Number of decimals to use in the highlighted value box, of the form STR(n, integers, decimals)

			// Ranges to apply various colors
			bool		frameGauge;							// Should the gauge be framed with an outline of the color the needle is pointing at?
			float		rangeStart;							// Lowest value for graduation
			float		rangeEnd;							// Highest value for graduation
			float		rangePercentLow;					// Percentage to render in lowRgb color
			float		rangePercentMid;					// Percentage beyond low to render in midRgb color
															// Note: Everything above that is rendered in the highRgb color

			// Current gauge needle value
			bool		colorizeNeedle;						// Should the needle be colorized by the low/mid/high color it's pointing at?
			float		needleValue;						// Current value the needle's pointing at
			bool		snapNeedle;							// Should the needle snap to the displayed value rather than the actual value?
		} gauge;
	};
};

// Structure for accessing RGB data within the DIB section
struct SRGB
{
	unsigned char	blu;
	unsigned char	grn;
	unsigned char	red;
};
