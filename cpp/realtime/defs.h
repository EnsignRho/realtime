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
	REALTIME_API int		realtime_subclass_form_as					(HWND tnHwndParent, int tnType, int tnX, int tnY, int tnWidth, int tnHeight, int tnUlRgb, int tnUrRgb, int tnLrRgb, int tnLlRgb, int tnCaptureUlX, int tnCaptureUlY);
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

	// For mover functions
	REALTIME_API void		realtime_mover_setup						(int tnHandle, int tnMarginVertical, int tnMarginHorizontal);
	REALTIME_API int		realtime_mover_create_object_with_text		(int tnHandle, int tnWidth, int tnHeight, char* tcText, int tnTextLength, int tnBackRgb, int tnForeRgb, float tfAlpha, char* tcFontName, int tnFontSize, int tnBold, int tnItaclics, int tnUnderline, int tnBorderRgb, int tnBorderThickness);
	REALTIME_API int		realtime_mover_acquire_object_from_rect		(int tnHandle, int tnHwndParent, int tnUlX, int tnUlY, int tnLrX, int tnLrY);
	REALTIME_API int		realtime_mover_acquire_from_file			(int tnHandle, char* tcBmp24Name, int tnBmp24NameLength);
	REALTIME_API int		realtime_mover_acquire_inner_rect			(int tnHandle, int tnObjectId, int tnUlX, int tnUlY, int tnLrX, int tnLrY);
	REALTIME_API int		realtime_mover_save_object					(int tnHandle, int tnObjectId, char* tcFilename, int tnFilenameLength);
	REALTIME_API int		realtime_mover_set_visible					(int tnHandle, int tnObjectId, int tnVisible);
	REALTIME_API int		realtime_mover_set_disposition_object		(int tnHandle, int tnObjectId, int tnDispositionObjectId, int tnDisposition);
	REALTIME_API int		realtime_mover_set_event_mask				(int tnHandle, int tnObjectId, int tnClick, int nRightClick, int tnMouseMove, int tnMouseEnter, int tnMouseLeave, int tnDragStart, int tnDragMove, int tnDragAbort, int tnDragDrop, int tnHover);
	REALTIME_API int		realtime_mover_overlay_object				(int tnhandle, int tnObjectId, int tnOverlayObjectId, int tnX, int tnY, int tnOverlayMethod, float tfAlp, int tnRgbMask);
	REALTIME_API int		realtime_mover_delete_object				(int tnHandle, int tnObjectId);
	REALTIME_API int		realtime_mover_setup_object					(int tnHandle, int tnObjectId, int tnCol, int tnRow, int tnDraggable, int tnAcceptsDrops, int tnCopiesOnDrop/*if no, moves on drop*/, int tnCallbackCode);
	REALTIME_API int		realtime_mover_recompute					(int tnHandle);
	REALTIME_API void		realtime_mover_redraw						(int tnHandle);


