//////////
//
// realtime.cpp
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
#include "realtime.h"
HINSTANCE ghInstance = NULL;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
			ghInstance = hModule;
			break;
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}




//////////
//
// Called to indicate what the HWND value we'll subclass for this control, and what type it is.
//
// Input:
//		tnHwndParent				= thisForm.hwnd
//		tnType						= 0=gauge, 1=graph, others undefined
//		tnx,tnY,tnWidth,tnHeight	= Bounding rectangle on thisForm where the gauge or graph should be drawn
//		tnUlRgb...tnLlRgb			= RGB(r,g,b) colors for each of the ordinal points on the rectangle
//
// Returns:
//		int							= a handle to the new window created on thisForm
//									  Note:  Use this handle for all other tnHandle requests
//
//////
	REALTIME_API int realtime_subclass_form_as(HWND tnHwndParent, int tnType, int tnX, int tnY, int tnWidth, int tnHeight, int tnUlRgb, int tnUrRgb, int tnLrRgb, int tnLlRgb)
	{
		SWindow*	wnd;
		WNDCLASSEX	wcex;


		// Are we in test mode?
		if (glTestMode)
			return(0);

		//////////
		// Make sure our type is correct
		//////
			if (tnType != 0 && tnType != 1)
				return(-1);		// Failure


		//////////
		// Append a new window entry to our chain
		//////
			wnd = iCreateNewSWindow();
			if (!wnd)
				return(-2);		// Failure


		//////////
		// Store the type, HWND and rect
		//////
			wnd->type		= tnType;
			wnd->hwndParent = tnHwndParent;
			wnd->ulRgb		= tnUlRgb;
			wnd->urRgb		= tnUrRgb;
			wnd->lrRgb		= tnLrRgb;
			wnd->llRgb		= tnLlRgb;
			SetRect(&wnd->rcParent, tnX,	tnY,	tnX + tnWidth,	tnY + tnHeight);
			SetRect(&wnd->rcThis,	0,		0,		tnWidth,		tnHeight);


		//////////
		// Create a child window
		//////
			if (gnAtom == NULL)
			{
				memset(&wcex, 0, sizeof(wcex));
				wcex.cbSize         = sizeof(WNDCLASSEX);
				wcex.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
				wcex.lpfnWndProc    = (WNDPROC)GetWindowLong(wnd->hwndParent, GWL_WNDPROC);
				wcex.hInstance      = GetModuleHandle(NULL);
				wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
				wcex.hbrBackground  = (HBRUSH)GetStockObject(WHITE_BRUSH);
				wcex.lpszClassName  = L"realtime";
				gnAtom				= RegisterClassEx(&wcex);
			}
			wnd->hwnd	= CreateWindowEx(WS_EX_NOPARENTNOTIFY, L"realtime", L"realtime", WS_CHILD, tnX, tnY, tnWidth, tnHeight, wnd->hwndParent, NULL, GetModuleHandle(NULL), NULL);

			// Are we good?
			if (wnd->hwnd)
			{
				// Yes!
				int error	= GetLastError();
				wnd->hdc	= GetDC(wnd->hwnd);
				wnd->hdc2	= CreateCompatibleDC(wnd->hdc);


			//////////
			// Create a DIB section of the appropriate size
			//////
				memset(&wnd->bmi, 0, sizeof(wnd->bmi));
				wnd->bmi.bmiHeader.biSize			= sizeof(wnd->bmi);
				wnd->bmi.bmiHeader.biWidth			= tnWidth;
				wnd->bmi.bmiHeader.biHeight			= tnHeight;
				wnd->bmi.bmiHeader.biCompression	= 0;
				wnd->bmi.bmiHeader.biPlanes			= 1;
				wnd->bmi.bmiHeader.biBitCount		= 24;
				wnd->bmi.bmiHeader.biXPelsPerMeter	= 3270;
				wnd->bmi.bmiHeader.biYPelsPerMeter	= 3270;
				// Compute the actual width
				wnd->actualWidth					= iComputeActualWidth(&wnd->bmi.bmiHeader);
				wnd->bmi.bmiHeader.biSizeImage		= wnd->actualWidth * tnHeight;
				wnd->hbmp = CreateDIBSection(wnd->hdc, &wnd->bmi, DIB_RGB_COLORS, (void**)&wnd->bits, NULL, 0);

				// Put the bitmap into the dc2
				SelectObject(wnd->hdc2, wnd->hbmp);

				// Render the gradient fill colors
				iGradient4Fill(wnd);


			//////////
			// Draw the window
			//////
				InvalidateRect(wnd->hwnd, &wnd->rcThis, true);
				wnd->oldWndProcAddress = (WNDPROC)GetWindowLong(wnd->hwnd, GWL_WNDPROC);
				SetWindowLong(wnd->hwnd, GWL_WNDPROC, (long)&realtimeWndProc);
				ShowWindow(wnd->hwnd, SW_SHOW);
			}


		//////////
		// Indicate success or failure
		//////
			return((int)wnd->hwnd);
	}




//////////
//
// Called to un-subclass the form
//
//////
	REALTIME_API void realtime_un_subclass_form(int tnHandle)
	{
		SWindow* wnd;


		// Are we in test mode?
		if (glTestMode)
			return;

		// Make sure we have something to subclass
		if (root)
		{
			// Iterate through all our windows
			wnd = iLocateWindow(tnHandle);
			if (wnd)
			{
				// Delete this entry
				iDeleteSWindow(wnd);
				// All done!
			}
		}
	}




//////////
//
// Called to show or hide a window
//
//////
	REALTIME_API void realtime_show_or_hide(int tnHandle, int tnShow)
	{
		SWindow* wnd;


		// Are we in test mode?
		if (glTestMode)
			return;

		// Make sure we have something to subclass
		if (root)
		{
			// Iterate through all our windows
			wnd = iLocateWindow(tnHandle);
			if (wnd)
			{
				// Delete this entry
				if (tnShow == 0)
				{
					// Hiding the window
					ShowWindow(wnd->hwnd, SW_HIDE);

				} else {
					// Showing the window
					ShowWindow(wnd->hwnd, SW_SHOW);
					InvalidateRect(wnd->hwnd, 0, FALSE);
				}
			}
		}
	}




//////////
//
// A way to enter a test mode, which then doesn't do anything, except validate communication
// exists between the caller and us.
//
//////
	REALTIME_API void realtime_test_mode(int tnEnabled)
	{
		glTestMode = (tnEnabled != 0);
	}




//////////
//
// Graph functions
//
//////
	REALTIME_API void realtime_graph_setup(int tnHandle, int tnBar1Rgb, int tnBar2Rgb, int tnDataRgb, char* tcFontName, int tnFontSize, int tnTextRgb, int tnDataPointThickness, float tfRangeUpper, float tfRangeLower, int tnSampleAverageCount, int tnGraduationIntegers, int tnGraduationDecimals)
	{
		SWindow*	wnd;


		// Are we in test mode?
		if (glTestMode)
			return;

		// Grab our window
		wnd = iLocateWindow(tnHandle);
		if (wnd)
		{
			// Set or update the data
			wnd->graph.bar1Rgb				= tnBar1Rgb;
			wnd->graph.bar2Rgb				= tnBar2Rgb;

			// Do the font and graduation stuff
			wnd->graph.decimals				= tnGraduationDecimals;
			wnd->graph.integers				= tnGraduationIntegers;
			wnd->graph.textRgb				= tnTextRgb;
			wnd->graph.fontSize				= tnFontSize;
			if (iCopyStringIfDifferent(&wnd->graph.fontName, tcFontName))
				iUpdateFont(wnd);		// Update the HFONT

			// Do the data point
			wnd->graph.dataRgb				= tnDataRgb;
			wnd->graph.dataPointThickness	= tnDataPointThickness;

			// Setup the ranges
			wnd->graph.rangeUpper			= tfRangeUpper;
			wnd->graph.rangeLower			= tfRangeLower;

			// Sample average count
			wnd->graph.sampleAverageCount	= tnSampleAverageCount;
		}
	}

	REALTIME_API void realtime_graph_setup2(int tnHandle, int tnGridVisible, float tfGridCenter, float tfGridSquareSize, int tnGridCenterRgb, int tnGridLinesRgb, float tfGridAlpha, int tnMarginLeft, int tnMarginRight, int tnRangeVisible, int tnRangeRgb, float tfRangeAlpha, int tnRangeAverageSamples)
	{
		SWindow*	wnd;


		// Are we in test mode?
		if (glTestMode)
			return;

		// Grab our window
		wnd = iLocateWindow(tnHandle);
		if (wnd)
		{
			// Set or update the grid data
			wnd->graph.gridVisible			= tnGridVisible;
			wnd->graph.gridCenter			= tfGridCenter;
			wnd->graph.gridSquareSize		= tfGridSquareSize;
			wnd->graph.gridCenterRgb		= tnGridCenterRgb;
			wnd->graph.gridLinesRgb			= tnGridLinesRgb;
			wnd->graph.gridAlpha			= tfGridAlpha;

			// Range information for graph data point line
			wnd->graph.rangeVisible			= tnRangeVisible;
			wnd->graph.rangeRgb				= tnRangeRgb;
			wnd->graph.rangeAlpha			= tfRangeAlpha;
			wnd->graph.rangeAverageSamples	= tnRangeAverageSamples;

			// Margins
			wnd->graph.marginLeft			= tnMarginLeft;
			wnd->graph.marginRight			= tnMarginRight;
		}
	}

	REALTIME_API void realtime_graph_add_data_point(int tnHandle, float tfRangeUpper, float tfRangeLower, float tfFloatDataPoint)
	{
		SWindow*	wnd;


		// Are we in test mode?
		if (glTestMode)
			return;

		// Grab our window
		wnd = iLocateWindow(tnHandle);
		if (wnd)
		{
			// Store the point
			iAppendGraphDataPoint(wnd, tfRangeUpper, tfRangeLower, tfFloatDataPoint);
		}
	}

	REALTIME_API void realtime_graph_rescale_samples(int tnHandle, float tfScaleFactor)
	{
		SWindow*	wnd;
		SDataPoint*	point;


		// Are we in test mode?
		if (glTestMode)
			return;

		// Grab our window
		wnd = iLocateWindow(tnHandle);
		if (wnd && wnd->graph.dataPoints)
		{
			point = wnd->graph.dataPoints;
			while (point)
			{
				// Adjust it by the scaling factor
				point->value = point->value * tfScaleFactor;

				// Move to next point
				point = point->next;
			}
		}
	}

	REALTIME_API void realtime_graph_delete_all_samples(int tnHandle)
	{
		SWindow*	wnd;
		SDataPoint*	point;
		SDataPoint*	pointNext;


		// Are we in test mode?
		if (glTestMode)
			return;

		// Grab our window
		wnd = iLocateWindow(tnHandle);
		if (wnd && wnd->graph.dataPoints)
		{
			// Iterate through all points deleting as we go
			point					= wnd->graph.dataPoints;
			wnd->graph.dataPoints	= NULL;
			while (point)
			{
				// Grab the next point
				pointNext = point->next;

				// Delete this one
				free(point);

				// Move to next point
				point = pointNext;
			}
		}
	}

	REALTIME_API void realtime_graph_redraw(int tnHandle)
	{
		SWindow* wnd;


		// Are we in test mode?
		if (glTestMode)
			return;

		// Grab our window
		wnd = iLocateWindow(tnHandle);
		if (wnd)
		{
			// Signal a refresh
			iRenderTheGraph(wnd);
		}
	}




