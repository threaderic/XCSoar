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

#include "NavigatorRenderer.hpp"
#include "Formatter/Units.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "Look/Look.hpp"
#include "Renderer/TextRenderer.hpp"
#include "Task/TaskBehaviour.hpp"
#include "Units/System.hpp"
#include "Waypoint/Waypoint.hpp"
#include "WaypointIconRenderer.hpp"
#include "WaypointRenderer.hpp"
#include "time/RoughTime.hpp"
#include "ui/canvas/Canvas.hpp"
#include "Screen/Layout.hpp"
#include "UIGlobals.hpp"
#include "Look/TaskLook.hpp"
#include "Look/WaypointLook.hpp"
#include "Look/NavigatorLook.hpp"
#include "Look/IconLook.hpp"
#include "Look/MapLook.hpp"
#include "Look/TaskLook.hpp"
#include "Math/Constants.hpp"
#include "ProgressBarRenderer.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Engine/Task/TaskManager.hpp"
#include "Engine/Task/Ordered/OrderedTask.hpp"
#include "Engine/Task/Ordered/Points/OrderedTaskPoint.hpp"
#include "Components.hpp"
#include "Engine/Task/Unordered/AlternateList.hpp"
#include "Formatter/LocalTimeFormatter.hpp"
#include "Formatter/TimeFormatter.hpp"


// standard
#include <iostream>
#include <string>


void
NavigatorRenderer::DrawFrame(Canvas &canvas, const PixelRect &rc, const NavigatorLook &look) {
  const auto top_left = rc.GetTopLeft();
  const int rc_height = rc.GetHeight();
  const int rc_width = rc.GetWidth();
  
  BulkPixelPoint polyline_frame[11];
  polyline_frame[0].x = top_left.x + 5 / 20.0 * rc_height; //
  polyline_frame[0].y = top_left.y; //
  polyline_frame[1].x = top_left.x + rc_width - 4 / 20.0 * rc_height; //
  polyline_frame[1].y = polyline_frame[0].y; //
  polyline_frame[2].x = top_left.x + rc_width - 2 / 20.0 * rc_height; //
  polyline_frame[2].y = top_left.y + 1 / 20.0 *rc_height;; //
  polyline_frame[3].x = top_left.x + rc_width; //
  polyline_frame[3].y = top_left.y + 10 / 20.0 * rc_height; //
  polyline_frame[4].x = polyline_frame[2].x; //
  polyline_frame[4].y = top_left.y + 19 / 20.0 *rc_height; //
  polyline_frame[5].x = polyline_frame[1].x; //
  polyline_frame[5].y = top_left.y + rc_height; //
  polyline_frame[6].x = polyline_frame[0].x; //
  polyline_frame[6].y = polyline_frame[5].y; //
  polyline_frame[7].x = top_left.x + 2 / 20.0 * rc_height ; //
  polyline_frame[7].y = polyline_frame[4].y; //
  polyline_frame[8].x = top_left.x; //
  polyline_frame[8].y = polyline_frame[3].y;//
  polyline_frame[9].x = polyline_frame[7].x; //
  polyline_frame[9].y = polyline_frame[2].y; //
  polyline_frame[10].x = polyline_frame[0].x; //
  polyline_frame[10].y = polyline_frame[0].y; //
  
  canvas.Select(look.background_pen);
  canvas.Select(look.background_brush);
  canvas.DrawPolygon( polyline_frame, 11);

  canvas.Select(look.frame_pen);
  canvas.Select(look.frame_brush);
  canvas.DrawPolyline( polyline_frame, 11);
}


