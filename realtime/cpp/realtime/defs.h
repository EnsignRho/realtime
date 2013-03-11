//////////
//
// defs.h
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
// Macros
//////
	#define red(x)					 (x & 0x0000ff)
	#define grn(x)					((x & 0x00ff00) >> 8)
	#define blu(x)					((x & 0xff0000) >> 16)
	#define rgb(r,g,b)				((unsigned int)r + ((unsigned int)g << 8) + ((unsigned int)b << 16))
	#define makePastel(x)			rgb((255 + red(x)) / 2, (255 + grn(x)) / 2, (255 + blu(x)) / 2)
	#define makeDarker(x)			rgb(red(x)/2, grn(x)/2, blu(x)/2)
	#define makeSomewhatDarker(x)	rgb(red(x) * 7/9, grn(x) * 7/9, blu(x) * 7/9)




//////////
// Forward declarations
//////
	REALTIME_API int		realtime_subclass_form_as					(HWND tnHwndParent, int tnType, int tnX, int tnY, int tnWidth, int tnHeight, int tnUlRgb, int tnUrRgb, int tnLrRgb, int tnLlRgb);
	REALTIME_API void		realtime_un_subclass_form					(int tnHandle);
	REALTIME_API void		realtime_show_or_hide						(int tnHandle, int tnShow);
	REALTIME_API void		realtime_test_mode							(int tnEnabled);

	// For graph functions
	REALTIME_API void		realtime_graph_setup						(int tnHandle, int tnBar1Rgb, int tnBar2Rgb, int tnDataRgb, char* tcFontName, int tnFontSize, int tnTextRgb, int tnDataPointThickness, float tfRangeUpper, float tfRangeLower, int tnSampleAverageCount, int tnGraduationIntegers, int tnGraduationDecimals);
	REALTIME_API void		realtime_graph_setup2						(int tnHandle, int tnGridVisible, float tfGridCenter, float tfGridSquareSize, int tnGridCenterRgb, int tnGridLinesRgb, float tfGridAlpha, int tnMarginLeft, int tnMarginRight, int rangeVisible, int rangeRgb, float rangeAlpha, int rangeAverageSamples);
	REALTIME_API void		realtime_graph_add_data_point				(int tnHandle, float tfRangeUpper, float tfRangeLower, float tfFloatDataPoint);
	REALTIME_API void		realtime_graph_rescale_samples				(int tnHandle, float tfScaleFactor);
	REALTIME_API void		realtime_graph_delete_all_samples			(int tnHandle);
	REALTIME_API void		realtime_graph_redraw						(int tnHandle);

	// For gauge functions
	REALTIME_API void		realtime_gauge_setup						(int tnHandle, char* tcTextVFontName, int tnTextVFontSize, int tnTextVRgb, int tnLowRgb, int tnMidRgb, int tnHighRgb, float tfRangeStart, float tfRangeEnd, float tfRangePercentLow, float tfRangePercentMid, int tnColorizeNeedle, int tnFrameGauge, int tnGraduationIntegers, int tnGraduationDecimals, int tnHighlightedIntegers, int tnHighlightedDecimals);
	REALTIME_API void		realtime_gauge_setup2						(int tnHandle, int tnSnapToDisplayedValue, float tfScaleFactor);
	REALTIME_API void		realtime_gauge_set_needle_position			(int tnHandle, float tfRangeStart, float tfRangeEnd, float tfNeedleValue);
	REALTIME_API void		realtime_gauge_redraw						(int tnHandle);


//////////
// Local/internal function prototype definitions
//////
	SWindow*				iCreateNewSWindow							(void);
	void					iDeleteSWindow								(SWindow* tsWnd);
	SWindow*				iLocateWindow								(int tnHandle);
	void					iAppendGraphDataPoint						(SWindow* tsWnd, float tfRangeUpper, float tfRangeLower, float tfFloatDataPoint);
	bool					iCopyStringIfDifferent						(char** tcDst, char* tcSrc);
	void					iUpdateFont									(SWindow* tsWnd);
	void					iGradient4Fill								(SWindow* tsWnd);
	void					iGradient4VerticalLine						(SWindow* tsWnd, int tnX, float tfRTop, float tfGTop, float tfBTop, float tfRBot, float tfGBot, float tfBBot);
	void					iGraphDrawBars								(SWindow* tsWnd);
	void					iGraphDrawGrid								(SWindow* tsWnd);
	void					iGraphOverlayGraduation						(SWindow* tsWnd);
	void					iGraphPopulateDataPoints					(SWindow* tsWnd);
	void					iGraphFindHighLowAvgPoints					(SWindow* tsWnd, int tnSkipTo, int tnSamples, float* tfHigh, float* tfLow, float* tfAvg);
	void					iApplyOverlayBmpFile						(SWindow* tsWnd, float tfX1, float tfY1, float tfX2, float tfY2, float tfTheta, unsigned char* tcOverlayBmpFile);
	void					iOverlayRectangle							(SWindow* tsWnd, int tnUlX, int tnUlY, int tnLrX, int tnLrY, int tnFillRgb, int tnFrameRgb);
	void					iDrawLineVertical							(SWindow* tsWnd, int tnX, int tnUY, int tnLY, int tnRgb);
	void					iDrawLineVerticalAlpha						(SWindow* tsWnd, int tnX, int tnUY, int tnLY, int tnRgb, float tfAlp);
	void					iDrawLineHorizontal							(SWindow* tsWnd, int tnLX, int tnRX, int tnY, int tnRgb);
	void					iDrawLineHorizontalAlpha					(SWindow* tsWnd, int tnLX, int tnRX, int tnY, int tnRgb, float tfAlp);
	void					iDrawLineArbitrary							(SWindow* tsWnd, float tfX1, float tfY1, float tfX2, float tfY2, int tnRgb);
	void					iDrawLineArbitraryColorPath					(SWindow* tsWnd, float tfX1, float tfY1, float tfX2, float tfY2, int tnRgb1, int tnRgb2);
	void					iGrayerLineArbitrary						(SWindow* tsWnd, float tfX1, float tfY1, float tfX2, float tfY2);
	void					iGaugeRenderBasic							(SWindow* tsWnd);
	void					iGaugeOverlayGraduation						(SWindow* tsWnd);
	void					iGaugeOverlayNeedle							(SWindow* tsWnd);
	void					iGaugeOverlayHighlightedValue				(SWindow* tsWnd);
	float					iGaugeGetSnappedNeedleValue					(SWindow* tsWnd);
	void					iAdjustRectangle							(SWindow* tsWnd, float tfXOrigin, float tfYOrigin, float tfTheta, RECT* lrc);
	bool					iIsBetween									(int tnTestValue, int tnValue1, int tnValue2);
	void					iLtrimBuffer								(char* tcData);
	int						iComputeActualWidth							(BITMAPINFOHEADER* tbi);
	void					iRenderTheGraph								(SWindow* tsWnd);
	void					iRenderTheGauge								(SWindow* tsWnd);
	DWORD WINAPI			buildGaugeWorkerThreadProc					(LPVOID lpParameter);
	DWORD WINAPI			buildGraphWorkerThreadProc					(LPVOID lpParameter);
	LRESULT CALLBACK		realtimeWndProc								(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
