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
			InitializeCriticalSection(&gcsUniqueId);
			break;
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
//		tnType						= 0=gauge, 1=graph, 2=mover, others undefined
//		tnx,tnY,tnWidth,tnHeight	= Bounding rectangle on thisForm where the gauge or graph should be drawn
//		tnUlRgb...tnLlRgb			= RGB(r,g,b) colors for each of the ordinal points on the rectangle
//
// Returns:
//		int							= a handle to the new window created on thisForm
//									  Note:  Use this handle for all other tnHandle requests
//
//////
	REALTIME_API int realtime_subclass_form_as(HWND tnHwndParent, int tnType, int tnX, int tnY, int tnWidth, int tnHeight, int tnUlRgb, int tnUrRgb, int tnLrRgb, int tnLlRgb, int tnCaptureUlX, int tnCaptureUlY)
	{
		SWindow*	wnd;
		WNDCLASSEX	wcex;


		// Are we in test mode?
		if (glTestMode)
			return(0);

		//////////
		// Make sure our type is correct
		//////
			if (tnType < 0 || tnType > _TYPE_MAX)
				return(-1);		// Failure, outside of recognized type range


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
			wnd->hwnd	= CreateWindowEx(0/*WS_EX_NOPARENTNOTIFY*/, L"realtime", L"realtime", WS_CHILD, tnX, tnY, tnWidth, tnHeight, wnd->hwndParent, NULL, GetModuleHandle(NULL), NULL);

			// Are we good?
			if (wnd->hwnd)
			{
				// Yes!
				int error		= GetLastError();
				wnd->hdc1		= GetDC(wnd->hwnd);
				wnd->bmpMain.hdc	= CreateCompatibleDC(wnd->hdc1);
				wnd->bmpBackground.hdc	= CreateCompatibleDC(wnd->hdc1);


			//////////
			// Create a DIB section of the appropriate size
			//////
				iPopulateEmpty24BitBitmapStructure(&wnd->bmpMain, tnWidth, tnHeight);
				wnd->bmpMain.hbmp = CreateDIBSection(wnd->hdc1, &wnd->bmpMain.bmi, DIB_RGB_COLORS, (void**)&wnd->bmpMain.bits, NULL, 0);

				// Put the bitmap into the dc2
				SelectObject(wnd->bmpMain.hdc, wnd->bmpMain.hbmp);

				// Have given us coordinates to capture the background bitmap image?
				if (tnCaptureUlX >= 0 && tnCaptureUlY >= 0)
				{
					//////////
					// Create a DIB section of the appropriate size
					//////
						iPopulateEmpty24BitBitmapStructure(&wnd->bmpBackground, tnWidth, tnHeight);
						wnd->bmpBackground.hbmp = CreateDIBSection(wnd->hdc1, &wnd->bmpBackground.bmi, DIB_RGB_COLORS, (void**)&wnd->bmpBackground.bits, NULL, 0);

					// Put the bitmap into the dc3
					SelectObject(wnd->bmpBackground.hdc, wnd->bmpBackground.hbmp);

					// Do the physical capture of the bitmap data there
					BitBlt(wnd->bmpBackground.hdc, 0, 0, tnWidth, tnHeight, GetDC(tnHwndParent), tnCaptureUlX, tnCaptureUlY, SRCCOPY);

// For debugging:
//					// Write it out to a file for examination by Rick, the developer, who is unsure sometimes of his own abilities, be they though gifts from God ... it is Rick to which he considers the possibility of failure, and not of God. :-)
//					BITMAPFILEHEADER bfh;
//					memset(&bfh, 0, sizeof(bfh));
//					bfh.bfType		= 'MB';
//					bfh.bfOffBits	= sizeof(bfh) + sizeof(wnd->bmi3.bmiHeader);
//					bfh.bfSize		= bfh.bfOffBits + wnd->bmi3.bmiHeader.biSizeImage;
//
//					FILE* lfh;
//					fopen_s(&lfh, "\\temp\\test.bmp", "wb+");
//					fwrite(&bfh, 1, sizeof(bfh), lfh);
//					fwrite(&wnd->bmi3.bmiHeader, 1, sizeof(wnd->bmi3.bmiHeader), lfh);
//					fwrite(wnd->bits3, 1, wnd->bmi3.bmiHeader.biSizeImage, lfh);
//					fclose(lfh);
				}

				// Render the gradient fill colors, or overlay the bitmap
				iGradient4FillOrBitmapOverlay(wnd, &wnd->bmpMain, &wnd->bmpBackground);


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


		// Make sure we have something to subclass
		if (gsRootWind)
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
		if (gsRootWind)
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
// Called to setup a new progress bar.
//
//////
	REALTIME_API void realtime_progressbar_setup(int tnHandle, float tfRangeMin, float tfRange1, float tfRange2, float tfRangeMax, int tnColor1, int tnColor2, int tnColor3, int tnBorderColor, int tnNeedleColor, int tnShowBorder, int tnShowNeedle, char* tcOrientation)
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
			wnd->pbar.fRangeMin			= tfRangeMin;
			wnd->pbar.fRange1			= tfRange1;
			wnd->pbar.fRange2			= tfRange2;
			wnd->pbar.fRangeMax			= tfRangeMax;
			wnd->pbar.fValue			= tfRangeMin;

			wnd->pbar.nColor1			= tnColor1;
			wnd->pbar.nColor2			= tnColor2;
			wnd->pbar.nColor3			= tnColor3;
			wnd->pbar.nBorderColor		= tnBorderColor;
			wnd->pbar.nNeedleColor		= tnNeedleColor;

			wnd->pbar.lShowBorder		= (tnShowBorder != 0);
			wnd->pbar.lShowNeedle		= (tnShowNeedle != 0);

			switch (tcOrientation[0])
			{
				case 'n':	// Vertical upward
				case 'N':
					wnd->pbar.nOrientation	= _DIRECTION_NORTH;
					break;

				default:
				case 'e':	// Horizontal right
				case 'E':
					wnd->pbar.nOrientation	= _DIRECTION_EAST;
					break;

				case 's':	// Vertical downward
				case 'S':
					wnd->pbar.nOrientation	= _DIRECTION_SOUTH;
					break;

				case 'w':	// Horizontal left
				case 'W':
					wnd->pbar.nOrientation	= _DIRECTION_WEST;
					break;
			}
		}
	}




//////////
//
// Called to setup a new progress bar.
//
//////
	REALTIME_API void realtime_progressbar_set_needle_position(int tnHandle, float tfValue)
	{
		SWindow*	wnd;


		// Are we in test mode?
		if (glTestMode)
			return;

		// Grab our window
		wnd = iLocateWindow(tnHandle);
		if (wnd)
			wnd->pbar.fValue = tfValue;
	}




//////////
//
// Called to setup a new progress bar.
//
//////
	REALTIME_API void realtime_progressbar_redraw(int tnHandle)
	{
		SWindow* wnd;


		// Are we in test mode?
		if (glTestMode)
			return;

		// Grab our window
		wnd = iLocateWindow(tnHandle);
		if (wnd)
			iRender(wnd);			// Signal a refresh
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
			iRender(wnd);			// Signal a refresh
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
			iRender(wnd);			// Signal a refresh
	}




//////////
//
// Creates a mover area where drag, drop, move, and insert animations can be handled
//
//////
	REALTIME_API void realtime_mover_setup(int tnHandle, int tnMarginVertical, int tnMarginHorizontal)
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
			wnd->mover.marginVertical		= tnMarginVertical;
			wnd->mover.marginHorizontal		= tnMarginHorizontal;
			if (wnd->mover.timer == NULL)
				wnd->mover.timer = SetTimer(NULL, (UINT_PTR)wnd, 20, &iiMoverTimerProc);

			// Redraw
			iRender(wnd);
		}
	}




	REALTIME_API int realtime_mover_create_object_with_text(int tnHandle, int tnWidth, int tnHeight, char* tcText, int tnTextLength, int tnBackRgb, int tnForeRgb, float tfAlpha, char* tcFontName, int tnFontSize, int tnBold, int tnItalics, int tnUnderline, int tnBorderRgb, int tnBorderThickness)
	{
		int			lnI, lnHeight, lnUlX, lnLrX, lnUlY, lnLrY;
		SWindow*	wnd;
		SMoverObj*	objNew;
		SBitmap		bmp;
		HFONT		lhfont1, lhfont2;
		RECT		lrc, lrc2;


		// Are we in test mode?
		if (glTestMode)
			return(-1);

		// Grab our window
		wnd = iLocateWindow(tnHandle);
		if (wnd)
		{
			// Create a new object
			objNew = iMoverAppendNewObject(wnd);
			if (objNew)
			{
				// Indicate that this object was not actually acquired from something graphical, but was created
				objNew->ox									= -1;
				objNew->oy									= -1;
				objNew->ofile								= NULL;
				objNew->oobjectId							= -1;

				// Create a DIB section of the appropriate size
				iPopulateEmpty24BitBitmapStructure(&objNew->bmp, tnWidth, tnHeight);
				objNew->bmp.hdc								= CreateCompatibleDC(wnd->hdc1);
				objNew->bmp.hbmp							= CreateDIBSection(objNew->bmp.hdc, &objNew->bmp.bmi, DIB_RGB_COLORS, (void**)&objNew->bmp.bits, NULL, 0);
				SelectObject(objNew->bmp.hdc, objNew->bmp.hbmp);

				// Create a temporary sister
				iPopulateEmpty24BitBitmapStructure(&bmp, tnWidth, tnHeight);
				bmp.hdc								= CreateCompatibleDC(wnd->hdc1);
				bmp.hbmp							= CreateDIBSection(bmp.hdc, &bmp.bmi, DIB_RGB_COLORS, (void**)&bmp.bits, NULL, 0);
				SelectObject(bmp.hdc, bmp.hbmp);

				// Create the font
				lnHeight	= -MulDiv(tnFontSize, GetDeviceCaps(bmp.hdc, LOGPIXELSY), 72);
				lhfont1		= CreateFontA(lnHeight, 0, 0, 0, ((tnBold != 0) ? FW_BOLD : FW_NORMAL), (tnItalics != 0), (tnUnderline != 0), false, ANSI_CHARSET, 0, 0, 0, 0, tcFontName);
				lhfont2		= (HFONT)SelectObject(bmp.hdc, lhfont1);

				// Find out how big the text will be with the indicated font
				SetRect(&lrc, 0, 0, tnWidth-1, tnHeight-1);
				DrawTextA(bmp.hdc, tcText, tnTextLength, &lrc, DT_CALCRECT);

				// Adjust the rectangle based on the graphics size to fit in our rectangle
				lrc2.left	= (tnWidth / 2) - ((lrc.right - lrc.left) / 2);
				lrc2.top	= (tnHeight / 2) - ((lrc.bottom - lrc.top) / 2);
				lrc2.right	= lrc2.left + (lrc.right - lrc.left);
				lrc2.bottom	= lrc2.top + (lrc.bottom - lrc.top);

				// Make sure it's within boundaries
				if (lrc2.left < 0)				lrc2.left = 0;
				if (lrc2.top < 0)				lrc2.top = 0;
				if (lrc2.right > tnWidth)		lrc2.right = tnWidth;
				if (lrc2.bottom > tnHeight)		lrc2.bottom = tnHeight;

				// Actually render the text in black and white onto the sister bitmap
				iFillRect(&bmp, rgb(0,0,0));
				SetTextColor(bmp.hdc, RGB(255,255,255));
				SetBkColor(bmp.hdc, RGB(0,0,0));
				SetBkMode(bmp.hdc, TRANSPARENT);
				DrawTextA(bmp.hdc, tcText, tnTextLength, &lrc2, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS);

				// Put the original font back in, and delete our font handle
				SelectObject(bmp.hdc, lhfont2);
				DeleteObject((HGDIOBJ)lhfont1);

				// Overlay the sister bitmap atop the original bitmap giving it the appropriate alpha channel consideration
				iFillRect(&objNew->bmp, tnBackRgb);
				//iOverlayBitmapViaMethod(&objNew->bmp, &bmp, 0, 0, _METHOD_OPAQUE, 0.0, -1);
				iOverlayBitmapViaColorizingAlphaBlend(&objNew->bmp, &bmp, tnBackRgb, tnForeRgb, tfAlpha);

				// Put a border around it
				lnUlX	= 0;
				lnUlY	= 0;
				lnLrX	= tnWidth - 1;
				lnLrY	= tnHeight - 1;
				for (lnI = 0; lnI < tnBorderThickness && lnUlX < lnLrX && lnUlY < lnLrY; lnI++)
				{
					// Top, bottom, right, left
					iDrawLineHorizontal(NULL, &objNew->bmp, lnUlX, lnLrX, lnUlY, tnBorderRgb);
					iDrawLineHorizontal(NULL, &objNew->bmp, lnUlX, lnLrX, lnLrY, tnBorderRgb);
					iDrawLineVertical(NULL, &objNew->bmp, lnLrX, lnUlY, lnLrY, tnBorderRgb);
					iDrawLineVertical(NULL, &objNew->bmp, lnUlX, lnUlY, lnLrY, tnBorderRgb);

					// Move everything in one
					++lnUlX;
					++lnUlY;
					--lnLrX;
					--lnLrY;
				}

				// Delete the sister bitmap

// 				// Write it out to a file for examination by Rick, the developer, who is unsure sometimes of his own abilities, be they though gifts from God ... it is Rick to which he considers the possibility of failure, and not of God. :-)
// 				BITMAPFILEHEADER bfh;
// 				memset(&bfh, 0, sizeof(bfh));
// 				bfh.bfType		= 'MB';
// 				bfh.bfOffBits	= sizeof(bfh) + sizeof(objNew->bmi.bmiHeader);
// 				bfh.bfSize		= bfh.bfOffBits + objNew->bmi.bmiHeader.biSizeImage;
// 
// 				FILE* lfh;
// 				fopen_s(&lfh, "\\temp\\test.bmp", "wb+");
// 				fwrite(&bfh, 1, sizeof(bfh), lfh);
// 				fwrite(&objNew->bmi.bmiHeader, 1, sizeof(objNew->bmi.bmiHeader), lfh);
// 				fwrite(objNew->bits, 1, objNew->bmi.bmiHeader.biSizeImage, lfh);
// 				fclose(lfh);
			}
		}
		return(objNew->objectId);
	}




//////////
//
// Called to read the VFP form and acquire whatever's at the rectangular coordinates
//
//////
	REALTIME_API int realtime_mover_acquire_object_from_rect(int tnHandle, int tnHwndParent, int tnUlX, int tnUlY, int tnLrX, int tnLrY)
	{
		SWindow*	wnd;
		SMoverObj*	objNew;


		// Are we in test mode?
		if (glTestMode)
			return(-1);

		// Grab our window
		wnd = iLocateWindow(tnHandle);
		if (wnd)
		{
			// Create a new object
			objNew = iMoverAppendNewObject(wnd);
			if (objNew)
			{
				// Store the original coordinates
				objNew->ox									= tnUlX;
				objNew->oy									= tnUlY;
				objNew->ofile								= NULL;
				objNew->oobjectId							= -1;

				// Create a DIB section of the appropriate size
				iPopulateEmpty24BitBitmapStructure(&objNew->bmp, tnLrX - tnUlX, tnLrY - tnUlY);
				objNew->bmp.bmi.bmiHeader.biSizeImage		= objNew->bmp.actualWidth * (tnLrY - tnUlY);
				objNew->bmp.hdc								= CreateCompatibleDC(wnd->hdc1);
				objNew->bmp.hbmp							= CreateDIBSection(objNew->bmp.hdc, &objNew->bmp.bmi, DIB_RGB_COLORS, (void**)&objNew->bmp.bits, NULL, 0);

				// Put the bitmap into the dc
				SelectObject(objNew->bmp.hdc, objNew->bmp.hbmp);

				// Do the physical capture of the bitmap data there
				BitBlt(objNew->bmp.hdc, 0, 0, tnLrX - tnUlX, tnLrY - tnUlY, GetDC((HWND)tnHwndParent), tnUlX, tnUlY, SRCCOPY);

// 				// Write it out to a file for examination by Rick, the developer, who is unsure sometimes of his own abilities, be they though gifts from God ... it is Rick to which he considers the possibility of failure, and not of God. :-)
// 				BITMAPFILEHEADER bfh;
// 				memset(&bfh, 0, sizeof(bfh));
// 				bfh.bfType		= 'MB';
// 				bfh.bfOffBits	= sizeof(bfh) + sizeof(objNew->bmi.bmiHeader);
// 				bfh.bfSize		= bfh.bfOffBits + objNew->bmi.bmiHeader.biSizeImage;
// 
// 				FILE* lfh;
// 				fopen_s(&lfh, "\\temp\\test.bmp", "wb+");
// 				fwrite(&bfh, 1, sizeof(bfh), lfh);
// 				fwrite(&objNew->bmi.bmiHeader, 1, sizeof(objNew->bmi.bmiHeader), lfh);
// 				fwrite(objNew->bits, 1, objNew->bmi.bmiHeader.biSizeImage, lfh);
// 				fclose(lfh);
			}
		}
		return(objNew->objectId);
	}




//////////
//
// Called to load the indicated 24-bit bitmap from disk, and create an object based on its image
//
//////
	REALTIME_API int realtime_mover_acquire_from_file(int tnHandle, char* tcBmp24Name, int tnBmp24NameLength)
	{
		int					lnResult, lnNumread1, lnNumread2, lnNumread3;
		SWindow*			wnd;
		SMoverObj*			objNew;
		FILE*				lfh;
		char				lcFilename[_MAX_PATH];
		BITMAPFILEHEADER	lbh;
		BITMAPINFOHEADER	lbi;
		char*				lbd;


		// Are we in test mode?
		if (glTestMode)
			return 0;

		lnResult = -1;
		wnd = iLocateWindow(tnHandle);
		if (wnd)
		{
			// Prepare the filename
			memset(lcFilename, 0, sizeof(lcFilename));
			memcpy(lcFilename, tcBmp24Name, min(sizeof(lcFilename) - 1, tnBmp24NameLength));

			// Try to open the file
			if (fopen_s(&lfh, lcFilename, "rb") == 0)
			{
				// The disk header may not be as complex as our lbi, so set everything else to NULL
				memset(&lbi, 0, sizeof(lbi));

				// Try to read the header
				lnNumread1	= fread(&lbh, 1, sizeof(lbh), lfh);
				lnNumread2	= fread(&lbi, 1, sizeof(lbi), lfh);
				if (lnNumread1 == sizeof(lbh) && lnNumread2 == sizeof(lbi) && iIsValid24BitBitmap(&lbh, &lbi))
				{
					// Read in the data bits
					lbd = (char*)malloc(lbi.biSizeImage);
					if (lbd)
					{
						// Load in the data
						fseek(lfh, lbh.bfOffBits, SEEK_SET);
						lnNumread3 = fread(lbd, 1, lbi.biSizeImage, lfh);
						if (lnNumread3 = lbi.biSizeImage)
						{
							// Create the object
							objNew = iMoverAppendNewObject(wnd);
							if (objNew)
							{
								// Indicate it was not originally acquired
								objNew->ox									= -1;
								objNew->oy									= -1;
								objNew->ofile								= iDuplicateString(lcFilename, strlen(lcFilename), true);
								objNew->oobjectId							= -1;

								// Copy the loaded data over to the new object
								memcpy(&objNew->bmp.bmi.bmiHeader, &lbi, min(sizeof(lbi), sizeof(objNew->bmp.bmi.bmiHeader)));
								objNew->bmp.bmi.bmiHeader.biSize = sizeof(objNew->bmp.bmi.bmiHeader);

								// Setup object constants
								objNew->bmp.actualWidth						= iComputeActualWidth(&objNew->bmp.bmi.bmiHeader);
								objNew->bmp.bmi.bmiHeader.biSizeImage		= objNew->bmp.actualWidth * lbi.biHeight;
								objNew->bmp.hdc								= CreateCompatibleDC(wnd->hdc1);
								objNew->bmp.hbmp							= CreateDIBSection(objNew->bmp.hdc, &objNew->bmp.bmi, DIB_RGB_COLORS, (void**)&objNew->bmp.bits, NULL, 0);

								// Put the bitmap into the dc
								SelectObject(objNew->bmp.hdc, objNew->bmp.hbmp);

								// Copy over the bitmap bits
								memcpy(objNew->bmp.bits, lbd, objNew->bmp.bmi.bmiHeader.biSizeImage);
								
								// Free our now redundant copy of the bits
								free(lbd);

								// Indicate our success
								lnResult = objNew->objectId;
							}
						}
					}
				}
				// Close the file
				fclose(lfh);
			}
		}
		// Indicate our success
		return(lnResult);
	}