void 
NavigatorRenderer::DrawText(Canvas &canvas, const Waypoint &wp_current, 
                                const PixelRect &rc, const NavigatorLook &look) {

  const auto &basic = CommonInterface::Basic();
  const auto &calculated = CommonInterface::Calculated();
  TCHAR current_speed[256] = "---";

  if (basic.ground_speed_available)
    FormatSpeed(current_speed, Units::ToSysUnit(basic.ground_speed, Unit::METER_PER_SECOND),
              Unit::KILOMETER_PER_HOUR, true, true);
  
  auto current_speed_s = static_cast<std::string>(current_speed);
    
  TextRenderer text_renderer;
  text_renderer.SetVCenter(true);
  text_renderer.SetControl(true);
  
  canvas.Select(*look.font);

  bool has_started = calculated.ordered_task_stats.start.task_started;

  auto time_elapsed = TimeStamp{FloatDuration{calculated.ordered_task_stats.total.time_elapsed}};
  RoughTimeDelta r;
  auto time_elapsed_s = FormatLocalTimeHHMM(time_elapsed, r).c_str();
  has_started ? time_elapsed_s : time_elapsed_s = "---";

  auto time_start = calculated.ordered_task_stats.start.time;
  auto time_start_s = FormatLocalTimeHHMM(time_start,
                                  CommonInterface::GetComputerSettings().utc_offset).c_str();
  has_started ? time_start_s : time_start_s = "---";

  auto time_local = FormatLocalTimeHHMM(basic.time,
                                  CommonInterface::GetComputerSettings().utc_offset).c_str();
  auto waypoint_s = static_cast<std::string>(wp_current.name);

  auto time_planned = TimeStamp{FloatDuration{calculated.ordered_task_stats.total.time_planned}};
  auto time_planned_s = FormatLocalTimeHHMM(time_planned, r).c_str();
  has_started ? time_planned_s : time_planned_s = "---";

  auto arrival_planned = TimeStamp{FloatDuration{time_start.ToDuration() + time_planned.ToDuration()}};
  auto arrival_planned_s = FormatLocalTimeHHMM(arrival_planned, 
                                  CommonInterface::GetComputerSettings().utc_offset).c_str(); 
  has_started ? arrival_planned_s : arrival_planned_s = "---";

  auto caption = _T("--- Navigator not available: ongoing // current speed: ") \
            + current_speed_s \
            + " ---\n\t\t\t\t                                      " \
            + waypoint_s \
            +_T("\n\t\t\t\t\t") \
            + time_start_s \
            +_T("\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t      ") \
            + time_local \
            +_T(" (") \
            + time_elapsed_s \
            +_T(")\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t     ") \
            + arrival_planned_s \
            +_T(" (") \
            + time_planned_s\
            + _T(")");
          
  canvas.SetBackgroundTransparent();
  canvas.SetTextColor(COLOR_RED);
  text_renderer.Draw(canvas, rc, caption);
}

