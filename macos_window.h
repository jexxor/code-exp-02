#pragma once

#ifdef __APPLE__

#include <SDL3/SDL.h>

#include <deque>
#include <string>

using Window = SDL_Window*;
using Cursor = SDL_Cursor*;
using Atom = unsigned long;
using KeySym = unsigned long;
using Status = int;
using XIM = void*;
using XIC = void*;

constexpr int False = 0, True = 1, None = 0;
constexpr int Success = 0, GrabSuccess = 0, GrabModeAsync = 0;
constexpr int PropModeReplace = 0, AnyPropertyType = 0, XBufferOverflow = -1;
constexpr unsigned long CurrentTime = 0;
constexpr unsigned ShiftMask = 1, ControlMask = 4, Mod1Mask = 8;
constexpr unsigned long ExposureMask = 1, StructureNotifyMask = 2, KeyPressMask = 4;
constexpr unsigned long ButtonPressMask = 8, ButtonReleaseMask = 16;
constexpr unsigned long PointerMotionMask = 32, LeaveWindowMask = 64;
constexpr unsigned Button1 = 1, Button2 = 2, Button3 = 3, Button4 = 4, Button5 = 5;
constexpr int Expose = 1, ConfigureNotify = 2, SelectionNotify = 3, SelectionRequest = 4;
constexpr int ButtonPress = 5, MotionNotify = 6, ButtonRelease = 7, KeyPress = 8;
constexpr int LeaveNotify = 9, ClientMessage = 10, DestroyNotify = 11;
constexpr Atom XA_PRIMARY = 1, XA_STRING = 2, XA_ATOM = 3, XA_CARDINAL = 4;
constexpr unsigned XC_left_ptr = 1, XC_xterm = 2, XC_hand2 = 3, XC_fleur = 4;
constexpr long PPosition = 1, PSize = 2, PWinGravity = 4;
constexpr int StaticGravity = 1;
constexpr int XIMPreeditNothing = 1, XIMStatusNothing = 2;
inline constexpr const char* XNInputStyle = "inputStyle";
inline constexpr const char* XNClientWindow = "clientWindow";
inline constexpr const char* XNFocusWindow = "focusWindow";

constexpr KeySym XK_a = 'a', XK_c = 'c', XK_f = 'f', XK_i = 'i', XK_l = 'l';
constexpr KeySym XK_o = 'o', XK_v = 'v', XK_x = 'x';
constexpr KeySym XK_A = 'A', XK_C = 'C', XK_F = 'F', XK_I = 'I', XK_L = 'L';
constexpr KeySym XK_O = 'O', XK_V = 'V', XK_X = 'X';
constexpr KeySym XK_BackSpace = SDLK_BACKSPACE, XK_Delete = SDLK_DELETE;
constexpr KeySym XK_Left = SDLK_LEFT, XK_Right = SDLK_RIGHT;
constexpr KeySym XK_Up = SDLK_UP, XK_Down = SDLK_DOWN;
constexpr KeySym XK_Home = SDLK_HOME, XK_End = SDLK_END;
constexpr KeySym XK_Page_Up = SDLK_PAGEUP, XK_Page_Down = SDLK_PAGEDOWN;
constexpr KeySym XK_Escape = SDLK_ESCAPE, XK_Tab = SDLK_TAB;
constexpr KeySym XK_Return = SDLK_RETURN, XK_KP_Enter = SDLK_KP_ENTER;
constexpr KeySym XK_F1 = SDLK_F1;
constexpr KeySym XF86XK_Back = 0x1008ff26, XF86XK_Forward = 0x1008ff27;

struct XKeyEvent {
    KeySym keycode = 0;
    unsigned state = 0;
    unsigned long time = 0;
    char text[64]{};
};
struct XButtonEvent {
    unsigned button = 0, state = 0;
    int x = 0, y = 0;
    unsigned long time = 0;
};
struct XMotionEvent { int x = 0, y = 0; };
struct XConfigureEvent { int width = 0, height = 0; };
struct XExposeEvent { int count = 0; };
struct XClientEvent {
    Atom message_type = 0;
    int format = 0;
    struct { long l[5]{}; } data;
};
struct XSelectionEvent {
    int type = 0;
    unsigned long serial = 0;
    int send_event = 0;
    void* display = nullptr;
    Window requestor = nullptr;
    Atom selection = 0, target = 0, property = 0;
    unsigned long time = 0;
};
struct XSelectionRequestEvent {
    Window requestor = nullptr;
    Atom selection = 0, target = 0, property = 0;
    unsigned long time = 0;
};
struct MacEvent {
    int type = 0;
    XKeyEvent xkey;
    XButtonEvent xbutton;
    XMotionEvent xmotion;
    XMotionEvent xcrossing;
    XConfigureEvent xconfigure;
    XExposeEvent xexpose;
    XClientEvent xclient;
    XSelectionEvent xselection;
    XSelectionRequestEvent xselectionrequest;
};
using WindowEvent = MacEvent;
struct XSizeHints {
    long flags = 0;
    int win_gravity = 0, width = 0, height = 0, x = 0, y = 0;
};
struct Display {
    Window window = nullptr;
    std::deque<MacEvent> events;
    std::string property;
};

Display* XOpenDisplay(const char*);
void XCloseDisplay(Display*);
int DefaultScreen(Display*);
Window RootWindow(Display*, int);
int ConnectionNumber(Display*);
void XMapWindow(Display*, Window);
void XFlush(Display*);
void XResizeWindow(Display*, Window, int width, int height);
void XMoveResizeWindow(Display*, Window, int x, int y, int width, int height);
void XStoreName(Display*, Window, const char*);
void XChangeProperty(Display*, Window, Atom, Atom, int, int, const unsigned char*, int);
void XSelectInput(Display*, Window, unsigned long);
Cursor XCreateFontCursor(Display*, unsigned);
void XDefineCursor(Display*, Window, Cursor);
void XFreeCursor(Display*, Cursor);
Atom XInternAtom(Display*, const char*, int);
void XSetWMProtocols(Display*, Window, Atom*, int);
void XSetWMNormalHints(Display*, Window, XSizeHints*);
int XPending(Display*);
void XNextEvent(Display*, MacEvent*);
int XFilterEvent(MacEvent*, Window);
int XGrabPointer(Display*, Window, int, unsigned, int, int, Window, Cursor, unsigned long);
void XUngrabPointer(Display*, unsigned long);
void XSetSelectionOwner(Display*, Atom, Window, unsigned long);
void XConvertSelection(Display*, Atom, Atom, Atom, Window, unsigned long);
int XGetWindowProperty(Display*, Window, Atom, long, long, int, Atom, Atom*, int*,
                       unsigned long*, unsigned long*, unsigned char**);
int XFree(void*);
void XSendEvent(Display*, Window, int, long, MacEvent*);
int XTranslateCoordinates(Display*, Window, Window, int, int, int*, int*, Window*);
KeySym XLookupKeysym(XKeyEvent*, int);
int XLookupString(XKeyEvent*, char*, int, KeySym*, void*);
int Xutf8LookupString(XIC, XKeyEvent*, char*, int, KeySym*, Status*);
char* XSetLocaleModifiers(const char*);
XIM XOpenIM(Display*, void*, void*, void*);
XIC XCreateIC(XIM, ...);
void XSetICFocus(XIC);
void XDestroyIC(XIC);
void XCloseIM(XIM);

#endif