//////////
//
// Sets up information necessary to render the gauge
//
//////
	REALTIME_API void realtime_gauge_setup(int tnHandle, char* tcFontName, int tnFontSize, int tnTextRgb, int tnLowRgb, int tnMidRgb, int tnHighRgb, float tfRangeStart, float tfRangeEnd, float tfRangePercentLow, float tfRangePercentMid, int tnColorizeNeedle, int tnFrameGauge, int tnGraduationIntegers, int tnGraduationDecimals, int tnHighlightedIntegers, int tnHighlightedDecimals)
	{
		SWindow*	wnd;


		// Are we in test mode?
		if (glTestMode)
			return;

		// Grab our window
		wnd = iLocateWindow(tnHandle);
		if (wnd)
		{
			// Do the font
			wnd->gauge.textRgb				= tnTextRgb;
			wnd->gauge.fontSize				= tnFontSize;
			if (iCopyStringIfDifferent(&wnd->gauge.fontName, tcFontName))
				iUpdateFont(wnd);		// Update the HFONT

			// Set or update the data
			wnd->gauge.lowRgb				= tnLowRgb;
			wnd->gauge.midRgb				= tnMidRgb;
			wnd->gauge.highRgb				= tnHighRgb;

			// Do the data point
			wnd->gauge.frameGauge			= (tnFrameGauge != 0);
			wnd->gauge.rangeStart			= tfRangeStart;
			wnd->gauge.rangeEnd				= tfRangeEnd;
			wnd->gauge.rangePercentLow		= tfRangePercentLow;
			wnd->gauge.rangePercentMid		= tfRangePercentMid;

			// Graduation information
			// For the graduation
			wnd->gauge.graduationIntegers	= tnGraduationIntegers;
			wnd->gauge.graduationDecimals	= tnGraduationDecimals;
			// For the highlighted value box
			wnd->gauge.highlightedIntegers	= tnHighlightedIntegers;
			wnd->gauge.highlightedDecimals	= tnHighlightedDecimals;

			// Should the needle be colorized?
			wnd->gauge.colorizeNeedle		= (tnColorizeNeedle != 0);
		}
	}

	REALTIME_API void realtime_gauge_setup2(int tnHandle, int tnSnapToDisplayedValue, float tfScaleFactor)
	{
		SWindow*	wnd;


		// Are we in test mode?
		if (glTestMode)
			return;

		// Grab our window
		wnd = iLocateWindow(tnHandle);
		if (wnd)
		{
			// Do the font
			wnd->gauge.snapNeedle	= (tnSnapToDisplayedValue != 0);
			wnd->gauge.scaleFactor	= tfScaleFactor;
		}
	}

	REALTIME_API void realtime_gauge_set_needle_position(int tnHandle, float tfRangeStart, float tfRangeEnd, float tfNeedleValue)
	{
		SWindow*	wnd;


		// Are we in test mode?
		if (glTestMode)
			return;

		// Grab our window
		wnd = iLocateWindow(tnHandle);
		if (wnd)
		{
			// Store the point
			wnd->gauge.needleValue = tfNeedleValue;
			if (wnd->gauge.snapNeedle)
				wnd->gauge.needleValue = iGaugeGetSnappedNeedleValue(wnd);
		}
	}

	REALTIME_API void realtime_gauge_redraw(int tnHandle)
	{
		SWindow* wnd;


		// Are we in test mode?
		if (glTestMode)
			return;

		// Grab our window
		wnd = iLocateWindow(tnHandle);
		if (wnd)
		{
			// Signal a refresh
			iRenderTheGauge(wnd);
		}
	}




//////////
//
// Called to append a new SWindow structure to the link list
//
//////
	SWindow* iCreateNewSWindow(void)
	{
		SWindow*	wndNew;
		SWindow*	wnd;
		SWindow**	wndLast;


		// Find out where this entry will go
		if (!root)
		{
			// This is the first one
			wndLast = &root;

		} else {
			// We append to the end of the chain
			wnd = root;
			while (wnd->next)
				wnd = wnd->next;
			wndLast = &wnd->next;
		}
		// When we get here, we know where this one will go

		// Create the new entry
		wndNew = (SWindow*)malloc(sizeof(SWindow));
		if (wndNew)
		{
			// Initialize the block to NULLs
			memset(wndNew, 0, sizeof(SWindow));

			// Update the back-link
			*wndLast = wndNew;

			// All done!
		}
		// Indicate our success or failure by returning the pointer
		return(wndNew);
	}




//////////
//
// Called to delete the indicated window from the chain (if it exists)
//
//////
	void iDeleteSWindow(SWindow* tsWnd)
	{
		SWindow*	wnd;
		SWindow**	wndLast;


		// Make sure we have an entry to iterate through
		if (root)
		{
			// Iterate through all our entries
			wnd		= root;
			wndLast	= &root;
			while (wnd)
			{
				if (wnd == tsWnd)
				{
					// This is our man to delete
					// Un-subclass the window
					SetWindowLong(wnd->hwnd, GWL_WNDPROC, (long)wnd->oldWndProcAddress);

					// Update the linked list
					*wndLast	= wnd->next;

					// Delete the window
					DestroyWindow(wnd->hwnd);

					// Delete the stuff
					DeleteDC(wnd->hdc);
					DeleteDC(wnd->hdc2);
					DeleteObject((HGDIOBJ)wnd->hbmp);

					// Delete self
					free(wnd);

					// All done
					return;
				}

				// Move to next entry
				wndLast	= &wnd->next;
				wnd		= wnd->next;
			}
		}
	}




//////////
//
// Called to locate the indicated handle (window) in the window link list
//
//////
	SWindow* iLocateWindow(int tnHandle)
	{
		SWindow* wnd;


		// Make sure we have a window "to look through" (so to speak)
		wnd = NULL;
		if (root)
		{
			// Iterate through all windows
			wnd = root;
			while (wnd)
			{
				if ((int)wnd->hwnd == tnHandle)
					return(wnd);		// This is it

				// Move to the next item
				wnd = wnd->next;
			}
			// If we get here, not found
		}
		// Indicate success or failure
		return(wnd);
	}




//////////
//
// Appends a graph data point to the indicated window
//
//////
	void iAppendGraphDataPoint(SWindow* tsWnd, float tfRangeUpper, float tfRangeLower, float tfFloatDataPoint)
	{
		SDataPoint* pointNew;


		// Store the upper and lower range
		tsWnd->graph.rangeUpper	= tfRangeUpper;
		tsWnd->graph.rangeLower	= tfRangeLower;

		// Create the new point
		pointNew = (SDataPoint*)malloc(sizeof(SDataPoint));
		if (!pointNew)
			return;

		// Store the value
		pointNew->value			= tfFloatDataPoint;

		// Insert before the existing first one (if any)
		pointNew->next			= tsWnd->graph.dataPoints;

		// data points now points here first
		tsWnd->graph.dataPoints	= pointNew;
	}




//////////
//
// If the string is not the same as what is already there, create a new copy
//
//////
	bool iCopyStringIfDifferent(char** tcDst, char* tcSrc)
	{
		if (tcDst && tcSrc)
		{
			if (*tcDst)
			{
				// There is already something here
				if (strlen(*tcDst) != strlen(tcSrc) || _memicmp(*tcDst, tcSrc, strlen(tcSrc)) != 0)
				{
					// They're different
					// Release the old
					free(*tcDst);

				} else {
					// They're the same
					return(false);
				}
			}
			//else this is the first entry


			// Create room for the new
			*tcDst = (char*)malloc(strlen(tcSrc) + 1);

			// If we created the memory block, copy the string
			if (*tcDst)
				memcpy(*tcDst, tcSrc, strlen(tcSrc) + 1);

			// We updated
			return(true);
		}
		// We did not have enough to update
		return(false);
	}




//////////
//
// Create a new windows hfont for the indicated fontName and point size
//
//////
	void iUpdateFont(SWindow* tsWnd)
	{
		int lnHeight;


		// Create the font
		if (tsWnd->type == _TYPE_GRAPH)
		{
			// Graph
			lnHeight = -MulDiv(tsWnd->graph.fontSize, GetDeviceCaps(tsWnd->hdc, LOGPIXELSY), 72);
			tsWnd->graph.textHFont = CreateFontA(lnHeight, 0, 0, 0, FW_NORMAL, false, false, false, ANSI_CHARSET, 0, 0, 0, 0, tsWnd->graph.fontName);

		} else {
			// Gauge
			lnHeight = -MulDiv(tsWnd->gauge.fontSize, GetDeviceCaps(tsWnd->hdc, LOGPIXELSY), 72);
			tsWnd->gauge.textHFont = CreateFontA(lnHeight, 0, 0, 0, FW_NORMAL, false, false, false, ANSI_CHARSET, 0, 0, 0, 0, tsWnd->graph.fontName);

			// For the gauge we also create a bolder font for the value display area
			lnHeight = -MulDiv(tsWnd->gauge.fontSize + 2, GetDeviceCaps(tsWnd->hdc, LOGPIXELSY), 72);
			tsWnd->gauge.textHFontBold = CreateFontA(lnHeight, 0, 0, 0, FW_BOLD, false, false, false, ANSI_CHARSET, 0, 0, 0, 0, tsWnd->graph.fontName);
		}
	}




