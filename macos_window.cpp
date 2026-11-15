#include "macos_window.h"

#ifdef __APPLE__

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace {
int pixels(Window window, float coordinate)
{
    return static_cast<int>(std::lround(coordinate * std::max(1.0f, SDL_GetWindowPixelDensity(window))));
}

unsigned modifiers(SDL_Keymod mod)
{
    return ((mod & SDL_KMOD_SHIFT) ? ShiftMask : 0) |
           ((mod & (SDL_KMOD_CTRL | SDL_KMOD_GUI)) ? ControlMask : 0) |
           ((mod & SDL_KMOD_ALT) ? Mod1Mask : 0);
}

void enqueue(Display* display, const SDL_Event& native)
{
    MacEvent event{};
    switch (native.type) {
    case SDL_EVENT_QUIT:
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        event.type = ClientMessage;
        event.xclient.message_type = XInternAtom(display, "WM_PROTOCOLS", False);
        event.xclient.format = 32;
        event.xclient.data.l[0] = static_cast<long>(XInternAtom(display, "WM_DELETE_WINDOW", False));
        break;
    case SDL_EVENT_WINDOW_EXPOSED:
        event.type = Expose;
        break;
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        event.type = ConfigureNotify;
        SDL_GetWindowSizeInPixels(display->window, &event.xconfigure.width,
                                  &event.xconfigure.height);
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        event.type = native.type == SDL_EVENT_MOUSE_BUTTON_DOWN ? ButtonPress : ButtonRelease;
        event.xbutton.button = native.button.button;
        event.xbutton.state = modifiers(SDL_GetModState());
        event.xbutton.x = pixels(display->window, native.button.x);
        event.xbutton.y = pixels(display->window, native.button.y);
        event.xbutton.time = static_cast<unsigned long>(native.button.timestamp / 1000000);
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        if (native.wheel.y == 0)
            return;
        event.type = ButtonPress;
        event.xbutton.button = (native.wheel.y * (native.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1 : 1)) > 0
                                   ? Button4 : Button5;
        event.xbutton.state = modifiers(SDL_GetModState());
        event.xbutton.x = pixels(display->window, native.wheel.mouse_x);
        event.xbutton.y = pixels(display->window, native.wheel.mouse_y);
        break;
    case SDL_EVENT_MOUSE_MOTION:
        event.type = MotionNotify;
        event.xmotion.x = pixels(display->window, native.motion.x);
        event.xmotion.y = pixels(display->window, native.motion.y);
        break;
    case SDL_EVENT_WINDOW_MOUSE_LEAVE: {
        event.type = LeaveNotify;
        float x = 0, y = 0;
        SDL_GetMouseState(&x, &y);
        event.xcrossing.x = pixels(display->window, x);
        event.xcrossing.y = pixels(display->window, y);
        break;
    }
    case SDL_EVENT_KEY_DOWN:
        event.type = KeyPress;
        event.xkey.keycode = native.key.key;
        event.xkey.state = modifiers(native.key.mod);
        event.xkey.time = static_cast<unsigned long>(native.key.timestamp / 1000000);
        break;
    case SDL_EVENT_TEXT_INPUT:
        event.type = KeyPress;
        event.xkey.state = modifiers(SDL_GetModState());
        std::strncpy(event.xkey.text, native.text.text, sizeof(event.xkey.text) - 1);
        break;
    default:
        return;
    }
    display->events.push_back(event);
}
} // namespace

