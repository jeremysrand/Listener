/*
 * main.c
 * Listener
 *
 * Created by Jeremy Rand on 2021-07-16.
 * Copyright (c) 2021 ___ORGANIZATIONNAME___. All rights reserved.
 *
 */


#pragma nda NDAOpen NDAClose NDAAction NDAInit 2 0x03FF "  Listener\\H**"


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
#include <locator.h>
#include <tcpip.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"


// Defines

#define LISTEN_PORT 19026

#define LISTEN_STATE_MSG 1
#define LISTEN_TEXT_MSG 2
#define LISTEN_SEND_MORE 3

#define WINDOW_CHAR_WIDTH 50

// Typedefs

typedef enum tListenState {
    LISTEN_STATE_START = 0,
    LISTEN_STATE_NETWORK_UNCONNECTED,
    LISTEN_STATE_NETWORK_CONNECTED,
    LISTEN_STATE_AWAITING_CONNECTION,
    LISTEN_STATE_AWAITING_ESTABLISH,
    LISTEN_STATE_AWAITING_MSG_HEADER,
    LISTEN_STATE_AWAITING_TEXT,
    
    LISTEN_STATE_ERROR,
    
    NUM_LISTEN_STATES
} tListenState;

typedef struct tMessageHeader {
    uint16_t messageType;
    uint16_t messageArg;
} tMessageHeader;


typedef struct tTextList tTextList;

typedef struct tTextListHeader {
    struct tTextList * next;
    uint16_t size;
    uint16_t position;
} tTextListHeader;

typedef struct tTextList {
    tTextListHeader header;
    char text[1];
} tTextList;

typedef struct tGlobals {
    BOOLEAN ndaActive;
    BOOLEAN tcpipStarted;
    BOOLEAN networkConnected;
    BOOLEAN hasListenIpid;
    BOOLEAN hasConnIpid;
    GrafPortPtr winPtr;
    tListenState state;
    Word listenIpid;
    Word connIpid;
    Word position;
    tTextList * textListHead;
    tTextList * textListTail;
    tMessageHeader messageHeader;
    tTextList * textTransfer;
    char line1[WINDOW_CHAR_WIDTH];
    char line2[WINDOW_CHAR_WIDTH];
    char line3[WINDOW_CHAR_WIDTH];
} tGlobals;

typedef void (*tStateHandler)(void);


// Forward declarations

void handleStartState(void);
void handleNetworkUnconnectedState(void);
void handleNetworkConnectedState(void);
void handleAwaitingConnectionState(void);
void handleAwaitingEstablishState(void);
void handleAwaitingMsgHeaderState(void);
void handleAwaitingTextState(void);
void handleErrorState(void);


// Globals

static tGlobals * globals = NULL;
static unsigned int userId;
static tStateHandler stateHandlers[NUM_LISTEN_STATES] = {
    handleStartState,
    handleNetworkUnconnectedState,
    handleNetworkConnectedState,
    handleAwaitingConnectionState,
    handleAwaitingEstablishState,
    handleAwaitingMsgHeaderState,
    handleAwaitingTextState,
    
    handleErrorState,
};
static char * stateMessages[NUM_LISTEN_STATES] = {
    "Starting network tools",
    "Connecting to network",
    "Creating listen socket",
    "Waiting for connection",
    "Establishing connection",
    "Connected to device",
    "Receiving text",
    
    ""
};


// Implementation

void NDAClose(void)
{
    if ((globals != NULL) &&
        (globals->ndaActive)) {
        CloseWindow(globals->winPtr);
        globals->winPtr = NULL;
        globals->ndaActive = FALSE;
    }
    
    ResourceShutDown();
}


void freeGlobals(void)
{
    tTextList * textList = globals->textListHead;
    
    if (globals->textTransfer != NULL)
        free(globals->textTransfer);
    
    while (textList != NULL) {
        tTextList * prev = textList;
        textList = textList->header.next;
        free(prev);
    }
    
    free(globals);
    globals = NULL;
}


void teardownNetwork(void)
{
    if (globals->hasConnIpid) {
        TCPIPAbortTCP(globals->connIpid);
        TCPIPLogout(globals->connIpid);
    }
    globals->hasConnIpid = FALSE;
    
    if (globals->hasListenIpid) {
        TCPIPCloseTCP(globals->listenIpid);
        TCPIPLogout(globals->listenIpid);
    }
    globals->hasListenIpid = FALSE;
    
    if (globals->networkConnected)
        TCPIPDisconnect(TRUE, NULL);
    globals->networkConnected = FALSE;
    
    if (globals->tcpipStarted)
        TCPIPShutDown();
    globals->tcpipStarted = FALSE;
}


void NDAInit(int code)
{
    /* When code is 1, this is tool startup, otherwise tool
     * shutdown.
     */
    if (code) {
        userId = MMStartUp();
        globals = malloc(sizeof(*globals));
        memset(globals, 0, sizeof(*globals));
    } else {
        if ((globals != NULL) &&
            (globals->ndaActive)) {
            NDAClose();
            teardownNetwork();
            freeGlobals();
        }
    }
}