//////////
//
// Paint the indicated gradient onto the bits
//
//////
	void iGradient4Fill(SWindow* tsWnd)
	{
		int		lnX;
		float	lfRTopStep, lfGTopStep, lfBTopStep;		// Color step for each pixel across the top
		float	lfRTop, lfGTop, lfBTop;					// Colors used for top bar
		float	lfRBot, lfGBot, lfBBot;					// Colors used for bottom bar
		float	lfRBotStep, lfGBotStep, lfBBotStep;		// Color step for each pixel across the bottom


		if (tsWnd && tsWnd->bits)
		{
			// Grab our colorings
			// Top
			lfRTop		= (float)red(tsWnd->ulRgb);
			lfGTop		= (float)grn(tsWnd->ulRgb);
			lfBTop		= (float)blu(tsWnd->ulRgb);
			lfRTopStep	= ((float)red(tsWnd->urRgb) - lfRTop) / (float)tsWnd->bmi.bmiHeader.biWidth;
			lfGTopStep	= ((float)grn(tsWnd->urRgb) - lfGTop) / (float)tsWnd->bmi.bmiHeader.biWidth;
			lfBTopStep	= ((float)blu(tsWnd->urRgb) - lfBTop) / (float)tsWnd->bmi.bmiHeader.biWidth;

			// Bottom
			lfRBot		= (float)red(tsWnd->llRgb);
			lfGBot		= (float)grn(tsWnd->llRgb);
			lfBBot		= (float)blu(tsWnd->llRgb);
			lfRBotStep	= ((float)red(tsWnd->lrRgb) - (float)red(tsWnd->llRgb)) / (float)tsWnd->bmi.bmiHeader.biWidth;
			lfGBotStep	= ((float)grn(tsWnd->lrRgb) - (float)grn(tsWnd->llRgb)) / (float)tsWnd->bmi.bmiHeader.biWidth;
			lfBBotStep	= ((float)blu(tsWnd->lrRgb) - (float)blu(tsWnd->llRgb)) / (float)tsWnd->bmi.bmiHeader.biWidth;

			// Iterate across the bitmap
			for (lnX = 0; lnX < tsWnd->bmi.bmiHeader.biWidth; lnX++)
			{
				// Draw this vertical line
				iGradient4VerticalLine(tsWnd, lnX, lfRTop, lfGTop, lfBTop, lfRBot, lfGBot, lfBBot);

				// Increase the top color by its stepping
				lfRTop	+= lfRTopStep;
				lfGTop	+= lfGTopStep;
				lfBTop	+= lfBTopStep;

				// Increase the bottom color by its stepping
				lfRBot	+= lfRBotStep;
				lfGBot	+= lfGBotStep;
				lfBBot	+= lfBBotStep;
			}
		}
	}

	void iGradient4VerticalLine(SWindow* tsWnd, int tnX, float tfRTop, float tfGTop, float tfBTop, float tfRBot, float tfGBot, float tfBBot)
	{
		int		lnY;
		float	lfRMidStep, lfGMidStep, lfBMidStep;		// Colors used for middle runner
		SRGB*	lrgb;


		// Compute the steps from top to bottom
		lfRMidStep	= (tfRBot - tfRTop) / (float)tsWnd->bmi.bmiHeader.biHeight;
		lfGMidStep	= (tfGBot - tfGTop) / (float)tsWnd->bmi.bmiHeader.biHeight;
		lfBMidStep	= (tfBBot - tfBTop) / (float)tsWnd->bmi.bmiHeader.biHeight;

		// Compute where we will be in the bits
		lrgb = (SRGB*)(tsWnd->bits + ((tsWnd->bmi.bmiHeader.biHeight - 1) * tsWnd->actualWidth) + tnX * 3);
		for (lnY = 0; lnY < tsWnd->bmi.bmiHeader.biHeight; lnY++)
		{
			// Fill in this portion
			lrgb->red	= (unsigned char)tfRTop;
			lrgb->grn	= (unsigned char)tfGTop;
			lrgb->blu	= (unsigned char)tfBTop;

			// Move to next color
			tfRTop		+= lfRMidStep;
			tfGTop		+= lfGMidStep;
			tfBTop		+= lfBMidStep;

			// Move to next row (bitmaps are stored upside down)
			lrgb = (SRGB*)((char*)lrgb - tsWnd->actualWidth);
		}
	}




//////////
//
// Graphing drawing algorithms
//
//////
	void iGraphDrawBars(SWindow* tsWnd)
	{
		bool	llBar1;
		int		lnY, lnStep;


		// Draw the graph bars (the background)
		llBar1	= true;
		lnStep	= min(tsWnd->bmi.bmiHeader.biHeight / 8, 32);
		for (lnY = lnStep / 2; lnY < tsWnd->bmi.bmiHeader.biHeight; lnY += lnStep, llBar1 = !llBar1)
		{
			// Only draw bar1 colors for now
			if (llBar1)
				iOverlayRectangle(tsWnd, tsWnd->graph.marginLeft, lnY, tsWnd->bmi.bmiHeader.biWidth - tsWnd->graph.marginRight, lnY + lnStep + 1, tsWnd->graph.bar1Rgb, tsWnd->graph.bar2Rgb);
		}
	}




//////////
//
// Draw the grid over the top of the existing control
//
//////
	void iGraphDrawGrid(SWindow* tsWnd)
	{
		int		lnX, lnY, lnStartY, lnXStep;
		float	lfI, lfValue1, lfValue2, lfStartY, lfValueUp, lfValueDn;


		// Do our center line
		lfValue1	= max(min(tsWnd->graph.gridCenter, tsWnd->graph.rangeUpper), tsWnd->graph.rangeLower);
		lfValue2	= tsWnd->graph.gridSquareSize;
		// Right now, our lfValue is known to be in the range of upper..lower


		//////////
		// Draw our center line
		//////
			lfStartY = (float)tsWnd->bmi.bmiHeader.biHeight - (((lfValue1 - tsWnd->graph.rangeLower) / (tsWnd->graph.rangeUpper - tsWnd->graph.rangeLower)) * (float)tsWnd->bmi.bmiHeader.biHeight);
			lnStartY = (int)lfStartY;
			for (lnY = lnStartY; lnY < lnStartY + 3; lnY++)
				iDrawLineHorizontalAlpha(tsWnd, tsWnd->graph.marginLeft, tsWnd->bmi.bmiHeader.biWidth, lnY, tsWnd->graph.gridCenterRgb, tsWnd->graph.gridAlpha);


		//////////
		// Draw our other horizontal lines
		//////
			for (lfI = 1.0f; (int)lfI < tsWnd->bmi.bmiHeader.biHeight; lfI++)
			{
				// Compute the values for this iteration
				lfValueUp	= lfValue1 + (lfI * lfValue2);
				lfValueDn	= lfValue1 - (lfI * lfValue2);

				// See if we're completely off the grid yet
				if (lfValueUp > tsWnd->graph.rangeUpper && lfValueDn < tsWnd->graph.rangeLower)
					break;		// We're done

				// Draw upper
				iDrawLineHorizontalAlpha(tsWnd, tsWnd->graph.marginLeft, tsWnd->bmi.bmiHeader.biWidth, 
											tsWnd->bmi.bmiHeader.biHeight - (int)(((lfValueUp - tsWnd->graph.rangeLower) / (tsWnd->graph.rangeUpper - tsWnd->graph.rangeLower)) * (float)tsWnd->bmi.bmiHeader.biHeight),
											tsWnd->graph.gridLinesRgb, tsWnd->graph.gridAlpha);

				// Draw lower
				iDrawLineHorizontalAlpha(tsWnd, tsWnd->graph.marginLeft, tsWnd->bmi.bmiHeader.biWidth, 
											tsWnd->bmi.bmiHeader.biHeight - (int)(((lfValueDn - tsWnd->graph.rangeLower) / (tsWnd->graph.rangeUpper - tsWnd->graph.rangeLower)) * (float)tsWnd->bmi.bmiHeader.biHeight),
											tsWnd->graph.gridLinesRgb, tsWnd->graph.gridAlpha);
			}


		//////////
		// Draw our vertical lines based on how many pixels each horizontal line was above
		//////
			lnXStep = max((tsWnd->bmi.bmiHeader.biWidth - tsWnd->graph.marginLeft) / 30, 5);
			for (lnX = tsWnd->graph.marginLeft; lnX < tsWnd->bmi.bmiHeader.biWidth; lnX += lnXStep)
				iDrawLineVerticalAlpha(tsWnd, lnX, 0, tsWnd->bmi.bmiHeader.biHeight - 1, tsWnd->graph.gridLinesRgb, tsWnd->graph.gridAlpha);
	}




//////////
//
// Called to overlay the graduation scale along the left
//
//////
	void iGraphOverlayGraduation(SWindow* tsWnd)
	{
		float	lfY, lfYStep, lfValue, lfValueStep;
		RECT	lrc;
		HGDIOBJ	lhOldFont;
		char	sprintfBuffer[64];
		char	buffer[64];


		// Draw the graduation bars
		iOverlayRectangle(tsWnd, tsWnd->graph.marginLeft, 0, tsWnd->graph.marginLeft + 10, tsWnd->bmi.bmiHeader.biHeight, makePastel(tsWnd->graph.bar1Rgb), makePastel(tsWnd->graph.textRgb));

		// Setup our HDC for writing properly
		SetBkMode(tsWnd->hdc2, TRANSPARENT);
		SetTextColor(tsWnd->hdc2, RGB(red(tsWnd->graph.textRgb), grn(tsWnd->graph.textRgb), blu(tsWnd->graph.textRgb)));
		lhOldFont = SelectObject(tsWnd->hdc2, tsWnd->graph.textHFont);

		// Setup our sprintf buffer for below
		if (tsWnd->graph.decimals >= 1)
		{
			// They want decimals
			sprintf_s(sprintfBuffer, sizeof(sprintfBuffer), "%%%u.%ulf\0", tsWnd->graph.integers + tsWnd->graph.decimals + 1, tsWnd->graph.decimals);

		} else {
			// Just integers
			sprintf_s(sprintfBuffer, sizeof(sprintfBuffer), "%%%u.0lf\0", tsWnd->graph.integers);
		}

		// Calculate how tall each item will be
		DrawTextA(tsWnd->hdc2, "1234567890.0", 12, &lrc, DT_CALCRECT);

		// Calculate our steppers
		lfValueStep	= (tsWnd->graph.rangeUpper - tsWnd->graph.rangeLower) / 8.0f;
		lfYStep		= (float)(tsWnd->bmi.bmiHeader.biHeight - (lrc.bottom - lrc.top)) / 8.0f;

		// Draw the graduation text
		lfValue = tsWnd->graph.rangeUpper;
		for (lfY = 0.0f; lfY < (float)tsWnd->bmi.bmiHeader.biHeight; lfY += lfYStep, lfValue -= lfValueStep)
		{
			// Compute our rect
			SetRect(&lrc, 0, (int)lfY, 64, (int)lfY + (int)lfYStep + 1);

			// Get our value
			memset(buffer, 0, sizeof(buffer));
			sprintf_s(buffer, sizeof(buffer), sprintfBuffer, lfValue);

			// Draw our text
			DrawTextA(tsWnd->hdc2, buffer, strlen(buffer), &lrc, DT_LEFT | DT_TOP);
		}

		// Restore our old handle
		SelectObject(tsWnd->hdc2, lhOldFont);
	}




	void iGraphPopulateDataPoints(SWindow* tsWnd)
	{
		int				lnI, lnX, lnY, lnPoint, lnStart, lnEnd;
		float			lfPercent, lfPercentHigh, lfPercentLow, lfValue, lfHigh, lfLow;
		SDataPoint*		pointNext;
		SDataPoint*		point;


		// Begin processing each point backwards
		if (!tsWnd->graph.dataPoints)
			return;

		// Iterate through each point
		point = tsWnd->graph.dataPoints;
		for (	lnI = 0, lnX = tsWnd->bmi.bmiHeader.biWidth - 1;
				point && lnX >= 0 && point;
				lnI++, lnX--)
		{
			if (tsWnd->graph.sampleAverageCount > 1)
			{
				// We need to average out a few points
				iGraphFindHighLowAvgPoints(tsWnd, max(lnI - tsWnd->graph.sampleAverageCount / 2, 0), tsWnd->graph.sampleAverageCount, NULL, NULL, &lfValue);

			} else {
				// Just use the one point
				lfValue = point->value;
			}

			if (tsWnd->graph.rangeVisible)
				iGraphFindHighLowAvgPoints(tsWnd, max(lnI - max(tsWnd->graph.rangeAverageSamples, 16) / 2, 0), max(tsWnd->graph.rangeAverageSamples, 16), &lfHigh, &lfLow, NULL);

			// Render this point
			if (lfValue >= tsWnd->graph.rangeUpper)
			{
				// It exceeds the upper bounds
				lnY = 0;

			} else if (lfValue <= tsWnd->graph.rangeLower) {
				// It subceeds the lower bounds (LOL! :-))
				lnY = tsWnd->bmi.bmiHeader.biHeight - (tsWnd->graph.dataPointThickness / 2);

			} else {
				// It's within the bounds
				lfPercent	= (lfValue - tsWnd->graph.rangeLower) / (tsWnd->graph.rangeUpper - tsWnd->graph.rangeLower);
				lnY			= tsWnd->bmi.bmiHeader.biHeight - (int)(lfPercent * (float)tsWnd->bmi.bmiHeader.biHeight);
			}

			// Draw the range first (so it's behind the data point)
			if (tsWnd->graph.rangeVisible != 0)
			{
				// Draw the range for this point
				lfPercentHigh	= (lfHigh - tsWnd->graph.rangeLower) / (tsWnd->graph.rangeUpper - tsWnd->graph.rangeLower);
				lfPercentLow	= (lfLow  - tsWnd->graph.rangeLower) / (tsWnd->graph.rangeUpper - tsWnd->graph.rangeLower);
				lnStart			= tsWnd->bmi.bmiHeader.biHeight - (int)(lfPercentHigh * (float)tsWnd->bmi.bmiHeader.biHeight);
				lnEnd			= tsWnd->bmi.bmiHeader.biHeight - (int)(lfPercentLow  * (float)tsWnd->bmi.bmiHeader.biHeight);
				iDrawLineVerticalAlpha(tsWnd, lnX, lnStart, lnEnd, tsWnd->graph.rangeRgb, tsWnd->graph.rangeAlpha);
			}

			// Draw the line at that point
			iDrawLineVertical(tsWnd, lnX, lnY - tsWnd->graph.dataPointThickness, lnY + tsWnd->graph.dataPointThickness, tsWnd->graph.dataRgb);

			// Move to the next point
			point = point->next;
		}

		// When we get here, if there are more data points, they no longer fit on the graph, delete them
		if (point)
		{
			// If we're averaging, skip past the extra ones we need for our averages
			for (lnI = 0; point && lnI < max(max(tsWnd->graph.sampleAverageCount, tsWnd->graph.rangeAverageSamples), 16); lnI++)
				point = point->next;

			// If we have more points than this, we delete the rest
			if (point)
			{
				// Make this point point no further, and every point that used to be after this point, delete them
				pointNext		= point->next;		// Get the next point (if any)
				point->next		= NULL;				// The last point points to no further point
				point			= pointNext;		// Point is now the point after the last point we rendered (if any)
				lnPoint			= 0;
				while (point)
				{
					// Grab the next point
					pointNext = point->next;

					// Delete this entry
					free(point);

					// Move to next entry
					point = pointNext;
				}
			}
		}
	}




