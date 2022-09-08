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

#include "Look/WaypointLook.hpp"
#include "Renderer/TextRenderer.hpp"
#include "Renderer/WaypointReachability.hpp"
#include "Renderer/WaypointRendererSettings.hpp"
#include "ui/dim/Point.hpp"
#include "Waypoint/Waypoint.hpp"
#include "util/StaticString.hxx"

struct PixelRect;
struct NavigatorLook;
struct AttitudeState;
class Canvas;
class TextRenderer;
class WaypointIconRenderer;

namespace NavigatorRenderer
{

  void Draw(Canvas &canvas, const PixelRect &rc,
            const NavigatorLook &look,
            const AttitudeState &attitude);

  void DrawText(Canvas &canvas, const PixelRect &rc,
            const NavigatorLook &look);

  // void DrawIcon(const WaypointRendererSettings &_settings,
  //                      const WaypointLook &_look,
  //                      Canvas &_canvas, bool _small_icons = false,
  //                      Angle _screen_rotation = Angle::Zero());
}