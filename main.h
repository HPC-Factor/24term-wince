// main.h: main ‚Ì•Ï”
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAIN_H__6C11D5EF_D552_4119_8EEC_18D320E7B791__INCLUDED_)
#define AFX_MAIN_H__6C11D5EF_D552_4119_8EEC_18D320E7B791__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

extern BOOL CALLBACK messageproc(HWND, UINT, WPARAM, LPARAM);
extern long dispactivate(WPARAM);
extern long disppalette();
extern void titledraw(HDC);
extern void titlesetup();
extern PCWSTR drawnew();
extern int screencur;
extern HWND wbase, wdisp, wtitle, wscroll;
extern HWND wmodal;

#endif // !defined(AFX_MAIN_H__6C11D5EF_D552_4119_8EEC_18D320E7B791__INCLUDED_)