//////////
//
// Called to search the data points and find out the high and low value
//
//////
	void iGraphFindHighLowAvgPoints(SWindow* tsWnd, int tnSkipTo, int tnSamples, float* tfHigh, float* tfLow, float* tfAvg)
	{
		int			lnSample;
		float		lfHigh, lfLow, lfAvg, lfCount;
		SDataPoint*	point;


		// Make sure we're on a graph
		if (tsWnd && tsWnd->type == _TYPE_GRAPH && tsWnd->graph.dataPoints)
		{
			// Get the min and max
			lfHigh		= tsWnd->graph.rangeLower;
			lfLow		= tsWnd->graph.rangeUpper;
			lfAvg		= 0.0f;
			point		= tsWnd->graph.dataPoints;
			lnSample	= 0;
			lfCount		= 0.0f;
			while (point && lnSample < tnSkipTo + tnSamples)
			{
				// Grab the range (if we're in the range of samples to consider)
				if (lnSample >= tnSkipTo)
				{
					lfHigh	= max(lfHigh,	max(min(point->value, tsWnd->graph.rangeUpper), tsWnd->graph.rangeLower));
					lfLow	= min(lfLow,	max(min(point->value, tsWnd->graph.rangeUpper), tsWnd->graph.rangeLower));

					// Compute for our average
					lfAvg	+= point->value;
					++lfCount;
				}

				// Move to next data point
				++lnSample;
				point = point->next;
			}
			// Compute our average
			lfAvg /= lfCount;

		} else {
			// No data points, just use the range
			lfHigh	= tsWnd->graph.rangeUpper;
			lfLow	= tsWnd->graph.rangeLower;
			lfAvg	= (lfHigh + lfLow) / 2.0f;
		}
		// Store our results
		if (tfHigh)		*tfHigh = lfHigh;
		if (tfLow)		*tfLow	= lfLow;
		if (tfAvg)		*tfAvg	= lfAvg;
	}




//////////
//
// Called to take the bitmap file and adjust the color data of the window image by the data from
// that bitmap file.  This has the effect of taking raw colors and making them look textured, or
// adding some shine, etc.
//
//////
	void iApplyOverlayBmpFile(SWindow* tsWnd, float tfX1, float tfY1, float tfX2, float tfY2, float tfTheta, unsigned char* tcOverlayBmpFile)
	{
		int					lnStep, lnStepCount, lnActualWidth;
		float				lfYWnd, lfXWnd, lfYBmp, lfXBmp, lfDeltaX, lfDeltaY, lfStepX, lfStepY, lfGrayPercent, lfAlp, lfMalp;
		BITMAPFILEHEADER*	lbh;		// bitmap header
		BITMAPINFOHEADER*	lbi;		// bitmap info
		char*				lbd;		// bitmap data bits
		SRGB*				lrgbWnd;
		SRGB*				lrgbBmp;


		// Get our pointers
		lbh = (BITMAPFILEHEADER*)tcOverlayBmpFile;
		lbi = (BITMAPINFOHEADER*)(lbh + 1);
		lbd = (char*)lbh + lbh->bfOffBits;

		// Compute the actual width of each line
		lnActualWidth = iComputeActualWidth(lbi);

		// Determine how many points we need to hit everything between
		lfDeltaX		= tfX2 - tfX1;
		lfDeltaY		= tfY2 - tfY1;
		lnStepCount		= (int)(sqrt(lfDeltaX*lfDeltaX + lfDeltaY*lfDeltaY) * 2.0f);
		lfStepX			= lfDeltaX / lnStepCount;
		lfStepY			= lfDeltaY / lnStepCount;

		// Setup our alpha values
		lfAlp			= 0.75f;
		lfMalp			= 1.0f - lfAlp;

		// Calculate our X position through the overlay bitmap
		// We know that tfTheta in this case will only be in the range _3PI_2-_PI6.._3PI_2+_PI6
		lfXBmp = (float)lbi->biWidth - (((tfTheta + (_PI2 - _PI6)) / (_2PI - _PI3)) * (float)lbi->biWidth);

		// Draw the points on this line
		for (	lnStep = 0, lfYWnd = tfY1, lfXWnd = tfX1;
				lnStep < lnStepCount;
				lnStep++, lfYWnd += lfStepY, lfXWnd += lfStepX	)
		{
			if (lfYWnd >= 0.0f && lfXWnd >= 0.0f && (int)lfYWnd < tsWnd->bmi.bmiHeader.biHeight && (int)lfXWnd < tsWnd->bmi.bmiHeader.biWidth)
			{
				// Get our lfYBmp coordinate
				lfYBmp = (float)lbi->biHeight - (((float)lnStep / (float)lnStepCount) * (float)lbi->biHeight);

				// Get the offset for this point
				lrgbWnd = (SRGB*)(tsWnd->bits + ((tsWnd->bmi.bmiHeader.biHeight - (int)lfYWnd - 1) * tsWnd->actualWidth) + ((int)lfXWnd * 3));
				lrgbBmp = (SRGB*)(lbd         + ((lbi->biHeight -                 (int)lfYBmp - 1) * lnActualWidth)      + ((int)lfXBmp * 3));

				// Render this point
				lfGrayPercent = (float)lrgbBmp->red / 255.0f;
				lrgbWnd->red = (unsigned char)(((float)lrgbWnd->red * lfAlp) + ((float)lrgbWnd->red * lfGrayPercent * lfMalp));
				lrgbWnd->grn = (unsigned char)(((float)lrgbWnd->grn * lfAlp) + ((float)lrgbWnd->grn * lfGrayPercent * lfMalp));
				lrgbWnd->blu = (unsigned char)(((float)lrgbWnd->blu * lfAlp) + ((float)lrgbWnd->blu * lfGrayPercent * lfMalp));

//  			lrgbWnd->red = (unsigned char)(((float)lrgbWnd->red * 0.90) + ((float)(255 - lrgbBmp->red) * 0.10));
//  			lrgbWnd->grn = (unsigned char)(((float)lrgbWnd->grn * 0.90) + ((float)(255 - lrgbBmp->grn) * 0.10));
//  			lrgbWnd->blu = (unsigned char)(((float)lrgbWnd->blu * 0.90) + ((float)(255 - lrgbBmp->blu) * 0.10));
// 
// 				lrgbWnd->red = lrgbBmp->red;
// 				lrgbWnd->grn = lrgbBmp->grn;
// 				lrgbWnd->blu = lrgbBmp->blu;
			}
		}
	}




	void iOverlayRectangle(SWindow* tsWnd, int tnUlX, int tnUlY, int tnLrX, int tnLrY, int tnFillRgb, int tnFrameRgb)
	{
		unsigned char	lnFrameRed, lnFrameGrn, lnFrameBlu, lnFillRed, lnFillGrn, lnFillBlu;
		int				lnY, lnX;
		SRGB*			lrgb;


		// Grab our colors
		// Fill colors
		lnFillRed	= (unsigned char)red(tnFillRgb);
		lnFillGrn	= (unsigned char)grn(tnFillRgb);
		lnFillBlu	= (unsigned char)blu(tnFillRgb);

		// Frame colors
		lnFrameRed	= (unsigned char)red(tnFrameRgb);
		lnFrameGrn	= (unsigned char)grn(tnFrameRgb);
		lnFrameBlu	= (unsigned char)blu(tnFrameRgb);

		// Draw the rectangle for all pixels that should be drawn vertically
		for (lnY = tnUlY; lnY < tnLrY; lnY++)
		{
			if (lnY >= 0 && lnY < tsWnd->bmi.bmiHeader.biHeight)
			{
				// Find out on what row this pixel data will go
				lrgb = (SRGB*)((unsigned char*)tsWnd->bits + ((tsWnd->bmi.bmiHeader.biHeight - lnY - 1) * tsWnd->actualWidth) + (tnUlX * 3));

				// Draw the rectangle for all pixels that should be drawn horizontally
				for (lnX = tnUlX; lnX < tnLrX; lnX++)
				{
					if (lnX >= 0 && lnX < tsWnd->bmi.bmiHeader.biWidth)
					{
						// Each pixel is either a frame or fill color
						if (lnY == tnUlY || lnX == tnUlX || lnY == tnLrY-1 || lnX == tnLrX-1)
						{
							// Frame color
							lrgb->red = lnFrameRed;
							lrgb->grn = lnFrameGrn;
							lrgb->blu = lnFrameBlu;

						} else {
							// Fill color
							lrgb->red = lnFillRed;
							lrgb->grn = lnFillGrn;
							lrgb->blu = lnFillBlu;
						}
					}

					// Move to next pixel
					++lrgb;
				}
			}
		}
	}




//////////
//
// Draws a vertical line at the indicated coordinates
//
//////
	void iDrawLineVertical(SWindow* tsWnd, int tnX, int tnUY, int tnLY, int tnRgb)
	{
		unsigned char	lnRed, lnGrn, lnBlu;
		int				lnY;
		SRGB*			lrgb;


		// Make sure this line would be visible
		if (tnX < 0 || tnX >= tsWnd->bmi.bmiHeader.biWidth)
			return;		// Nothing to do

		// Grab our colors
		lnRed	= (unsigned char)red(tnRgb);
		lnGrn	= (unsigned char)grn(tnRgb);
		lnBlu	= (unsigned char)blu(tnRgb);

		// Draw the rectangle for all pixels that should be drawn vertically
		for (lnY = tnUY; lnY < tnLY; lnY++)
		{
			if (lnY >= 0 && lnY < tsWnd->bmi.bmiHeader.biHeight)
			{
				// Find out on what row this pixel data will go
				lrgb = (SRGB*)((unsigned char*)tsWnd->bits + ((tsWnd->bmi.bmiHeader.biHeight - lnY - 1) * tsWnd->actualWidth) + (tnX * 3));

				// Store the color
				lrgb->red = lnRed;
				lrgb->grn = lnGrn;
				lrgb->blu = lnBlu;
			}
		}
	}




