/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2021 The XCSoar Project
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

#include "NavigatorLook.hpp"
#include "Screen/Layout.hpp"
#include "ui/canvas/Color.hpp"
#include "ui/canvas/Pen.hpp"

void
NavigatorLook::Initialise(bool _inverse, const Font &_font)
{
  font = &_font;

  Color pen_frame_color, brush_frame_color;

  if(!_inverse) {
    pen_frame_color = frame_color;
    brush_frame_color = background_color;
  }
  else {
    pen_frame_color = frame_color_inv;
    brush_frame_color = background_color_inv;
  }

  frame_brush.Create(pen_frame_color);
  frame_pen.Create(Layout::ScalePenWidth(1), pen_frame_color);
  
  background_brush.Create(brush_frame_color);
  background_pen.Create(Layout::ScalePenWidth(1), brush_frame_color);

  aircraft_pen.Create(Layout::Scale(2), COLOR_BLACK);

  sky_brush.Create(sky_color);
  sky_pen.Create(Layout::Scale(1), DarkColor(sky_color));

  terrain_brush.Create(terrain_color);
  terrain_pen.Create(Layout::Scale(1), COLOR_GRAY);
}