//////////
// Local/internal function prototype definitions
//////
	void					iGraphDrawBars								(SWindow* tsWnd, SBitmap* bmp);
	void					iGraphDrawGrid								(SWindow* tsWnd, SBitmap* bmp);
	void					iGraphOverlayGraduation						(SWindow* tsWnd, SBitmap* bmp);
	void					iGraphPopulateDataPoints					(SWindow* tsWnd, SBitmap* bmp);
	void					iGraphFindHighLowAvgPoints					(SWindow* tsWnd, int tnSkipTo, int tnSamples, float* tfHigh, float* tfLow, float* tfAvg);

	void					iGaugeRenderBasic							(SWindow* tsWnd, SBitmap* bmp);
	void					iGaugeOverlayGraduation						(SWindow* tsWnd, SBitmap* bmp);
	void					iGaugeOverlayNeedle							(SWindow* tsWnd, SBitmap* bmp);
	void					iGaugeOverlayHighlightedValue				(SWindow* tsWnd, SBitmap* bmp);
	float					iGaugeGetSnappedNeedleValue					(SWindow* tsWnd);

	SMoverObj*				iMoverAppendNewObject						(SWindow* tsWnd);
	SMoverObj*				iMoverLocateObject							(SWindow* tsWnd, int tnObjectId);
	void					iMoverDrawObjects							(SWindow* tsWnd, SBitmap* bmp);
	void					iMoverDrawSnaps								(SWindow* tsWnd, SBitmap* bmp, SMoverObj* moSnap);
	void					iMoverOverlayBitmap							(SWindow* tsWnd, SBitmap* bmp, SMoverObj* mo, SMoverPos* mop, float tfAlp);
	void					iMoverFillRect								(SWindow* tsWnd, SBitmap* bmp, SMoverObj* mo, SMoverPos* mop, float tfAlp, int tnRgb);
	void					iMoverGetObjectExtents						(SWindow* tsWnd, int* tnMaxCol, int* tnMaxRow, int* tnMaxWidth, int* tnMaxHeight);
	void					iiMoverSetPosition							(SMoverPos* mp, SMoverPos* mpRef, int snapDirection, int x, int y, int width, int height);
	bool					iiMoverTrackMouseMovement					(SWindow* tsWnd, bool tlForce);
	bool					iiMoverSnapPositionHasMoved					(SWindow* tsWnd, SMoverPos* snap);
	SMoverPos*				iiMoverDetermineSnapPosition				(SWindow* tsWnd, SMoverPos* snapReal, SMoverObj** objSnappedOn);
	bool					iiMoverIsPointInThisPosition				(SMoverPos* mp, int x, int y);
	void					iiMoverKickOffAnimationsViaSnapPosition		(SWindow* tsWnd, SMoverPos* snap);
	void					iiMoverDetermineSnapAdjustments				(SWindow* tsWnd, SMoverObj* obj, SMoverPos* snapCandidate, int* tnRowDelta, int* tnColDelta);
	void					iiMoverAnimateTo							(SWindow* tsWnd, SMoverObj* obj, int tnCol, int tnRow);
	bool					iiMoverTrackMouseLeftButtonReleased			(SWindow* tsWnd);
	bool					iiMoverTrackMouseLeftButtonPressed			(SWindow* tsWnd);
	bool					iiMoverTrackMouseRightButtonReleased		(SWindow* tsWnd);
	bool					iiMoverTrackMouseRightButtonPressed			(SWindow* tsWnd);
	void					iiMoverDeleteObjectChain					(SMoverObj** objRoot);

	int						iGetNextUniqueId							(void);
	SWindow*				iCreateNewSWindow							(void);
	void					iDeleteSWindow								(SWindow* tsWnd);
	SWindow*				iLocateWindow								(int tnHandle);
	SWindow*				iLocateWindowByParentHwnd					(HWND hwndParent);
	void					iAppendGraphDataPoint						(SWindow* tsWnd, float tfRangeUpper, float tfRangeLower, float tfFloatDataPoint);
	bool					iCopyStringIfDifferent						(char** tcDst, char* tcSrc);
	void					iUpdateFont									(SWindow* tsWnd);
	void					iGradient4FillOrBitmapOverlay				(SWindow* tsWnd, SBitmap* bmp2, SBitmap* bmp3);
	void					iGradient4VerticalLine						(SWindow* tsWnd, SBitmap* bmp, int tnX, float tfRTop, float tfGTop, float tfBTop, float tfRBot, float tfGBot, float tfBBot);
	bool					iIsValid24BitBitmap							(BITMAPFILEHEADER* tbh, BITMAPINFOHEADER* tbi);
	void					iApplyOverlayBmpFile						(SWindow* tsWnd, SBitmap* bmp, float tfX1, float tfY1, float tfX2, float tfY2, float tfTheta, unsigned char* tcOverlayBmpFile);
	void					iOverlayBitmap								(SWindow* tsWnd, SBitmap* bmpDst, SBitmap* bmpSrc, int tnX, int tnY, float tfAlpha);
	void					iOverlayRectangle							(SWindow* tsWnd, SBitmap* bmp, int tnUlX, int tnUlY, int tnLrX, int tnLrY, int tnFillRgb, int tnFrameRgb);
	void					iDrawLineVertical							(SWindow* tsWnd, SBitmap* bmp, int tnX, int tnUY, int tnLY, int tnRgb);
	void					iDrawLineVerticalAlpha						(SWindow* tsWnd, SBitmap* bmp, int tnX, int tnUY, int tnLY, int tnRgb, float tfAlp);
	void					iDrawLineHorizontal							(SWindow* tsWnd, SBitmap* bmp, int tnLX, int tnRX, int tnY, int tnRgb);
	void					iDrawLineHorizontalAlpha					(SWindow* tsWnd, SBitmap* bmp, int tnLX, int tnRX, int tnY, int tnRgb, float tfAlp);
	void					iDrawLineArbitrary							(SWindow* tsWnd, SBitmap* bmp, float tfX1, float tfY1, float tfX2, float tfY2, int tnRgb);
	void					iDrawLineArbitraryColorPath					(SWindow* tsWnd, SBitmap* bmp, float tfX1, float tfY1, float tfX2, float tfY2, int tnRgb1, int tnRgb2);
	void					iGrayerLineArbitrary						(SWindow* tsWnd, SBitmap* bmp, float tfX1, float tfY1, float tfX2, float tfY2);
	void					iAdjustRectangle							(SWindow* tsWnd, SBitmap* bmp, float tfXOrigin, float tfYOrigin, float tfTheta, RECT* lrc);
	bool					iIsBetween									(int tnTestValue, int tnValue1, int tnValue2);
	void					iLtrimBuffer								(char* tcData);
	int						iComputeActualWidth							(BITMAPINFOHEADER* tbi);

	void					iRender										(SWindow* tsWnd);
	DWORD WINAPI			buildGaugeWorkerThreadProc					(LPVOID lpParameter);
	DWORD WINAPI			buildGraphWorkerThreadProc					(LPVOID lpParameter);
	DWORD WINAPI			buildMoverWorkerThreadProc					(LPVOID lpParameter);
	LRESULT CALLBACK		realtimeWndProc								(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	VOID CALLBACK			iiMoverTimerProc							(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
	void					iPaintWindow								(SWindow* tsWnd);
	void					iStoreMouseData								(SMouse* mouse, WPARAM wParam, LPARAM lParam);
	void					iiSignalEventsBasedOnMouseChanges			(SWindow* tsWnd);