Display* XOpenDisplay(const char*) { return new Display; }
void XCloseDisplay(Display* display) { delete display; }
int DefaultScreen(Display*) { return 0; }
Window RootWindow(Display*, int) { return nullptr; }
int ConnectionNumber(Display*) { return -1; }
void XMapWindow(Display* display, Window window)
{
    display->window = window;
    SDL_ShowWindow(window);
    SDL_StartTextInput(window);
}
void XFlush(Display*) {}
void XResizeWindow(Display*, Window window, int width, int height)
{
    const float density = std::max(1.0f, SDL_GetWindowPixelDensity(window));
    SDL_SetWindowSize(window, static_cast<int>(std::lround(width / density)),
                      static_cast<int>(std::lround(height / density)));
}
void XMoveResizeWindow(Display* display, Window window, int x, int y, int width, int height)
{
    SDL_SetWindowPosition(window, x, y);
    XResizeWindow(display, window, width, height);
}
void XStoreName(Display*, Window window, const char* title) { SDL_SetWindowTitle(window, title); }
void XChangeProperty(Display*, Window, Atom, Atom, int, int, const unsigned char*, int) {}
void XSelectInput(Display*, Window, unsigned long) {}
Cursor XCreateFontCursor(Display*, unsigned shape)
{
    auto kind = SDL_SYSTEM_CURSOR_DEFAULT;
    if (shape == XC_xterm)
        kind = SDL_SYSTEM_CURSOR_TEXT;
    else if (shape == XC_hand2)
        kind = SDL_SYSTEM_CURSOR_POINTER;
    else if (shape == XC_fleur)
        kind = SDL_SYSTEM_CURSOR_MOVE;
    return SDL_CreateSystemCursor(kind);
}
void XDefineCursor(Display*, Window, Cursor cursor) { SDL_SetCursor(cursor); }
void XFreeCursor(Display*, Cursor cursor) { SDL_DestroyCursor(cursor); }
Atom XInternAtom(Display*, const char* name, int)
{
    Atom hash = 14695981039346656037ull;
    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(name); *p; ++p)
        hash = (hash ^ *p) * 1099511628211ull;
    return hash;
}
void XSetWMProtocols(Display*, Window, Atom*, int) {}
void XSetWMNormalHints(Display*, Window, XSizeHints*) {}
int XPending(Display* display)
{
    SDL_Event native{};
    while (SDL_PollEvent(&native))
        enqueue(display, native);
    return static_cast<int>(display->events.size());
}
void XNextEvent(Display* display, MacEvent* event)
{
    *event = display->events.front();
    display->events.pop_front();
}
int XFilterEvent(MacEvent*, Window) { return False; }
int XGrabPointer(Display*, Window, int, unsigned, int, int, Window, Cursor, unsigned long)
{
    return SDL_CaptureMouse(true) ? GrabSuccess : 1;
}
void XUngrabPointer(Display*, unsigned long) { SDL_CaptureMouse(false); }
void XSetSelectionOwner(Display*, Atom, Window, unsigned long) {}
void XConvertSelection(Display* display, Atom selection, Atom target, Atom property, Window window,
                       unsigned long time)
{
    char* clipboard = SDL_GetClipboardText();
    display->property = clipboard ? clipboard : "";
    SDL_free(clipboard);
    MacEvent event{};
    event.type = SelectionNotify;
    event.xselection = {SelectionNotify, 0, True, display, window, selection, target, property, time};
    display->events.push_back(event);
}
int XGetWindowProperty(Display* display, Window, Atom, long, long, int, Atom, Atom* type,
                       int* format, unsigned long* count, unsigned long* remaining,
                       unsigned char** data)
{
    *type = XA_STRING;
    *format = 8;
    *count = display->property.size();
    *remaining = 0;
    *data = static_cast<unsigned char*>(std::malloc(*count + 1));
    if (!*data)
        return 1;
    std::memcpy(*data, display->property.c_str(), *count + 1);
    return Success;
}
int XFree(void* data) { std::free(data); return 0; }
void XSendEvent(Display*, Window, int, long, MacEvent*) {}
int XTranslateCoordinates(Display*, Window window, Window, int, int, int* x, int* y,
                          Window* child)
{
    SDL_GetWindowPosition(window, x, y);
    *child = nullptr;
    return True;
}
KeySym XLookupKeysym(XKeyEvent* event, int)
{
    return event->keycode;
}
int XLookupString(XKeyEvent* event, char* text, int capacity, KeySym* symbol, void*)
{
    if (symbol)
        *symbol = XLookupKeysym(event, 0);
    const auto size = std::min(static_cast<std::size_t>(capacity), std::strlen(event->text));
    std::memcpy(text, event->text, size);
    return static_cast<int>(size);
}
int Xutf8LookupString(XIC, XKeyEvent* event, char* text, int capacity, KeySym* symbol,
                      Status* status)
{
    const auto size = std::strlen(event->text);
    if (size > static_cast<std::size_t>(capacity)) {
        *status = XBufferOverflow;
        return static_cast<int>(size);
    }
    *status = 0;
    return XLookupString(event, text, capacity, symbol, nullptr);
}
char* XSetLocaleModifiers(const char* text) { return const_cast<char*>(text); }
XIM XOpenIM(Display*, void*, void*, void*) { return reinterpret_cast<XIM>(1); }
XIC XCreateIC(XIM, ...) { return reinterpret_cast<XIC>(1); }
void XSetICFocus(XIC) {}
void XDestroyIC(XIC) {}
void XCloseIM(XIM) {}

#endif