//////////
//
// Draws a vertical line at the indicated coordinates with an alpha setting
//
//////
	void iDrawLineVerticalAlpha(SWindow* tsWnd, int tnX, int tnUY, int tnLY, int tnRgb, float tfAlp)
	{
		float	lfRed, lfGrn, lfBlu;
		float	lfMalp;
		int		lnY;
		SRGB*	lrgb;


		// Make sure this line would be visible
		if (tnX < 0 || tnX >= tsWnd->bmi.bmiHeader.biWidth)
			return;		// Nothing to do

		// Grab our colors
		lfRed	= (float)red(tnRgb);
		lfGrn	= (float)grn(tnRgb);
		lfBlu	= (float)blu(tnRgb);

		// Make sure our alpha setting is in the range 0.0..1.0
		tfAlp	= max(min(tfAlp, 1.0f), 0.0f);
		lfMalp	= 1.0f - tfAlp;

		// Draw the rectangle for all pixels that should be drawn vertically
		for (lnY = tnUY; lnY < tnLY; lnY++)
		{
			if (lnY >= 0 && lnY < tsWnd->bmi.bmiHeader.biHeight)
			{
				// Find out on what row this pixel data will go
				lrgb = (SRGB*)((unsigned char*)tsWnd->bits + ((tsWnd->bmi.bmiHeader.biHeight - lnY - 1) * tsWnd->actualWidth) + (tnX * 3));

				// Overlay the color atop whatever's already there using the alpha setting
				lrgb->red = (unsigned char)((tfAlp * lfRed) + ((float)lrgb->red * lfMalp));
				lrgb->grn = (unsigned char)((tfAlp * lfGrn) + ((float)lrgb->grn * lfMalp));
				lrgb->blu = (unsigned char)((tfAlp * lfBlu) + ((float)lrgb->blu * lfMalp));
			}
		}
	}




//////////
//
// DRaws a horizontal draw line at the indicated coordinates
//
//////
	void iDrawLineHorizontal(SWindow* tsWnd, int tnLX, int tnRX, int tnY, int tnRgb)
	{
		unsigned char	lnRed, lnGrn, lnBlu;
		int				lnX;
		SRGB*			lrgb;


		// Make sure this line would be visible
		if (tnY < 0 || tnY >= tsWnd->bmi.bmiHeader.biHeight)
			return;		// Nothing to do

		// Grab our colors
		lnRed	= (unsigned char)red(tnRgb);
		lnGrn	= (unsigned char)grn(tnRgb);
		lnBlu	= (unsigned char)blu(tnRgb);

		// Find out on what row this pixel data will go
		lrgb = (SRGB*)((unsigned char*)tsWnd->bits + ((tsWnd->bmi.bmiHeader.biHeight - tnY - 1) * tsWnd->actualWidth) + (tnLX * 3));
		for (lnX = tnLX; lnX < tnRX; lnX++)
		{
			if (lnX >= 0 && lnX < tsWnd->bmi.bmiHeader.biWidth)
			{
				// Store the color
				lrgb->red = lnRed;
				lrgb->grn = lnGrn;
				lrgb->blu = lnBlu;
			}

			// Move to next pixel horizontally
			++lrgb;
		}
	}





//////////
//
// DRaws a horizontal draw line at the indicated coordinates with an alpha setting
//
//////
	void iDrawLineHorizontalAlpha(SWindow* tsWnd, int tnLX, int tnRX, int tnY, int tnRgb, float tfAlp)
	{
		float	lfRed, lfGrn, lfBlu;
		float	lfMalp;
		int		lnX;
		SRGB*	lrgb;


		// Make sure this line would be visible
		if (tnY < 0 || tnY >= tsWnd->bmi.bmiHeader.biHeight)
			return;		// Nothing to do

		// Grab our colors
		lfRed	= (float)red(tnRgb);
		lfGrn	= (float)grn(tnRgb);
		lfBlu	= (float)blu(tnRgb);

		// Make sure our alpha setting is in the range 0.0..1.0
		tfAlp	= max(min(tfAlp, 1.0f), 0.0f);
		lfMalp	= 1.0f - tfAlp;

		// Find out on what row this pixel data will go
		lrgb = (SRGB*)((unsigned char*)tsWnd->bits + ((tsWnd->bmi.bmiHeader.biHeight - tnY - 1) * tsWnd->actualWidth) + (tnLX * 3));
		for (lnX = tnLX; lnX < tnRX; lnX++)
		{
			if (lnX >= 0 && lnX < tsWnd->bmi.bmiHeader.biWidth)
			{
				// Overlay the color atop whatever's already there using the alpha setting
				lrgb->red = (unsigned char)((tfAlp * lfRed) + ((float)lrgb->red * lfMalp));
				lrgb->grn = (unsigned char)((tfAlp * lfGrn) + ((float)lrgb->grn * lfMalp));
				lrgb->blu = (unsigned char)((tfAlp * lfBlu) + ((float)lrgb->blu * lfMalp));
			}
			// Move to next pixel horizontally
			++lrgb;
		}
	}




//////////
//
// Draws an arbitrary line between two points without anti-aliasing
//
//////
	void iDrawLineArbitrary(SWindow* tsWnd, float tfX1, float tfY1, float tfX2, float tfY2, int tnRgb)
	{
		int				lnStep, lnStepCount;
		unsigned char	lnRed, lnGrn, lnBlu;
		float			lfY, lfX, lfDeltaX, lfDeltaY, lfStepX, lfStepY;
		SRGB*			lrgb;


		// Grab our colors
		lnRed = red(tnRgb);
		lnGrn = grn(tnRgb);
		lnBlu = blu(tnRgb);

		// Determine how many points we need to hit everything between
		lfDeltaX		= tfX2 - tfX1;
		lfDeltaY		= tfY2 - tfY1;
		lnStepCount		= (int)(sqrt(lfDeltaX*lfDeltaX + lfDeltaY*lfDeltaY) * 1.10);
		lfStepX			= lfDeltaX / lnStepCount;
		lfStepY			= lfDeltaY / lnStepCount;

		// Draw the points on this line
		for (	lnStep = 0, lfY = tfY1, lfX = tfX1;
				lnStep < lnStepCount;
				lnStep++, lfY += lfStepY, lfX += lfStepX	)
		{
			if (lfY >= 0.0f && lfX >= 0.0f && (int)lfY < tsWnd->bmi.bmiHeader.biHeight && (int)lfX < tsWnd->bmi.bmiHeader.biWidth)
			{
				// Get the offset for this point
				lrgb = (SRGB*)(tsWnd->bits + ((tsWnd->bmi.bmiHeader.biHeight - (int)lfY - 1) * tsWnd->actualWidth) + ((int)lfX * 3));

				// Render this point
				lrgb->red = lnRed;
				lrgb->grn = lnGrn;
				lrgb->blu = lnBlu;
			}
		}
	}




//////////
//
// Draws an arbitrary line between two points without anti-alasing, and it follows a color path
// beginning at tnRgb1 and ending at tnRgb2
//
//////
	void iDrawLineArbitraryColorPath(SWindow* tsWnd, float tfX1, float tfY1, float tfX2, float tfY2, int tnRgb1, int tnRgb2)
	{
		int				lnStep, lnStepCount;
		float			lfY, lfX, lfDeltaX, lfDeltaY, lfStepX, lfStepY, lfRed, lfGrn, lfBlu, lfRedStep, lfGrnStep, lfBluStep;
		SRGB*			lrgb;


		// Determine how many points we need to hit everything between
		lfDeltaX		= tfX2 - tfX1;
		lfDeltaY		= tfY2 - tfY1;
		lnStepCount		= (int)(sqrt(lfDeltaX*lfDeltaX + lfDeltaY*lfDeltaY) * 1.10);
		lfStepX			= lfDeltaX / lnStepCount;
		lfStepY			= lfDeltaY / lnStepCount;

		// Grab our colors
		lfRed		= (float)red(tnRgb1);
		lfGrn		= (float)grn(tnRgb1);
		lfBlu		= (float)blu(tnRgb1);
		lfRedStep	= (float)(red(tnRgb2) - lfRed) / (float)lnStepCount;
		lfGrnStep	= (float)(grn(tnRgb2) - lfGrn) / (float)lnStepCount;
		lfBluStep	= (float)(blu(tnRgb2) - lfBlu) / (float)lnStepCount;

		// Draw the points on this line
		for (	lnStep = 0, lfY = tfY1, lfX = tfX1;
			lnStep < lnStepCount;
			lnStep++, lfY += lfStepY, lfX += lfStepX, lfRed += lfRedStep, lfGrn += lfGrnStep, lfBlu += lfBluStep	)
		{
			if (lfY >= 0.0f && lfX >= 0.0f && (int)lfY < tsWnd->bmi.bmiHeader.biHeight && (int)lfX < tsWnd->bmi.bmiHeader.biWidth)
			{
				// Get the offset for this point
				lrgb = (SRGB*)(tsWnd->bits + ((tsWnd->bmi.bmiHeader.biHeight - (int)lfY - 1) * tsWnd->actualWidth) + ((int)lfX * 3));

				// Render this point
				lrgb->red = (unsigned char)lfRed;
				lrgb->grn = (unsigned char)lfGrn;
				lrgb->blu = (unsigned char)lfBlu;
			}
		}
	}




//////////
//
// Draws an arbitrary line between two points without anti-aliasing
//
//////
	void iGrayerLineArbitrary(SWindow* tsWnd, float tfX1, float tfY1, float tfX2, float tfY2)
	{
		int				lnStep, lnStepCount;
		float			lfY, lfX, lfDeltaX, lfDeltaY, lfStepX, lfStepY, lfGray;
		SRGB*			lrgb;


		// Determine how many points we need to hit everything between
		lfDeltaX		= tfX2 - tfX1;
		lfDeltaY		= tfY2 - tfY1;
		lnStepCount		= (int)(sqrt(lfDeltaX*lfDeltaX + lfDeltaY*lfDeltaY) * 1.10);
		lfStepX			= lfDeltaX / lnStepCount;
		lfStepY			= lfDeltaY / lnStepCount;

		// Draw the points on this line
		for (	lnStep = 0, lfY = tfY1, lfX = tfX1;
				lnStep < lnStepCount;
				lnStep++, lfY += lfStepY, lfX += lfStepX	)
		{
			if (lfY >= 0.0f && lfX >= 0.0f && (int)lfY < tsWnd->bmi.bmiHeader.biHeight && (int)lfX < tsWnd->bmi.bmiHeader.biWidth)
			{
				// Get the offset for this point
				lrgb = (SRGB*)(tsWnd->bits + ((tsWnd->bmi.bmiHeader.biHeight - (int)lfY - 1) * tsWnd->actualWidth) + ((int)lfX * 3));

				// Make this point grayer than it is
				lfGray = ((0.35f * (float)lrgb->red) + (0.54f * (float)lrgb->grn) + (0.11f * (float)lrgb->blu)) * 0.75f;
				lrgb->red = (unsigned char)(0.50f * (float)lrgb->red + 0.50f * lfGray);
				lrgb->grn = (unsigned char)(0.50f * (float)lrgb->grn + 0.50f * lfGray);
				lrgb->blu = (unsigned char)(0.50f * (float)lrgb->blu + 0.50f * lfGray);
			}
		}
	}




