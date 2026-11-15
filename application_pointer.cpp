#include "application.h"

bool Application::over_text(Vec2 screen)
{
    const auto target = text_at(screen);
    bool inside = false;
    text_byte_at(target, screen, &inside);
    return inside;
}

void Application::update_cursor()
{
    Cursor cursor = normal_cursor;
    if (text_selection.dragging)
        cursor = text_cursor;
    else if (drag_button) {
        if (!window_manager.moving && (!chat || !chat->scrolling))
            cursor = drag_cursor;
    } else if ((chat && CanvasWindowManager::contains(chat->close_bounds, mouse_position)) ||
               std::ranges::any_of(chat_windows, [&](const auto& card) {
                   return CanvasWindowManager::contains(card->close_bounds, mouse_position);
               }) ||
               connection_marker_at(mouse_position)) {
        cursor = link_cursor;
    } else if (viewport().contains(mouse_position) && over_text(mouse_position)) {
        cursor = text_link_at(mouse_position).empty() ? text_cursor : link_cursor;
    }
    if (cursor != active_cursor) {
        XDefineCursor(display, window, cursor);
        active_cursor = cursor;
    }
}
