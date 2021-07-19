/*
 * main.c
 * Listener
 *
 * Created by Jeremy Rand on 2021-07-16.
 * Copyright (c) 2021 ___ORGANIZATIONNAME___. All rights reserved.
 *
 */


#pragma nda NDAOpen NDAClose NDAAction NDAInit 5 0x03FF "  Listener\\H**"


#include <orca.h>
#include <GSOS.h>
#include <QuickDraw.h>
#include <Window.h>
#include <Desk.h>
#include <Event.h>
#include <Resources.h>
#include <MiscTool.h>
#include <Memory.h>
#include <Loader.h>
#include <tcpip.h>

#include <string.h>

#include "main.h"


static BOOLEAN ndaActive = FALSE;
static GrafPortPtr winPtr = NULL;
static unsigned int userId;

static char line1[50];
static char line2[50];
static char line3[50];


void NDAClose(void)
{
    if (ndaActive) {
        CloseWindow(winPtr);
        winPtr = NULL;
        ndaActive = FALSE;
    }
    
    ResourceShutDown();
}


void NDAInit(int code)
{
    /* When code is 1, this is tool startup, otherwise tool
     * shutdown.
     */
    strcpy(line1, "Hello, world!");
    strcpy(line2, "Hello, world!");
    strcpy(line3, "Hello, world!");
    if (code) {
        ndaActive = FALSE;
        userId = MMStartUp();
    } else {
        if (ndaActive)
            NDAClose();
    }
}


#pragma databank 1
void DrawContents(void)
{
    Rect frame;
    GetPortRect(&frame);
    frame.v2 -= frame.v1;
    frame.h2 -= frame.h1;
    frame.h1 = 0;
    frame.v1 = 0;
    EraseRect(&frame);
    
    PenNormal();
    MoveTo(7,10);
    DrawCString(line1);
    MoveTo(7,20);
    DrawCString(line2);
    MoveTo(7,30);
    DrawCString(line3);
}
#pragma databank 0


GrafPortPtr NDAOpen(void)
{
    unsigned int oldResourceApp;
    LevelRecGS levelDCB;
    unsigned int oldLevel;
    SysPrefsRecGS prefsDCB;
    unsigned int oldPrefs;
    
    if (ndaActive)
        return NULL;
    
    levelDCB.pCount = 2;
    GetLevelGS(&levelDCB);
    oldLevel = levelDCB.level;
    levelDCB.level = 0;
    SetLevelGS(&levelDCB);
    
    prefsDCB.pCount = 1;
    GetSysPrefsGS(&prefsDCB);
    oldPrefs = prefsDCB.preferences;
    prefsDCB.preferences = (prefsDCB.preferences & 0x1fff) | 0x8000;
    SetSysPrefsGS(&prefsDCB);
    
    oldResourceApp = OpenResourceFileByID(readEnable, userId);
    
    winPtr = NewWindow2("\p Listener ", 0, DrawContents, NULL, 0x02, windowRes, rWindParam1);
    
    SetSysWindow(winPtr);
    ShowWindow(winPtr);
    SelectWindow(winPtr);
    
    ndaActive = TRUE;
    
    prefsDCB.preferences = oldPrefs;
    SetSysPrefsGS(&prefsDCB);
    
    levelDCB.level = oldLevel;
    SetLevelGS(&levelDCB);
    
    SetCurResourceApp(oldResourceApp);
    
    return winPtr;
}


void HandleRun(void)
{
    static BOOLEAN keySent = FALSE;
    
    if (winPtr == NULL)
        return;
    
    if (winPtr == FrontWindow())
        return;
    
    if (keySent)
        return;
    
    PostEvent(keyDownEvt, 0x00C0004A);
    keySent = TRUE;
}


void HandleControl(EventRecord *event)
{
}


void InvalidateWindow(void)
{
    Rect frame;
    SetPort(winPtr);
    GetPortRect(&frame);
    frame.v2 -= frame.v1;
    frame.h2 -= frame.h1;
    frame.h1 = 0;
    frame.v1 = 0;
    InvalRect(&frame);
}


void HandleKey(EventRecord *event)
{
    if (winPtr != NULL) {
        sprintf(line1, "what = $%X", event->what);
        sprintf(line2, "message = $%lX", event->message);
        sprintf(line3, "modifiers = $%X", event->modifiers);
        InvalidateWindow();
    }
}


void HandleCursor(void)
{
}


void HandleMenu(int menuItem)
{
}


BOOLEAN NDAAction(EventRecord *sysEvent, int code)
{
    static EventRecord localEvent;
    unsigned int eventCode;
    BOOLEAN result = FALSE;
    
    switch (code) {
        case runAction:
            HandleRun();
            break;
            
        case eventAction:
            BlockMove((Pointer)sysEvent, (Pointer)&localEvent, 16);
            localEvent.wmTaskMask = 0x001FFFFF;
            eventCode = TaskMasterDA(0, &localEvent);
            switch(eventCode) {
                case updateEvt:
                    BeginUpdate(winPtr);
                    DrawContents();
                    EndUpdate(winPtr);
                    break;
                    
                case wInControl:
                    HandleControl(&localEvent);
                    break;
                    
                case keyDownEvt:
                case autoKeyEvt:
                    HandleKey(&localEvent);
                    break;
            }
            break;
            
        case cursorAction:
            HandleCursor();
            break;
            
        case cutAction:
        case copyAction:
        case pasteAction:
        case clearAction:
            result = TRUE;
            HandleMenu(code);
            break;
    }
    
    return result;
}