//////////
//
// Render the basic gauge
//
//////
	void iGaugeRenderBasic(SWindow* tsWnd)
	{
		int		lnStep, lnRgb, lnRgbDarker, lnLast, lnThis;
		float	lfStep, lfStepCount, lfTheta, lfThetaStep, lfPercent, lfStart;
		float	lfX1, lfY1, lfX2, lfY2, lfXOrigin, lfYOrigin;
		bool	llDrawSegment;


		//////////
		// Affix our origin
		//////
			lfXOrigin	= (float)tsWnd->bmi.bmiHeader.biWidth  / 2.0f;
			lfYOrigin	= (float)tsWnd->bmi.bmiHeader.biHeight / 2.0f;


		//////////
		// Render the basic colors
		//////
			// Iterate from pi/6 less than 6pi/4, around to pi/6 greater than 6pi/4, crossing 0
			lfStepCount	= (float)tsWnd->bmi.bmiHeader.biWidth * _3PI_2;
			lfThetaStep	= -((_2PI - _PI3) / lfStepCount);					// Iterating around that many radians in that many steps
			lfTheta		= _3PI_2 - _PI6;
			lnLast		= _LOW_RANGE;
			lnThis		= _LOW_RANGE;
			for (lfStep = 0; lfStep < lfStepCount; lfStep++)
			{
				// Draw this segment
				// Furthest point away from the origin
				lfX1 = lfXOrigin + (lfXOrigin * tsWnd->gauge.scaleFactor * _INNER_RADIUS * cos(lfTheta));
				lfY1 = lfYOrigin - (lfYOrigin * tsWnd->gauge.scaleFactor * _INNER_RADIUS * sin(lfTheta));

				// Closest point to the origin
				lfX2 = lfXOrigin + (lfXOrigin * tsWnd->gauge.scaleFactor * _OUTER_RADIUS * cos(lfTheta));
				lfY2 = lfYOrigin - (lfYOrigin * tsWnd->gauge.scaleFactor * _OUTER_RADIUS * sin(lfTheta));

				// Grab our colors
				lfPercent = 100.0f * lfStep / lfStepCount;
				if (lfPercent < tsWnd->gauge.rangePercentLow)
				{
					// We're in the low range
					lnRgb	= tsWnd->gauge.lowRgb;
					lnThis	= _LOW_RANGE;

				} else if (lfPercent < tsWnd->gauge.rangePercentLow + tsWnd->gauge.rangePercentMid) {
					// We're in the mid range
					lnRgb	= tsWnd->gauge.midRgb;
					lnThis	= _MID_RANGE;

				} else {
					// We're in the high range
					lnRgb	= tsWnd->gauge.highRgb;
					lnThis	= _HIGH_RANGE;
				}

				// Draw a line between these two points
				iDrawLineArbitrary(tsWnd, lfX1, lfY1, lfX2, lfY2, lnRgb);

/*
 * A future enhancement:
 * We need to note these coordinates for rendering after the entire gauge is rendered
				// We've moved into a new region
				if (lnLast != lnThis)
				{
					// Redraw the last line we drew, but do it darker for a border
					iDrawLineArbitrary(tsWnd, lfX1, lfY1, lfX2, lfY2, makeDarker(lnRgb));
				}
*/

				// Update our toggle
				lnLast = lnThis;

				// Move over for next iteration
				lfTheta += lfThetaStep;
			}


		//////////
		// Frame the gauge if we are supposed to frame it
		//////
			if (tsWnd->gauge.frameGauge)
			{
				// Colorize based on where we are
				lfPercent = 100.0f * ((tsWnd->gauge.needleValue - tsWnd->gauge.rangeStart) / (tsWnd->gauge.rangeEnd - tsWnd->gauge.rangeStart));
				if (lfPercent < tsWnd->gauge.rangePercentLow)
				{
					// It's in the low range
					lnRgb		= tsWnd->gauge.lowRgb;
					lnRgbDarker	= makePastel(makeSomewhatDarker(tsWnd->gauge.lowRgb));

				} else if (lfPercent < tsWnd->gauge.rangePercentLow + tsWnd->gauge.rangePercentMid) {
					// It's in the middle range
					lnRgb		= tsWnd->gauge.midRgb;
					lnRgbDarker	= makePastel(makeSomewhatDarker(tsWnd->gauge.midRgb));

				} else {
					// It's in the high range
					lnRgb		= tsWnd->gauge.highRgb;
					lnRgbDarker	= makePastel(makeSomewhatDarker(tsWnd->gauge.highRgb));
				}

				// Iterate from pi/6 less than 6pi/4, around to pi/6 greater than 6pi/4, crossing 0
				lfStepCount	= (float)tsWnd->bmi.bmiHeader.biWidth * _2PI;
				lfThetaStep	= -((_2PI - _PI3) / lfStepCount);					// Iterating around that many radians in that many steps
				lfTheta		= _3PI_2 - _PI6;
				for (lfStep = 0; lfStep < lfStepCount; lfStep++)
				{
					//////////
					// Inner ring
					//////
						lfX1 = lfXOrigin + (lfXOrigin * tsWnd->gauge.scaleFactor * _INNER_RADIUS_FRAME1 * cos(lfTheta));
						lfY1 = lfYOrigin - (lfYOrigin * tsWnd->gauge.scaleFactor * _INNER_RADIUS_FRAME1 * sin(lfTheta));
						lfX2 = lfXOrigin + (lfXOrigin * tsWnd->gauge.scaleFactor * _INNER_RADIUS_FRAME2 * cos(lfTheta));
						lfY2 = lfYOrigin - (lfYOrigin * tsWnd->gauge.scaleFactor * _INNER_RADIUS_FRAME2 * sin(lfTheta));
						iDrawLineArbitrary(tsWnd, lfX1, lfY1, lfX2, lfY2, lnRgbDarker);

					//////////
					// Outer ring
					//////
						lfX1 = lfXOrigin + (lfXOrigin * tsWnd->gauge.scaleFactor * _OUTER_RADIUS_FRAME1 * cos(lfTheta));
						lfY1 = lfYOrigin - (lfYOrigin * tsWnd->gauge.scaleFactor * _OUTER_RADIUS_FRAME1 * sin(lfTheta));
						lfX2 = lfXOrigin + (lfXOrigin * tsWnd->gauge.scaleFactor * _OUTER_RADIUS_FRAME2 * cos(lfTheta));
						lfY2 = lfYOrigin - (lfYOrigin * tsWnd->gauge.scaleFactor * _OUTER_RADIUS_FRAME2 * sin(lfTheta));
						iDrawLineArbitrary(tsWnd, lfX1, lfY1, lfX2, lfY2, lnRgb);

					// Move over for next iteration
					lfTheta += lfThetaStep;
				}
			}



		//////////
		// Add the tick marks
		//////
			// Iterate from pi/6 less than 6pi/4, around to pi/6 greater than 6pi/4, crossing 0
			lfThetaStep	= -((_2PI - _PI3) / 100.0f);				// Iterating around that many radians in 100 steps
			lfTheta		= _3PI_2 - _PI6;
			for (lnStep = 0; lnStep <= 100; lnStep++)
			{
				// Find out if we're on a big tick mark
				llDrawSegment = false;
				if (lnStep % 10 == 0)
				{
					// Big tick mark
					lfStart			= _INNER_TICK_BIG;
					llDrawSegment	= true;

				} else if (lnStep % 5 == 0) {
					// Medium tick mark
					lfStart = _INNER_TICK_MEDIUM;

				} else {
					// Regular tick mark
					lfStart = _INNER_TICK_SMALL;
				}

				// Furthest point away from the origin
				lfX1 = lfXOrigin + (lfXOrigin * tsWnd->gauge.scaleFactor * lfStart * cos(lfTheta));
				lfY1 = lfYOrigin - (lfYOrigin * tsWnd->gauge.scaleFactor * lfStart * sin(lfTheta));

				// Closest point to the origin
				lfX2 = lfXOrigin + (lfXOrigin * tsWnd->gauge.scaleFactor * _OUTER_TICK * cos(lfTheta));
				lfY2 = lfYOrigin - (lfYOrigin * tsWnd->gauge.scaleFactor * _OUTER_TICK * sin(lfTheta));

				// Draw a line between these two points
				iDrawLineArbitrary(tsWnd, lfX1, lfY1, lfX2, lfY2, tsWnd->gauge.textRgb);

				// If we are to draw a segment down, do so
				if (llDrawSegment)
				{
					// Furthest point away from the origin
					lfX1 = lfXOrigin + (lfXOrigin * tsWnd->gauge.scaleFactor * _INNER_RADIUS_GRAY * cos(lfTheta));
					lfY1 = lfYOrigin - (lfYOrigin * tsWnd->gauge.scaleFactor * _INNER_RADIUS_GRAY * sin(lfTheta));

					// Closest point to the origin
					lfX2 = lfXOrigin + (lfXOrigin * tsWnd->gauge.scaleFactor * lfStart * cos(lfTheta));
					lfY2 = lfYOrigin - (lfYOrigin * tsWnd->gauge.scaleFactor * lfStart * sin(lfTheta));

					// Draw a line between these two points
					iGrayerLineArbitrary(tsWnd, lfX1, lfY1, lfX2, lfY2);
				}

				// Move over for next iteration
				lfTheta += lfThetaStep;
			}


		//////////
		// Augment color data with our overlay
		//////
			// Iterate from pi/6 less than 6pi/4, around to pi/6 greater than 6pi/4, crossing 0
			lfStepCount	= (float)tsWnd->bmi.bmiHeader.biWidth * _3PI_2;
			lfThetaStep	= -((_2PI - _PI3) / lfStepCount);					// Iterating around that many radians in that many steps
			lfTheta		= _3PI_2 - _PI6;
			lnLast		= _LOW_RANGE;
			lnThis		= _LOW_RANGE;
			for (lfStep = 0; lfStep < lfStepCount; lfStep++)
			{
				// Draw this segment
				// Furthest point away from the origin
				lfX1 = lfXOrigin + (lfXOrigin * tsWnd->gauge.scaleFactor * _INNER_RADIUS * cos(lfTheta));
				lfY1 = lfYOrigin - (lfYOrigin * tsWnd->gauge.scaleFactor * _INNER_RADIUS * sin(lfTheta));

				// Closest point to the origin
				lfX2 = lfXOrigin + (lfXOrigin * tsWnd->gauge.scaleFactor * _OUTER_RADIUS * cos(lfTheta));
				lfY2 = lfYOrigin - (lfYOrigin * tsWnd->gauge.scaleFactor * _OUTER_RADIUS * sin(lfTheta));

				// Apply the overlay coloring
				iApplyOverlayBmpFile(tsWnd, lfX1, lfY1, lfX2, lfY2, lfTheta, (unsigned char*)cgc_Overlay);

				// Move over for next iteration
				lfTheta += lfThetaStep;
			}
	}




