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


#include "Engine/Waypoint/Ptr.hpp"
#include "Engine/Task/TaskType.hpp"
#include "Look/InfoBoxLook.hpp"

struct PixelRect;
struct PixelPoint;
struct NavigatorLook;
struct InfoBoxLook;
struct AttitudeState;
class Canvas;
class TextRenderer;
class WaypointIconRenderer;
struct TaskLook;
struct TaskSummary;


namespace NavigatorRenderer {
/**
* This function is used to create the frame of the Navigator and
* also the frame of the waypoint
*/
void
DrawFrame(Canvas &canvas, const PixelRect &rc, const NavigatorLook &look) noexcept;

/**
* Draw information texts about the current task (ordered task) or 
* the current target (unordered task) 
* e.g. waypoint distance, start time, planned duration time, ...
*/
void
DrawText(Canvas &canvas, TaskType tp, const Waypoint &wp_current, const PixelRect &rc,
         const NavigatorLook &look, const InfoBoxLook &iblook, bool inverse) noexcept;

/**
* Draw the progress of the current task with presntation of each taskpoint
*/
void
DrawProgressTask(const TaskSummary &summary, Canvas &canvas,
                 const PixelRect &rc, const NavigatorLook &look,
                 const TaskLook &look_task, bool inverse) noexcept;

/**
* Draw the icon of the current task and of the previous task
*/
void
DrawWaypointsIconsTitle(Canvas &canvas, const WaypointPtr waypoint_before,
                        const WaypointPtr waypoint_current, unsigned task_size,
                        const NavigatorLook &look, bool inverse) noexcept;
}