//////////
//
// Called to create an object based on part or all of an existing object
//
//////
	REALTIME_API int realtime_mover_acquire_inner_rect(int tnHandle, int tnObjectId, int tnUlX, int tnUlY, int tnLrX, int tnLrY)
	{
		int				lnResult;
		SWindow*		wnd;
		SMoverObj*		obj;
		SMoverObj*		objNew;


		// Are we in test mode?
		if (glTestMode)
			return 0;

		// Locate the window
		lnResult = -1;
		wnd = iLocateWindow(tnHandle);
		if (wnd)
		{
			obj = iMoverLocateObject(wnd, tnObjectId);
			if (obj)
			{
				// Create a new object
				objNew = iMoverAppendNewObject(wnd);
				if (objNew)
				{
					// See if they want all of some of it extracted
					if (tnUlX == -1)	tnUlX = 0;
					if (tnUlY == -1)	tnUlY = 0;
					if (tnLrX == -1)	tnLrX = obj->bmp.bmi.bmiHeader.biWidth;
					if (tnLrY == -1)	tnLrY = obj->bmp.bmi.bmiHeader.biHeight;

					// Extract the indicated portion
					iExtractBitmap(&objNew->bmp, &obj->bmp, wnd->hdc1, tnUlX, tnUlY, tnLrX, tnLrY);

					// Indicate that this object was not actually acquired from something raw
					objNew->ox			= tnUlX;
					objNew->oy			= tnUlY;
					objNew->ofile		= NULL;
					objNew->oobjectId	= tnObjectId;

					// All done!
					lnResult = objNew->objectId;
				}
			}
		}
		// Indicate our status
		return(lnResult);
	}




//////////
//
// Save the indicated object to disk
//
//////
	REALTIME_API int realtime_mover_save_object(int tnHandle, int tnObjectId, char* tcFilename, int tnFilenameLength)
	{
		int					lnResult, lnNumWritten1, lnNumWritten2, lnNumWritten3;
		FILE*				lfh;
		char				lcFilename[_MAX_PATH];
		SWindow*			wnd;
		SMoverObj*			obj;
		BITMAPFILEHEADER	bfh;


		// Are we in test mode?
		if (glTestMode)
			return 0;

		lnResult = -1;
		wnd = iLocateWindow(tnHandle);
		if (wnd && tcFilename && tnFilenameLength != 0)
		{
			obj = iMoverLocateObject(wnd, tnObjectId);
			if (obj)
			{
				// Prepare the filename
				memset(lcFilename, 0, sizeof(lcFilename));
				memcpy(lcFilename, tcFilename, min(sizeof(lcFilename) - 1, tnFilenameLength));

				// Create the disk header
				memset(&bfh, 0, sizeof(bfh));
				bfh.bfType		= 'MB';
				bfh.bfOffBits	= sizeof(bfh) + sizeof(obj->bmp.bmi.bmiHeader);
				bfh.bfSize		= bfh.bfOffBits + obj->bmp.bmi.bmiHeader.biSizeImage;

				// Try to create the indicated file
				if (fopen_s(&lfh, lcFilename, "wb+") == 0)
				{
					lnNumWritten1	= fwrite(&bfh,						1, sizeof(bfh),							lfh);
					lnNumWritten2	= fwrite(&obj->bmp.bmi.bmiHeader,	1, sizeof(obj->bmp.bmi.bmiHeader),		lfh);
					lnNumWritten3	= fwrite(obj->bmp.bits,				1, obj->bmp.bmi.bmiHeader.biSizeImage,	lfh);
					fclose(lfh);

					// See if we were successful
					lnResult = ((lnNumWritten1 == sizeof(bfh)) && (lnNumWritten2 = sizeof(obj->bmp.bmi.bmiHeader)) && (lnNumWritten3 && obj->bmp.bmi.bmiHeader.biSizeImage));
				}
			}
		}
		// Indicate our success
		return(lnResult);
	}




REALTIME_API int realtime_mover_set_visible(int tnHandle, int tnObjectId, int tnVisible)
{
	SWindow*	wnd;
	SMoverObj*	obj;


	// Are we in test mode?
	if (glTestMode)
		return 0;

	// Grab our window
	wnd = iLocateWindow(tnHandle);
	if (wnd)
	{
		// Search for the indicated object
		obj = iMoverLocateObject(wnd, tnObjectId);
		if (obj)
		{
			obj->visible = tnVisible;
			return(tnObjectId);
		}
	}
	// Failure
	return(0);
}




REALTIME_API int realtime_mover_set_disposition_object(int tnHandle, int tnObjectId, int tnDispositionObjectId, int tnDisposition)
{
	SWindow*	wnd;
	SMoverObj*	obj;
	SMoverObj*	objDisp;


	// Are we in test mode?
	if (glTestMode)
		return 0;

	// Grab our window
	wnd = iLocateWindow(tnHandle);
	if (wnd)
	{
		// Search for the indicated object
		obj = iMoverLocateObject(wnd, tnObjectId);
		if (obj)
		{
			// Try to find the disposition object
			objDisp = iMoverLocateObject(wnd, tnDispositionObjectId);
			if (objDisp)
			{
				// Store it
				switch (tnDisposition)
				{
					case _DISP_NORMAL:
						obj->dispNormal = tnDispositionObjectId;
						break;
					case _DISP_OVER:
						obj->dispOver = tnDispositionObjectId;
						break;
					case _DISP_DOWN:
						obj->dispDown = tnDispositionObjectId;
						break;
					case _DISP_HOVER:
						obj->dispHover = tnDispositionObjectId;
						break;
					case _DISP_DRAGGING:
						obj->dispDragging = tnDispositionObjectId;
						break;
					default:
						return(0);		// Unknown disposition
				}
				// If we get here, we're good
				return(tnObjectId);
			}
		}
	}
	// Failure
	return(0);
}




REALTIME_API int realtime_mover_set_event_mask(int tnHandle, int tnObjectId, int tnClick, int tnRightClick, int tnMouseMove, int tnMouseEnter, int tnMouseLeave, int tnDragStart, int tnDragMove, int tnDragAbort, int tnDragDrop, int tnHover)
{
	SWindow*	wnd;
	SMoverObj*	obj;


	// Are we in test mode?
	if (glTestMode)
		return 0;

	// Grab our window
	wnd = iLocateWindow(tnHandle);
	if (wnd)
	{
		// Search for the indicated object
		obj = iMoverLocateObject(wnd, tnObjectId);
		if (obj)
		{
			// Store the event mask settings
			obj->eventClick			= tnClick;						// Responds to left-click events
			obj->eventRightClick	= tnRightClick;					// Responds to right-click events
			obj->eventMouseMove		= tnMouseMove;					// Responds to mouse move events
			obj->eventMouseEnter	= tnMouseEnter;					// Responds to mouse enter events
			obj->eventMouseLeave	= tnMouseLeave;					// Responds to mouse leave events
			obj->eventDragStart		= tnDragStart;					// Responds to drag start events
			obj->eventDragMove		= tnDragMove;					// Responds to drag move events (in lieu of mouseMove events while dragging)
			obj->eventDragAbort		= tnDragAbort;					// Responds to drag abort events (drag was not completed, dropped in some non-droppable area)
			obj->eventDragDrop		= tnDragDrop;					// Responds to drag drop events (dropped in a droppable area)
			obj->eventHover			= tnHover;						// Responds to hover events

			// We're good!
			return(tnObjectId);
		}
	}
	// Failure
	return(0);
}




REALTIME_API int realtime_mover_overlay_object(int tnhandle, int tnObjectId, int tnOverlayObjectId, int tnX, int tnY, int tnOverlayMethod, float tfAlp, int tnRgbMask)
{
	SWindow*	wnd;
	SMoverObj*	objDst;
	SMoverObj*	objSrc;


	// Are we in test mode?
	if (glTestMode)
		return 0;

	// Grab our window
	wnd = iLocateWindow(tnhandle);
	if (wnd)
	{
		// Locate our objects
		objDst = iMoverLocateObject(wnd, tnObjectId);
		objSrc = iMoverLocateObject(wnd, tnOverlayObjectId);
		if (objDst && objSrc)
			iOverlayBitmapViaMethod(&objDst->bmp, &objSrc->bmp, tnX, tnY, tnOverlayMethod, tfAlp, tnRgbMask);
	}
	// Failure
	return(0);
}




REALTIME_API int realtime_mover_reacquire(int tnHandle, int tnObjectId)
{
	SWindow*	wnd;
	SMoverObj*	obj;


	// Are we in test mode?
	if (glTestMode)
		return 0;

	// Grab our window
	wnd = iLocateWindow(tnHandle);
	if (wnd)
	{
		obj = iMoverLocateObject(wnd, tnObjectId);
		if (obj)
		{
			// Do the physical capture of the bitmap data there
			BitBlt(obj->bmp.hdc, 0, 0, obj->bmp.bmi.bmiHeader.biWidth, obj->bmp.bmi.bmiHeader.biHeight, GetDC((HWND)wnd->hwndParent), obj->ox, obj->oy, SRCCOPY);
		}
	}

	// Failure
	return(0);

}




REALTIME_API int realtime_mover_delete_object(int tnHandle, int tnObjectId)
{
	SWindow* wnd;
	

	// Are we in test mode?
	if (glTestMode)
		return 0;

	// Grab our window
	wnd = iLocateWindow(tnHandle);
	if (wnd)
		return(iMoverDeleteObject(wnd, tnObjectId));

	// Failure
	return(0);
}




//////////
//
// Specifies information about an object
//
//////
	REALTIME_API int realtime_mover_setup_object(int tnHandle, int tnObjectId, int tnCol, int tnRow, int tnDraggable, int tnAcceptsDrops, int tnCopiesOnDrop/*if no, moves on drop*/, int tnCallbackCode)
	{
		SWindow*	wnd;
		SMoverObj*	obj;


		// Are we in test mode?
		if (glTestMode)
			return(-1);

		// Grab our window
		wnd = iLocateWindow(tnHandle);
		if (wnd)
		{
			// Search for the indicated object
			obj = iMoverLocateObject(wnd, tnObjectId);
			if (obj)
			{
				// Set the values
				obj->col			= tnCol;				// Column for this item
				obj->row			= tnRow;				// Row for this item
				obj->real.col		= tnCol;				// Col for this item
				obj->real.row		= tnRow;				// Row for this item
				obj->draggable		= tnDraggable;			// Is this item draggable?
				obj->acceptsDrops	= tnAcceptsDrops;		// Does this item accept other items dropped onto it?
				obj->copiesOnDrop	= tnCopiesOnDrop;		// if no, moves on drop
				obj->callbackCode	= tnCallbackCode;		// if non-zero, then signals the parent hwnd of events

				// We're good
				return(0);
			}
			// If we get here, object not found for window
			return(-1);
		}
		// If we get here, window not found
		return(-2);
	}




	// Called to recompute the actual mover position of each part
	REALTIME_API int realtime_mover_recompute(int tnHandle)
	{
		int			lnCol, lnFoundRow, lnWidthMid, lnWidthEdge, lnHeightMid, lnHeightEdge, lnMarginVertical, lnMarginHorizontal, lnMarginVerticalHalf, lnMarginHorizontalHalf, lnWidthFull, lnHeightFull;
		SWindow*	wnd;
		SMoverObj*	obj;
		SMoverObj*	objExtent;


		// Are we in test mode?
		if (glTestMode)
			return(-1);

		// Grab our window
		wnd = iLocateWindow(tnHandle);
		if (wnd)
		{
			// Search for the indicated object
			iMoverGetObjectExtents(wnd, &wnd->mover.maxCol, &wnd->mover.maxRow, &wnd->mover.maxWidth, &wnd->mover.maxHeight);

			// Create our constants
			lnMarginVertical		= wnd->mover.marginVertical;
			lnMarginHorizontal		= wnd->mover.marginHorizontal;
			lnMarginVerticalHalf	= lnMarginVertical / 2;
			lnMarginHorizontalHalf	= lnMarginHorizontal / 2;

			// Iterate through the items, positioning them
			obj = wnd->mover.firstObject;
			while (obj)
			{
				if (obj->visible != 0)
				{
					// Position this item
					obj->real.x			= (float)(lnMarginHorizontal + (obj->real.col - 1) * (wnd->mover.maxWidth  + lnMarginHorizontal));
					obj->real.y			= (float)(lnMarginVertical + (obj->real.row - 1) * (wnd->mover.maxHeight + lnMarginVertical));
					obj->real.width		= obj->bmp.bmi.bmiHeader.biWidth;
					obj->real.height	= obj->bmp.bmi.bmiHeader.biHeight;

					// Copy real to current
					memcpy(&obj->curr, &obj->real, sizeof(SMoverPos));
					iiMoverAnimateTo(wnd, obj, obj->real.col, obj->real.row);

					// Recompute the snap positions
					lnWidthFull		= obj->bmp.bmi.bmiHeader.biWidth;
					lnHeightFull	= obj->bmp.bmi.bmiHeader.biHeight;
					lnWidthMid		= (int)((float)lnWidthFull  * _MIDDLE_WIDTH);
					lnWidthEdge		= (int)((float)lnWidthFull  * _EDGE_WIDTH);
					lnHeightMid		= (int)((float)lnHeightFull * _MIDDLE_HEIGHT);
					lnHeightEdge	= (int)((float)lnHeightFull * _EDGE_HEIGHT);

					// The basic directions can be thought of as pieces of tape laid atop the original rectangle object.
					// There's tape in the middle 70% of the top, bottom, as well as the full height of the sides.
					iiMoverSetPosition(&obj->snapNorth,	&obj->curr,	_DIRECTION_NORTH,	(int)obj->real.x + lnWidthEdge,					(int)obj->real.y - lnMarginVerticalHalf,			lnWidthMid,									lnMarginVerticalHalf  + lnHeightEdge);
					iiMoverSetPosition(&obj->snapSouth,	&obj->curr,	_DIRECTION_SOUTH,	(int)obj->real.x + lnWidthEdge,					(int)obj->real.y + lnHeightFull - lnHeightEdge,		lnWidthMid,									lnHeightEdge + lnMarginVerticalHalf);
					iiMoverSetPosition(&obj->snapWest,	&obj->curr,	_DIRECTION_WEST,	(int)obj->real.x - lnMarginHorizontalHalf,		(int)obj->real.y,									lnMarginHorizontalHalf + lnWidthEdge,		lnHeightFull);
					iiMoverSetPosition(&obj->snapEast,	&obj->curr,	_DIRECTION_EAST,	(int)obj->real.x + lnWidthFull - lnWidthEdge,	(int)obj->real.y,									lnWidthEdge + lnMarginHorizontalHalf,		lnHeightFull);
					iiMoverSetPosition(&obj->snapDrop,	&obj->curr,	_DIRECTION_DROP,	(int)obj->real.x + lnWidthEdge,					(int)obj->real.y + lnHeightEdge,					lnWidthMid,									lnHeightMid + 2);
				}

				// Move to next object
				obj = obj->next;
			}

			// When we get here, we need to extend the snaps out to the borders of the window
			for (lnCol = 1; lnCol <= wnd->mover.maxCol; lnCol++)
			{
				lnFoundRow	= 0;
				objExtent	= NULL;
				obj			= wnd->mover.firstObject;
				while (obj)
				{
					if (obj->visible != 0)
					{
						// Is this on the column?
						if (obj->col == lnCol)
						{
							if (!objExtent || obj->row > lnFoundRow)
							{
								// Note our new furthest extent object
								objExtent	= obj;
								lnFoundRow	= obj->row;
							}

							// If we're on the right-most column, we also need to extend the east entry more
							if (lnCol == wnd->mover.maxCol)
							{
								// All of these items get extended right to the end of the bitmap
								if (obj->snapEast.x < wnd->bmpMain.bmi.bmiHeader.biWidth)
									obj->snapEast.width = wnd->bmpMain.bmi.bmiHeader.biWidth - (int)obj->snapEast.x;
							}

						}
					}
					// Move to next object
					obj = obj->next;
				}

				// When we get here we found the furthest extent
				if (objExtent)
				{
					// Adjust the south snap position to the end of the bitmap
					objExtent->snapSouth.height = wnd->bmpMain.bmi.bmiHeader.biHeight - (int)objExtent->snapSouth.y;
				}
			}

			// Re-render after these assignments
			iiMoverTrackMouseMovement(wnd, true);
			iRender(wnd);

			// When we get here, we're good
			return(0);
		}
		// If we get here, error
		return(-1);
	}




//////////
//
// Returns information about the graphical qualities of this object
//
//////
	REALTIME_API void realtime_mover_get_graphic_extents(int tnHandle, int tnObjectId, char* tcXHome16, char* tcYHome16, char* tcWidth16, char* tcHeight16, char* tcXDest16, char* tcYDest16, char* tcXStep16, char* tcYStep16, char* tcAnimationStepCount16)
	{
		SWindow*	wnd;
		SMoverObj*	obj;


		// Are we in test mode?
		if (glTestMode)
			return;

		// Locate the window
		wnd = iLocateWindow(tnHandle);
		if (wnd)
		{
			obj = iMoverLocateObject(wnd, tnObjectId);
			if (obj)
			{
				// Store the information
				iStoreFloat(tcXHome16,				obj->real.x);
				iStoreFloat(tcYHome16,				obj->real.y);
				iStoreInt(tcWidth16,				obj->bmp.bmi.bmiHeader.biWidth);
				iStoreInt(tcHeight16,				obj->bmp.bmi.bmiHeader.biHeight);
				iStoreFloat(tcXDest16,				obj->curr.x);
				iStoreFloat(tcYDest16,				obj->curr.y);
				iStoreFloat(tcXStep16,				obj->curr.xStep);
				iStoreFloat(tcYStep16,				obj->curr.yStep);
				iStoreInt(tcAnimationStepCount16,	obj->curr.stepCount);
			}
		}
		// If we get here, failure
	}




	REALTIME_API void realtime_mover_redraw(int tnHandle)
	{
		// Are we in test mode?
		if (glTestMode)
			return;

		// Re-render our window
		iRender(iLocateWindow(tnHandle));
	}




//////////
//
// Setup the parameters for a wheel
//
//////
	REALTIME_API void realtime_phwheel_setup(int tnHandle, int tnInner, int tnOutter, int tnGap, int tnPeriod, int tnKey, int tnColorInner, int tnColorOutter)
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
			wnd->phwheel.nInner			= tnInner;
			wnd->phwheel.nOutter		= tnOutter;
			wnd->phwheel.nGap			= tnGap;
			wnd->phwheel.nPeriod		= tnPeriod;
			wnd->phwheel.nKey			= tnKey;

			wnd->phwheel.nColorInner	= tnColorInner;
			wnd->phwheel.nColorOutter	= tnColorOutter;
		}
	}




//////////
//
// Issue a wheel redraw
//
//////
	REALTIME_API void realtime_phwheel_redraw(int tnHandle)
	{
		SWindow* wnd;


		// Are we in test mode?
		if (glTestMode)
			return;

		// Grab our window
		wnd = iLocateWindow(tnHandle);
		if (wnd)
			iRender(wnd);			// Signal a refresh
	}




//////////
//
// Setup the parameters for a DNA strip
//
//////
	REALTIME_API void realtime_phdna_setup(int tnHandle, int tnInner, int tnOutter, int tnGap, int tnPeriod, int tnKey)
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
			wnd->phwheel.nInner		= tnInner;
			wnd->phwheel.nOutter	= tnOutter;
			wnd->phwheel.nGap		= tnGap;
			wnd->phwheel.nPeriod	= tnPeriod;
			wnd->phwheel.nKey		= tnKey;
		}
	}




//////////
//
// Issue a DNA redraw
//
//////
	REALTIME_API void realtime_phdna_redraw(int tnHandle)
	{
		SWindow* wnd;


		// Are we in test mode?
		if (glTestMode)
			return;

		// Grab our window
		wnd = iLocateWindow(tnHandle);
		if (wnd)
			iRender(wnd);			// Signal a refresh
	}