void 
NavigatorRenderer::DrawProgressTask(const TaskSummary& summary, Canvas &canvas, 
                                    const PixelRect &rc, const NavigatorLook &look, const TaskLook &look_task, bool inverse) 
{
  const int rc_height = rc.GetHeight();
  const int rc_width = rc.GetWidth();

  // render the progress bar
  PixelRect r{static_cast<int>(5 / 24.0 * rc_height),
                rc_height-static_cast<int>(2.5 / 24.0 * rc_height),
                rc_width-static_cast<int>(5 / 24.0 * rc_height), 
                rc_height-static_cast<int>(0.5/ 24.0 * rc_height)};
  
  bool task_has_started = CommonInterface::Calculated().task_stats.start.task_started;
  bool task_is_finished = CommonInterface::Calculated().task_stats.task_finished;

  unsigned int progression{};

  if(task_has_started && !task_is_finished)
    progression = 100*(1 - summary.p_remaining);
  else if(task_has_started && task_is_finished)
    progression = 100;
  else
   progression = 0;


  DrawSimpleProgressBar(canvas, r, progression, 0, 100);

  canvas.Select(look.frame_brush);

  // render the waypoints on the progress bar
  const Pen pen_f(Layout::ScalePenWidth(1), inverse ? COLOR_WHITE : COLOR_BLACK);
  const Pen pen_fi(Layout::ScalePenWidth(1), inverse ? COLOR_BLACK : COLOR_WHITE);
  canvas.Select(pen_f);

  bool target{true};
  unsigned i = 0;
  for (auto it = summary.pts.begin(); it != summary.pts.end(); ++it, ++i) {
    auto p = it->p;

    const PixelPoint position_waypoint(p * (rc_width - 10 / 24.0 * rc_height) + 5 / 24.0 * rc_height,
                        rc_height - static_cast<int>(1.5 / 24.0 * rc_height));

    int w = Layout::Scale(2);

    /* search for the next Waypoint to reach and draw two horizontal lines left and right
     * if one Waypoint has been missed, the two lines are also drawn */
    if (!it->achieved && target) {
      canvas.Select(pen_f);
      canvas.DrawLine(position_waypoint.At(-w, 0.5*w), 
                      position_waypoint.At(-2*w, 0.5*w));
      canvas.DrawLine(position_waypoint.At(w, 0.5*w), 
                      position_waypoint.At(2*w, 0.5*w));

      canvas.DrawLine(position_waypoint.At(-w, -0.5*w), 
                      position_waypoint.At(-2*w, -0.5*w));
      canvas.DrawLine(position_waypoint.At(w, -0.5*w), 
                      position_waypoint.At(2*w, -0.5*w));

      target = false;
    }

    if (i == summary.active) {
      // search for the Waypoint on which the user is looking for and draw two vertical lines left and right
      canvas.Select(pen_fi);
      canvas.DrawLine(position_waypoint.At(-1*w, w), 
                      position_waypoint.At(-1*w, -w));
      canvas.DrawLine(position_waypoint.At(1*w, w), 
                      position_waypoint.At(1*w, -w));
  
      canvas.Select(pen_f);
      canvas.DrawLine(position_waypoint.At(-2*w, w), 
                      position_waypoint.At(-2*w, -w));
      canvas.DrawLine(position_waypoint.At(2*w, w), 
                      position_waypoint.At(2*w, -w));

      if (it->achieved)
        canvas.Select(look_task.hbGreen);
      else
        canvas.Select(look_task.hbOrange);
      w = Layout::Scale(2);

    } else if (i < summary.active) {
      if (it->achieved)
        canvas.Select(look_task.hbGreen);
      else
        canvas.Select(look_task.hbNotReachableTerrain);
      w = Layout::Scale(2);

    } else {
      if (it->achieved)
        canvas.Select(look_task.hbGreen);
      else
        canvas.Select(look_task.hbLightGray);

      w = Layout::Scale(1);
    }

    canvas.DrawRectangle(PixelRect{position_waypoint}.WithMargin(w));

    }
  }

  void 
  NavigatorRenderer::DrawWaypointsIconsTitle(Canvas &canvas, const NavigatorLook &look) {
    
    const int rc_height = canvas.GetHeight();
    const int rc_width = canvas.GetWidth();

    const WaypointRendererSettings &waypoint_settings = CommonInterface::GetMapSettings().waypoint;
    const WaypointLook &waypoint_look = UIGlobals::GetMapLook().waypoint;
    
    WaypointIconRenderer waypoint_icon_renderer{waypoint_settings, waypoint_look, canvas};
    const PixelPoint position_waypoint_left{rc_width/20, rc_height*1/2};
    const PixelPoint position_waypoint_centered{rc_width/4, rc_height*1/2};
    const PixelPoint position_waypoint_right{rc_width*18/20, rc_height*1/2};
    
    WaypointPtr waypoint_before;
    WaypointPtr waypoint_current;

    unsigned task_size{};
    unsigned i{};
    
    if(protected_task_manager != nullptr) {
      ProtectedTaskManager::Lease lease(*protected_task_manager);

      const OrderedTask &task = lease->GetOrderedTask();

      task_size = task.TaskSize();
      waypoint_current = task.GetActiveTaskPoint()->GetWaypointPtr();
      i = task.GetActiveIndex();
      
      if(i == 0 || i == 1)
        waypoint_before = task.GetPoint(0).GetWaypointPtr();
      else
        waypoint_before = task.GetPoint(i-1).GetWaypointPtr();

      // std::cout << "\nhas started? " << task.TaskStarted() << "\nhas finished? " << task.GetStats().task_finished << std::endl;
    }
    
    // std::cout << "ainddex" << i << "yepyep " << waypoint_current->name << "  rtrtr :" << waypoint_current->elevation << std::endl;
    // std::cout << "cqscn " << waypoint_before->name << "  rtrtr :" << waypoint_before->elevation << std::endl;
    WaypointReachability wr = WaypointReachability::UNREACHABLE;

    if(protected_task_manager != nullptr && task_size > 3) {
      waypoint_icon_renderer.Draw(*waypoint_before, position_waypoint_left, wr , true);
      waypoint_icon_renderer.Draw(*waypoint_current, position_waypoint_centered, wr , true);
      waypoint_icon_renderer.Draw(*waypoint_current, position_waypoint_right, wr , true);
    }

    DrawText(canvas, *waypoint_current, canvas.GetRect(), look);

  }