void InvalidateWindow(void)
{
    Rect frame;
    SetPort(globals->winPtr);
    GetPortRect(&frame);
    frame.v2 -= frame.v1;
    frame.h2 -= frame.h1;
    frame.h1 = 0;
    frame.v1 = 0;
    InvalRect(&frame);
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
    DrawCString(globals->line1);
    MoveTo(7,20);
    DrawCString(globals->line2);
    MoveTo(7,30);
    DrawCString(globals->line3);
}
#pragma databank 0


GrafPortPtr NDAOpen(void)
{
    unsigned int oldResourceApp;
    LevelRecGS levelDCB;
    unsigned int oldLevel;
    SysPrefsRecGS prefsDCB;
    unsigned int oldPrefs;
    
    if (globals->ndaActive)
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
    
    globals->winPtr = NewWindow2("\p Listener ", 0, DrawContents, NULL, 0x02, windowRes, rWindParam1);
    
    SetSysWindow(globals->winPtr);
    ShowWindow(globals->winPtr);
    SelectWindow(globals->winPtr);
    
    globals->ndaActive = TRUE;
    
    prefsDCB.preferences = oldPrefs;
    SetSysPrefsGS(&prefsDCB);
    
    levelDCB.level = oldLevel;
    SetLevelGS(&levelDCB);
    
    SetCurResourceApp(oldResourceApp);
    
    return globals->winPtr;
}


void enterErrorState(char * errorString, Word errorCode)
{
    strcpy(globals->line1, errorString);
    sprintf(globals->line2, "  Error code = $%04x", errorCode);
    InvalidateWindow();
    teardownNetwork();
    globals->state = LISTEN_STATE_ERROR;
}


void newState(tListenState state)
{
    strcpy(globals->line1, stateMessages[state]);
    InvalidateWindow();
    globals->state = state;
}


void handleStartState(void)
{
    LoadOneTool(54, 0x200);     // Load Marinetti
    if (toolerror()) {
        enterErrorState("Unable to load Marinetti", toolerror());
        return;
    }
    
    if (!TCPIPStatus()) {
        TCPIPStartUp();
        if (toolerror()) {
            enterErrorState("Unable to start Marinetti", toolerror());
            return;
        }
        globals->tcpipStarted = TRUE;
    }
    
    if (TCPIPGetConnectStatus()) {
        newState(LISTEN_STATE_NETWORK_CONNECTED);
    } else {
        newState(LISTEN_STATE_NETWORK_UNCONNECTED);
    }

}


void handleNetworkUnconnectedState(void)
{
    TCPIPPoll();
    TCPIPConnect(NULL);
    if ((!toolerror()) &&
        (TCPIPGetConnectStatus())) {
        globals->networkConnected = TRUE;
        newState(LISTEN_STATE_NETWORK_CONNECTED);
    } else {
        enterErrorState("Unable to connect to network", toolerror());
    }
}


void handleNetworkConnectedState(void)
{
    Word error;
    
    TCPIPPoll();
    globals->listenIpid = TCPIPLogin(userId, 0, 0, 0, 64);
    if (toolerror()) {
        enterErrorState("Unable to create socket", toolerror());
        return;
    }
    
    TCPIPSetSourcePort(globals->listenIpid, LISTEN_PORT);
    if (toolerror()) {
        enterErrorState("Unable to set port number", toolerror());
        return;
    }
    
    error = TCPIPListenTCP(globals->listenIpid);
    if (error != terrOK) {
        TCPIPLogout(globals->listenIpid);
        enterErrorState("Unable to listen on socket", error);
        return;
    }
    globals->hasListenIpid = TRUE;
    newState(LISTEN_STATE_AWAITING_CONNECTION);
}


void handleAwaitingConnectionState(void)
{
    TCPIPPoll();
    globals->connIpid = TCPIPAcceptTCP(globals->listenIpid, 0);
    switch (toolerror()) {
        case terrOK:
            globals->hasConnIpid = TRUE;
            newState(LISTEN_STATE_AWAITING_ESTABLISH);
            break;
            
        case terrNOINCOMING:
            break;
            
        default:
            enterErrorState("Unable to accept connection", toolerror());
            break;
    }
}


void handleAwaitingEstablishState(void)
{
    static srBuff srBuffer;
    
    TCPIPPoll();
    
    TCPIPStatusTCP(globals->connIpid, &srBuffer);
    if (toolerror()) {
        enterErrorState("Unable to get connection state", toolerror());
        return;
    }
    switch (srBuffer.srState) {
        case TCPSSYNSENT:
        case TCPSSYNRCVD:
            break;
            
        case TCPSESTABLISHED:
            newState(LISTEN_STATE_AWAITING_MSG_HEADER);
            globals->position = 0;
            break;
            
        default:
            enterErrorState("Unexpected TCP state", srBuffer.srState);
            break;
    }
}


