/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2022 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#pragma once

#include "ui/canvas/Color.hpp"
#include "ui/canvas/Pen.hpp"
#include "ui/canvas/Brush.hpp"

class Font;

struct NavigatorLook {
  bool inverse;

  const Font *font;

  static constexpr Color background_color{COLOR_WHITE};
  static constexpr Color background_color_inv{COLOR_BLACK};
  static constexpr Color frame_color{COLOR_BLACK};
  static constexpr Color frame_color_inv{COLOR_WHITE};
  
  Pen frame_pen;
  Brush frame_brush;

  Pen background_pen;
  Brush background_brush;

  Pen aircraft_pen;

  static constexpr Color sky_color{0x0a, 0xb9, 0xf3};
  Brush sky_brush;
  Pen sky_pen;

  static constexpr Color terrain_color{0x80, 0x45, 0x15};
  Brush terrain_brush;
  Pen terrain_pen;

  void Initialise(bool _inverse, const Font &_font);
};
