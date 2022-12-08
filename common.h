// common.h: ‹¤’Ê’è‹`
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMMON_H__763A7DA8_5692_4450_A9EE_9C1FC1CF43C2__INCLUDED_)
#define AFX_COMMON_H__763A7DA8_5692_4450_A9EE_9C1FC1CF43C2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define numof(a) (sizeof(a)/sizeof(*(a)))
extern HWND wdebug;

extern HINSTANCE theInst;
struct Screen;
extern Screen* theScreen;
struct Draw;
extern Draw* theDraw;

#endif // !defined(AFX_COMMON_H__763A7DA8_5692_4450_A9EE_9C1FC1CF43C2__INCLUDED_)
