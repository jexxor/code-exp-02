#include "macos_window.h"

#include <stdexcept>
#include <string>

namespace {
void expect(bool condition, const char* message)
{
    if (!condition)
        throw std::runtime_error(message);
}

WindowEvent take(Display* display, int type)
{
    while (XPending(display)) {
        WindowEvent event{};
        XNextEvent(display, &event);
        if (event.type == type)
            return event;
    }
    throw std::runtime_error("Expected window event was not delivered");
}
} // namespace

int main()
{
    expect(SDL_Init(SDL_INIT_VIDEO), "SDL video initialization failed");
    auto* window = SDL_CreateWindow("Input test", 120, 80, 0);
    expect(window, "SDL dummy window creation failed");
    auto* display = XOpenDisplay(nullptr);
    XMapWindow(display, window);
    while (XPending(display)) {
        WindowEvent discarded{};
        XNextEvent(display, &discarded);
    }

    SDL_Event native{};
    native.type = SDL_EVENT_KEY_DOWN;
    native.key.key = SDLK_A;
    native.key.mod = SDL_KMOD_GUI;
    expect(SDL_PushEvent(&native), "Could not queue a keyboard event");
    auto key = take(display, KeyPress);
    expect(XLookupKeysym(&key.xkey, 0) == XK_a && (key.xkey.state & ControlMask),
           "Command key must use the existing shortcut modifier");

    native = {};
    native.type = SDL_EVENT_TEXT_INPUT;
    native.text.text = "é";
    expect(SDL_PushEvent(&native), "Could not queue text input");
    key = take(display, KeyPress);
    char text[16]{};
    expect(XLookupString(&key.xkey, text, sizeof(text), nullptr, nullptr) == 2 &&
               std::string(text) == "é",
           "Committed UTF-8 text must reach the existing text input");

    native = {};
    native.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    native.button.button = SDL_BUTTON_RIGHT;
    native.button.x = 20;
    native.button.y = 30;
    expect(SDL_PushEvent(&native), "Could not queue a pointer event");
    const auto button = take(display, ButtonPress);
    expect(button.xbutton.button == Button3 && button.xbutton.x == 20 && button.xbutton.y == 30,
           "Right click must reach the canvas at the same coordinates");

    native = {};
    native.type = SDL_EVENT_MOUSE_WHEEL;
    native.wheel.y = 1;
    expect(SDL_PushEvent(&native), "Could not queue a scroll event");
    expect(take(display, ButtonPress).xbutton.button == Button4,
           "Upward scroll must use the existing zoom direction");

    expect(SDL_SetClipboardText("paste from macOS"), "Could not set the SDL clipboard");
    const auto property = XInternAtom(display, "SOURCE_CANVAS_PASTE", False);
    XConvertSelection(display, XInternAtom(display, "CLIPBOARD", False), XA_STRING, property,
                      window, CurrentTime);
    const auto selection = take(display, SelectionNotify);
    expect(selection.xselection.property == property, "Paste property was not delivered");
    Atom kind = None;
    int format = 0;
    unsigned long count = 0, remaining = 0;
    unsigned char* data = nullptr;
    expect(XGetWindowProperty(display, window, property, 0, 256, True, AnyPropertyType, &kind,
                              &format, &count, &remaining, &data) == Success &&
               format == 8 && std::string(reinterpret_cast<char*>(data), count) ==
                                  "paste from macOS",
           "Clipboard text must reach the application paste handler");
    XFree(data);

    XCloseDisplay(display);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
