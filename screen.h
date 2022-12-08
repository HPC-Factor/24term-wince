// screen.h: Screen クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCREEN_H__8C8E06D6_4568_48B2_862F_4EDDBFB7C046__INCLUDED_)
#define AFX_SCREEN_H__8C8E06D6_4568_48B2_862F_4EDDBFB7C046__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "config.h"

#define SC_KANJI   0x0100
#define SC_KANJI2  0x0080
#define SC_COLOR   0x0e00
#define SC_BLINK   0x1000
#define SC_BOLD    0x2000
#define SC_ULINE   0x4000
#define SC_REVERSE 0x8000

struct Screen {
  HANDLE thread;
  CRITICAL_SECTION lock; //iy,ybase,selstart,selend,yscroll,copyに関するlock
  int ybase;
  int selstart, selend;
  int ymax;
  int yscroll;
  int copy[24];     //y==copy[y]なら移動なし、-1ならコピー元なし
  int *iy;          //iy[(y-ybase)%ymax]がバッファ内容 y=0:最下行
  WORD (*text)[82];
  long *dirty;      //dirty[iy[y]]&0xff ... dirty[iy[y]]>>8 が書き換え範囲
                    //lock不要にするために1変数
  int dirtyx;       //dirty開始位置 ... cxまでがdirty

  enum { STbase, STkanji, STesc, STcsi, STiac, STchar
  } state;
  int curx, cury;
  WORD* textp;
  int kanji1;
  int rolltop, rolltail;
  int csimode;
  int csiargc;
  int csiargv[16];
  WORD attr; //文字の色
  int iac, iacsn;
  BYTE iacs[32];
  int charset;

  WORD attrfg; //前景色+フラグ
  WORD attrbg; //背景色
  int sendset;	//0=ascii/1=jis-roman/2=半角かな/3=漢字
  int recvset;

  void print(const char*, int);
  void printk(const char*, int);
  void rollup(int, int);
  void rolldown(int, int);
  void enddirty();
  void textpupdate();
  void clearbefore();
  void clearafter();
  void nextline();

  volatile DWORD sock;
  volatile int sockmode; //0=接続中 1=接続 -2=エラー -1=終了
  int socktype; //1=TCPIP/2=排他制御socket(SSL)/3=IRDA/4=Serial
  int sockfreeze; //エラー時には1で送信を停止
  HANDLE socklock; //送受信lock、SSL時のみ使用
  int ssl; //1=ssl-telnet-connecting/2=ssl connected

  HANDLE printlock; //ローカルecho時のみ使用

  int rts, dtr;

  void init();
  PCWSTR allocate();
  PCWSTR open();
  static DWORD WINAPI screenwrap(void* parm);
  void main();
  void write(const char*p, int n);
  void writek(const char*p, int n); //漢字を意識して書き込み
  void close();
  void release();

  static int CALLBACK certwrap(DWORD, void*, DWORD, BLOB*, DWORD);
  int certhook(DWORD, DWORD, BLOB*, DWORD);
  int certreq;

  Config config;
  Config configback; //オリジナルの色を保持
  WCHAR error[_MAX_PATH]; //error string
};

#endif // !defined(AFX_SCREEN_H__8C8E06D6_4568_48B2_862F_4EDDBFB7C046__INCLUDED_)