//////////
//
// Progress bar drawing algorithms
//
//////
	// Based on tsWnd->pbar->fValue, and fRangeMin and fRangeMax, only the revelant part will be drawn
	// |----part 1----|----part 2----|----part 3----|
	void iPbarOverlay(SWindow* tsWnd, SBitmap* bmp)
	{
		if (tsWnd->pbar.nOrientation == _DIRECTION_EAST || tsWnd->pbar.nOrientation == _DIRECTION_WEST)
		{
			// East/west are horizontal
			iiPbarOverlay_eastWest(tsWnd, bmp);

		} else {
			// North/south are vertical
			iiPbarOverlay_northSouth(tsWnd, bmp);
		}
	}

	void iiPbarOverlay_eastWest(SWindow* tsWnd, SBitmap* bmp)
	{
		int		lnX, lnTarget, lnMaxPixel, lnWidth, lnHeight, lnSeg1Pixels, lnSeg2Pixels, lnSeg3Pixels;
		float	lfMin, lfMax, lfWidth, lfDelta, lfValue, lfSeg1, lfSeg2, lfSeg3, lfLow;


		//////////
		// Make sure the values are sane
		//////
			lfMin			= min(tsWnd->pbar.fRangeMin, tsWnd->pbar.fRangeMax);
			lfMax			= max(tsWnd->pbar.fRangeMin, tsWnd->pbar.fRangeMax);
			lfSeg1			= tsWnd->pbar.fRange1 - lfMin;
			lfSeg2			= (tsWnd->pbar.fRange2 - tsWnd->pbar.fRange1) - lfMin;
			lfSeg3			= (lfMax - tsWnd->pbar.fRange2) - lfMin;
			lfValue			= max(min(tsWnd->pbar.fValue, lfMax), lfMin);
			lfDelta			= (lfMax - lfMin);
			lnWidth			= tsWnd->bmpMain.bmi.bmiHeader.biWidth;
			lfWidth			= (float)tsWnd->bmpMain.bmi.bmiHeader.biWidth;
			lnHeight		= tsWnd->bmpMain.bmi.bmiHeader.biHeight - 1;
			lfLow			= 0.15f;
		

		//////////
		// Avoid division-by-zero errors
		//////
			if (lfDelta == 0.0f)
				return;


		//////////
		// Determine how much of this will be colored
		//////
			lnMaxPixel		= (int)(lfWidth * ((lfValue - lfMin) / lfDelta));
			lnSeg1Pixels	= (int)(lfWidth * (lfSeg1 / lfDelta));
			lnSeg2Pixels	= (int)(lfWidth * (lfSeg2 / lfDelta));
			lnSeg3Pixels	= (int)(lfWidth * (lfSeg3 / lfDelta));


		//////////
		// Draw the min range
		//////
			// Draw segment 1
			for (lnX = 0; lnX < lnSeg1Pixels; lnX++)
			{
				// Vertical line
				if (tsWnd->pbar.nOrientation == _DIRECTION_EAST)
				{
					// Going east
					iDrawLineVerticalAlpha(tsWnd, bmp, lnX, 0, lnHeight, tsWnd->pbar.nColor1, ((lnX < lnMaxPixel) ? 1.0f : lfLow));

				} else {
					// Going west
					iDrawLineVerticalAlpha(tsWnd, bmp, lnWidth - lnX, 0, lnHeight, tsWnd->pbar.nColor1, ((lnX < lnMaxPixel) ? 1.0f : lfLow));
				}
			}


		//////////
		// Draw the middle range
		//////
			// Draw segment 2
			for (lnTarget = lnSeg1Pixels + lnSeg2Pixels; lnX < lnTarget; lnX++)
			{
				// Vertical line
				if (tsWnd->pbar.nOrientation == _DIRECTION_EAST)
				{
					// Going east
					iDrawLineVerticalAlpha(tsWnd, bmp, lnX, 0, lnHeight, tsWnd->pbar.nColor2, ((lnX < lnMaxPixel) ? 1.0f : lfLow));

				} else {
					// Going west
					iDrawLineVerticalAlpha(tsWnd, bmp, lnWidth - lnX, 0, lnHeight, tsWnd->pbar.nColor2, ((lnX < lnMaxPixel) ? 1.0f : lfLow));
				}
			}


		//////////
		// Draw the max range
		//////
			// Draw segment 3
			for (lnTarget = lnSeg1Pixels + lnSeg2Pixels + lnSeg3Pixels; lnX < lnTarget; lnX++)
			{
				// Vertical line
				if (tsWnd->pbar.nOrientation == _DIRECTION_EAST)
				{
					// Going east
					iDrawLineVerticalAlpha(tsWnd, bmp, lnX, 0, lnHeight, tsWnd->pbar.nColor3, ((lnX < lnMaxPixel) ? 1.0f : lfLow));

				} else {
					// Going west
					iDrawLineVerticalAlpha(tsWnd, bmp, lnWidth - lnX, 0, lnHeight, tsWnd->pbar.nColor3, ((lnX < lnMaxPixel) ? 1.0f : lfLow));
				}
			}


		//////////
		// Draw vertical bars at lnSeg1Pixels and lnSeg2Pixels
		//////
			if (tsWnd->pbar.nOrientation == _DIRECTION_EAST)
			{
				// Going east
				iDrawLineVerticalAlpha(tsWnd, bmp, lnSeg1Pixels - 1,				0, lnHeight, tsWnd->pbar.nBorderColor, lfLow);
				iDrawLineVerticalAlpha(tsWnd, bmp, lnSeg1Pixels,					0, lnHeight, tsWnd->pbar.nBorderColor, 0.80f);

				iDrawLineVerticalAlpha(tsWnd, bmp, lnSeg1Pixels     + lnSeg2Pixels,	0, lnHeight, tsWnd->pbar.nBorderColor, 0.80f);
				iDrawLineVerticalAlpha(tsWnd, bmp, lnSeg1Pixels + 1 + lnSeg2Pixels,	0, lnHeight, tsWnd->pbar.nBorderColor, lfLow);

			} else {
				// Going west
				iDrawLineVerticalAlpha(tsWnd, bmp, lnWidth - (lnSeg1Pixels - 1),				0, lnHeight, tsWnd->pbar.nBorderColor, lfLow);
				iDrawLineVerticalAlpha(tsWnd, bmp, lnWidth - (lnSeg1Pixels),					0, lnHeight, tsWnd->pbar.nBorderColor, 0.80f);

				iDrawLineVerticalAlpha(tsWnd, bmp, lnWidth - (lnSeg1Pixels     + lnSeg2Pixels),	0, lnHeight, tsWnd->pbar.nBorderColor, 0.80f);
				iDrawLineVerticalAlpha(tsWnd, bmp, lnWidth - (lnSeg1Pixels + 1 + lnSeg2Pixels),	0, lnHeight, tsWnd->pbar.nBorderColor, lfLow);
			}
	}

	void iiPbarOverlay_northSouth(SWindow* tsWnd, SBitmap* bmp)
	{
		int		lnY, lnTarget, lnMaxPixel, lnWidth, lnHeight, lnSeg1Pixels, lnSeg2Pixels, lnSeg3Pixels;
		float	lfMin, lfMax, lfWidth, lfDelta, lfValue, lfSeg1, lfSeg2, lfSeg3, lfLow;


		//////////
		// Make sure the values are sane
		//////
			lfMin			= min(tsWnd->pbar.fRangeMin, tsWnd->pbar.fRangeMax);
			lfMax			= max(tsWnd->pbar.fRangeMin, tsWnd->pbar.fRangeMax);
			lfSeg1			= tsWnd->pbar.fRange1 - lfMin;
			lfSeg2			= (tsWnd->pbar.fRange2 - tsWnd->pbar.fRange1) - lfMin;
			lfSeg3			= (lfMax - tsWnd->pbar.fRange2) - lfMin;
			lfValue			= max(min(tsWnd->pbar.fValue, lfMax), lfMin);
			lfDelta			= (lfMax - lfMin);
			lnWidth			= tsWnd->bmpMain.bmi.bmiHeader.biWidth;
			lfWidth			= (float)tsWnd->bmpMain.bmi.bmiHeader.biWidth;
			lnHeight		= tsWnd->bmpMain.bmi.bmiHeader.biHeight - 1;
			lfLow			= 0.15f;
		

		//////////
		// Avoid division-by-zero errors
		//////
			if (lfDelta == 0.0f)
				return;


		//////////
		// Determine how much of this will be colored
		//////
			lnMaxPixel		= (int)(lnHeight * ((lfValue - lfMin) / lfDelta));
			lnSeg1Pixels	= (int)(lnHeight * (lfSeg1 / lfDelta));
			lnSeg2Pixels	= (int)(lnHeight * (lfSeg2 / lfDelta));
			lnSeg3Pixels	= (int)(lnHeight * (lfSeg3 / lfDelta));


		//////////
		// Draw the min range
		//////
			// Draw segment 1
			for (lnY = 0; lnY < lnSeg1Pixels; lnY++)
			{
				// Horizontal line
				if (tsWnd->pbar.nOrientation == _DIRECTION_SOUTH)
				{
					// Going South
					iDrawLineHorizontalAlpha(tsWnd, bmp, 0, lnWidth, lnY, tsWnd->pbar.nColor1, ((lnY < lnMaxPixel) ? 1.0f : lfLow));

				} else {
					// Going north
					iDrawLineHorizontalAlpha(tsWnd, bmp, 0, lnWidth, lnHeight - lnY, tsWnd->pbar.nColor1, ((lnY < lnMaxPixel) ? 1.0f : lfLow));
				}
			}


		//////////
		// Draw the middle range
		//////
			// Draw segment 2
			for (lnTarget = lnSeg1Pixels + lnSeg2Pixels; lnY < lnTarget; lnY++)
			{
				// Horizontal line
				if (tsWnd->pbar.nOrientation == _DIRECTION_SOUTH)
				{
					// Going south
					iDrawLineHorizontalAlpha(tsWnd, bmp, 0, lnWidth, lnY, tsWnd->pbar.nColor2, ((lnY < lnMaxPixel) ? 1.0f : lfLow));

				} else {
					// Going north
					iDrawLineHorizontalAlpha(tsWnd, bmp, 0, lnWidth, lnHeight - lnY, tsWnd->pbar.nColor2, ((lnY < lnMaxPixel) ? 1.0f : lfLow));
				}
			}


		//////////
		// Draw the max range
		//////
			// Draw segment 3
			for (lnTarget = lnSeg1Pixels + lnSeg2Pixels + lnSeg3Pixels; lnY < lnTarget; lnY++)
			{
				// Horizontal line
				if (tsWnd->pbar.nOrientation == _DIRECTION_SOUTH)
				{
					// Going south
					iDrawLineHorizontalAlpha(tsWnd, bmp, 0, lnWidth, lnY, tsWnd->pbar.nColor3, ((lnY < lnMaxPixel) ? 1.0f : lfLow));

				} else {
					// Going north
					iDrawLineHorizontalAlpha(tsWnd, bmp, 0, lnWidth, lnHeight - lnY, tsWnd->pbar.nColor3, ((lnY < lnMaxPixel) ? 1.0f : lfLow));
				}
			}


		//////////
		// Draw horizontal bars at lnSeg1Pixels and lnSeg2Pixels
		//////
			if (tsWnd->pbar.nOrientation == _DIRECTION_SOUTH)
			{
				// Going south
				iDrawLineHorizontalAlpha(tsWnd, bmp, 0, lnWidth, lnSeg1Pixels - 1,								tsWnd->pbar.nBorderColor, lfLow);
				iDrawLineHorizontalAlpha(tsWnd, bmp, 0, lnWidth, lnSeg1Pixels,									tsWnd->pbar.nBorderColor, 0.80f);

				iDrawLineHorizontalAlpha(tsWnd, bmp, 0, lnWidth, lnSeg1Pixels     + lnSeg2Pixels,				tsWnd->pbar.nBorderColor, 0.80f);
				iDrawLineHorizontalAlpha(tsWnd, bmp, 0, lnWidth, lnSeg1Pixels + 1 + lnSeg2Pixels,				tsWnd->pbar.nBorderColor, lfLow);

			} else {
				// Going north
				iDrawLineHorizontalAlpha(tsWnd, bmp, 0, lnWidth, lnHeight - (lnSeg1Pixels - 1),					tsWnd->pbar.nBorderColor, lfLow);
				iDrawLineHorizontalAlpha(tsWnd, bmp, 0, lnWidth, lnHeight - (lnSeg1Pixels),						tsWnd->pbar.nBorderColor, 0.80f);

				iDrawLineHorizontalAlpha(tsWnd, bmp, 0, lnWidth, lnHeight - (lnSeg1Pixels     + lnSeg2Pixels),	tsWnd->pbar.nBorderColor, 0.80f);
				iDrawLineHorizontalAlpha(tsWnd, bmp, 0, lnWidth, lnHeight - (lnSeg1Pixels + 1 + lnSeg2Pixels),	tsWnd->pbar.nBorderColor, lfLow);
			}
	}




//////////
//
// Overlay a needle where it goes
//
//////
	void iPbarOverlayNeedle(SWindow* tsWnd, SBitmap* bmp)
	{
		int		lnX, lnY, lnSpan, lnNeedleCenterPixel, lnWidth, lnHeight;
		float	lfMin, lfMax, lfWidth, lfHeight, lfDelta, lfValue, lfRed, lfGrn, lfBlu, lfRedInc, lfGrnInc, lfBluInc;
		union {
			int		_color;
			SRGB	color;
		};


		//////////
		// Make sure the values are sane
		//////
			lfMin			= min(tsWnd->pbar.fRangeMin, tsWnd->pbar.fRangeMax);
			lfMax			= max(tsWnd->pbar.fRangeMin, tsWnd->pbar.fRangeMax);
			lfValue			= max(min(tsWnd->pbar.fValue, lfMax), lfMin);
			lfDelta			= (lfMax - lfMin);
			lnWidth			= tsWnd->bmpMain.bmi.bmiHeader.biWidth;
			lnHeight		= tsWnd->bmpMain.bmi.bmiHeader.biHeight - 1;
			lfWidth			= (float)tsWnd->bmpMain.bmi.bmiHeader.biWidth;
			lfHeight		= (float)tsWnd->bmpMain.bmi.bmiHeader.biHeight;
		

		//////////
		// Avoid division-by-zero errors
		//////
			if (lfDelta == 0.0f)
				return;


		//////////
		// Compute our color deltas
		//////
			_color		= tsWnd->pbar.nNeedleColor;
			lfRed		= (float)color.red;
			lfGrn		= (float)color.grn;
			lfBlu		= (float)color.blu;
			_color		= tsWnd->pbar.nBorderColor;


		if (tsWnd->pbar.nOrientation == _DIRECTION_WEST || tsWnd->pbar.nOrientation == _DIRECTION_EAST)
		{
			//////////
			// Draw the needle vertically
			//////
				lfRedInc	= ((float)color.red - lfRed) / (float)lnHeight;
				lfGrnInc	= ((float)color.grn - lfGrn) / (float)lnHeight;
				lfBluInc	= ((float)color.blu - lfBlu) / (float)lnHeight;
				lnNeedleCenterPixel = (int)(lfWidth * ((lfValue - lfMin) / lfDelta));
				for (lnX = lnNeedleCenterPixel, lnY = lnHeight / 3, lnSpan = 0; lnY < lnHeight; lnY++, lnSpan = min(lnSpan + 1, 5), lfRed += lfRedInc, lfGrn += lfGrnInc, lfBlu += lfBluInc)
				{
					// Set the color
					color.red = (u8)lfRed;
					color.grn = (u8)lfGrn;
					color.blu = (u8)lfBlu;

					// Horizontal line (as it moves vertically, it draws the needle vertically)
					if (tsWnd->pbar.nOrientation == _DIRECTION_EAST)
					{
						// Going east
						iDrawLineHorizontalAlpha(tsWnd, bmp, lnNeedleCenterPixel - lnSpan, lnNeedleCenterPixel + lnSpan, lnY, _color, 1.0f);

					} else {
						// Going west
						iDrawLineHorizontalAlpha(tsWnd, bmp, bmp->bmi.bmiHeader.biWidth - (lnNeedleCenterPixel + lnSpan), bmp->bmi.bmiHeader.biWidth - (lnNeedleCenterPixel - lnSpan), lnY, _color, 1.0f);
					}
				}

		} else {
			//////////
			// Draw the needle horizontally
			//////
				lfRedInc	= ((float)color.red - lfRed) / (float)lnWidth;
				lfGrnInc	= ((float)color.grn - lfGrn) / (float)lnWidth;
				lfBluInc	= ((float)color.blu - lfBlu) / (float)lnWidth;
				lnNeedleCenterPixel = (int)(lfHeight * ((lfValue - lfMin) / lfDelta));
				for (lnY = lnNeedleCenterPixel, lnX = lnWidth * 2 / 3, lnSpan = 0; lnX > 0; lnX--, lnSpan = min(lnSpan + 1, 5), lfRed += lfRedInc, lfGrn += lfGrnInc, lfBlu += lfBluInc)
				{
					// Set the color
					color.red = (u8)lfRed;
					color.grn = (u8)lfGrn;
					color.blu = (u8)lfBlu;

					// Vertical line (as it moves horizontally, it draws the needle horizontally)
					if (tsWnd->pbar.nOrientation == _DIRECTION_SOUTH)
					{
						// Going east
						iDrawLineVerticalAlpha(tsWnd, bmp, lnX, lnNeedleCenterPixel - lnSpan, lnNeedleCenterPixel + lnSpan, _color, 1.0f);

					} else {
						// Going west
						iDrawLineVerticalAlpha(tsWnd, bmp, lnX, bmp->bmi.bmiHeader.biHeight - (lnNeedleCenterPixel + lnSpan), bmp->bmi.bmiHeader.biHeight - (lnNeedleCenterPixel - lnSpan), _color, 1.0f);
					}
				}
		}
	}




//////////
//
// Graphing drawing algorithms
//
//////
	void iGraphDrawBars(SWindow* tsWnd, SBitmap* bmp)
	{
		bool	llBar1;
		int		lnY, lnStep, lnCount;


		// Draw the graph bars (the background)
		llBar1	= true;
		lnStep	= max((bmp->bmi.bmiHeader.biHeight / 8) - 1, 8);
		for (lnY = 3, lnCount = 0; lnCount < 8 && lnY < bmp->bmi.bmiHeader.biHeight; lnY += lnStep, llBar1 = !llBar1, lnCount++)
		{
			// Only draw bar1 colors for now
			if (llBar1)		iOverlayRectangle(tsWnd, bmp, tsWnd->graph.marginLeft, lnY, bmp->bmi.bmiHeader.biWidth - tsWnd->graph.marginRight, lnY + lnStep + 1, tsWnd->graph.bar1Rgb, makeDarker(tsWnd->graph.bar1Rgb));
			else			iOverlayRectangle(tsWnd, bmp, tsWnd->graph.marginLeft, lnY, bmp->bmi.bmiHeader.biWidth - tsWnd->graph.marginRight, lnY + lnStep + 1, tsWnd->graph.bar2Rgb, makeDarker(tsWnd->graph.bar2Rgb));
		}
	}




//////////
//
// Draw the grid over the top of the existing control
//
//////
	void iGraphDrawGrid(SWindow* tsWnd, SBitmap* bmp)
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
			lfStartY = (float)bmp->bmi.bmiHeader.biHeight - (((lfValue1 - tsWnd->graph.rangeLower) / (tsWnd->graph.rangeUpper - tsWnd->graph.rangeLower)) * (float)bmp->bmi.bmiHeader.biHeight);
			lnStartY = (int)lfStartY;
			for (lnY = lnStartY; lnY < lnStartY + 3; lnY++)
				iDrawLineHorizontalAlpha(tsWnd, bmp, tsWnd->graph.marginLeft, bmp->bmi.bmiHeader.biWidth, lnY, tsWnd->graph.gridCenterRgb, tsWnd->graph.gridAlpha);


		//////////
		// Draw our other horizontal lines
		//////
			for (lfI = 1.0f; (int)lfI < bmp->bmi.bmiHeader.biHeight; lfI++)
			{
				// Compute the values for this iteration
				lfValueUp	= lfValue1 + (lfI * lfValue2);
				lfValueDn	= lfValue1 - (lfI * lfValue2);

				// See if we're completely off the grid yet
				if (lfValueUp > tsWnd->graph.rangeUpper && lfValueDn < tsWnd->graph.rangeLower)
					break;		// We're done

				// Draw upper
				iDrawLineHorizontalAlpha(tsWnd, bmp, tsWnd->graph.marginLeft, bmp->bmi.bmiHeader.biWidth, 
											bmp->bmi.bmiHeader.biHeight - (int)(((lfValueUp - tsWnd->graph.rangeLower) / (tsWnd->graph.rangeUpper - tsWnd->graph.rangeLower)) * (float)bmp->bmi.bmiHeader.biHeight),
											tsWnd->graph.gridLinesRgb, tsWnd->graph.gridAlpha);

				// Draw lower
				iDrawLineHorizontalAlpha(tsWnd, bmp, tsWnd->graph.marginLeft, bmp->bmi.bmiHeader.biWidth, 
											bmp->bmi.bmiHeader.biHeight - (int)(((lfValueDn - tsWnd->graph.rangeLower) / (tsWnd->graph.rangeUpper - tsWnd->graph.rangeLower)) * (float)bmp->bmi.bmiHeader.biHeight),
											tsWnd->graph.gridLinesRgb, tsWnd->graph.gridAlpha);
			}


		//////////
		// Draw our vertical lines based on how many pixels each horizontal line was above
		//////
			lnXStep = max((bmp->bmi.bmiHeader.biWidth - tsWnd->graph.marginLeft) / 30, 5);
			for (lnX = tsWnd->graph.marginLeft; lnX < bmp->bmi.bmiHeader.biWidth; lnX += lnXStep)
				iDrawLineVerticalAlpha(tsWnd, bmp, lnX, 0, bmp->bmi.bmiHeader.biHeight - 1, tsWnd->graph.gridLinesRgb, tsWnd->graph.gridAlpha);
	}




