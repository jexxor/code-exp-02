#pragma once

#include "handle.h"

#include <cairo.h>
#include <glib-object.h>
#include <pango/pango.h>

template <class Object> using GObjectHandle = UniqueHandle<Object, g_object_unref>;
using GErrorHandle = UniqueHandle<GError, g_error_free>;
using GStringHandle = UniqueHandle<gchar, g_free>;

using LayoutHandle = GObjectHandle<PangoLayout>;
using AttributesHandle = UniqueHandle<PangoAttrList, pango_attr_list_unref>;
using FontHandle = UniqueHandle<PangoFontDescription, pango_font_description_free>;
using SurfaceHandle = UniqueHandle<cairo_surface_t, cairo_surface_destroy>;
using LayoutIterHandle = UniqueHandle<PangoLayoutIter, pango_layout_iter_free>;
using TabsHandle = UniqueHandle<PangoTabArray, pango_tab_array_free>;
using CairoHandle = UniqueHandle<cairo_t, cairo_destroy>;
using FontOptionsHandle = UniqueHandle<cairo_font_options_t, cairo_font_options_destroy>;

inline FontHandle font_description(const char* family, int pixels)
{
    FontHandle font{pango_font_description_from_string(family)};
    pango_font_description_set_absolute_size(font.get(), pixels * PANGO_SCALE);
    return font;
}