void handleAwaitingMsgHeaderState(void)
{
    static srBuff srBuffer;
    static rrBuff readResponseBuf;
    tTextList * textTransfer;
    
    TCPIPPoll();
    
    TCPIPStatusTCP(globals->connIpid, &srBuffer);
    if (toolerror()) {
        enterErrorState("Unable to get connection state", toolerror());
        return;
    }
    if (srBuffer.srState != TCPSESTABLISHED) {
        TCPIPAbortTCP(globals->connIpid);
        TCPIPLogout(globals->connIpid);
        globals->hasConnIpid = FALSE;
        newState(LISTEN_STATE_AWAITING_CONNECTION);
        return;
    }
    Word error = TCPIPReadTCP(globals->connIpid, 0,
                              ((uint32_t)(&(globals->messageHeader))) + globals->position,
                              sizeof(globals->messageHeader) - globals->position,
                              &readResponseBuf);
    if (error != tcperrOK) {
        enterErrorState("Unable to read from connection", error);
        return;
    }
    
    globals->position += readResponseBuf.rrBuffCount;
    if (globals->position < sizeof(globals->messageHeader))
        return;
    
    switch (globals->messageHeader.messageType) {
        case LISTEN_STATE_MSG:
            if (globals->messageHeader.messageArg)
                strcpy(globals->line2, "  Listening...");
            else
                globals->line2[0] = '\0';
            InvalidateWindow();
            globals->position = 0;
            break;
        
        case LISTEN_TEXT_MSG:
            textTransfer = malloc(sizeof(tTextListHeader) + globals->messageHeader.messageArg);
            if (textTransfer == NULL) {
                enterErrorState("Unable to get memory for text", 0);
                return;
            }
            globals->textTransfer = textTransfer;
            textTransfer->header.next = NULL;
            textTransfer->header.size = globals->messageHeader.messageArg;
            textTransfer->header.position = 0;
            newState(LISTEN_STATE_AWAITING_TEXT);
            if (FrontWindow() == globals->winPtr)
                SendBehind((GrafPortPtr)toBottom, globals->winPtr);
            break;
            
        default:
            enterErrorState("Unexpected message type", globals->messageHeader.messageType);
    }
}

void handleAwaitingTextState(void)
{
    static srBuff srBuffer;
    static rrBuff readResponseBuf;
    tTextList * textTransfer = globals->textTransfer;
    
    TCPIPPoll();
    
    TCPIPStatusTCP(globals->connIpid, &srBuffer);
    if (toolerror()) {
        enterErrorState("Unable to get connection state", toolerror());
        return;
    }
    if (srBuffer.srState != TCPSESTABLISHED) {
        TCPIPAbortTCP(globals->connIpid);
        TCPIPLogout(globals->connIpid);
        globals->hasConnIpid = FALSE;
        newState(LISTEN_STATE_AWAITING_CONNECTION);
        return;
    }
    
    Word error = TCPIPReadTCP(globals->connIpid, 0,
                             (uint32_t)(&(textTransfer->text[textTransfer->header.position])),
                              textTransfer->header.size - textTransfer->header.position,
                              &readResponseBuf);
    if (error != tcperrOK) {
        enterErrorState("Unable to read text from connection", error);
        return;
    }
    
    textTransfer->header.position += readResponseBuf.rrBuffCount;
    if (textTransfer->header.position < textTransfer->header.size)
        return;
    
    if (globals->textListTail != NULL) {
        globals->textListTail->header.next = textTransfer;
    } else {
        globals->textListHead = textTransfer;
        strcpy(globals->line3, "    Typing...");
        InvalidateWindow();
    }
    
    textTransfer->header.position = 0;
    globals->textListTail = textTransfer;
    globals->textTransfer = NULL;
    newState(LISTEN_STATE_AWAITING_MSG_HEADER);
    globals->position = 0;
}

void handleErrorState(void)
{
    // Do nothing.  Once we have entered the error state, then nothing more to do.
}


void runStateMachine(void)
{
    stateHandlers[globals->state]();
}


void sendKey(void)
{
    tTextList * textList = globals->textListHead;
    
    PostEvent(keyDownEvt, 0x00C00000 | textList->text[textList->header.position]);
    textList->header.position++;
    if (textList->header.position < textList->header.size)
        return;
    
    if (textList == globals->textListTail) {
        globals->textListTail = NULL;
        globals->line3[0] = '\0';
        InvalidateWindow();
    }
    
    globals->textListHead = textList->header.next;
    free(textList);
    
    // If there is no more text to type, let the other end know we are ready for more.
    if ((globals->textListHead == NULL) &&
        ((globals->state == LISTEN_STATE_AWAITING_TEXT) ||
         (globals->state == LISTEN_STATE_AWAITING_MSG_HEADER))) {
        uint16_t msg = LISTEN_SEND_MORE;
        TCPIPWriteTCP(globals->connIpid, (Pointer)&msg, sizeof(msg), FALSE, FALSE);
    }
}


void HandleRun(void)
{
    if (globals == NULL)
        return;
    
    runStateMachine();
    if (globals->textListHead != NULL)
        sendKey();
}


void HandleControl(EventRecord *event)
{
}


void HandleKey(EventRecord *event)
{
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
                    BeginUpdate(globals->winPtr);
                    DrawContents();
                    EndUpdate(globals->winPtr);
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