//////////
//
// Called to overlay the graduation scale along the left
//
//////
	void iGraphOverlayGraduation(SWindow* tsWnd, SBitmap* bmp)
	{
		float	lfY, lfYStep, lfValue, lfValueStep;
		RECT	lrc;
		HGDIOBJ	lhOldFont;
		char	sprintfBuffer[64];
		char	buffer[64];


		// Draw the graduation bars
		iOverlayRectangle(tsWnd, bmp, tsWnd->graph.marginLeft, 0, tsWnd->graph.marginLeft + 10, bmp->bmi.bmiHeader.biHeight, makePastel(tsWnd->graph.bar1Rgb), tsWnd->graph.bar1Rgb);

		// Setup our HDC for writing properly
		SetBkMode(bmp->hdc, TRANSPARENT);
		SetTextColor(bmp->hdc, RGB(red(tsWnd->graph.textRgb), grn(tsWnd->graph.textRgb), blu(tsWnd->graph.textRgb)));
		lhOldFont = SelectObject(bmp->hdc, tsWnd->graph.textHFont);

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
		DrawTextA(bmp->hdc, "1234567890.0", 12, &lrc, DT_CALCRECT);

		// Calculate our steppers
		lfValueStep	= (tsWnd->graph.rangeUpper - tsWnd->graph.rangeLower) / 8.0f;
		lfYStep		= (float)(bmp->bmi.bmiHeader.biHeight - (lrc.bottom - lrc.top)) / 8.0f;

		// Draw the graduation text
		lfValue = tsWnd->graph.rangeUpper;
		for (lfY = 0.0f; lfY < (float)bmp->bmi.bmiHeader.biHeight; lfY += lfYStep, lfValue -= lfValueStep)
		{
			// Compute our rect
			SetRect(&lrc, 0, (int)lfY, 64, (int)lfY + (int)lfYStep + 1);

			// Get our value
			memset(buffer, 0, sizeof(buffer));
			sprintf_s(buffer, sizeof(buffer), sprintfBuffer, lfValue);

			// Draw our text
			DrawTextA(bmp->hdc, buffer, strlen(buffer), &lrc, DT_LEFT | DT_TOP);
		}

		// Restore our old handle
		SelectObject(bmp->hdc, lhOldFont);
	}




	void iGraphPopulateDataPoints(SWindow* tsWnd, SBitmap* bmp)
	{
		int				lnI, lnX, lnY, lnPoint, lnStart, lnEnd;
		float			lfPercent, lfPercentHigh, lfPercentLow, lfValue, lfHigh, lfLow, lfAlp;
		SDataPoint*		pointNext;
		SDataPoint*		point;


		// Begin processing each point backwards
		if (!tsWnd->graph.dataPoints)
			return;

		// Iterate through each point
		point = tsWnd->graph.dataPoints;
		for (	lnI = 0, lnX = bmp->bmi.bmiHeader.biWidth - 1;
				point && lnX >= tsWnd->graph.marginLeft && point;
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
				lnY = bmp->bmi.bmiHeader.biHeight - (tsWnd->graph.dataPointThickness / 2);

			} else {
				// It's within the bounds
				lfPercent	= (lfValue - tsWnd->graph.rangeLower) / (tsWnd->graph.rangeUpper - tsWnd->graph.rangeLower);
				lnY			= bmp->bmi.bmiHeader.biHeight - (int)(lfPercent * (float)bmp->bmi.bmiHeader.biHeight);
			}

			// Draw the range first (so it's behind the data point)
			// When we get close to the left margin, begin alpha-blending the images out of existence
			lfAlp = min(max(1.0f - (((float)tsWnd->graph.marginLeft + 20.0f - (float)lnX) / 20.0f), 0.00f), 1.0f);
			if (tsWnd->graph.rangeVisible != 0)
			{
				// Draw the range for this point
				lfPercentHigh	= (lfHigh - tsWnd->graph.rangeLower) / (tsWnd->graph.rangeUpper - tsWnd->graph.rangeLower);
				lfPercentLow	= (lfLow  - tsWnd->graph.rangeLower) / (tsWnd->graph.rangeUpper - tsWnd->graph.rangeLower);
				lnStart			= bmp->bmi.bmiHeader.biHeight - (int)(lfPercentHigh * (float)bmp->bmi.bmiHeader.biHeight);
				lnEnd			= bmp->bmi.bmiHeader.biHeight - (int)(lfPercentLow  * (float)bmp->bmi.bmiHeader.biHeight);
				iDrawLineVerticalAlpha(tsWnd, bmp, lnX, lnStart, lnEnd, tsWnd->graph.rangeRgb, tsWnd->graph.rangeAlpha * lfAlp);
			}

			// Draw the line at that point
			if (lfAlp != 1.0f)
			{
				// The last few points we draw with decreasing degrees of alpha
				iDrawLineVerticalAlpha(tsWnd, bmp, lnX, lnY - tsWnd->graph.dataPointThickness, lnY + tsWnd->graph.dataPointThickness, tsWnd->graph.dataRgb, lfAlp);

			} else {
				// Draw opaque
				iDrawLineVertical(tsWnd, bmp, lnX, lnY - tsWnd->graph.dataPointThickness, lnY + tsWnd->graph.dataPointThickness, tsWnd->graph.dataRgb);
			}

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
// Render the basic gauge
//
//////
	void iGaugeRenderBasic(SWindow* tsWnd, SBitmap* bmp)
	{
		int		lnStep, lnRgb, lnRgbDarker, lnLast, lnThis;
		float	lfStep, lfStepCount, lfTheta, lfThetaStep, lfPercent, lfStart;
		float	lfX1, lfY1, lfX2, lfY2, lfXOrigin, lfYOrigin;
		bool	llDrawSegment;


		//////////
		// Affix our origin
		//////
			lfXOrigin	= (float)bmp->bmi.bmiHeader.biWidth  / 2.0f;
			lfYOrigin	= (float)bmp->bmi.bmiHeader.biHeight / 2.0f;


		//////////
		// Render the basic colors
		//////
			// Iterate from pi/6 less than 6pi/4, around to pi/6 greater than 6pi/4, crossing 0
			lfStepCount	= (float)bmp->bmi.bmiHeader.biWidth * _3PI_2;
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
				iDrawLineArbitrary(tsWnd, bmp, lfX1, lfY1, lfX2, lfY2, lnRgb);

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
				lfStepCount	= (float)bmp->bmi.bmiHeader.biWidth * _2PI;
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
						iDrawLineArbitrary(tsWnd, bmp, lfX1, lfY1, lfX2, lfY2, lnRgbDarker);

					//////////
					// Outer ring
					//////
						lfX1 = lfXOrigin + (lfXOrigin * tsWnd->gauge.scaleFactor * _OUTER_RADIUS_FRAME1 * cos(lfTheta));
						lfY1 = lfYOrigin - (lfYOrigin * tsWnd->gauge.scaleFactor * _OUTER_RADIUS_FRAME1 * sin(lfTheta));
						lfX2 = lfXOrigin + (lfXOrigin * tsWnd->gauge.scaleFactor * _OUTER_RADIUS_FRAME2 * cos(lfTheta));
						lfY2 = lfYOrigin - (lfYOrigin * tsWnd->gauge.scaleFactor * _OUTER_RADIUS_FRAME2 * sin(lfTheta));
						iDrawLineArbitrary(tsWnd, bmp, lfX1, lfY1, lfX2, lfY2, lnRgb);

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
				iDrawLineArbitrary(tsWnd, bmp, lfX1, lfY1, lfX2, lfY2, tsWnd->gauge.textRgb);

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
					iGrayerLineArbitrary(tsWnd, bmp, lfX1, lfY1, lfX2, lfY2);
				}

				// Move over for next iteration
				lfTheta += lfThetaStep;
			}


		//////////
		// Augment color data with our overlay
		//////
			// Iterate from pi/6 less than 6pi/4, around to pi/6 greater than 6pi/4, crossing 0
			lfStepCount	= (float)bmp->bmi.bmiHeader.biWidth * _3PI_2;
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
				iApplyOverlayBmpFile(tsWnd, bmp, lfX1, lfY1, lfX2, lfY2, lfTheta, (unsigned char*)cgc_Overlay);

				// Move over for next iteration
				lfTheta += lfThetaStep;
			}
	}




//////////
//
// Called to overlay the graduation for the quantities relating to the gauge
//
//////
	void iGaugeOverlayGraduation(SWindow* tsWnd, SBitmap* bmp)
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
			lfXOrigin	= (float)bmp->bmi.bmiHeader.biWidth  / 2.0f;
			lfYOrigin	= (float)bmp->bmi.bmiHeader.biHeight / 2.0f;
		

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
			SetBkMode(bmp->hdc, TRANSPARENT);
			SetTextColor(bmp->hdc, RGB(red(tsWnd->gauge.textRgb), grn(tsWnd->gauge.textRgb), blu(tsWnd->gauge.textRgb)));
			lhOldFont = SelectObject(bmp->hdc, tsWnd->gauge.textHFont);

			// Calculate how tall each item will be
			DrawTextA(bmp->hdc, "1234567890.0", 12, &lrc, DT_CALCRECT);
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
				DrawTextA(bmp->hdc, buffer, strlen(buffer), &lrcSize, DT_CALCRECT);

				// Adjust our rectangle
				SetRect(&lrc, (int)lfX - (lrcSize.right / 2), (int)lfY - (lrcSize.bottom / 2), (int)lfX + (lrcSize.right / 2), (int)lfY + (lrcSize.bottom / 2));
				iAdjustRectangle(tsWnd, bmp, lfXOrigin, lfYOrigin, lfTheta, &lrc);

				// Draw our text
				DrawTextA(bmp->hdc, buffer, strlen(buffer), &lrc, DT_LEFT | DT_TOP);

				// Move over for next iteration
				lfTheta += lfThetaStep;
				lfValue	+= lfValueStep;
			}


		//////////
		// Restore our old font handle
		//////
			SelectObject(bmp->hdc, lhOldFont);
	}




//////////
//
// Called to overlay the needle
//
//////
	void iGaugeOverlayNeedle(SWindow* tsWnd, SBitmap* bmp)
	{
		int		lnRgb;
		float	lfPercent, lfTheta, lfXOrigin, lfYOrigin, lfX, lfY, lfDeltaX, lfDeltaY, lfNeedleValue;


		//////////
		// Affix our origin
		//////
			lfXOrigin	= (float)bmp->bmi.bmiHeader.biWidth  / 2.0f;
			lfYOrigin	= (float)bmp->bmi.bmiHeader.biHeight / 2.0f;


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
				iDrawLineArbitraryColorPath(tsWnd, bmp, lfX + lfDeltaX * _OUTER_OSCILLATION, lfY + lfDeltaY * _OUTER_OSCILLATION, lfXOrigin + lfDeltaX * _INNER_OSCILLATION, lfYOrigin + lfDeltaY * _INNER_OSCILLATION, 0, lnRgb);
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
				iDrawLineArbitrary(tsWnd, bmp, lfXOrigin + lfDeltaX * _OUTER_OSCILLATION, lfYOrigin + lfDeltaY * _OUTER_OSCILLATION, lfXOrigin, lfYOrigin, rgb(128,128,128));
			}
	}




//////////
//
// Called to overlay the actual value in a printed text form
//
//////
	void iGaugeOverlayHighlightedValue(SWindow* tsWnd, SBitmap* bmp)
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
			lfXOrigin	= (float)bmp->bmi.bmiHeader.biWidth  / 2.0f;
			lfYOrigin	= (float)bmp->bmi.bmiHeader.biHeight / 2.0f;
		

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
			SetBkMode(bmp->hdc, TRANSPARENT);
			SetTextColor(bmp->hdc, RGB(red(tsWnd->gauge.textRgb), grn(tsWnd->gauge.textRgb), blu(tsWnd->gauge.textRgb)));
			lhOldFont = SelectObject(bmp->hdc, tsWnd->gauge.textHFontBold);


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
							(int)(((bmp->bmi.bmiHeader.biHeight - 1) + (int)(lfYOrigin - lfDeltaY)) / 2) + (int)(lfYOrigin * 0.05f));
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
			iOverlayRectangle(tsWnd, bmp, lrc.left, lrc.top, lrc.right, lrc.bottom, lnRgb, lnRgbDarker);

			// Increase the bounding size of the rectangle in case the user went haywire with decimals
			lrc.left	= 0;
			lrc.right	= bmp->bmi.bmiHeader.biWidth;

			// All text must be rendered from that location closest to the center
			memset(buffer, 0, sizeof(buffer));
			sprintf_s(buffer, sizeof(buffer), sprintfBuffer, tsWnd->gauge.needleValue);

			// Trim off the spaces
			iLtrimBuffer(buffer);

			// Draw our text
			DrawTextA(bmp->hdc, buffer, strlen(buffer), &lrc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);


		//////////
		// Restore our old font handle
		//////
			SelectObject(bmp->hdc, lhOldFont);
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
// Called to append a new mover object to the window's mover object chain
//
//////
	SMoverObj* iMoverAppendNewObject(SWindow* tsWnd)
	{
		SMoverObj*		moNew;
		SMoverObj*		mo;
		SMoverObj**		moPrev;


		// Make sure our environment is sane
		if (!tsWnd)
			return(NULL);

		// See where we are
		if (!tsWnd->mover.firstObject)
		{
			// This is the first one
			moPrev = &tsWnd->mover.firstObject;

		} else {
			// There are entries, see where this one falls
			mo = tsWnd->mover.firstObject;
			while (mo->next)
				mo = mo->next;
			moPrev = &mo->next;
		}

		// Create the new entry
		moNew = (SMoverObj*)malloc(sizeof(SMoverObj));
		if (moNew)
		{
			// Initialize our entry
			memset(moNew, 0, sizeof(SMoverObj));

			// Update the back link
			*moPrev = moNew;

			// Set the unique Id and default visible status
			moNew->objectId	= iGetNextUniqueId();
			moNew->visible	= 1;
		}
		// Indicate our status
		return(moNew);
	}




//////////
//
// Called to find the object
//
//////
	SMoverObj* iMoverLocateObject(SWindow* tsWnd, int tnObjectId)
	{
		SMoverObj*	obj;


		// Make sure our environment is sane
		if (tsWnd)
		{
			// Iterate through all objects
			obj = tsWnd->mover.firstObject;
			while (obj)
			{
				// Is this the correct object?
				if (obj->objectId == tnObjectId)
					return(obj);

				// Move to next object
				obj = obj->next;
			}
		}
		// Failure
		return(NULL);
	}




//////////
//
// Called to delete a specific object
//
//////
	int iMoverDeleteObject(SWindow* tsWnd, int tnObjectId)
	{
		SMoverObj*	objNext;
		SMoverObj*	obj;
		SMoverObj**	objPrev;


		// Make sure our environment is sane
		if (tsWnd && tsWnd->type == _TYPE_MOVER && tsWnd->mover.firstObject)
		{
			obj		= tsWnd->mover.firstObject;
			objPrev	= &tsWnd->mover.firstObject;
			while (obj)
			{
				// Grab the next entry
				objNext = obj->next;

				// Is this the object to delete?
				if (obj->objectId == tnObjectId)
				{
					// Yes, update the backlink to point to the entry after this (if any)
					*objPrev = obj->next;

					// Delete this entry's data
					DeleteObject((HGDIOBJ)obj->bmp.hbmp);
					DeleteDC(obj->bmp.hdc);

					// Delete this entry
					free(obj);

					// All done!
					return(tnObjectId);
				}

				// Move to the next entry
				obj = objNext;
			}
		}
		// Failure
		return(0);
	}




//////////
//
// Called to draw the objects at their actual or current positions (depending on whether or not
// they're moving at the current time)
//
//////
	void iMoverDrawObjects(SWindow* tsWnd, SBitmap* bmp)
	{
		SMoverObj*	obj;
		SMoverObj*	objSnap;
		SMoverPos	snapSouth;


		// Make sure our environment is sane
		if (tsWnd)
		{
			// Iterate through every object
			objSnap	= NULL;
			obj		= tsWnd->mover.firstObject;
			while (obj)
			{
				if (obj->visible != 0)
				{
					if (tsWnd->mover.dragObject && (tsWnd->mover.snapReal.row != 0 || tsWnd->mover.snapReal.col != 0))
					{
						// Draw this item at its current location
						iMoverOverlayBitmap(tsWnd, bmp, obj, &obj->curr, 0.33f);

					} else {
						// Draw this item at its current location
						iMoverOverlayBitmap(tsWnd, bmp, obj, &obj->real, 1.0f);
					}

					// If this is the item in the snap position
					if (tsWnd->mover.snap.row != 0 && tsWnd->mover.snap.col != 0 && obj->real.row == tsWnd->mover.snapReal.row && obj->real.col == tsWnd->mover.snapReal.col)
						objSnap = obj;

					// If need be, draw the snap to rectangles
					if (gnDrawSnaps == 2)
						iMoverDrawSnaps(tsWnd, bmp, obj);
				}

				// Move to next object
				obj = obj->next;
			}

			// Draw the field that would be snapped
			if (objSnap)
			{
				// Draw the snapped item where it would go
				if (tsWnd->mover.dragObject)
				{
					if (tsWnd->mover.snap.snapDirection == _DIRECTION_SOUTH)
					{
						// We need to draw this item one row below where it is, because the triggering snap event was on the row above
						memcpy(&snapSouth, &objSnap->real, sizeof(SMoverPos));
						snapSouth.y += tsWnd->mover.maxHeight + tsWnd->mover.marginVertical;
						iMoverOverlayBitmap(tsWnd, bmp, tsWnd->mover.dragObject, &snapSouth, 1.0f);

					} else if (tsWnd->mover.snap.snapDirection == _DIRECTION_EAST) {
						// We need to draw this item one column right of where it is, because the triggering snap event was on the column to the left
						memcpy(&snapSouth, &objSnap->real, sizeof(SMoverPos));
						snapSouth.x += tsWnd->mover.maxWidth + tsWnd->mover.marginHorizontal;
						iMoverOverlayBitmap(tsWnd, bmp, tsWnd->mover.dragObject, &snapSouth, 1.0f);

					} else {
						// Draw it at its normal location
						iMoverOverlayBitmap(tsWnd, bmp, tsWnd->mover.dragObject, &objSnap->real, 1.0f);
					}
				}


				// If need be, draw the snap to rectangles
				if (gnDrawSnaps == 1)
					iMoverDrawSnaps(tsWnd, bmp, objSnap);
			}
		}
	}

	void iMoverDrawSnaps(SWindow* tsWnd, SBitmap* bmp, SMoverObj* moSnap)
	{
		if (tsWnd->mover.snap.snapDirection == _DIRECTION_NORTH)
			iMoverFillRect(tsWnd, bmp, moSnap, &moSnap->snapNorth,	0.5f, RGB(222,22,222));
		else if (tsWnd->mover.snap.snapDirection == _DIRECTION_SOUTH)
			iMoverFillRect(tsWnd, bmp, moSnap, &moSnap->snapSouth,	0.5f, RGB(222,22,222));
		else if (tsWnd->mover.snap.snapDirection == _DIRECTION_WEST)
			iMoverFillRect(tsWnd, bmp, moSnap, &moSnap->snapWest,	0.5f, RGB(222,22,222));
		else if (tsWnd->mover.snap.snapDirection == _DIRECTION_EAST)
			iMoverFillRect(tsWnd, bmp, moSnap, &moSnap->snapEast,	0.5f, RGB(222,22,222));
		else if (tsWnd->mover.snap.snapDirection == _DIRECTION_DROP)
			iMoverFillRect(tsWnd, bmp, moSnap, &moSnap->snapDrop,	0.5f, RGB(222,22,222));
	}




//////////
//
// Called to overlay the indicated bitmap
//
//////
	void iMoverOverlayBitmap(SWindow* tsWnd, SBitmap* bmp, SMoverObj* mo, SMoverPos* mop, float tfAlpha)
	{
		// Make sure our environment is sane
		if (tsWnd && bmp && mo && mop && mop->col != 0 && mop->row != 0)
			iOverlayBitmap(bmp, &mo->bmp, (int)mop->x, (int)mop->y, tfAlpha);		// Draw it at the indicated alpha level at the location where it's at right now
	}




