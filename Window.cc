//--------------------------------------------------------------------------
// Window.cpp - Implementation of simple X-Window control
//--------------------------------------------------------------------------
// Author: Lam H. Dao <daohailam(at)yahoo(dot)com>
//--------------------------------------------------------------------------
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//--------------------------------------------------------------------------
#include "Window.h"
//-----------------------------------------------------------------------------
extern Display *display;
//-----------------------------------------------------------------------------
extern int screen;
extern Visual *ScreenVisual;
extern Colormap ScreenColormap;
extern Window Root;
//----------------------------------------------------------------------------
WinControl::WinControl(int mask)
{
	parent = Root;
	handler = None;
	pctrl = NULL;
	attr.event_mask = mask;
	border = 0;
	x = y = 0;
}
//----------------------------------------------------------------------------
WinControl::WinControl(Window p, int mask, int b)
{
	parent = p;
	handler = None;
	pctrl = NULL;
	attr.event_mask = mask;
	border = b;
	x = y = 0;
}
//----------------------------------------------------------------------------
WinControl::~WinControl()
{
	XDestroyWindow(display, handler);
}
//----------------------------------------------------------------------------
void WinControl::SetEventMask(int mask)
{
	if (handler != None) return;
	attr.event_mask |= mask;
}
//----------------------------------------------------------------------------
void WinControl::ClearEventMask(int mask)
{
	if (handler != None) return;
	attr.event_mask &= ~mask;
}
//----------------------------------------------------------------------------
void WinControl::SetCursor(Cursor c)
{
	if (handler == None)
		attr.cursor = c;
	else
		XDefineCursor( display, handler, c );
}
//----------------------------------------------------------------------------
void WinControl::SetBorderWidth(int w)
{
	if (handler == None)
		border = w;
	else
		XSetWindowBorderWidth(display, handler, w);
}
//----------------------------------------------------------------------------
void WinControl::CreateHandler()
{
	CreateHandler(x, y, width, height);
}
//----------------------------------------------------------------------------
void WinControl::CreateHandler(int w, int h)
{
	CreateHandler(0, 0, w, h);
}
//----------------------------------------------------------------------------
void WinControl::CreateHandler(int x, int y, int w, int h)
{
	if (handler != None) return;

	int blackColor = BlackPixel(display, screen);
	int whiteColor = WhitePixel(display, screen);

	handler = XCreateSimpleWindow(display, Root,
				this->x=x, this->y=y, width=w, height=h,
				border, blackColor, blackColor);
    Atom delWindow = XInternAtom(display, "WM_DELETE_WINDOW", 0);
    XSetWMProtocols(display, handler, &delWindow, 1);
	XSelectInput(display, handler, attr.event_mask);
}
//----------------------------------------------------------------------------
void WinControl::Resize(int w, int h)
{
	if (handler == None) return;
	XResizeWindow(display, handler, width=w, height=h);
}
//----------------------------------------------------------------------------
void WinControl::MoveTo(int x, int y)
{
	this->x = x;
	this->y = y;
	if (handler != None)
		XMoveWindow(display, handler, x, y);
}
//----------------------------------------------------------------------------
void WinControl::Raise()
{
	if (handler != None) XRaiseWindow(display, handler);
}
//----------------------------------------------------------------------------
void WinControl::Lower()
{
	if (handler != None) XLowerWindow(display, handler);
}
//----------------------------------------------------------------------------
void WinControl::Show()
{
	XMapWindow(display, handler);
	SetVisibled(true);
}
//----------------------------------------------------------------------------
void WinControl::Hide()
{
	XUnmapWindow(display, handler);
	SetVisibled(false);
}
//----------------------------------------------------------------------------
void WinControl::ShowAt(int x, int y)
{
	XMoveWindow(display, handler, this->x=x, this->y=y);
	XMapWindow(display, handler);
}
//----------------------------------------------------------------------------
void WinControl::SetFocus()
{
	XSetInputFocus(display, handler, RevertToParent, CurrentTime);
}
//----------------------------------------------------------------------------