//////////
//
// Called to overlay the graduation for the quantities relating to the gauge
//
//////
	void iGaugeOverlayGraduation(SWindow* tsWnd)
	{
		int		lnStep;
		float	lfTheta, lfThetaStep, lfValue, lfValueStep, lfHeight;
		float	lfX, lfY, lfXOrigin, lfYOrigin;
		RECT	lrc, lrcSize;
		HGDIOBJ	lhOldFont;
		char	sprintfBuffer[64];
		char	buffer[64];


		//////////
		// Affix our origin
		//////
			lfXOrigin	= (float)tsWnd->bmi.bmiHeader.biWidth  / 2.0f;
			lfYOrigin	= (float)tsWnd->bmi.bmiHeader.biHeight / 2.0f;
		

		//////////
		// Prepare the sprintf buffer
		//////
			if (tsWnd->gauge.graduationDecimals >= 1)
			{
				// They want decimals
				sprintf_s(sprintfBuffer, sizeof(sprintfBuffer), "%%%u.%ulf\0", tsWnd->gauge.graduationIntegers + tsWnd->gauge.graduationDecimals + 1, tsWnd->gauge.graduationDecimals);

			} else {
				// Just integers
				sprintf_s(sprintfBuffer, sizeof(sprintfBuffer), "%%%u.0lf\0", tsWnd->gauge.graduationIntegers);
			}


		//////////
		// Setup our HDC for writing properly
		//////
			SetBkMode(tsWnd->hdc2, TRANSPARENT);
			SetTextColor(tsWnd->hdc2, RGB(red(tsWnd->gauge.textRgb), grn(tsWnd->gauge.textRgb), blu(tsWnd->gauge.textRgb)));
			lhOldFont = SelectObject(tsWnd->hdc2, tsWnd->gauge.textHFont);

			// Calculate how tall each item will be
			DrawTextA(tsWnd->hdc2, "1234567890.0", 12, &lrc, DT_CALCRECT);
			lfHeight = (float)(lrc.bottom - lrc.top);


		//////////
		// Add the graduation
		//////
			// Iterate from pi/6 less than 6pi/4, around to pi/6 greater than 6pi/4, crossing 0
			lfThetaStep	= -((_2PI - _PI3) / 10.0f);				// Iterating around that many radians in 100 steps
			lfTheta		= _3PI_2 - _PI6;
			lfValue		= tsWnd->gauge.rangeStart;
			lfValueStep	= (tsWnd->gauge.rangeEnd - tsWnd->gauge.rangeStart) / 10.0f;
			for (lnStep = 0; lnStep <= 10; lnStep++)
			{
				// Get the point to the outer-most edge
				lfX = lfXOrigin + (lfXOrigin * ((1.0f + tsWnd->gauge.scaleFactor) / 2.0f) * _TEXT_RADIUS * cos(lfTheta));
				lfY = lfYOrigin - (lfYOrigin * ((1.0f + tsWnd->gauge.scaleFactor) / 2.0f) * _TEXT_RADIUS * sin(lfTheta));

				// All text must be rendered from that location closest to the center
				memset(buffer, 0, sizeof(buffer));
				sprintf_s(buffer, sizeof(buffer), sprintfBuffer, lfValue);

				// Calculate how big our text is
				SetRect(&lrcSize, 0, 0, 20, 200);
				DrawTextA(tsWnd->hdc2, buffer, strlen(buffer), &lrcSize, DT_CALCRECT);

				// Adjust our rectangle
				SetRect(&lrc, (int)lfX - (lrcSize.right / 2), (int)lfY - (lrcSize.bottom / 2), (int)lfX + (lrcSize.right / 2), (int)lfY + (lrcSize.bottom / 2));
				iAdjustRectangle(tsWnd, lfXOrigin, lfYOrigin, lfTheta, &lrc);

				// Draw our text
				DrawTextA(tsWnd->hdc2, buffer, strlen(buffer), &lrc, DT_LEFT | DT_TOP);

				// Move over for next iteration
				lfTheta += lfThetaStep;
				lfValue	+= lfValueStep;
			}


		//////////
		// Restore our old font handle
		//////
			SelectObject(tsWnd->hdc2, lhOldFont);
	}




//////////
//
// Called to overlay the needle
//
//////
	void iGaugeOverlayNeedle(SWindow* tsWnd)
	{
		int		lnRgb;
		float	lfPercent, lfTheta, lfXOrigin, lfYOrigin, lfX, lfY, lfDeltaX, lfDeltaY, lfNeedleValue;


		//////////
		// Affix our origin
		//////
			lfXOrigin	= (float)tsWnd->bmi.bmiHeader.biWidth  / 2.0f;
			lfYOrigin	= (float)tsWnd->bmi.bmiHeader.biHeight / 2.0f;


		//////////
		// Find our percentage from min..max
		//////
			lfNeedleValue = tsWnd->gauge.needleValue;
			if (lfNeedleValue <= tsWnd->gauge.rangeStart)
			{
				// It's at or below the minimum, so make it the minimum
				lfPercent = 0.0f;

			} else if (lfNeedleValue >= tsWnd->gauge.rangeEnd) {
				// It's at or above the maximum, so make it the maximum
				lfPercent = 1.0f;

			} else {
				// It's in range, compute it
				lfPercent = (lfNeedleValue - tsWnd->gauge.rangeStart) / (tsWnd->gauge.rangeEnd - tsWnd->gauge.rangeStart);
			}


		//////////
		// Compute our theta based on this percent
		//////
			lfTheta = (_3PI_2 - _PI6) - (lfPercent * (_2PI - _PI3));


		//////////
		// Compute our needle positions
		//////
			lfX = lfXOrigin + (lfXOrigin * tsWnd->gauge.scaleFactor * _NEEDLE_RADIUS * cos(lfTheta));
			lfY = lfYOrigin - (lfYOrigin * tsWnd->gauge.scaleFactor * _NEEDLE_RADIUS * sin(lfTheta));


		//////////
		// Compute our needle colors
		//////
			if (tsWnd->gauge.colorizeNeedle)
			{
				// Colorize based on where we are
				lfPercent = 100.0f * ((tsWnd->gauge.needleValue - tsWnd->gauge.rangeStart) / (tsWnd->gauge.rangeEnd - tsWnd->gauge.rangeStart));
				if (lfPercent < tsWnd->gauge.rangePercentLow)
				{
					// It's in the low range
					lnRgb = tsWnd->gauge.lowRgb;

				} else if (lfPercent < tsWnd->gauge.rangePercentLow + tsWnd->gauge.rangePercentMid) {
					// It's in the middle range
					lnRgb = tsWnd->gauge.midRgb;

				} else {
					// It's in the high range
					lnRgb = tsWnd->gauge.highRgb;
				}

			} else {
				// No colorizing, just give it some gray
				lnRgb = rgb(64,64,64);
			}


		/////////
		// Oscillate around these points and draw lines between them
		/////
			for (lfTheta = 0; lfTheta < _2PI; lfTheta += _2PI / 50)
			{
				// Compute our oscillation
				lfDeltaX = cos(lfTheta);
				lfDeltaY = sin(lfTheta);

				// Draw a line between these two points
				iDrawLineArbitraryColorPath(tsWnd, lfX + lfDeltaX * _OUTER_OSCILLATION, lfY + lfDeltaY * _OUTER_OSCILLATION, lfXOrigin + lfDeltaX * _INNER_OSCILLATION, lfYOrigin + lfDeltaY * _INNER_OSCILLATION, 0, lnRgb);
			}


		/////////
		// Oscillate around for a center hub
		/////
			for (lfTheta = 0; lfTheta < _2PI; lfTheta += _2PI / 25)
			{
				// Compute our oscillation
				lfDeltaX = cos(lfTheta);
				lfDeltaY = sin(lfTheta);

				// Draw a line between these two points
				iDrawLineArbitrary(tsWnd, lfXOrigin + lfDeltaX * _OUTER_OSCILLATION, lfYOrigin + lfDeltaY * _OUTER_OSCILLATION, lfXOrigin, lfYOrigin, rgb(128,128,128));
			}
	}




//////////
//
// Called to overlay the actual value in a printed text form
//
//////
	void iGaugeOverlayHighlightedValue(SWindow* tsWnd)
	{
		int		lnRgb, lnRgbDarker;
		float	lfDeltaXInner, lfDeltaYInner, lfDeltaXOuter, lfDeltaYOuter;
		float	lfDeltaX, lfDeltaY, lfTheta, lfXOrigin, lfYOrigin, lfPercent;
		RECT	lrc;
		HGDIOBJ	lhOldFont;
		char	sprintfBuffer[64];
		char	buffer[64];


		//////////
		// Affix our origin
		//////
			lfXOrigin	= (float)tsWnd->bmi.bmiHeader.biWidth  / 2.0f;
			lfYOrigin	= (float)tsWnd->bmi.bmiHeader.biHeight / 2.0f;
		

		//////////
		// Prepare the sprintf buffer
		//////
			if (tsWnd->gauge.highlightedDecimals >= 1)
			{
				// They want decimals
				sprintf_s(sprintfBuffer, sizeof(sprintfBuffer), "%%%u.%ulf\0", tsWnd->gauge.highlightedIntegers + tsWnd->gauge.highlightedDecimals + 1, tsWnd->gauge.highlightedDecimals);

			} else {
				// Just integers
				sprintf_s(sprintfBuffer, sizeof(sprintfBuffer), "%%%u.0lf\0", tsWnd->gauge.highlightedIntegers);
			}


		//////////
		// Setup our HDC for writing properly
		//////
			SetBkMode(tsWnd->hdc2, TRANSPARENT);
			SetTextColor(tsWnd->hdc2, RGB(red(tsWnd->gauge.textRgb), grn(tsWnd->gauge.textRgb), blu(tsWnd->gauge.textRgb)));
			lhOldFont = SelectObject(tsWnd->hdc2, tsWnd->gauge.textHFontBold);


		//////////
		// Compute our relative points
		//////
			// Theta on the left and right
			lfTheta		= _3PI_2 - _PI6;

			// Furthest point away from the origin
			lfDeltaXInner	= lfXOrigin * tsWnd->gauge.scaleFactor * _INNER_RADIUS * cos(lfTheta);
			lfDeltaYInner	= lfYOrigin * tsWnd->gauge.scaleFactor * _INNER_RADIUS * sin(lfTheta);
			lfDeltaXOuter	= lfXOrigin * tsWnd->gauge.scaleFactor * _OUTER_RADIUS * cos(lfTheta);
			lfDeltaYOuter	= lfYOrigin * tsWnd->gauge.scaleFactor * _OUTER_RADIUS * sin(lfTheta);
			lfDeltaX		= (lfDeltaXInner + lfDeltaXOuter) / 2.0f;
			lfDeltaY		= (lfDeltaYInner + lfDeltaYOuter) / 2.0f;


		//////////
		// Build our bounding rectangle
		//////
			SetRect(&lrc,	(int)(lfXOrigin + lfDeltaX + (lfXOrigin * 0.02f)),
							(int)(lfYOrigin - lfDeltaY + (lfYOrigin * 0.05f)),
							(int)(lfXOrigin - lfDeltaX - (lfXOrigin * 0.02f)),
							(int)(((tsWnd->bmi.bmiHeader.biHeight - 1) + (int)(lfYOrigin - lfDeltaY)) / 2) + (int)(lfYOrigin * 0.05f));
			lrc.bottom = lrc.top + (int)((float)(lrc.bottom - lrc.top) * tsWnd->gauge.scaleFactor);


		//////////
		// Draw our rectangle there, and overlay our text atop
		//////
			lfPercent = 100.0f * ((tsWnd->gauge.needleValue - tsWnd->gauge.rangeStart) / (tsWnd->gauge.rangeEnd - tsWnd->gauge.rangeStart));
			if (lfPercent < tsWnd->gauge.rangePercentLow)
			{
				// It's in the low range
				lnRgb		= tsWnd->gauge.lowRgb;
				lnRgbDarker	= makeDarker(tsWnd->gauge.lowRgb);

			} else if (lfPercent < tsWnd->gauge.rangePercentLow + tsWnd->gauge.rangePercentMid) {
				// It's in the middle range
				lnRgb		= tsWnd->gauge.midRgb;
				lnRgbDarker	= makeDarker(tsWnd->gauge.midRgb);

			} else {
				// It's in the high range
				lnRgb		= tsWnd->gauge.highRgb;
				lnRgbDarker	= makeDarker(tsWnd->gauge.highRgb);
			}
			// Draw the box in that color
			iOverlayRectangle(tsWnd, lrc.left, lrc.top, lrc.right, lrc.bottom, lnRgb, lnRgbDarker);

			// Increase the bounding size of the rectangle in case the user went haywire with decimals
			lrc.left	= 0;
			lrc.right	= tsWnd->bmi.bmiHeader.biWidth;

			// All text must be rendered from that location closest to the center
			memset(buffer, 0, sizeof(buffer));
			sprintf_s(buffer, sizeof(buffer), sprintfBuffer, tsWnd->gauge.needleValue);

			// Trim off the spaces
			iLtrimBuffer(buffer);

			// Draw our text
			DrawTextA(tsWnd->hdc2, buffer, strlen(buffer), &lrc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);


		//////////
		// Restore our old font handle
		//////
			SelectObject(tsWnd->hdc2, lhOldFont);
	}