//////////
//
// Called to fill in the rectangle
//
//////
	void iMoverFillRect(SWindow* tsWnd, SBitmap* bmp, SMoverObj* mo, SMoverPos* mop, float tfAlp, int tnRgb)
	{
		int		lnX, lnY;
		float	lfMalp, lfRed, lfGrn, lfBlu;
		SRGB*	lrgb;


		// Setup our alpha and color values
		lfMalp = 1.0f - tfAlp;
		lfRed	= (float)red(tnRgb);
		lfGrn	= (float)grn(tnRgb);
		lfBlu	= (float)blu(tnRgb);

		// Do the rectangle
		for (lnY = (int)mop->y; lnY < (int)mop->y + mop->height; lnY++)
		{
			if (lnY >= 0 && lnY < bmp->bmi.bmiHeader.biHeight)
			{
				// Get the offset for this point
				lrgb = (SRGB*)(bmp->bits + ((bmp->bmi.bmiHeader.biHeight - lnY - 1) * bmp->actualWidth) + ((int)mop->x * 3));

				// Iterate for the width
				for (lnX = (int)mop->x; lnX < (int)mop->x + mop->width; lnX++)
				{
					if (lnX >= 0 && lnX < bmp->bmi.bmiHeader.biWidth)
					{
						lrgb->red = (unsigned char)(((float)lrgb->red * lfMalp) + (lfRed * tfAlp));
						lrgb->grn = (unsigned char)(((float)lrgb->grn * lfMalp) + (lfGrn * tfAlp));
						lrgb->blu = (unsigned char)(((float)lrgb->blu * lfMalp) + (lfBlu * tfAlp));
					}
					// Move to next pixel
					++lrgb;
				}
			}
		}
	}




//////////
//
// Called to get the maximum extents for each row and columns
//
//////
	void iMoverGetObjectExtents(SWindow* tsWnd, int* tnMaxCol, int* tnMaxRow, int* tnMaxWidth, int* tnMaxHeight)
	{
		SMoverObj*	obj;
		SMoverPos*	mop;


		// Make sure our environment is sane
		if (tsWnd && tnMaxCol && tnMaxRow)
		{
			// Reset our max values
			*tnMaxCol		= 0;
			*tnMaxRow		= 0;
			*tnMaxWidth		= 0;
			*tnMaxHeight	= 0;

			// Iterate through everything
			obj = tsWnd->mover.firstObject;
			while (obj)
			{
				// Use the actual location
				mop = &obj->real;

				// Check the extent column
				if (mop->col > *tnMaxCol)
					*tnMaxCol = mop->col;

				// Check the extent row
				if (mop->row > *tnMaxRow)
					*tnMaxRow = mop->row;

				// Grab the width
				if (obj->bmp.bmi.bmiHeader.biWidth > *tnMaxWidth)
					*tnMaxWidth = obj->bmp.bmi.bmiHeader.biWidth;

				// Grab the height
				if (obj->bmp.bmi.bmiHeader.biHeight > *tnMaxHeight)
					*tnMaxHeight = obj->bmp.bmi.bmiHeader.biHeight;

				// Move to next object
				obj = obj->next;
			}
		}
	}

	// Set the indicated position based on hard values
	void iiMoverSetPosition(SMoverPos* mp, SMoverPos* mpRef, int snapDirection, int x, int y, int width, int height)
	{
		// Copy over our reference first
		memcpy(mp, mpRef, sizeof(SMoverPos));

		// Make the specific adjustments
		mp->x				= (float)x;
		mp->y				= (float)y;
		mp->width			= width;
		mp->height			= height;
		mp->snapDirection	= snapDirection;
	}

	bool iiMoverTrackMouseMovement(SWindow* tsWnd, bool tlForce)
	{
		SMoverPos*	snapCandidate;
		SMoverPos	snapReal;
		SMoverObj*	objSnap;


		// Is it a sane environment?
		if ((tlForce && tsWnd->mouseLast.isValid) || tsWnd->mouseLast.isValid)
		{
			// Get a shorthand for our mouse positions
			if (tlForce || tsWnd->mouseLast.x != tsWnd->mouseCurrent.x || tsWnd->mouseLast.y != tsWnd->mouseCurrent.y)
			{
				// It has moved
				// Yes, signal a possible new snap position based on this movement
				snapCandidate = iiMoverDetermineSnapPosition(tsWnd, &snapReal, &objSnap);
				if (tlForce || iiMoverSnapPositionHasMoved(tsWnd, snapCandidate))
				{
					// It has moved to a new position
					iiMoverKickOffAnimationsViaSnapPosition(tsWnd, &snapReal);

					// Store the new snap position
					if (snapCandidate)
					{
						memcpy(&tsWnd->mover.snap,		snapCandidate,	sizeof(SMoverPos));
						memcpy(&tsWnd->mover.snapReal,	&snapReal,		sizeof(SMoverPos));
					}
					
					// All done
					return(true);
				}
			}
		}
		// If we get here, nope
		return(false);
	}

	bool iiMoverSnapPositionHasMoved(SWindow* tsWnd, SMoverPos* snap)
	{
		if (snap && (snap->row != tsWnd->mover.snapReal.row || snap->col != tsWnd->mover.snapReal.col || snap->snapDirection != tsWnd->mover.snapReal.snapDirection))
			return(true);
		// Nope
		return(false);
	}

	SMoverPos* iiMoverDetermineSnapPosition(SWindow* tsWnd, SMoverPos* snapReal, SMoverObj** objSnappedOn)
	{
		int			lnX, lnY;
		SMoverObj*	obj;


		// Iterate through the objects to see if the mouse coordinate hits a snap area
		obj = tsWnd->mover.firstObject;
		lnX	= tsWnd->mouseCurrent.x;
		lnY	= tsWnd->mouseCurrent.y;
		while (obj)
		{
			// Is this one in range?
			if (iiMoverIsPointInThisPosition(&obj->snapNorth, lnX, lnY))
			{
				memcpy(snapReal, &obj->real, sizeof(SMoverPos));
				snapReal->snapDirection = _DIRECTION_NORTH;
				*objSnappedOn = obj;
				return(&obj->snapNorth);
			}

			if (iiMoverIsPointInThisPosition(&obj->snapSouth, lnX, lnY))
			{
				memcpy(snapReal, &obj->real, sizeof(SMoverPos));
				snapReal->snapDirection = _DIRECTION_SOUTH;
				*objSnappedOn = obj;
				return(&obj->snapSouth);
			}

			if (iiMoverIsPointInThisPosition(&obj->snapWest, lnX, lnY))
			{
				memcpy(snapReal, &obj->real, sizeof(SMoverPos));
				snapReal->snapDirection = _DIRECTION_WEST;
				*objSnappedOn = obj;
				return(&obj->snapWest);
			}

			if (iiMoverIsPointInThisPosition(&obj->snapEast, lnX, lnY))
			{
				memcpy(snapReal, &obj->real, sizeof(SMoverPos));
				snapReal->snapDirection = _DIRECTION_EAST;
				*objSnappedOn = obj;
				return(&obj->snapEast);
			}

			if (iiMoverIsPointInThisPosition(&obj->snapDrop, lnX, lnY))
			{
				memcpy(snapReal, &obj->real, sizeof(SMoverPos));
				snapReal->snapDirection = _DIRECTION_DROP;
				*objSnappedOn = obj;
				return(&obj->snapDrop);
			}

			// Move to next object
			obj = obj->next;
		}

		// If we get here, it's not in range
		return(NULL);
	}

	bool iiMoverIsPointInThisPosition(SMoverPos* mp, int x, int y)
	{
		// Is it in range
		if (x >= mp->x && x <= mp->x + mp->width && y >= mp->y && y <= mp->y + mp->height)
			return(true);

		// If we get here, failure
		return(false);
	}

	// Sets up every candidate position based on its relative position to the snap candidate
	void iiMoverKickOffAnimationsViaSnapPosition(SWindow* tsWnd, SMoverPos* snap)
	{
		int			lnRowDelta, lnColDelta;
		SMoverObj*	obj;


		// Iterate through the objects to see if the mouse coordinate hits a snap area
		obj = tsWnd->mover.firstObject;
		while (obj)
		{
			// Determine how much (if any) this object should move based on the snap position
			iiMoverDetermineSnapAdjustments(tsWnd, obj, snap, &lnRowDelta, &lnColDelta);

			// Move to next object
			obj = obj->next;
		}
		// If we get here, it's not in range
	}

	void iiMoverDetermineSnapAdjustments(SWindow* tsWnd, SMoverObj* obj, SMoverPos* snap, int* tnRowDelta, int* tnColDelta)
	{
		if (snap)
		{
			switch (((tsWnd->mover.dragObject) ? snap->snapDirection : _DIRECTION_DROP))
			{
				case _DIRECTION_NORTH:
					// Everything greater than or equal this row in this column gets snapped down one row
					if (obj->real.col == snap->col && obj->real.row >= snap->row)
					{
						// This one gets snapped down a row
						iiMoverAnimateTo(tsWnd, obj, obj->real.col, obj->real.row + 1);

					} else {
						// It stays where it is
						iiMoverAnimateTo(tsWnd, obj, obj->real.col, obj->real.row);
					}
					break;

				case _DIRECTION_SOUTH:
					// Everything greater than this row in this column gets snapped down one row
					if (obj->real.col == snap->col && obj->real.row > snap->row)
					{
						// This one gets snapped down a row
						iiMoverAnimateTo(tsWnd, obj, obj->real.col, obj->real.row + 1);

					} else {
						// It stays where it is
						iiMoverAnimateTo(tsWnd, obj, obj->real.col, obj->real.row);
					}
					break;

				case _DIRECTION_WEST:
					// Everything greater than or equal to this column gets snapped east one column
					if (obj->real.col >= snap->col)		iiMoverAnimateTo(tsWnd, obj, obj->real.col + 1,	obj->real.row);			// This one gets snapped over a column
					else								iiMoverAnimateTo(tsWnd, obj, obj->real.col,		obj->real.row);			// It stays where it is
					break;

				case _DIRECTION_EAST:
					// Everything greater than this column gets snapped east one column
					if (obj->real.col > snap->col)		iiMoverAnimateTo(tsWnd, obj, obj->real.col + 1,	obj->real.row);			// This one gets snapped over a column
					else								iiMoverAnimateTo(tsWnd, obj, obj->real.col,		obj->real.row);			// It stays where it is
					break;

				case _DIRECTION_DROP:
					// Nothing gets moved
					// Animate the object back back to its real location
					iiMoverAnimateTo(tsWnd, obj, obj->real.col, obj->real.row);
					break;
			}

		} else {
			// There is no snap entry, so we move it toward its real row
			// Animate the object back back to its real location
			iiMoverAnimateTo(tsWnd, obj, obj->real.col, obj->real.row);
		}
	}

	void iiMoverAnimateTo(SWindow* tsWnd, SMoverObj* obj, int tnCol, int tnRow)
	{
		int		lnMarginV, lnMarginH, lnWidth, lnHeight;
		float	lfRealX, lfRealY, lfCurrX, lfCurrY, lfDelta, lfDeltaX, lfDeltaY;


		// From obj->curr, we need to move to the location that would be indicated
		if (obj->curr.row == 0 || obj->curr.col == 0)
		{
			// We are animating to its real location
			memcpy(&obj->curr, &obj->real, sizeof(SMoverPos));
			// No animation needs to be setup, because it will just be drawn at that home location

		}

		// Store the new column and row
		obj->curr.row	= tnRow;
		obj->curr.col	= tnCol;

		// We are animating to the defined location
		// Create our constants
		lnMarginV		= tsWnd->mover.marginVertical;
		lnMarginH		= tsWnd->mover.marginHorizontal;
		lnWidth			= obj->bmp.bmi.bmiHeader.biWidth;
		lnHeight		= obj->bmp.bmi.bmiHeader.biHeight;

		// Move from where it is to where it should be
		// Get position of real item and current item
		lfRealX			= (float)(lnMarginH + (obj->real.col - 1) * (tsWnd->mover.maxWidth  + lnMarginH));
		lfRealY			= (float)(lnMarginV + (obj->real.row - 1) * (tsWnd->mover.maxHeight + lnMarginV));
		lfCurrX			= (float)(lnMarginH + (obj->curr.col - 1) * (tsWnd->mover.maxWidth  + lnMarginH));
		lfCurrY			= (float)(lnMarginV + (obj->curr.row - 1) * (tsWnd->mover.maxHeight + lnMarginV));

		// See if any movement is required
		lfDeltaX	= (lfCurrX - obj->curr.x);
		lfDeltaY	= (lfCurrY - obj->curr.y);
		lfDelta		= sqrt((lfDeltaX*lfDeltaX) + (lfDeltaY*lfDeltaY));
		if (lfDelta != 0.0f)
		{
			// We have some movement
			obj->curr.stepCount = 10;					// Number of steps
			obj->curr.xStep		= lfDeltaX / 10.0f;		// Setup the X step per tick
			obj->curr.yStep		= lfDeltaY / 10.0f;		// Setup the Y step per tick
			// Right now, the animation is setup
			tsWnd->mover.isAnimating = true;

		} else {
			// No movement is required
			obj->curr.stepCount = 0;
		}
	}

	bool iiMoverTrackMouseLeftButtonReleased(SWindow* tsWnd)
	{
		if (tsWnd->mouseLast.isValid)
		{
			tsWnd->mover.isAnimating	= false;
			tsWnd->mover.snapReal.row	= 0;
			tsWnd->mover.snapReal.col	= 0;
			tsWnd->mover.dragObject		= NULL;
			iiMoverKickOffAnimationsViaSnapPosition(tsWnd, NULL);
			return(true);
		}
		// If we get here, we did not signal anything
		return(false);
	}

	bool iiMoverTrackMouseLeftButtonPressed(SWindow* tsWnd)
	{
		SMoverPos*	snapCandidate;
		SMoverPos	snapReal;
		SMoverObj*	objSnap;


		// Is it a sane environment?
		if (tsWnd->mouseLast.isValid)
		{
			// Get a shorthand for our mouse positions
			snapCandidate = iiMoverDetermineSnapPosition(tsWnd, &snapReal, &objSnap);
			if (snapCandidate && objSnap->draggable)
			{
				// It has moved to a new position, and it is draggable
				tsWnd->mover.dragObject = objSnap;

				// Begin moving everything that can be moved based on this item being dragged
				iiMoverKickOffAnimationsViaSnapPosition(tsWnd, &snapReal);

				// Store the new snap position
				if (snapCandidate)
				{
					memcpy(&tsWnd->mover.snap,		snapCandidate,	sizeof(SMoverPos));
					memcpy(&tsWnd->mover.snapReal,	&snapReal,		sizeof(SMoverPos));
				}

				// All done
				return(true);
			}
		}
		// If we get here, we did not signal anything
		return(false);
	}

	bool iiMoverTrackMouseRightButtonReleased(SWindow* tsWnd)
	{
		// We do not process any right-mouse events right now, but they can be handled here
		return(true);
	}

	bool iiMoverTrackMouseRightButtonPressed(SWindow* tsWnd)
	{
		// We do not process any right-mouse events right now, but they can be handled here
		gnDrawSnaps = (++gnDrawSnaps) % 2;
		return(true);
	}

	void iiMoverDeleteObjectChain(SMoverObj** objRoot)
	{
		SMoverObj*	objNext;
		SMoverObj*	obj;


		if (objRoot && *objRoot)
		{
			obj = *objRoot;
			while (obj)
			{
				// Grab the next entry
				objNext = obj->next;

				// Delete this entry's data
				DeleteObject((HGDIOBJ)obj->bmp.hbmp);
				DeleteDC(obj->bmp.hdc);

				// Delete this entry
				free(obj);

				// Move to the next entry
				obj = objNext;
			}
			// When we get here, all done
			*objRoot = NULL;
		}
	}

	void iPHWheelOverlay(SWindow* tsWnd, SBitmap* bmp)
	{
		u32					lnI;
		int					lnInner, lnOutter, lnBackoff, lnThisSegment, lnSegment, lnSegments, lnSegmentStep, colorBorderInner, colorBorderOutter, colorLight, colorDark;
		bool				llMatch, llMatchInner, llMatchOutter, llBackoff;
		float				lfX, lfY, lfTheta, lfThetaInc1, lfThetaInc2, lfRadiusInnerMin, lfRadiusInnerMax, lfRadiusOutterMin, lfRadiusOutterMax, lfCosThetaMin, lfSinThetaMax, lfCosTheta2, lfSinTheta2;
		float				lfOriginX, lfOriginY, lfX_ll, lfY_ll, lfX_ul, lfY_ul, lfX_lr, lfY_lr, lfX_ur, lfY_ur;
		RECT				lrc;
		char				buffer[32];
		SBuilder*			inners;
		SBuilder*			outters;
		SWheelPointData		wpd;
		SWheelPointData*	wpdp;


		// Make sure we have been populated
		if (tsWnd->phwheel.nPeriod != 0)
		{
			//////////
			// Create our builders
			//////
				iBuilder_createAndInitialize(&inners, -1);
				iBuilder_createAndInitialize(&outters, -1);


			//////////
			// Rotate around drawing segment lines for the inner and outter rings
			//////
				lfRadiusInnerMin	= (float)(bmp->bmi.bmiHeader.biWidth / 2) * 0.50f;
				lfRadiusInnerMax	= (float)(bmp->bmi.bmiHeader.biWidth / 2) * 0.60f;
				lfRadiusOutterMin	= (float)(bmp->bmi.bmiHeader.biWidth / 2) * 0.85f;
				lfRadiusOutterMax	= (float)(bmp->bmi.bmiHeader.biWidth / 2) * 0.95f;
				lfOriginX			= (float)(bmp->bmi.bmiHeader.biWidth / 2);
				lfOriginY			= (float)(bmp->bmi.bmiHeader.biHeight / 2);
				lnSegmentStep		= 6;
				lnSegments			= tsWnd->phwheel.nPeriod / lnSegmentStep;
				lfThetaInc1			= _2PI / (float)lnSegments;
				lfThetaInc2			= lfThetaInc1;// * 0.90f;
				lfTheta				= 0.0f - (lfThetaInc1 * 0.50f/*0.45f*/) - _PI2;
				colorLight			= RGB(215,215,215);
				colorDark			= RGB(128,128,128);
				colorBorderInner	= iCombineColors(tsWnd->phwheel.nColorInner, rgb(0,0,0), 0.75f);
				colorBorderOutter	= iCombineColors(tsWnd->phwheel.nColorInner, rgb(0,0,0), 0.75f);


			//////////
			// Determine the starting point
			///////
				lnInner		= tsWnd->phwheel.nInner;
				lnOutter	= tsWnd->phwheel.nOutter;
				for ( ; lnInner > 7 || lnOutter > 7; )
				{
					if (lnInner > 7)		lnInner		-= 6;
					if (lnOutter > 7)		lnOutter	-= 6;
				}


			//////////
			// Iterate around
			//////
				for (lnI = 0; lnI < 2; lnI++)
				{
					for (lnSegment = 0, lnBackoff = 0; lnSegment < lnSegments; lnSegment++, lfTheta += lfThetaInc1)
					{
						//////////
						// Compute radians
						//////
							lfCosThetaMin	= cos(lfTheta);
							lfCosTheta2		= cos(lfTheta + lfThetaInc2);
							lfSinThetaMax	= sin(lfTheta);
							lfSinTheta2		= sin(lfTheta + lfThetaInc2);


						//////////
						// Inner rings
						/////
							if (lnSegment < lnSegments / 2)
							{
								// 12 o'clock to 6 o'clock, use normal outter ring
								// Left segment
								lfX_ll = lfRadiusInnerMin * lfCosThetaMin;
								lfY_ll = lfRadiusInnerMin * lfSinThetaMax;
								lfX_ul = lfRadiusInnerMax * lfCosThetaMin;
								lfY_ul = lfRadiusInnerMax * lfSinThetaMax;

								// Right segment
								lfX_lr = lfRadiusInnerMin * lfCosTheta2;
								lfY_lr = lfRadiusInnerMin * lfSinTheta2;
								lfX_ur = lfRadiusInnerMax * lfCosTheta2;
								lfY_ur = lfRadiusInnerMax * lfSinTheta2;

								// Midpoint
								wpd.x = lfOriginX + ((lfX_ul + lfX_ur) / 2.0f);
								wpd.y = lfOriginY + ((lfY_ul + lfY_ur) / 2.0f);

							} else {
								// 6 o'clock to 12 o'clock, use inverted rings
								// Left segment
								lfX_ll = lfRadiusOutterMin * lfCosThetaMin;
								lfY_ll = lfRadiusOutterMin * lfSinThetaMax;
								lfX_ul = lfRadiusOutterMax * lfCosThetaMin;
								lfY_ul = lfRadiusOutterMax * lfSinThetaMax;

								// Right segment
								lfX_lr = lfRadiusOutterMin * lfCosTheta2;
								lfY_lr = lfRadiusOutterMin * lfSinTheta2;
								lfX_ur = lfRadiusOutterMax * lfCosTheta2;
								lfY_ur = lfRadiusOutterMax * lfSinTheta2;

								// Midpoint
								wpd.x = lfOriginX + ((lfX_ll + lfX_lr) / 2.0f);
								wpd.y = lfOriginY + ((lfY_ll + lfY_lr) / 2.0f);
							}

							// Draw left segment, top, bottom
							lnThisSegment	= lnInner + ((lnSegment - lnBackoff) * lnSegmentStep);
							llMatchInner	= (lnThisSegment % tsWnd->phwheel.nInner  == 0);
							llMatchOutter	= (lnThisSegment % tsWnd->phwheel.nOutter == 0);
							llMatch			= (llMatchInner || llMatchOutter);
							llBackoff		= (lnSegment == 0);
							if (llBackoff)
							{
								// This is a segment at 12 o'clock or 6 o'clock, and it should be blank
								++lnBackoff;
								if (lnI == 0)
								{
									iFillRectArbitrary(tsWnd, bmp,	lfOriginX + lfX_ul, lfOriginY + lfY_ul, lfOriginX + lfX_ur, lfOriginY + lfY_ur,
																	lfOriginX + lfX_lr, lfOriginY + lfY_lr, lfOriginX + lfX_ll, lfOriginY + lfY_ll,
																	colorDark,
																	true, colorBorderInner);
								}

							} else {
								// Draw normally
								if (lnI == 0)
								{
									iFillRectArbitrary(tsWnd, bmp,	lfOriginX + lfX_ul, lfOriginY + lfY_ul, lfOriginX + lfX_ur, lfOriginY + lfY_ur,
																	lfOriginX + lfX_lr, lfOriginY + lfY_lr, lfOriginX + lfX_ll, lfOriginY + lfY_ll,
																	((llMatch) ? ((lnSegment < lnSegments / 2) ? tsWnd->phwheel.nColorInner : tsWnd->phwheel.nColorOutter) : colorLight),
																	true, colorBorderInner);
								}

								// Label it
								if (llMatch && lnI == 1)
								{
									lfX = (lfX_ll + lfX_lr + lfX_ul + lfX_ur) / 4.0f;
									lfY = (lfY_ll + lfY_lr + lfY_ul + lfY_ur) / 4.0f;
									sprintf(buffer, "%d\0", lnThisSegment);
									DrawTextA(bmp->hdc, buffer, strlen(buffer), &lrc, DT_LEFT | DT_CALCRECT);
									SetRect(&lrc,	(int)(lfOriginX + lfX) - (lrc.right - lrc.left) / 2,
													(int)(lfOriginY + lfY) - (lrc.bottom - lrc.top) / 2,
													(int)(lfOriginX + lfX) + (lrc.right - lrc.left) / 2,
													(int)(lfOriginY + lfY) + (lrc.bottom - lrc.top) / 2);
									SetBkMode(bmp->hdc, TRANSPARENT);
									DrawTextA(bmp->hdc, buffer, strlen(buffer), &lrc, DT_LEFT);
								}
							}

							// Was this an inner match?
							if (llMatchInner)
								iBuilder_appendData(inners, (u8*)&wpd, sizeof(wpd));

							// Was this an outter value match?
							if (llMatchOutter)
								iBuilder_appendData(outters, (u8*)&wpd, sizeof(wpd));


						//////////
						// Outter rings
						//////
							if (lnSegment < lnSegments / 2)
							{
								// 12 o'clock to 6 o'clock, use normal outter ring
								// Left segment
								lfX_ll = lfRadiusOutterMin * lfCosThetaMin;
								lfY_ll = lfRadiusOutterMin * lfSinThetaMax;
								lfX_ul = lfRadiusOutterMax * lfCosThetaMin;
								lfY_ul = lfRadiusOutterMax * lfSinThetaMax;

								// Right segment
								lfX_lr = lfRadiusOutterMin * lfCosTheta2;
								lfY_lr = lfRadiusOutterMin * lfSinTheta2;
								lfX_ur = lfRadiusOutterMax * lfCosTheta2;
								lfY_ur = lfRadiusOutterMax * lfSinTheta2;

								// Midpoint
								wpd.x = lfOriginX + ((lfX_ll + lfX_lr) / 2.0f);
								wpd.y = lfOriginY + ((lfY_ll + lfY_lr) / 2.0f);

							} else {
								// 6 o'clock to 12 o'clock, use inverted rings
								// Left segment
								lfX_ll = lfRadiusInnerMin * lfCosThetaMin;
								lfY_ll = lfRadiusInnerMin * lfSinThetaMax;
								lfX_ul = lfRadiusInnerMax * lfCosThetaMin;
								lfY_ul = lfRadiusInnerMax * lfSinThetaMax;

								// Right segment
								lfX_lr = lfRadiusInnerMin * lfCosTheta2;
								lfY_lr = lfRadiusInnerMin * lfSinTheta2;
								lfX_ur = lfRadiusInnerMax * lfCosTheta2;
								lfY_ur = lfRadiusInnerMax * lfSinTheta2;

								// Midpoint
								wpd.x = lfOriginX + ((lfX_ul + lfX_ur) / 2.0f);
								wpd.y = lfOriginY + ((lfY_ul + lfY_ur) / 2.0f);
							}

							// Draw left segment, top, bottom
							lnThisSegment	= lnOutter + ((lnSegment - lnBackoff) * lnSegmentStep);
							llMatchInner	= (lnThisSegment % tsWnd->phwheel.nInner  == 0);
							llMatchOutter	= (lnThisSegment % tsWnd->phwheel.nOutter == 0);
							llMatch			= (llMatchInner || llMatchOutter);
							if (llBackoff)
							{
								// This is a segment at 12 o'clock or 6 o'clock, and it should be blank
								if (lnI == 0)
								{
									iFillRectArbitrary(tsWnd, bmp,	lfOriginX + lfX_ul, lfOriginY + lfY_ul, lfOriginX + lfX_ur, lfOriginY + lfY_ur,
																	lfOriginX + lfX_lr, lfOriginY + lfY_lr, lfOriginX + lfX_ll, lfOriginY + lfY_ll,
																	colorDark,
																	true, colorBorderOutter);
								}

							} else {
								// Draw normally
								if (lnI == 0)
								{
									iFillRectArbitrary(tsWnd, bmp,	lfOriginX + lfX_ul, lfOriginY + lfY_ul, lfOriginX + lfX_ur, lfOriginY + lfY_ur,
																	lfOriginX + lfX_lr, lfOriginY + lfY_lr, lfOriginX + lfX_ll, lfOriginY + lfY_ll,
																	((llMatch) ? ((lnSegment < lnSegments / 2) ? tsWnd->phwheel.nColorOutter : tsWnd->phwheel.nColorInner) : colorLight),
																	true, colorBorderOutter);
								}

								// Label it
								if (llMatch && lnI == 1)
								{
									lfX = (lfX_ll + lfX_lr + lfX_ul + lfX_ur) / 4.0f;
									lfY = (lfY_ll + lfY_lr + lfY_ul + lfY_ur) / 4.0f;
									sprintf(buffer, "%d\0", lnOutter + ((lnSegment - lnBackoff) * lnSegmentStep));
									DrawTextA(bmp->hdc, buffer, strlen(buffer), &lrc, DT_LEFT | DT_CALCRECT);
									SetRect(&lrc,	(int)(lfOriginX + lfX) - (lrc.right - lrc.left) / 2,
										(int)(lfOriginY + lfY) - (lrc.bottom - lrc.top) / 2,
										(int)(lfOriginX + lfX) + (lrc.right - lrc.left) / 2,
										(int)(lfOriginY + lfY) + (lrc.bottom - lrc.top) / 2);
									SetBkMode(bmp->hdc, TRANSPARENT);
									DrawTextA(bmp->hdc, buffer, strlen(buffer), &lrc, DT_LEFT);
								}
							}

							// Was this an inner match?
							if (llMatchInner)
								iBuilder_appendData(inners, (u8*)&wpd, sizeof(wpd));

							// Was this an outter match?
							if (llMatchOutter)
								iBuilder_appendData(outters, (u8*)&wpd, sizeof(wpd));

					}
				}


			//////////
			// Overlay lines connecting the points
			//////
				for (lnI = sizeof(SWheelPointData), wpdp = ((SWheelPointData*)inners->buffer) + 1; lnI < inners->populatedLength; lnI += sizeof(SWheelPointData), wpdp++)
				{
					// Draw this line
					iDrawLineArbitrary(tsWnd, bmp, (wpdp-1)->x, (wpdp-1)->y, wpdp->x, wpdp->y, rgb(0,128,255));
				}

				for (lnI = sizeof(SWheelPointData), wpdp = ((SWheelPointData*)outters->buffer) + 1; lnI < outters->populatedLength; lnI += sizeof(SWheelPointData), wpdp++)
				{
					// Draw this line
					iDrawLineArbitrary(tsWnd, bmp, (wpdp-1)->x, (wpdp-1)->y, wpdp->x, wpdp->y, rgb(255,128,0));
				}


			//////////
			// Clean house
			//////
				iBuilder_freeAndRelease(&inners);
				iBuilder_freeAndRelease(&outters);
		}
	}

	void iPHDnaOverlay(SWindow* tsWnd, SBitmap* bmp)
	{
	}




//////////
//
// Called to return the next unique id
//
//////
	int iGetNextUniqueId(void)
	{
		int lnUniqueId;

		EnterCriticalSection(&gcsUniqueId);
		lnUniqueId = gnNextUniqueId++;
		LeaveCriticalSection(&gcsUniqueId);
		return(lnUniqueId);
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
		if (!gsRootWind)
		{
			// This is the first one
			wndLast = &gsRootWind;

		} else {
			// We append to the end of the chain
			wnd = gsRootWind;
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
		if (gsRootWind)
		{
			// Iterate through all our entries
			wnd		= gsRootWind;
			wndLast	= &gsRootWind;
			while (wnd)
			{
				if (wnd == tsWnd)
				{
					// This is our man to delete
					// Perform specific things by type
					switch (wnd->type)
					{
						case _TYPE_MOVER:
							KillTimer(NULL, wnd->mover.timer);
							iiMoverDeleteObjectChain(&wnd->mover.firstObject);
							break;
					}

					// Un-subclass the window
					SetWindowLong(wnd->hwnd, GWL_WNDPROC, (long)wnd->oldWndProcAddress);

					// Update the linked list
					*wndLast	= wnd->next;

					// Delete the window
					DestroyWindow(wnd->hwnd);

					// Delete the stuff
					DeleteDC(wnd->hdc1);
					DeleteDC(wnd->bmpMain.hdc);
					DeleteObject((HGDIOBJ)wnd->bmpMain.hbmp);

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
		if (gsRootWind)
		{
			// Iterate through all windows
			wnd = gsRootWind;
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
// Called to locate the indicated parent hwnd (window) in the window link list
//
//////
	SWindow* iLocateWindowByParentHwnd(HWND hwndParent)
	{
		SWindow* wnd;


		// Make sure we have a window "to look through" (so to speak)
		wnd = NULL;
		if (gsRootWind)
		{
			// Iterate through all windows
			wnd = gsRootWind;
			while (wnd)
			{
				if (wnd->hwndParent == hwndParent)
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

			// Copy our string
			*tcDst = iDuplicateString(tcSrc, strlen(tcSrc), true);

			// We updated
			return(true);
		}
		// We did not have enough to update
		return(false);
	}




//////////
//
// Called to duplicate the string
//
//////
	char* iDuplicateString(char* tcSrc, int tnSrcLength, bool tlNullTerminate)
	{
		char* ldata;


		// Make sure our environment is sane
		ldata = NULL;
		if (tcSrc && tnSrcLength > 0)
		{
			ldata = (char*)malloc(tnSrcLength + ((tlNullTerminate) ? 1 : 0));
			if (ldata)
			{
				// Copy the raw data
				memcpy(ldata, tcSrc, tnSrcLength);

				// Should it be null-terminated?
				if (tlNullTerminate)
					ldata[tnSrcLength] = 0;		// Yes
			}
		}
		// Indicate our status
		return(ldata);
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
			lnHeight = -MulDiv(tsWnd->graph.fontSize, GetDeviceCaps(tsWnd->hdc1, LOGPIXELSY), 72);
			tsWnd->graph.textHFont = CreateFontA(lnHeight, 0, 0, 0, FW_NORMAL, false, false, false, ANSI_CHARSET, 0, 0, 0, 0, tsWnd->graph.fontName);

		} else {
			// Gauge
			lnHeight = -MulDiv(tsWnd->gauge.fontSize, GetDeviceCaps(tsWnd->hdc1, LOGPIXELSY), 72);
			tsWnd->gauge.textHFont = CreateFontA(lnHeight, 0, 0, 0, FW_NORMAL, false, false, false, ANSI_CHARSET, 0, 0, 0, 0, tsWnd->graph.fontName);

			// For the gauge we also create a bolder font for the value display area
			lnHeight = -MulDiv(tsWnd->gauge.fontSize + 2, GetDeviceCaps(tsWnd->hdc1, LOGPIXELSY), 72);
			tsWnd->gauge.textHFontBold = CreateFontA(lnHeight, 0, 0, 0, FW_BOLD, false, false, false, ANSI_CHARSET, 0, 0, 0, 0, tsWnd->graph.fontName);
		}
	}




//////////
//
// Paint the indicated gradient onto the bits
//
//////
	void iGradient4FillOrBitmapOverlay(SWindow* tsWnd, SBitmap* bmp2, SBitmap* bmp3)
	{
		int		lnX, lnY;
		float	lfRTopStep, lfGTopStep, lfBTopStep;		// Color step for each pixel across the top
		float	lfRTop, lfGTop, lfBTop;					// Colors used for top bar
		float	lfRBot, lfGBot, lfBBot;					// Colors used for bottom bar
		float	lfRBotStep, lfGBotStep, lfBBotStep;		// Color step for each pixel across the bottom
		SRGB*	lrgb2;
		SRGB*	lrgb3;


		if (tsWnd)
		{
			if (bmp2->bits && !bmp3->bits)
			{
				// We are doing the gradient fill
				// Grab our colorings
				// Top
				lfRTop		= (float)red(tsWnd->ulRgb);
				lfGTop		= (float)grn(tsWnd->ulRgb);
				lfBTop		= (float)blu(tsWnd->ulRgb);
				lfRTopStep	= ((float)red(tsWnd->urRgb) - lfRTop) / (float)bmp2->bmi.bmiHeader.biWidth;
				lfGTopStep	= ((float)grn(tsWnd->urRgb) - lfGTop) / (float)bmp2->bmi.bmiHeader.biWidth;
				lfBTopStep	= ((float)blu(tsWnd->urRgb) - lfBTop) / (float)bmp2->bmi.bmiHeader.biWidth;

				// Bottom
				lfRBot		= (float)red(tsWnd->llRgb);
				lfGBot		= (float)grn(tsWnd->llRgb);
				lfBBot		= (float)blu(tsWnd->llRgb);
				lfRBotStep	= ((float)red(tsWnd->lrRgb) - (float)red(tsWnd->llRgb)) / (float)bmp2->bmi.bmiHeader.biWidth;
				lfGBotStep	= ((float)grn(tsWnd->lrRgb) - (float)grn(tsWnd->llRgb)) / (float)bmp2->bmi.bmiHeader.biWidth;
				lfBBotStep	= ((float)blu(tsWnd->lrRgb) - (float)blu(tsWnd->llRgb)) / (float)bmp2->bmi.bmiHeader.biWidth;

				// Iterate across the bitmap
				for (lnX = 0; lnX < bmp2->bmi.bmiHeader.biWidth; lnX++)
				{
					// Draw this vertical line
					iGradient4VerticalLine(tsWnd, bmp2, lnX, lfRTop, lfGTop, lfBTop, lfRBot, lfGBot, lfBBot);

					// Increase the top color by its stepping
					lfRTop	+= lfRTopStep;
					lfGTop	+= lfGTopStep;
					lfBTop	+= lfBTopStep;

					// Increase the bottom color by its stepping
					lfRBot	+= lfRBotStep;
					lfGBot	+= lfGBotStep;
					lfBBot	+= lfBBotStep;
				}

			} else {
				// Overlay bitmap
				for (lnY = 0; lnY < bmp2->bmi.bmiHeader.biHeight && lnY < bmp3->bmi.bmiHeader.biHeight; lnY++)
				{
					lrgb2 = (SRGB*)(bmp2->bits + ((bmp2->bmi.bmiHeader.biHeight - lnY - 1) * bmp2->actualWidth));
					lrgb3 = (SRGB*)(bmp3->bits + ((bmp3->bmi.bmiHeader.biHeight - lnY - 1) * bmp3->actualWidth));
					for (lnX = 0; lnX < bmp2->bmi.bmiHeader.biWidth && lnX < bmp3->bmi.bmiHeader.biWidth; lnX++)
					{
						// Copy this pixel
						lrgb2->red	= lrgb3->red;
						lrgb2->grn	= lrgb3->grn;
						lrgb2->blu	= lrgb3->blu;

						// Move to next pixel
						++lrgb2;
						++lrgb3;
					}
				}
			}
		}
	}

	void iGradient4VerticalLine(SWindow* tsWnd, SBitmap* bmp, int tnX, float tfRTop, float tfGTop, float tfBTop, float tfRBot, float tfGBot, float tfBBot)
	{
		int		lnY;
		float	lfRMidStep, lfGMidStep, lfBMidStep;		// Colors used for middle runner
		SRGB*	lrgb;


		// Compute the steps from top to bottom
		lfRMidStep	= (tfRBot - tfRTop) / (float)bmp->bmi.bmiHeader.biHeight;
		lfGMidStep	= (tfGBot - tfGTop) / (float)bmp->bmi.bmiHeader.biHeight;
		lfBMidStep	= (tfBBot - tfBTop) / (float)bmp->bmi.bmiHeader.biHeight;

		// Compute where we will be in the bits
		lrgb = (SRGB*)(bmp->bits + ((bmp->bmi.bmiHeader.biHeight - 1) * bmp->actualWidth) + tnX * 3);
		for (lnY = 0; lnY < bmp->bmi.bmiHeader.biHeight; lnY++)
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
			lrgb = (SRGB*)((char*)lrgb - bmp->actualWidth);
		}
	}




//////////
//
// Validates that the bitmap header is correct
//
//////
	bool iIsValid24BitBitmap(BITMAPFILEHEADER* tbh, BITMAPINFOHEADER* tbi)
	{
		if (tbh && tbi)
		{
			if (tbh->bfType != 'MB')											return(false);		// All bitmap files begin with "BM" as first two bytes
			if (tbh->bfOffBits != sizeof(BITMAPFILEHEADER) + tbi->biSize)		return(false);		// Sizing calculations verify data validity
			if (tbh->bfSize != tbh->bfOffBits + tbi->biSizeImage)				return(false);		// Sizing calculations verify data validity
			// When we get here, the file header is good, and part of the info header (or so it seems)

			if (tbi->biPlanes != 1)					return(false);		// All 24-bit bitmaps are 1 plane
			if (tbi->biBitCount != 24)				return(false);		// Other forms are valid, but we only recognize 24-bit bitmaps (for now)
			if (tbi->biCompression != 0)			return(false);		// We do not support any compression formats
			// When we get here, we're good

			// Indicate our success
			return(true);
		}
		// If we get here, failure
		return(false);
	}




//////////
//
// Called to take the bitmap file and adjust the color data of the window image by the data from
// that bitmap file.  This has the effect of taking raw colors and making them look textured, or
// adding some shine, etc.
//
//////
	void iApplyOverlayBmpFile(SWindow* tsWnd, SBitmap* bmp, float tfX1, float tfY1, float tfX2, float tfY2, float tfTheta, unsigned char* tcOverlayBmpFile)
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
			if (lfYWnd >= 0.0f && lfXWnd >= 0.0f && (int)lfYWnd < bmp->bmi.bmiHeader.biHeight && (int)lfXWnd < bmp->bmi.bmiHeader.biWidth)
			{
				// Get our lfYBmp coordinate
				lfYBmp = (float)lbi->biHeight - (((float)lnStep / (float)lnStepCount) * (float)lbi->biHeight);

				// Get the offset for this point
				lrgbWnd = (SRGB*)(bmp->bits + ((bmp->bmi.bmiHeader.biHeight - (int)lfYWnd - 1) * bmp->actualWidth) + ((int)lfXWnd * 3));
				lrgbBmp = (SRGB*)(lbd       + ((lbi->biHeight -               (int)lfYBmp - 1) * lnActualWidth)    + ((int)lfXBmp * 3));

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




//////////
//
// Called to overlay the indicated bitmap using the specified alpha
//
//////
	void iOverlayBitmap(SBitmap* bmpDst, SBitmap* bmpSrc, int tnX, int tnY, float tfAlp)
	{
		iOverlayBitmapViaMethod(bmpDst, bmpSrc, tnX, tnY, _METHOD_ALPHA, tfAlp, 0);
	}




//////////
//
// Called to overlay the indicated bitmap using the indicated method
//
//////
	void iOverlayBitmapViaMethod(SBitmap* bmpDst, SBitmap* bmpSrc, int tnX, int tnY, int tnOverlayMethod, float tfAlp, int tnRgbMask)
	{
		unsigned char	lnRedMask, lnGrnMask, lnBluMask;
		int				lnX, lnY;
		float			lfMalp;
		SRGB*			lrgbs;
		SRGB*			lrgbd;


		if (bmpDst && bmpDst->bits && bmpDst->hbmp && bmpSrc && bmpSrc->bits && bmpSrc->hbmp)
		{
			if (tnOverlayMethod == _METHOD_OPAQUE || tnOverlayMethod == _METHOD_ALPHA || tnOverlayMethod == _METHOD_MASKED || tnOverlayMethod == (_METHOD_ALPHA + _METHOD_MASKED))
			{
				// For every row, copy it pixel by pixel
				lnRedMask	= red(tnRgbMask);
				lnGrnMask	= grn(tnRgbMask);
				lnBluMask	= blu(tnRgbMask);
				lfMalp		= 1.0f - tfAlp;
				for (lnY = 0; lnY + tnY < bmpDst->bmi.bmiHeader.biHeight && lnY < bmpSrc->bmi.bmiHeader.biHeight; lnY++)
				{
					// We we drawing on the target?
					if (lnY + tnY >= 0)
					{
						// Get our pointers into source and destination
						lrgbs = (SRGB*)(bmpSrc->bits + ((bmpSrc->bmi.bmiHeader.biHeight - lnY - 1)       * bmpSrc->actualWidth));
						lrgbd = (SRGB*)(bmpDst->bits + ((bmpDst->bmi.bmiHeader.biHeight - lnY - tnY - 1) * bmpDst->actualWidth) + (tnX * 3));

						// For every pixel across this bitmap, do the copy
						for (lnX = 0; lnX + tnX < bmpDst->bmi.bmiHeader.biWidth && lnX < bmpSrc->bmi.bmiHeader.biWidth; lnX++)
						{
							// Are we drawing on the target area?
							if (lnX + tnX >= 0)
							{
								// Copy this pixel
								switch (tnOverlayMethod)
								{
									case _METHOD_OPAQUE:
										// Just copy
										lrgbd->red	= lrgbs->red;
										lrgbd->grn	= lrgbs->grn;
										lrgbd->blu	= lrgbs->blu;
										break;

									case _METHOD_ALPHA:
										// Alpha blend
										lrgbd->red	= (unsigned char)(((float)lrgbd->red * lfMalp) + ((float)lrgbs->red * tfAlp));
										lrgbd->grn	= (unsigned char)(((float)lrgbd->grn * lfMalp) + ((float)lrgbs->grn * tfAlp));
										lrgbd->blu	= (unsigned char)(((float)lrgbd->blu * lfMalp) + ((float)lrgbs->blu * tfAlp));
										break;

									case _METHOD_MASKED:
										// Copy all except
										if (lrgbs->red != lnRedMask && lrgbs->grn != lnGrnMask && lrgbs->blu != lnBluMask)
										{
											lrgbd->red	= lrgbs->red;
											lrgbd->grn	= lrgbs->grn;
											lrgbd->blu	= lrgbs->blu;
										}
										break;

									case _METHOD_ALPHA + _METHOD_MASKED:
										// Alpha blend if not a mask color
										if (lrgbs->red != lnRedMask && lrgbs->grn != lnGrnMask && lrgbs->blu != lnBluMask)
										{
											lrgbd->red	= (unsigned char)(((float)lrgbd->red * lfMalp) + ((float)lrgbs->red * tfAlp));
											lrgbd->grn	= (unsigned char)(((float)lrgbd->grn * lfMalp) + ((float)lrgbs->grn * tfAlp));
											lrgbd->blu	= (unsigned char)(((float)lrgbd->blu * lfMalp) + ((float)lrgbs->blu * tfAlp));
										}
										break;
								}
							}

							// Move to next pixel
							++lrgbd;
							++lrgbs;
						}
					}
				}
			}
		}
	}




//////////
//
// Extracts the indicated bitmap out of the src, populating the bits into the destination.
// Note:  It is assumed that even if bmpDst is currently populated, it has been freed and is
//        ready to be overwritten.
//
//////
	void iExtractBitmap(SBitmap* bmpDst, SBitmap* bmpSrc, HDC thdc, int tnUlX, int tnUlY, int tnLrX, int tnLrY)
	{
		int		lnX, lnY;
		SRGB*	lrgbs;
		SRGB*	lrgbd;


		// Make sure our environment is sane
		if (bmpDst && bmpSrc && tnUlX < tnLrX && tnUlY < tnLrY)
		{
			// Create the new structure
			// Create a DIB section of the appropriate size
			memset(&bmpDst->bmi, 0, sizeof(bmpDst->bmi));
			bmpDst->bmi.bmiHeader.biSize			= sizeof(bmpDst->bmi.bmiHeader);
			bmpDst->bmi.bmiHeader.biWidth			= tnLrX - tnUlX;
			bmpDst->bmi.bmiHeader.biHeight			= tnLrY - tnUlY;
			bmpDst->bmi.bmiHeader.biCompression		= 0;
			bmpDst->bmi.bmiHeader.biPlanes			= 1;
			bmpDst->bmi.bmiHeader.biBitCount		= 24;
			bmpDst->bmi.bmiHeader.biXPelsPerMeter	= 3270;
			bmpDst->bmi.bmiHeader.biYPelsPerMeter	= 3270;
			// Compute the actual width
			bmpDst->actualWidth						= iComputeActualWidth(&bmpDst->bmi.bmiHeader);
			bmpDst->bmi.bmiHeader.biSizeImage		= bmpDst->actualWidth * (tnLrY - tnUlY);
			bmpDst->hdc		= CreateCompatibleDC(thdc);
			bmpDst->hbmp	= CreateDIBSection(bmpDst->hdc, &bmpDst->bmi, DIB_RGB_COLORS, (void**)&bmpDst->bits, NULL, 0);

			// Put the bitmap into the dc
			SelectObject(bmpDst->hdc, bmpDst->hbmp);

			// Extract and copy the bits as we go, row by row
			for (lnY = 0; lnY < bmpSrc->bmi.bmiHeader.biHeight; lnY++)
			{
				// Compute the source and destination pointers
				lrgbs = (SRGB*)(bmpSrc->bits + ((bmpSrc->bmi.bmiHeader.biHeight - lnY - 1)         * bmpSrc->actualWidth));
				lrgbd = (SRGB*)(bmpDst->bits + ((bmpDst->bmi.bmiHeader.biHeight - lnY - tnUlY - 1) * bmpDst->actualWidth) + (tnUlX * 3));

				// For every pixel horizontally on this line, copy it
				for (lnX = 0; lnX < bmpSrc->bmi.bmiHeader.biWidth; lnX++)
				{
					// Are we within bounds?
					if (lnY + tnUlY < bmpDst->bmi.bmiHeader.biHeight && lnX + tnUlX < bmpDst->bmi.bmiHeader.biWidth)
					{
						// Copy this pixel
						lrgbd->red	= lrgbs->red;
						lrgbd->grn	= lrgbs->grn;
						lrgbd->blu	= lrgbs->blu;

					} else {
						// Fill it with black
						lrgbd->red	= 0;
						lrgbd->grn	= 0;
						lrgbd->blu	= 0;
					}
					// Move to next pixel
					++lrgbs;
					++lrgbd;
				}
			}
		}
	}




	void iOverlayRectangle(SWindow* tsWnd, SBitmap* bmp, int tnUlX, int tnUlY, int tnLrX, int tnLrY, int tnFillRgb, int tnFrameRgb)
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
			if (lnY >= 0 && lnY < bmp->bmi.bmiHeader.biHeight)
			{
				// Find out on what row this pixel data will go
				lrgb = (SRGB*)((unsigned char*)bmp->bits + ((bmp->bmi.bmiHeader.biHeight - lnY - 1) * bmp->actualWidth) + (tnUlX * 3));

				// Draw the rectangle for all pixels that should be drawn horizontally
				for (lnX = tnUlX; lnX < tnLrX; lnX++)
				{
					if (lnX >= 0 && lnX < bmp->bmi.bmiHeader.biWidth)
					{
						// Each pixel is either a frame or fill color
						if (lnY == tnUlY || lnX <= tnUlX+1 || lnY >= tnLrY-2 || lnX >= tnLrX-2)
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
	void iDrawLineVertical(SWindow* tsWnd, SBitmap* bmp, int tnX, int tnUY, int tnLY, int tnRgb)
	{
		unsigned char	lnRed, lnGrn, lnBlu;
		int				lnY;
		SRGB*			lrgb;


		// Make sure this line would be visible
		if (tnX < 0 || tnX >= bmp->bmi.bmiHeader.biWidth)
			return;		// Nothing to do

		// Grab our colors
		lnRed	= (unsigned char)red(tnRgb);
		lnGrn	= (unsigned char)grn(tnRgb);
		lnBlu	= (unsigned char)blu(tnRgb);

		// Draw the rectangle for all pixels that should be drawn vertically
		for (lnY = tnUY; lnY < tnLY; lnY++)
		{
			if (lnY >= 0 && lnY < bmp->bmi.bmiHeader.biHeight)
			{
				// Find out on what row this pixel data will go
				lrgb = (SRGB*)((unsigned char*)bmp->bits + ((bmp->bmi.bmiHeader.biHeight - lnY - 1) * bmp->actualWidth) + (tnX * 3));

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
	void iDrawLineVerticalAlpha(SWindow* tsWnd, SBitmap* bmp, int tnX, int tnUY, int tnLY, int tnRgb, float tfAlp)
	{
		float	lfRed, lfGrn, lfBlu;
		float	lfMalp;
		int		lnY;
		SRGB*	lrgb;


		// Make sure this line would be visible
		if (tnX < 0 || tnX >= bmp->bmi.bmiHeader.biWidth)
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
			if (lnY >= 0 && lnY < bmp->bmi.bmiHeader.biHeight)
			{
				// Find out on what row this pixel data will go
				lrgb = (SRGB*)((unsigned char*)bmp->bits + ((bmp->bmi.bmiHeader.biHeight - lnY - 1) * bmp->actualWidth) + (tnX * 3));

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
	void iDrawLineHorizontal(SWindow* tsWnd, SBitmap* bmp, int tnLX, int tnRX, int tnY, int tnRgb)
	{
		unsigned char	lnRed, lnGrn, lnBlu;
		int				lnX;
		SRGB*			lrgb;


		// Make sure this line would be visible
		if (tnY < 0 || tnY >= bmp->bmi.bmiHeader.biHeight)
			return;		// Nothing to do

		// Grab our colors
		lnRed	= (unsigned char)red(tnRgb);
		lnGrn	= (unsigned char)grn(tnRgb);
		lnBlu	= (unsigned char)blu(tnRgb);

		// Find out on what row this pixel data will go
		lrgb = (SRGB*)((unsigned char*)bmp->bits + ((bmp->bmi.bmiHeader.biHeight - tnY - 1) * bmp->actualWidth) + (tnLX * 3));
		for (lnX = tnLX; lnX < tnRX; lnX++)
		{
			if (lnX >= 0 && lnX < bmp->bmi.bmiHeader.biWidth)
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
	void iDrawLineHorizontalAlpha(SWindow* tsWnd, SBitmap* bmp, int tnLX, int tnRX, int tnY, int tnRgb, float tfAlp)
	{
		float	lfRed, lfGrn, lfBlu;
		float	lfMalp;
		int		lnX;
		SRGB*	lrgb;


		// Make sure this line would be visible
		if (tnY < 0 || tnY >= bmp->bmi.bmiHeader.biHeight)
			return;		// Nothing to do

		// Grab our colors
		lfRed	= (float)red(tnRgb);
		lfGrn	= (float)grn(tnRgb);
		lfBlu	= (float)blu(tnRgb);

		// Make sure our alpha setting is in the range 0.0..1.0
		tfAlp	= max(min(tfAlp, 1.0f), 0.0f);
		lfMalp	= 1.0f - tfAlp;

		// Find out on what row this pixel data will go
		lrgb = (SRGB*)((unsigned char*)bmp->bits + ((bmp->bmi.bmiHeader.biHeight - tnY - 1) * bmp->actualWidth) + (tnLX * 3));
		for (lnX = tnLX; lnX < tnRX; lnX++)
		{
			if (lnX >= 0 && lnX < bmp->bmi.bmiHeader.biWidth)
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
	void iDrawLineArbitrary(SWindow* tsWnd, SBitmap* bmp, float tfX1, float tfY1, float tfX2, float tfY2, int tnRgb)
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
			if (lfY >= 0.0f && lfX >= 0.0f && (int)lfY < bmp->bmi.bmiHeader.biHeight && (int)lfX < bmp->bmi.bmiHeader.biWidth)
			{
				// Get the offset for this point
				lrgb = (SRGB*)(bmp->bits + ((bmp->bmi.bmiHeader.biHeight - (int)lfY - 1) * bmp->actualWidth) + ((int)lfX * 3));

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
	void iDrawLineArbitraryColorPath(SWindow* tsWnd, SBitmap* bmp, float tfX1, float tfY1, float tfX2, float tfY2, int tnRgb1, int tnRgb2)
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
			if (lfY >= 0.0f && lfX >= 0.0f && (int)lfY < bmp->bmi.bmiHeader.biHeight && (int)lfX < bmp->bmi.bmiHeader.biWidth)
			{
				// Get the offset for this point
				lrgb = (SRGB*)(bmp->bits + ((bmp->bmi.bmiHeader.biHeight - (int)lfY - 1) * bmp->actualWidth) + ((int)lfX * 3));

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
	void iGrayerLineArbitrary(SWindow* tsWnd, SBitmap* bmp, float tfX1, float tfY1, float tfX2, float tfY2)
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
			if (lfY >= 0.0f && lfX >= 0.0f && (int)lfY < bmp->bmi.bmiHeader.biHeight && (int)lfX < bmp->bmi.bmiHeader.biWidth)
			{
				// Get the offset for this point
				lrgb = (SRGB*)(bmp->bits + ((bmp->bmi.bmiHeader.biHeight - (int)lfY - 1) * bmp->actualWidth) + ((int)lfX * 3));

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
// Called to make sure all points of the text are visible.  Moves in on a line toward the origin
// based on the direction of theta
//
//////
	void iAdjustRectangle(SWindow* tsWnd, SBitmap* bmp, float tfXOrigin, float tfYOrigin, float tfTheta, RECT* lrc)
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
			if (iIsBetween(lrc->left + lnDeltaX, 0, bmp->bmi.bmiHeader.biWidth) && iIsBetween(lrc->right + lnDeltaX, 0, bmp->bmi.bmiHeader.biWidth))
			{
				// Horizontal is good, how about vertical?
				if (iIsBetween(lrc->top + lnDeltaY, 0, bmp->bmi.bmiHeader.biHeight) && iIsBetween(lrc->bottom + lnDeltaY, 0, bmp->bmi.bmiHeader.biHeight))
				{
					// Vertical is good, we're there
					llFound = true;
					break;
				}
			}

			// See if it's within the bounds
			lnDeltaX = -lnDeltaX;
			if (iIsBetween(lrc->left + lnDeltaX, 0, bmp->bmi.bmiHeader.biWidth) && iIsBetween(lrc->right + lnDeltaX, 0, bmp->bmi.bmiHeader.biWidth))
			{
				// Horizontal is good, how about vertical?
				if (iIsBetween(lrc->top + lnDeltaY, 0, bmp->bmi.bmiHeader.biHeight) && iIsBetween(lrc->bottom + lnDeltaY, 0, bmp->bmi.bmiHeader.biHeight))
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
// Converts values to strings if they are valid
//
//////
	void iStoreFloat(char* tcDest16, float tfValue)
	{
		if (tcDest16)
		{
			memset(tcDest16, 32, 16);
			sprintf_s(tcDest16, 16, "%08.2f", tfValue);
		}
	}

	void iStoreInt(char* tcDest16, int tnValue)
	{
		if (tcDest16)
		{
			memset(tcDest16, 32, 16);
			sprintf_s(tcDest16, 16, "%08u", tnValue);
		}
	}




//////////
//
// Called to create an empty 24-bit bitmap
//
//////
	void iPopulateEmpty24BitBitmapStructure(SBitmap* bmp, int tnWidth, int tnHeight)
	{
		memset(&bmp->bmi, 0, sizeof(bmp->bmi));
		bmp->bmi.bmiHeader.biSize			= sizeof(bmp->bmi.bmiHeader);
		bmp->bmi.bmiHeader.biWidth			= tnWidth;
		bmp->bmi.bmiHeader.biHeight			= tnHeight;
		bmp->bmi.bmiHeader.biCompression	= 0;
		bmp->bmi.bmiHeader.biPlanes			= 1;
		bmp->bmi.bmiHeader.biBitCount		= 24;
		bmp->bmi.bmiHeader.biXPelsPerMeter	= 3270;
		bmp->bmi.bmiHeader.biYPelsPerMeter	= 3270;
		bmp->actualWidth					= iComputeActualWidth(&bmp->bmi.bmiHeader);
		bmp->bmi.bmiHeader.biSizeImage		= bmp->actualWidth * tnHeight;
	}




//////////
//
// Called to fill in the indicated rectangle with a solid color
//
//////
	void iFillRect(SBitmap* bmp, int tnBackRgb)
	{
		int		lnX, lnY;
		char	lnRed, lnGrn, lnBlu;
		SRGB*	lrgb;


		// Make sure our environment is sane
		if (bmp && tnBackRgb != -1)
		{
			// Grab our colors
			lnRed = red(tnBackRgb);
			lnGrn = grn(tnBackRgb);
			lnBlu = blu(tnBackRgb);

			// Iterate for every row
			for (lnY = 0; lnY < bmp->bmi.bmiHeader.biHeight; lnY++)
			{
				// Compute the pointer for this row
				lrgb = (SRGB*)(bmp->bits + ((bmp->bmi.bmiHeader.biHeight - lnY - 1) * bmp->actualWidth));
				for (lnX = 0; lnX < bmp->bmi.bmiHeader.biWidth; lnX++)
				{
					// Set this pixel
					lrgb->red	= lnRed;
					lrgb->grn	= lnGrn;
					lrgb->blu	= lnBlu;

					// Move to next pixel
					++lrgb;
				}
			}
		}
	}




//////////
//
// Called to frame the indicated rectangle with a solid color
//
//////
	void iFrameRect(SBitmap* bmp, int tnBackRgb)
	{
		int		lnX, lnY;
		char	lnRed, lnGrn, lnBlu;
		SRGB*	lrgb;


		// Make sure our environment is sane
		if (bmp && tnBackRgb != -1)
		{
			// Grab our colors
			lnRed = red(tnBackRgb);
			lnGrn = grn(tnBackRgb);
			lnBlu = blu(tnBackRgb);

			// Iterate for every row
			for (lnY = 0; lnY < bmp->bmi.bmiHeader.biHeight; lnY++)
			{
				// Compute the pointer for this row
				lrgb = (SRGB*)(bmp->bits + ((bmp->bmi.bmiHeader.biHeight - lnY - 1) * bmp->actualWidth));
				if (lnY == 0 || lnY == bmp->bmi.bmiHeader.biHeight - 1)
				{
					// Do top and bottom
					for (lnX = 0; lnX < bmp->bmi.bmiHeader.biWidth; lnX++, lrgb++)
					{
						// Set this pixel
						lrgb->red	= lnRed;
						lrgb->grn	= lnGrn;
						lrgb->blu	= lnBlu;
					}

				} else {
					// Do left and right side
					// Left side
					lrgb->red	= lnRed;
					lrgb->grn	= lnGrn;
					lrgb->blu	= lnBlu;

					// Right side
					lrgb += bmp->bmi.bmiHeader.biWidth - 1;
					lrgb->red	= lnRed;
					lrgb->grn	= lnGrn;
					lrgb->blu	= lnBlu;
				}
			}
		}
	}




//////////
//
// Called to frame an arbitrary rectangle
//
//////
	void iFillRectArbitrary(SWindow* tsWnd, SBitmap* bmp, float tfXUl, float tfYUl, float tfXUr, float tfYUr, float tfXLr, float tfYLr, float tfXLl, float tfYLl, int tnRgb, bool tlFrame, int tnFrameRgb)
	{
		// Draw one normally, and draw another rotated (to hide integer aliasing on pixels ... Ugh! :-))
		iiFillRectArbitrary(tsWnd, bmp, tfXUl, tfYUl, tfXUr, tfYUr, tfXLr, tfYLr, tfXLl, tfYLl, tnRgb, tlFrame, tnFrameRgb);
		iiFillRectArbitrary(tsWnd, bmp, tfXUr, tfYUr, tfXLr, tfYLr, tfXLl, tfYLl, tfXUl, tfYUl, tnRgb, tlFrame, tnFrameRgb);
	}

	void iiFillRectArbitrary(SWindow* tsWnd, SBitmap* bmp, float tfXUl, float tfYUl, float tfXUr, float tfYUr, float tfXLr, float tfYLr, float tfXLl, float tfYLl, int tnRgb, bool tlFrame, int tnFrameRgb)
	{
		int		lnStep, lnStepCount;
		float	lfYL, lfXL, lfYR, lfXR, lfDeltaXL, lfDeltaYL, lfDeltaXR, lfDeltaYR, lfStepXL, lfStepYL, lfStepXR, lfStepYR;


		// Determine how many points we need to hit everything between
		lfDeltaXL		= tfXUl - tfXLl;
		lfDeltaYL		= tfYUl - tfYLl;
		lfDeltaXR		= tfXUr - tfXLr;
		lfDeltaYR		= tfYUr - tfYLr;

		// Prepare the stepping for each point
		lnStepCount		= max((int)(sqrt(lfDeltaXL*lfDeltaXL + lfDeltaYL*lfDeltaYL) * 2.0f), (int)(sqrt(lfDeltaXR*lfDeltaXR + lfDeltaYR*lfDeltaYR) * 2.0f));
		lfStepXL		= lfDeltaXL / (float)lnStepCount;
		lfStepYL		= lfDeltaYL / (float)lnStepCount;
		lfStepXR		= lfDeltaXR / (float)lnStepCount;
		lfStepYR		= lfDeltaYR / (float)lnStepCount;

		// Draw the points on this line
		for (	lnStep = 0, lfXL = tfXLl, lfYL = tfYLl, lfXR = tfXLr, lfYR = tfYLr;
				lnStep < lnStepCount;
				lnStep++, lfYL += lfStepYL, lfXL += lfStepXL, lfYR += lfStepYR, lfXR += lfStepXR	)
		{
			// Draw this line
			iDrawLineArbitrary(tsWnd, bmp, lfXL, lfYL, lfXR, lfYR, tnRgb);
		}

		// Frame if need be
		if (tlFrame)
		{
			// Left, top, right, bottom
			iDrawLineArbitrary(tsWnd, bmp, tfXLl, tfYLl, tfXUl, tfYUl, tnFrameRgb);
			iDrawLineArbitrary(tsWnd, bmp, tfXUl, tfYUl, tfXUr, tfYUr, tnFrameRgb);
			iDrawLineArbitrary(tsWnd, bmp, tfXUr, tfYUr, tfXLr, tfYLr, tnFrameRgb);
			iDrawLineArbitrary(tsWnd, bmp, tfXLr, tfYLr, tfXLl, tfYLl, tnFrameRgb);
		}
	}




//////////
//
// Called to overlay the indicated bitmap atop the other one using its black and white pixelation
// as color data to apply to the backRgb and foreRgb based on alpha settings.
//
//////
	void iOverlayBitmapViaColorizingAlphaBlend(SBitmap* bmpDst, SBitmap* bmpSrc, int tnBackRgb, int tnForeRgb, float tfAlpha)
	{
		int		lnX, lnY;
		float	lfRedBack, lfGrnBack, lfBluBack;
		float	lfRedFore, lfGrnFore, lfBluFore;
		float	lfRedThis, lfGrnThis, lfBluThis;
		float	malp, lalp1, lmalp1;
		SRGB*	lrgbs;
		SRGB*	lrgbd;


		// Make sure our environment is sane
		if (bmpDst && bmpSrc)
		{
			// Grab our colors
			// Background
			lfRedBack = (float)red(tnBackRgb);
			lfGrnBack = (float)grn(tnBackRgb);
			lfBluBack = (float)blu(tnBackRgb);
			// Foreground
			lfRedFore = (float)red(tnForeRgb);
			lfGrnFore = (float)grn(tnForeRgb);
			lfBluFore = (float)blu(tnForeRgb);

			// Compute our malp
			malp = 1.0f - tfAlpha;

			// Iterate for every row
			for (lnY = 0; lnY < bmpSrc->bmi.bmiHeader.biHeight; lnY++)
			{
				// Compute the pointer for this row
				lrgbs = (SRGB*)(bmpSrc->bits + ((bmpSrc->bmi.bmiHeader.biHeight - lnY - 1) * bmpSrc->actualWidth));
				lrgbd = (SRGB*)(bmpDst->bits + ((bmpDst->bmi.bmiHeader.biHeight - lnY - 1) * bmpDst->actualWidth));
				for (lnX = 0; lnX < bmpSrc->bmi.bmiHeader.biWidth; lnX++)
				{
					// We go through two alpha blendings here.
					// In the first, we derive the pixel colorization for red, grn, and blu for this one pixel based on the lrgbs color information
					lalp1	= (float)lrgbs->red / 255.0f;
					lmalp1	= 1.0f - lalp1;
					// Right now, we have our percentages for this pixel

					// Create this pixel's actual color
					lfRedThis	= (lfRedBack*lmalp1) + (lfRedFore*lalp1);
					lfGrnThis	= (lfGrnBack*lmalp1) + (lfGrnFore*lalp1);
					lfBluThis	= (lfBluBack*lmalp1) + (lfBluFore*lalp1);
					// Right now, we have our colors for this pixel

					// In the second, we put that color atop the existing color data using the alpha settings
					lrgbd->red	= (unsigned char)((((float)lrgbd->red)*malp) + (lfRedThis*tfAlpha));
					lrgbd->grn	= (unsigned char)((((float)lrgbd->grn)*malp) + (lfGrnThis*tfAlpha));
					lrgbd->blu	= (unsigned char)((((float)lrgbd->blu)*malp) + (lfBluThis*tfAlpha));

					// Move to next pixel
					++lrgbs;
					++lrgbd;
				}
			}
		}
	}
;




//////////
//
// Combines the two colors with the indicated weight
//
//////
	int iCombineColors(int tnColor1, int tnColor2, float tfColor1Weight)
	{
		float lfMalp;
		union {
			SRGB	color1;
			int		_color1;
		};
		union {
			SRGB	color2;
			int		_color2;
		};
		union {
			SRGB	color;
			int		_color;
		};


		//////////
		// Create the color
		//////
			lfMalp		= 1.0f - tfColor1Weight;
			_color1		= tnColor1;
			_color2		= tnColor2;
			color.red	= (unsigned char)(((float)color1.red * tfColor1Weight) + ((float)color2.red * lfMalp));
			color.grn	= (unsigned char)(((float)color1.grn * tfColor1Weight) + ((float)color2.grn * lfMalp));
			color.blu	= (unsigned char)(((float)color1.blu * tfColor1Weight) + ((float)color2.blu * lfMalp));


		//////////
		// Computed color
		//////
			return(_color);
	}




//////////
//
// Called to render the graph, gauge or mover (if it's not already rendering one)
//
//////
	void iRender(SWindow* tsWnd)
	{
		if (tsWnd)
		{
			// Are we already busy?
			if (!tsWnd->threadBusy)
			{
				// Indicate we're now busy
				tsWnd->threadBusy	= true;
				tsWnd->threadCalls	= 0;

				// Spawn the thread
				     if (tsWnd->type == _TYPE_GRAPH)		tsWnd->threadHandle = CreateThread(NULL, 0, buildGraphWorkerThreadProc,		(void*)tsWnd, 0, &tsWnd->threadId);
				else if (tsWnd->type == _TYPE_GAUGE)		tsWnd->threadHandle = CreateThread(NULL, 0, buildGaugeWorkerThreadProc,		(void*)tsWnd, 0, &tsWnd->threadId);
				else if (tsWnd->type == _TYPE_MOVER)		tsWnd->threadHandle = CreateThread(NULL, 0, buildMoverWorkerThreadProc,		(void*)tsWnd, 0, &tsWnd->threadId);
				else if (tsWnd->type == _TYPE_PBAR)			tsWnd->threadHandle = CreateThread(NULL, 0, buildPbarWorkerThreadProc,		(void*)tsWnd, 0, &tsWnd->threadId);
				else if (tsWnd->type == _TYPE_PHWHEEL)		tsWnd->threadHandle = CreateThread(NULL, 0, buildPHWheelWorkerThreadProc,	(void*)tsWnd, 0, &tsWnd->threadId);
				else if (tsWnd->type == _TYPE_PHDNA)		tsWnd->threadHandle = CreateThread(NULL, 0, buildPHDnaWorkerThreadProc,		(void*)tsWnd, 0, &tsWnd->threadId);
				else										_asm nop;
				// Note:  Called thread will reset busy upon termination

			} else {
				// Indicate we've had additional calls while previously rendering
				++tsWnd->threadCalls;
			}
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
			// Reset our call count
			wnd->threadCalls = 0;

			// Fill the background color
			iGradient4FillOrBitmapOverlay(wnd, &wnd->bmpMain, &wnd->bmpBackground);

			// Draw the basic gauge
			iGaugeRenderBasic(wnd, &wnd->bmpMain);

			// Overlay gauge
			iGaugeOverlayGraduation(wnd, &wnd->bmpMain);

			// Overlay needle
			iGaugeOverlayNeedle(wnd, &wnd->bmpMain);

			// Overlay value
			iGaugeOverlayHighlightedValue(wnd, &wnd->bmpMain);

			// All done!
			// Refresh the graph for the redraw
			InvalidateRect(wnd->hwnd, NULL, false);
			iPaintWindow(wnd);
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
			// Reset our call count
			wnd->threadCalls = 0;

			// Fill the background color
			iGradient4FillOrBitmapOverlay(wnd, &wnd->bmpMain, &wnd->bmpBackground);

			// Draw the graph
			iGraphDrawBars(wnd, &wnd->bmpMain);

			// Overlay the graduation
			iGraphOverlayGraduation(wnd, &wnd->bmpMain);

			// Show the grid
			if (wnd->graph.gridVisible != 0)
				iGraphDrawGrid(wnd, &wnd->bmpMain);

			// Populate the data points
			iGraphPopulateDataPoints(wnd, &wnd->bmpMain);

			// All done!
			// Refresh the graph for the redraw
			InvalidateRect(wnd->hwnd, NULL, false);
			iPaintWindow(wnd);
		}

		// Thread terminates
		CloseHandle(wnd->threadHandle);
		wnd->threadBusy = false;
		ExitThread(0);
	}

	DWORD WINAPI buildMoverWorkerThreadProc(LPVOID lpParameter)
	{
		SWindow* wnd;


		// Restore the parameter
		wnd = (SWindow*)lpParameter;
		if (wnd)
		{
			// Reset our call count
			wnd->threadCalls = 0;

			// Fill the background color
			iGradient4FillOrBitmapOverlay(wnd, &wnd->bmpMain, &wnd->bmpBackground);

			// Overlay / draw each object
			iMoverDrawObjects(wnd, &wnd->bmpMain);

			// All done!
			// Refresh the graph for the redraw
			InvalidateRect(wnd->hwnd, NULL, false);
			iPaintWindow(wnd);
		}

		// Thread terminates
		CloseHandle(wnd->threadHandle);
		wnd->threadBusy = false;
		ExitThread(0);
	}

	DWORD WINAPI buildPbarWorkerThreadProc(LPVOID lpParameter)
	{
		SWindow* wnd;


		// Restore the parameter
		wnd = (SWindow*)lpParameter;
		if (wnd)
		{
			// Reset our call count
			wnd->threadCalls = 0;

			// Fill the background color
			iGradient4FillOrBitmapOverlay(wnd, &wnd->bmpMain, &wnd->bmpBackground);

			// Overlay the progress bar
			iPbarOverlay(wnd, &wnd->bmpMain);

			// Overlay the border
			if (wnd->pbar.lShowBorder)
				iFrameRect(&wnd->bmpMain, wnd->pbar.nBorderColor);

			// Overlay needle
			iPbarOverlayNeedle(wnd, &wnd->bmpMain);

			// All done!
			// Refresh the graph for the redraw
			InvalidateRect(wnd->hwnd, NULL, false);
			iPaintWindow(wnd);
		}

		// Thread terminates
		CloseHandle(wnd->threadHandle);
		wnd->threadBusy = false;
		ExitThread(0);
	}

	DWORD WINAPI buildPHWheelWorkerThreadProc(LPVOID lpParameter)
	{
		SWindow* wnd;


		// Restore the parameter
		wnd = (SWindow*)lpParameter;
		if (wnd)
		{
			// Reset our call count
			wnd->threadCalls = 0;

			// Fill the background color
			iGradient4FillOrBitmapOverlay(wnd, &wnd->bmpMain, &wnd->bmpBackground);

			// Overlay the prime harmonics wheel
			iPHWheelOverlay(wnd, &wnd->bmpMain);

			// All done!
			// Refresh the graph for the redraw
			InvalidateRect(wnd->hwnd, NULL, false);
			iPaintWindow(wnd);
		}

		// Thread terminates
		CloseHandle(wnd->threadHandle);
		wnd->threadBusy = false;
		ExitThread(0);
	}

	DWORD WINAPI buildPHDnaWorkerThreadProc(LPVOID lpParameter)
	{
		SWindow* wnd;


		// Restore the parameter
		wnd = (SWindow*)lpParameter;
		if (wnd)
		{
			// Reset our call count
			wnd->threadCalls = 0;

			// Fill the background color
			iGradient4FillOrBitmapOverlay(wnd, &wnd->bmpMain, &wnd->bmpBackground);

			// Overlay the prime harmonics wheel
			iPHDnaOverlay(wnd, &wnd->bmpMain);

			// All done!
			// Refresh the graph for the redraw
			InvalidateRect(wnd->hwnd, NULL, false);
			iPaintWindow(wnd);
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
		SWindow*	wnd;
//		RECT		lrc;
//		HDC			lhdc;
		

		// If we are painting, paint our areas
		if (gsRootWind)
		{
			wnd = iLocateWindow((int)hwnd);
			if (wnd)
			{
				switch (uMsg)
				{
					case WM_PAINT:
						iPaintWindow(wnd);
						return(0);

/*
					case WM_NCACTIVATE:
					case WM_NCPAINT:
						DefWindowProc(hwnd, uMsg, wParam, lParam);
						SetRect(&lrc, 2, 2, 22, 22);
						lhdc = GetWindowDC(hwnd);
						FillRect(lhdc, &lrc, (HBRUSH)GetStockObject(BLACK_BRUSH));
						ReleaseDC(hwnd, lhdc);
						return 0;
						break;
*/

					case WM_ERASEBKGND:
						// Ignore it
						return(0);

					case WM_MOUSEMOVE:
					case WM_LBUTTONDOWN:
					case WM_LBUTTONUP:
					case WM_RBUTTONDOWN:
					case WM_RBUTTONUP:
						iStoreMouseData(&wnd->mouseCurrent, wParam, lParam);
						iiSignalEventsBasedOnMouseChanges(wnd);
						break;
				}
			}
		}
		// Do normal drawing
		return(DefWindowProc(hwnd, uMsg, wParam, lParam));
	}





//////////
//
// Callback to handle animations
//
//////
	VOID CALLBACK iiMoverTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
	{
		int			lnX, lnY;
		bool		llMovedOne, llLeft, llRight;
		SWindow*	wnd;
		SMoverObj*	obj;
		POINT		pt;
		RECT		lrc;


		// Grab the cursor position
		llLeft	= (GetAsyncKeyState(VK_LBUTTON) != 0);
		llRight	= (GetAsyncKeyState(VK_RBUTTON) != 0);
		GetCursorPos(&pt);


		// Iterate through every window that is the right type and move anything that needs moved
		wnd = gsRootWind;
		while (wnd)
		{
			// Is this a mover?
			if (wnd && wnd->type == _TYPE_MOVER)
			{
				GetWindowRect((HWND)wnd->hwnd, &lrc);
				lnX	= pt.x - lrc.left;
				lnY	= pt.y - lrc.top;

				// Grab current mouse information
				wnd->mouseCurrent.isValid		= true;
				wnd->mouseCurrent.x				= lnX;
				wnd->mouseCurrent.y				= lnY;
				wnd->mouseCurrent.leftButton	= llLeft;
				wnd->mouseCurrent.rightButton	= llRight;

				// Signal any mouse movements
				iiSignalEventsBasedOnMouseChanges(wnd);

				// Based on animations, move things
				llMovedOne	= false;
				obj			= wnd->mover.firstObject;
				while (obj)
				{
					// See if this item needs animated
					if (obj->curr.stepCount > 0)
					{
						// This one needs moved
						llMovedOne = true;

						// Move it
						--obj->curr.stepCount;
						obj->curr.x += obj->curr.xStep;
						obj->curr.y += obj->curr.yStep;
					}

					// Move to next object
					obj = obj->next;
				}

				// Now, if we moved one, we need to re-render it
				if (llMovedOne)
					iRender(wnd);
			}

			// Move to next window
			wnd = wnd->next;
		}
	}

	void iPaintWindow(SWindow* tsWnd)
	{
		HDC			lhdc;
		PAINTSTRUCT ps;


		// Begin painting
		lhdc = BeginPaint((HWND)tsWnd->hwnd, &ps);

		// Draw our bitmap
		BitBlt(	lhdc, 
				0, 0, tsWnd->rcThis.right, tsWnd->rcThis.bottom,
				tsWnd->bmpMain.hdc, 0, 0, SRCCOPY);

		// End painting
		EndPaint((HWND)tsWnd->hwnd, &ps);

		// Validate the area we just painted
		ValidateRect((HWND)tsWnd->hwnd, &tsWnd->rcThis);

		// Validate the rectangle of the parent where this HWND occupies
		ValidateRect((HWND)tsWnd->hwndParent, &tsWnd->rcParent);
	}

	void iStoreMouseData(SMouse* mouse, WPARAM wParam, LPARAM lParam)
	{
		mouse->isValid		= true;
		mouse->x			= (int)LOWORD(lParam); 
		mouse->y			= (int)HIWORD(lParam); 
		mouse->leftButton	= ((wParam & MK_LBUTTON) != 0);
		mouse->rightButton	= ((wParam & MK_RBUTTON) != 0);
	}

	void iiSignalEventsBasedOnMouseChanges(SWindow* tsWnd)
	{
		bool llReRender, llAnyMove, llLeft, llRight;


		// If we have a valid set of mouse moves (or more), then process
		if (tsWnd->type == _TYPE_MOVER && tsWnd->mouseLast.isValid)
		{
			// If we get here, we have to see if anything changed
			llAnyMove	= ((tsWnd->mouseLast.x != tsWnd->mouseCurrent.x) || (tsWnd->mouseLast.y != tsWnd->mouseCurrent.y));
			llLeft		= (tsWnd->mouseLast.leftButton	!= tsWnd->mouseCurrent.leftButton);
			llRight		= (tsWnd->mouseLast.rightButton	!= tsWnd->mouseCurrent.rightButton);
			if (llAnyMove || llLeft || llRight)
			{
				// Do we re-render?
				llReRender	= false;

				// The mouse moved
				if (llAnyMove)
					llReRender |= iiMoverTrackMouseMovement(tsWnd, false);

				// The left button changed
				if (llLeft)
				{
					if (tsWnd->mouseLast.leftButton)
						llReRender |= iiMoverTrackMouseLeftButtonReleased(tsWnd);		// The mouse left button has been released
					else
						llReRender |= iiMoverTrackMouseLeftButtonPressed(tsWnd);		// The mouse left button has been pressed
				}

				// The right button changed
				if (llRight)
				{
					if (tsWnd->mouseLast.rightButton)
						llReRender |= iiMoverTrackMouseRightButtonReleased(tsWnd);		// The mouse right button has been released
					else
						llReRender |= iiMoverTrackMouseRightButtonPressed(tsWnd);		// The mouse right button has been pressed
				}

				// If we need to re-render, then do so
				if (llReRender)
					iRender(tsWnd);		// Re-render after this movement
			}
		}

		// Update the mouse data
		memcpy(&tsWnd->mouseLast, &tsWnd->mouseCurrent, sizeof(SMouse));
	}