//////////
//
// Called to retrieve the snapped needle value rather than the actual needle value.  This is an
// option designed to keep the needle position inline with actual values when the graduation is
// very small, such as fractions down to the 10,000ths.
//
//////
	float iGaugeGetSnappedNeedleValue(SWindow* tsWnd)
	{
		char	sprintfBuffer[64];
		char	buffer[64];


		//////////
		// Prepare the sprintf buffer
		//////
			if (tsWnd->gauge.highlightedDecimals >= 1)
			{
				// They want decimals
				sprintf_s(sprintfBuffer, sizeof(sprintfBuffer), "%%%u.%ulf\0", tsWnd->gauge.highlightedIntegers + tsWnd->gauge.highlightedDecimals + 1, tsWnd->gauge.highlightedDecimals);

			} else {
				// Just integers
				sprintf_s(sprintfBuffer, sizeof(sprintfBuffer), "%%%u.0lf\0", tsWnd->gauge.highlightedIntegers);
			}


		//////////
		// Assemble our display value
		//////
			memset(buffer, 0, sizeof(buffer));
			sprintf_s(buffer, sizeof(buffer), sprintfBuffer, tsWnd->gauge.needleValue);
			iLtrimBuffer(buffer);


		//////////
		// Get our snapped value
		//////
			return((float)atof(buffer));
	}




//////////
//
// Called to make sure all points of the text are visible.  Moves in on a line toward the origin
// based on the direction of theta
//
//////
	void iAdjustRectangle(SWindow* tsWnd, float tfXOrigin, float tfYOrigin, float tfTheta, RECT* lrc)
	{
		float	lfMultiplier, lfYRadius, lfXRadius;
		int		lnDeltaX, lnDeltaY;
		bool	llFound;


		// Begin at the beginning
		lfYRadius	= tfYOrigin;
		lfXRadius	= tfXOrigin;
		llFound		= false;
		for (lfMultiplier = 1.0f; lfMultiplier >= 0.60f; lfMultiplier -= 0.05f)
		{
			// Compute how much we're adjusting this go around
			lnDeltaX = (int)((lfXRadius * _TEXT_RADIUS * cos(tfTheta)) - (lfXRadius * _TEXT_RADIUS * lfMultiplier * cos(tfTheta)));
			lnDeltaY = (int)((lfYRadius * _TEXT_RADIUS * sin(tfTheta)) - (lfYRadius * _TEXT_RADIUS * lfMultiplier * sin(tfTheta)));

			// See if it's within the bounds
			if (iIsBetween(lrc->left + lnDeltaX, 0, tsWnd->bmi.bmiHeader.biWidth) && iIsBetween(lrc->right + lnDeltaX, 0, tsWnd->bmi.bmiHeader.biWidth))
			{
				// Horizontal is good, how about vertical?
				if (iIsBetween(lrc->top + lnDeltaY, 0, tsWnd->bmi.bmiHeader.biHeight) && iIsBetween(lrc->bottom + lnDeltaY, 0, tsWnd->bmi.bmiHeader.biHeight))
				{
					// Vertical is good, we're there
					llFound = true;
					break;
				}
			}

			// See if it's within the bounds
			lnDeltaX = -lnDeltaX;
			if (iIsBetween(lrc->left + lnDeltaX, 0, tsWnd->bmi.bmiHeader.biWidth) && iIsBetween(lrc->right + lnDeltaX, 0, tsWnd->bmi.bmiHeader.biWidth))
			{
				// Horizontal is good, how about vertical?
				if (iIsBetween(lrc->top + lnDeltaY, 0, tsWnd->bmi.bmiHeader.biHeight) && iIsBetween(lrc->bottom + lnDeltaY, 0, tsWnd->bmi.bmiHeader.biHeight))
				{
					// Vertical is good, we're there
					llFound = true;
					break;
				}
			}
		}
		// When we get here, we're ready to adjust
		lrc->left	+= lnDeltaX;
		lrc->right	+= lnDeltaX;
		lrc->top	+= lnDeltaY;
		lrc->bottom	+= lnDeltaY;
	}




//////////
//
// Checks to see if this is between the bounding range
//
//////
	bool iIsBetween(int tnTestValue, int tnValue1, int tnValue2)
	{
		if (tnValue1 >= tnValue2)
		{
			// tnValue1 is bigger than or equal to tnValue2
			return(tnTestValue <= tnValue1 && tnTestValue >= tnValue2);

		} else {
			// tnValue2 is bigger than or equal to tnValue1
			return(tnTestValue <= tnValue2 && tnTestValue >= tnValue1);
		}
	}




//////////
//
// Moves the string over to skip past leading spaces
//
//////
	void iLtrimBuffer(char* tcData)
	{
		int lnI, lnJ;


		// Make sure we have something to do
		if (tcData)
		{
			// For every character, move it past the leading spaces
			for (lnI = 0; tcData[lnI] == 32; )
				++lnI;
			
			// Are we done?
			if (lnI == 0)
				return;		// Yes, already trimmed

			// Move everything over
			for (lnJ = 0; tcData[lnI] != 0; lnJ++, lnI++)
				tcData[lnJ] = tcData[lnI];

			// Pad with NULLs
			for ( ; lnJ < lnI; lnJ++)
				tcData[lnJ] = 0;
		}
	}




//////////
//
// Called to get the bitmap's actual width rounded up to the nearest dword
//
//////
	int iComputeActualWidth(BITMAPINFOHEADER* tbi)
	{
		if ((tbi->biWidth * 3) % 4 != 0)
		{
			// It has to be rounded up to be dword aligned
			return(tbi->biWidth * 3 + (4 - ((tbi->biWidth * 3) % 4)));

		} else {
			// It is evenly divisible by 4 when multiplied by 3 (dword aligned)
			return(tbi->biWidth * 3);
		}
	}




//////////
//
// Called to render the graph (if it's not already rendering one)
//
//////
	void iRenderTheGraph(SWindow* tsWnd)
	{
		if (!tsWnd->threadBusy)
		{
			// Indicate we're hammered
			tsWnd->threadBusy = true;

			// Spawn the thread
			tsWnd->threadHandle = CreateThread(NULL, 0, buildGraphWorkerThreadProc, (void*)tsWnd, 0, &tsWnd->threadId);
		}
	}




//////////
//
// Called to render the graph (if it's not already rendering one)
//
//////
	void iRenderTheGauge(SWindow* tsWnd)
	{
		if (!tsWnd->threadBusy)
		{
			// Indicate we're hammered
			tsWnd->threadBusy = true;

			// Spawn the thread
			tsWnd->threadHandle = CreateThread(NULL, 0, buildGaugeWorkerThreadProc, (void*)tsWnd, 0, &tsWnd->threadId);
		}
	}




//////////
//
// Worker threads for building the real-time gauges and graphs
//
//////
	DWORD WINAPI buildGaugeWorkerThreadProc(LPVOID lpParameter)
	{
		SWindow* wnd;


		// Restore the parameter
		wnd = (SWindow*)lpParameter;
		if (wnd)
		{
			// Fill the background color
			iGradient4Fill(wnd);

			// Draw the basic gauge
			iGaugeRenderBasic(wnd);

			// Overlay gauge
			iGaugeOverlayGraduation(wnd);

			// Overlay needle
			iGaugeOverlayNeedle(wnd);

			// Overlay value
			iGaugeOverlayHighlightedValue(wnd);

			// All done!
			// Refresh the graph for the redraw
			InvalidateRect(wnd->hwnd, NULL, false);
		}

		// Thread terminates
		CloseHandle(wnd->threadHandle);
		wnd->threadBusy = false;
		ExitThread(0);
	}

	DWORD WINAPI buildGraphWorkerThreadProc(LPVOID lpParameter)
	{
		SWindow* wnd;


		// Restore the parameter
		wnd = (SWindow*)lpParameter;
		if (wnd)
		{
			// Fill the background color
			iGradient4Fill(wnd);

			// Draw the graph
			iGraphDrawBars(wnd);

			// Overlay the graduation
			iGraphOverlayGraduation(wnd);

			// Show the grid
			if (wnd->graph.gridVisible != 0)
				iGraphDrawGrid(wnd);

			// Populate the data points
			iGraphPopulateDataPoints(wnd);

			// All done!
			// Refresh the graph for the redraw
			InvalidateRect(wnd->hwnd, NULL, false);
		}

		// Thread terminates
		CloseHandle(wnd->threadHandle);
		wnd->threadBusy = false;
		ExitThread(0);
	}




//////////
//
// Callback to intercept the HWND to draw the overlain color image
//
//////
	LRESULT CALLBACK realtimeWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		HDC			lhdc;
		PAINTSTRUCT ps;
		SWindow*	wnd;
		

		// If we are painting, paint our areas
		if (root)
		{
			wnd = iLocateWindow((int)hwnd);
			if (wnd)
			{
				if (uMsg == WM_PAINT)
				{
					// Begin painting
					lhdc = BeginPaint(hwnd, &ps);

					// Draw our bitmap
					BitBlt(	lhdc, 
							0, 0, wnd->rcThis.right, wnd->rcThis.bottom,
							wnd->hdc2, 0, 0, SRCCOPY);

					// End painting
					EndPaint(hwnd, &ps);
					ValidateRect(hwnd, &wnd->rcThis);

					// Validate the rectangle of the parent
					ValidateRect(wnd->hwndParent, &wnd->rcParent);
					return(0);

				} else if (uMsg == WM_ERASEBKGND) {
					// Ignore it
					return(0);
				}
			}
		}
		// Do normal drawing
		return(DefWindowProc(hwnd, uMsg, wParam, lParam));
	}
