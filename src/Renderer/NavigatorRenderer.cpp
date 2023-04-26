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
#include "CompassRenderer.hpp"
#include "Components.hpp"
#include "Engine/GlideSolvers/GlideResult.hpp"
#include "Engine/GlideSolvers/GlideState.hpp"
#include "Engine/GlideSolvers/MacCready.hpp"
#include "Engine/Task/Ordered/OrderedTask.hpp"
#include "Engine/Task/Ordered/Points/OrderedTaskPoint.hpp"
#include "Engine/Task/TaskManager.hpp"
#include "Engine/Task/Unordered/AlternateList.hpp"
#include "Formatter/LocalTimeFormatter.hpp"
#include "Formatter/TimeFormatter.hpp"
#include "Formatter/Units.hpp"
#include "Formatter/UserUnits.hpp"
#include "Geo/GeoVector.hpp"
#include "InfoBoxes/InfoBoxLayout.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "Look/FontDescription.hpp"
#include "Look/IconLook.hpp"
#include "Look/InfoBoxLook.hpp"
#include "Look/Look.hpp"
#include "Look/MapLook.hpp"
#include "Look/NavigatorLook.hpp"
#include "Look/TaskLook.hpp"
#include "Look/WaypointLook.hpp"
#include "MapSettings.hpp"
#include "Math/Angle.hpp"
#include "Math/Constants.hpp"
#include "NextArrowRenderer.hpp"
#include "ProgressBarRenderer.hpp"
#include "Renderer/TextRenderer.hpp"
#include "Screen/Layout.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Task/TaskBehaviour.hpp"
#include "Task/TaskType.hpp"
#include "UIGlobals.hpp"
#include "UnitSymbolRenderer.hpp"
#include "Units/System.hpp"
#include "Units/Units.hpp"
#include "Version.hpp"
#include "Waypoint/Waypoint.hpp"
#include "WaypointIconRenderer.hpp"
#include "WaypointRenderer.hpp"
#include "WindArrowRenderer.hpp"
#include "tchar.h"
#include "time/RoughTime.hpp"
#include "time/Stamp.hpp"
#include "ui/canvas/Canvas.hpp"
#include "ui/canvas/Color.hpp"
#include "ui/canvas/Font.hpp"
#include "ui/dim/Point.hpp"
#include "ui/dim/Rect.hpp"
#include "ui/dim/Size.hpp"
#include "util/Macros.hpp"
#include "util/StaticString.hxx"
#include "util/StringBuffer.hxx"

// standard
#include <cmath>
#include <iostream>
#include <ostream>

void
NavigatorRenderer::DrawFrame(Canvas &canvas, const PixelRect &rc,
                             const NavigatorLook &look) noexcept {
  const auto top_left = rc.GetTopLeft();
  const int rc_height = rc.GetHeight();
  const int rc_width = rc.GetWidth();

  // the divisions below are not simplified deliberately to understand frame
  // construction
  const PixelPoint pt0 = {top_left.x + rc_height * 5 / 20, top_left.y};
  const PixelPoint pt1 = {top_left.x + rc_width - rc_height * 4 / 20, pt0.y};
  const PixelPoint pt2 = {
    top_left.x + rc_width - rc_height * 2 / 20, top_left.y + rc_height * 1 / 20};
  const PixelPoint pt3 = {top_left.x + rc_width, top_left.y + rc_height * 10 / 20};
  const PixelPoint pt4 = {pt2.x, top_left.y + rc_height * 19 / 20};
  const PixelPoint pt5 = {pt1.x, top_left.y + rc_height};
  const PixelPoint pt6 = {pt0.x, pt5.y};
  const PixelPoint pt7 = {top_left.x + rc_height * 2 / 20, pt4.y};
  const PixelPoint pt8 = {top_left.x, pt3.y};
  const PixelPoint pt9 = {pt7.x, pt2.y};

  const BulkPixelPoint polygone_frame[] = {
    pt0, pt1, pt2, pt3, pt4, pt5, pt6, pt7, pt8, pt9};

  // const BulkPixelPoint polygone_frame1[] = {
  // {top_left.x + rc_height * 5 / 20, top_left.y}, // pt0 = ..., ...
  // {top_left.x + rc_width - rc_height * 4 / 20, top_left.y}, // pt1 = ...,
  // pt0.y {top_left.x + rc_width - rc_height * 2 / 20, // pt2 = ..., ...
  //  top_left.y + rc_height * 1 / 20},
  // {top_left.x + rc_width, top_left.y + rc_height * 10 / 20}, // pt3 = ...,
  // ... {top_left.x + rc_width - rc_height * 2 / 20, // pt4 = pt2.x, ...
  //  top_left.y + rc_height * 19 / 20},
  // {top_left.x + rc_width - rc_height * 4 / 20, top_left.y + rc_height}, //
  // pt5 = pt1.X, ... {top_left.x + rc_height * 5 / 20, top_left.y +
  // rc_height},
  // // pt6 = pt0.x, pt5.y {top_left.x + rc_height * 2 / 20, top_left.y +
  // rc_height * 19 / 20},      // pt7 = ..., pt4.y {top_left.x, top_left.y +
  // rc_height * 10 / 20},                           // pt8 = ..., pt3.y
  // {top_left.x + rc_height * 2 / 20, top_left.y + rc_height * 1 / 20}}; //
  // pt9 = pt7.x, pt2.y                                                // pt9 =
  // pt7.x, pt2.y

  canvas.Select(look.frame_pen);
  canvas.Select(look.background_brush);
  canvas.DrawPolygon(polygone_frame, ARRAY_SIZE(polygone_frame));
}

void
NavigatorRenderer::DrawText(
  Canvas &canvas,
  TaskType tp,
  [[maybe_unused]] const Waypoint &wp_current,
  const PixelRect &rc,
  [[maybe_unused]] const NavigatorLook &look,
  [[maybe_unused]] const InfoBoxLook &iblook,
  bool inverse) noexcept {
  const auto &basic = CommonInterface::Basic();
  const auto &calculated = CommonInterface::Calculated();

  bool has_started = calculated.ordered_task_stats.start.task_started;

  const int rc_width = rc.GetWidth();
  const int rc_height = rc.GetHeight();

  // PREPARE TEXTS
  const RoughTimeDelta utc_offset1{
    CommonInterface::GetComputerSettings().utc_offset};
  const RoughTimeDelta utc_no_offset{};

  StaticString<8> time_elapsed_s;
  if (tp == TaskType::ORDERED && has_started && basic.time_available) {
    const auto time_elapsed = TimeStamp{
      FloatDuration{calculated.ordered_task_stats.total.time_elapsed}};
    const BasicStringBuffer<char, 8> time_elapsed_s_tmp =
      FormatLocalTimeHHMM(time_elapsed, utc_no_offset);
    time_elapsed_s.Format("%s", time_elapsed_s_tmp.c_str());
  } else
    time_elapsed_s.Format("%s", "--:--");
  // TCHAR time_elapsed_s[10];
  // const auto time_elapsed_s_tmp =
  //   FormatLocalTimeHHMM(time_elapsed, utc_offset1).c_str();
  // if (has_started)
  //   snprintf(time_elapsed_s, ARRAY_SIZE(time_elapsed_s), _T("%s"),
  //            time_elapsed_s_tmp);
  // else
  //   snprintf(time_elapsed_s, ARRAY_SIZE(time_elapsed_s), _T("%s"), "--:--");

  StaticString<8> time_start_s;
  const auto time_start = calculated.ordered_task_stats.start.time;
  if (tp == TaskType::ORDERED && has_started && basic.time_available)
    time_start_s.Format("%s", FormatLocalTimeHHMM(time_start, utc_offset1).c_str());
  else
    time_start_s.Format("%s", "--:--");
  // TCHAR time_start_s[10];
  // const auto time_start = calculated.ordered_task_stats.start.time;
  // if (has_started)
  //   snprintf(time_start_s, ARRAY_SIZE(time_start_s), _T("%s"),
  //            FormatLocalTimeHHMM(time_start, utc_offset1).c_str());
  // else
  //   snprintf(time_start_s, ARRAY_SIZE(time_start_s), _T("%s"), "--:--");

  StaticString<8> time_local_s;
  time_local_s.clear();
  if (basic.time_available) {
    const BasicStringBuffer<TCHAR, 8> time =
      FormatLocalTimeHHMM(basic.time, utc_offset1);
    time_local_s.AppendFormat("%s", time.c_str());
  } else {
    time_local_s.Format("%s", "--:--");
  }

  // TCHAR time_local_s[10];
  // snprintf(time_local_s, ARRAY_SIZE(time_local_s), _T("%s"),
  //          FormatLocalTimeHHMM(time, utc_offset1).c_str());

  StaticString<8> time_planned_s;
  TimeStamp time_planned{};
  if (tp == TaskType::ORDERED && has_started && basic.time_available) {
    time_planned = TimeStamp{
      FloatDuration{calculated.ordered_task_stats.total.time_planned}};
    time_planned_s.Format(
      "%s", FormatLocalTimeHHMM(time_planned, utc_no_offset).c_str());
  } else if (tp != TaskType::ORDERED && basic.time_available) {
    time_planned =
      TimeStamp{FloatDuration{calculated.task_stats.total.time_planned}};
    time_planned_s.Format(
      "%s", FormatLocalTimeHHMM(time_planned, utc_no_offset).c_str());
  } else
    time_planned_s.Format("%s", "--:--");
  // TCHAR time_planned_s[10];
  // const auto time_planned =
  //   TimeStamp{FloatDuration{calculated.ordered_task_stats.total.time_planned}};
  // if (has_started)
  //   snprintf(time_planned_s, ARRAY_SIZE(time_planned_s), _T("%s"),
  //            FormatLocalTimeHHMM(time_planned, utc_offset1).c_str());
  // else
  //   snprintf(time_planned_s, ARRAY_SIZE(time_planned_s), _T("%s"), "--:--");

  StaticString<8> arrival_planned_s;
  TimeStamp arrival_planned{};
  if ((tp == TaskType::ORDERED && has_started) && basic.time_available) {
    arrival_planned = TimeStamp{
      FloatDuration{time_start.ToDuration() + time_planned.ToDuration()}};
    arrival_planned_s.Format(
      "%s", FormatLocalTimeHHMM(arrival_planned, utc_offset1).c_str());
  } else if (tp != TaskType::ORDERED && basic.time_available) {
    arrival_planned = TimeStamp{
      FloatDuration{basic.time.ToDuration() + time_planned.ToDuration()}};
    arrival_planned_s.Format(
      "%s", FormatLocalTimeHHMM(arrival_planned, utc_offset1).c_str());
  } else
    arrival_planned_s.Format("%s", "--:--");
  // TCHAR arrival_planned_s[10];
  // const auto arrival_planned = TimeStamp{
  //   FloatDuration{time_start.ToDuration() + time_planned.ToDuration()}};
  // if (has_started)
  //   snprintf(arrival_planned_s, ARRAY_SIZE(arrival_planned_s), _T("%s"),
  //            FormatLocalTimeHHMM(arrival_planned, utc_offset1).c_str());
  // else
  //   snprintf(arrival_planned_s, ARRAY_SIZE(arrival_planned_s), _T("%s"), "--:--");

  // e_WP_Name
  StaticString<50> waypoint_name_s;
  waypoint_name_s.Format("%s", wp_current.name.c_str());
  // TCHAR waypoint_name_s[40];
  // snprintf(waypoint_name_s, ARRAY_SIZE(waypoint_name_s), _T("%s"), wp_current.name.c_str());

  // e_WP_Distance
  StaticString<20> waypoint_distance_s;
  auto precision_waypoint_distance{0};
  auto waypoint_distance{.0};
  if (tp == TaskType::ORDERED) {
    waypoint_distance =
      calculated.ordered_task_stats.current_leg.vector_remaining.distance;
  } else {
    waypoint_distance = calculated.task_stats.current_leg.vector_remaining.distance;
  }

  if (waypoint_distance < 5000.0)
    precision_waypoint_distance = 1;

  FormatUserDistance(waypoint_distance, waypoint_distance_s.data(), false,
                     precision_waypoint_distance);



  // TCHAR waypoint_distance_s[20];
  // auto precision_waypoint_distance{0};
  // const auto waypoint_distance{
  //   calculated.ordered_task_stats.current_leg.vector_remaining.distance};
  // if (waypoint_distance < 5000.0)
  //   precision_waypoint_distance = 1;
  // FormatUserDistance(
  //   waypoint_distance, waypoint_distance_s, true, precision_waypoint_distance);


  // e_WP_AltReq
  // TODO: or e_WP_H ?
  StaticString<20> waypoint_altitude_diff_s;
  auto waypoint_altitude_diff{.0};
  if (tp == TaskType::ORDERED) {
    waypoint_altitude_diff = calculated.ordered_task_stats.current_leg
                               .solution_remaining.GetRequiredAltitude();
  } else {
    waypoint_altitude_diff =
      calculated.task_stats.current_leg.solution_remaining.GetRequiredAltitude();
  }
  FormatAltitude(waypoint_altitude_diff_s.data(), waypoint_altitude_diff,
                 Units::GetUserAltitudeUnit(), false);
  // TCHAR waypoint_altitude_diff_s[20];
  // FormatAltitude(
  //   waypoint_altitude_diff_s,
  //   calculated.ordered_task_stats.current_leg.solution_remaining.GetRequiredAltitude(),
  //   Units::GetUserAltitudeUnit(), true);

  // e_SpeedTaskAvg
  StaticString<20> waypoint_average_speed_s;
  if (tp == TaskType::ORDERED && has_started) {
    FormatUserSpeed(calculated.task_stats.total.travelled.GetSpeed(),
                    waypoint_average_speed_s.data(), false, 0);
  } else {
    waypoint_average_speed_s.Format("%s", "---");
  }
  // TCHAR waypoint_average_speed_s[20];
  // FormatUserSpeed(calculated.task_stats.total.travelled.GetSpeed(),
  //                 waypoint_average_speed_s, true, 0);
  // if (!has_started) {
  //   snprintf(waypoint_average_speed_s, ARRAY_SIZE(waypoint_average_speed_s),
  //            _T("%s"), "---");
  // }

  // e_WP_GR
  StaticString<20> waypoint_GR_s;
  auto waypoint_GR{0};
  if (tp == TaskType::ORDERED) {
    waypoint_GR = std::round(calculated.ordered_task_stats.current_leg.gradient);
  } else {
    waypoint_GR = std::round(calculated.task_stats.current_leg.gradient);
  }
  waypoint_GR_s.Format("%d:1", waypoint_GR);
  // TCHAR waypoint_GR_s[20];
  // const int waypoint_GR =
  //   std::round(calculated.ordered_task_stats.current_leg.gradient);
  // _stprintf(waypoint_GR_s, _T("%d:1"), waypoint_GR);

  // e_Speed_GPS
  StaticString<20> current_speed_s;
  FormatUserSpeed(basic.ground_speed, current_speed_s.data(), false, 0);
  // TCHAR current_speed_s[20];
  // FormatUserSpeed(basic.ground_speed, current_speed_s, true, 0);

  // e_HeightGPS
  StaticString<20> current_altitude_s;
  FormatUserAltitude(basic.gps_altitude, current_altitude_s.data(), false);
  // TCHAR current_altitude_s[20];
  // FormatUserAltitude(basic.gps_altitude, current_altitude_s, true);

  // e_WP_BearingDiff
  StaticString<20> waypoint_direction_s;
  // TCHAR waypoint_direction_s[20];
  Angle bearing_diff{};

  if (!basic.track_available)
    bearing_diff.Zero();
  else if (tp == TaskType::ORDERED) {
    bearing_diff =
      calculated.ordered_task_stats.current_leg.vector_remaining.bearing -
      basic.track;
  } else {
    bearing_diff = calculated.task_stats.current_leg.vector_remaining.bearing -
      basic.track;
  }

  const int waypoint_direction = std::round(bearing_diff.AsDelta().Degrees());
  waypoint_direction_s.Format("%d°", waypoint_direction);
  // _stprintf(waypoint_direction_s, _T("< %d°"), waypoint_direction);

  StaticString<100> informations_next_waypoint1_s;
  informations_next_waypoint1_s.Format(
    "%s   %s  %s", waypoint_distance_s.c_str(),
    waypoint_altitude_diff_s.c_str(), waypoint_GR_s.c_str());

  StaticString<100> informations_next_waypoint1_unit_s;
  informations_next_waypoint1_unit_s.Format(
    "%s   %s", waypoint_distance_s.c_str(), waypoint_altitude_diff_s.c_str());

  // TCHAR informations_next_waypoint1_s[100];
  // if (canvas.GetWidth() < canvas.GetHeight() * 4)
  //   _stprintf(informations_next_waypoint1_s, _T("%s  %s"), waypoint_distance_s,
  //             waypoint_altitude_diff_s);
  // else
  //   _stprintf(informations_next_waypoint1_s, _T("%s  %s  %s"),
  //             waypoint_distance_s, waypoint_altitude_diff_s, waypoint_GR_s);

  StaticString<20> times_local_elapsed_s;
  times_local_elapsed_s.Format(
    "%s (%s)", time_local_s.c_str(), time_elapsed_s.c_str());
  // TCHAR times_local_elapsed_s[20];
  // snprintf(times_local_elapsed_s, ARRAY_SIZE(times_local_elapsed_s),
  //          _T("%s (%s)"), time_local_s, time_elapsed_s);

  StaticString<20> times_arrival_planned_s;
  if (canvas.GetWidth() > canvas.GetHeight() * 2.8)
    times_arrival_planned_s.Format(
      "%s (%s)", arrival_planned_s.c_str(), time_planned_s.c_str());
  else
    times_arrival_planned_s.Format("%s", arrival_planned_s.c_str());
  // TCHAR times_arrival_planned_s[50];
  // if (canvas.GetWidth() > canvas.GetHeight() * 5)
  //   _stprintf(times_arrival_planned_s, _T("%s (%s)"), arrival_planned_s,
  //             time_planned_s);
  // else
  //   _stprintf(times_arrival_planned_s, _T("%s"), arrival_planned_s);

  // ---- RENDER TEXTS
  // ---- Draw texts relative to current informations
  Font font;
  PixelSize text_size{};
  int text_height_position{};
  unsigned int font_height{};
  unsigned int unit_height{};
  unsigned int ascent_height{};
  PixelPoint unit_p{};
  Unit unit{Unit::KILOMETER};

  canvas.SetBackgroundTransparent();

  if (!inverse)
    canvas.SetTextColor(COLOR_BLACK);
  else
    canvas.SetTextColor(COLOR_WHITE);

  // -- Draw Waypoint informations: distance, altitude, glide ratio
  if (canvas.GetWidth() > canvas.GetHeight() * 3.7)
    font_height = rc_height * 35 / 200;
  else
    font_height = rc_height * 26 / 200;

  PixelPoint pixelpoint_informations_next_waypoint1{
    static_cast<int>(rc_width * 40 / 200), static_cast<int>(rc_height * 10 / 100)};

  font.Load(FontDescription(Layout::VptScale(font_height)));
  canvas.Select(font);
  canvas.DrawClippedText(
    pixelpoint_informations_next_waypoint1,
    {{0, 0}, {static_cast<int>(rc_width * 2 / 3), static_cast<int>(rc_height)}},
    informations_next_waypoint1_s);

  // Draw Waypoint units
  unit = Units::GetUserDistanceUnit();
  unit_height = static_cast<unsigned int>(font_height * 4 / 10);
  text_size = canvas.CalcTextSize(waypoint_distance_s.c_str());
  font.Load(FontDescription(Layout::VptScale(unit_height)));
  ascent_height = UnitSymbolRenderer::GetAscentHeight(font, unit);
  unit_p = pixelpoint_informations_next_waypoint1.At(
    text_size.width, text_size.height - ascent_height * 1.6);
  canvas.Select(font);
  UnitSymbolRenderer::Draw(canvas, unit_p, unit, iblook.unit_fraction_pen);

  unit = Units::GetUserAltitudeUnit();
  font.Load(FontDescription(Layout::VptScale(font_height)));
  text_size = canvas.CalcTextSize(informations_next_waypoint1_unit_s.c_str());
  font.Load(FontDescription(Layout::VptScale(unit_height)));
  ascent_height = UnitSymbolRenderer::GetAscentHeight(font, unit);
  unit_p = pixelpoint_informations_next_waypoint1.At(
    text_size.width, text_size.height - ascent_height * 1.6);
  canvas.Select(font);
  UnitSymbolRenderer::Draw(canvas, unit_p, unit, iblook.unit_fraction_pen);

  // -- Next waypoint's name
  if (canvas.GetWidth() > canvas.GetHeight() * 3.7)
    font_height = rc_height * 42 / 200;
  else
    font_height = rc_height * 30 / 200;

  font.Load(FontDescription(Layout::VptScale(font_height)));
  canvas.Select(font);
  canvas.DrawClippedText(
    {static_cast<int>(rc_width * 40 / 200), static_cast<int>(rc_height * 31 / 100)},
    {{0, 0}, {static_cast<int>(rc_width * 13 / 20), static_cast<int>(rc_height)}},
    waypoint_name_s);

  // ---- Current speed / Waypoint's direction / Current Altitude
  int placement_text_horizontal{};
  int text_width{};

  if (canvas.GetWidth() > canvas.GetHeight() * 3.7) {
    font_height = rc_height * 29 / 200;
    text_width = 30;
    placement_text_horizontal = rc_width * 90 / 100 - text_width;
  } else {
    font_height = rc_height * 22 / 200;
    text_width = 20;
    placement_text_horizontal = rc_width * 90 / 100 - text_width;
  }


  if (canvas.GetWidth() > canvas.GetHeight() * 3.1) {
    // -- Current speed
    font.Load(FontDescription(Layout::VptScale(font_height)));
    const PixelPoint pixelpoint_current_speed{
      placement_text_horizontal, static_cast<int>(rc_height * 8 / 100)};
    canvas.Select(font);
    canvas.DrawClippedText(
      pixelpoint_current_speed,
      {{0, 0}, {static_cast<int>(rc_width), static_cast<int>(rc_height)}},
      current_speed_s);

    // Draw speed units
    unit = Units::GetUserSpeedUnit();
    unit_height = static_cast<unsigned int>(font_height * 38 / 100);
    font.Load(FontDescription(Layout::VptScale(unit_height)));
    text_size = canvas.CalcTextSize(current_speed_s.c_str());
    ascent_height = UnitSymbolRenderer::GetAscentHeight(iblook.unit_font, unit);
    unit_p = pixelpoint_current_speed.At(text_size.width, text_size.height / 10);
    canvas.Select(font);
    UnitSymbolRenderer::Draw(canvas, unit_p, unit, iblook.unit_fraction_pen);

    // -- Waypoint's direction
    font.Load(FontDescription(Layout::VptScale(font_height)));
    canvas.Select(font);
    canvas.DrawClippedText(
      {placement_text_horizontal, static_cast<int>(rc_height * 25 / 100)},
      {{0, 0}, {static_cast<int>(rc_width), static_cast<int>(rc_height)}},
      waypoint_direction_s);

    // -- Current Altitude
    font.Load(FontDescription(Layout::VptScale(font_height)));
    const PixelPoint pixelpoint_altitude{
      placement_text_horizontal, static_cast<int>(rc_height * 43 / 100)};
    canvas.Select(font);
    canvas.DrawClippedText(
      pixelpoint_altitude,
      {{0, 0}, {static_cast<int>(rc_width), static_cast<int>(rc_height)}},
      current_altitude_s);

    // Draw Altitude unit
    unit = Units::GetUserAltitudeUnit();
    unit_height = static_cast<unsigned int>(font_height * 0.5);
    font.Load(FontDescription(Layout::VptScale(unit_height)));
    text_size = canvas.CalcTextSize(current_altitude_s.c_str());
    ascent_height = UnitSymbolRenderer::GetAscentHeight(iblook.unit_font, unit);
    unit_p = pixelpoint_altitude.At(text_size.width, text_size.height / 2.2);
    UnitSymbolRenderer::Draw(canvas, unit_p, unit, iblook.unit_fraction_pen);
  }

  // -- Task informations: average speed
  if (canvas.GetWidth() > canvas.GetHeight() * 3.4)
    font_height = rc_height * 35 / 200;
  else
    font_height = rc_height * 24 / 200;

  font.Load(FontDescription(Layout::VptScale(font_height)));
  text_size = canvas.CalcTextSize(waypoint_average_speed_s.c_str());
  PixelPoint pixelpoint_average_speed{static_cast<int>(rc_width * 5 / 200 + 6),
                                      static_cast<int>(rc_height * 8 / 100)};
  canvas.Select(font);
  canvas.DrawClippedText(
    pixelpoint_average_speed,
    {{0, 0}, {static_cast<int>(rc_width), static_cast<int>(rc_height)}},
    waypoint_average_speed_s);

  // Draw average speed unit
  unit = Units::GetUserTaskSpeedUnit();
  unit_height = static_cast<unsigned int>(font_height * 0.5);
  font.Load(FontDescription(Layout::VptScale(unit_height)));
  ascent_height = UnitSymbolRenderer::GetAscentHeight(font, unit);
  unit_p = pixelpoint_average_speed.At(text_size.width, text_size.height / 10);
  UnitSymbolRenderer::Draw(canvas, unit_p, unit, iblook.unit_fraction_pen);

  // ---- Draw texts relative to start / current / elapsed / planned times
  if (canvas.GetWidth() > canvas.GetHeight() * 3.6)
    font.Load(FontDescription(Layout::VptScale(rc_height * 32 / 200)));
  else
    font.Load(FontDescription(Layout::VptScale(rc_height * 24 / 200)));

  text_size = canvas.CalcTextSize(times_local_elapsed_s.c_str());
  text_height_position =
    static_cast<int>(rc_height * 180 / 200 - text_size.height);
  const PixelRect pixelrect_times{rc.BottomAligned(rc_width)};

  // -- Draw text start time
  const PixelPoint pixelpoint_time_start{
    static_cast<int>(rc_width * 5 / 200 + 8), text_height_position};
  canvas.DrawClippedText(pixelpoint_time_start, pixelrect_times, time_start_s);
  const PixelRect pixelrect_time_elapsed{rc.BottomAligned(rc_width)};

  // -- Draw text current time / elapsed time
  int text_size_tmp{};
  if (canvas.GetWidth() > canvas.GetHeight() * 2.8)
    text_size_tmp = text_size.width * 3 / 4;
  else
    text_size_tmp = text_size.width / 2;

  const PixelPoint pixelpoint_time_elapsed{
    static_cast<int>(rc_width * 100 / 200 - text_size_tmp), text_height_position};
  canvas.DrawClippedText(
    pixelpoint_time_elapsed, pixelrect_time_elapsed, times_local_elapsed_s);

  // -- Draw text planned time
  text_size = canvas.CalcTextSize(times_arrival_planned_s.c_str());
  const PixelRect pixelrect_arrival_planned{rc.BottomAligned(rc_width)};
  const PixelPoint pixelpoint_times_arrival_planned{
    static_cast<int>(rc_width - (rc_width * 5 / 200 + 8) - text_size.width),
    text_height_position};
  canvas.DrawClippedText(pixelpoint_times_arrival_planned,
                         pixelrect_arrival_planned, times_arrival_planned_s);

  // -- Draw direction arrow / North direction
  NextArrowRenderer next_arrow{UIGlobals::GetLook().wind_arrow_info_box};
  PixelRect pixelrect_next_arrow{
    {static_cast<int>(rc_width * 2 / 5), -static_cast<int>(rc_height / 4)},
    {static_cast<int>(rc_height), static_cast<int>(rc_height)}};
  pixelrect_next_arrow.Offset(rc_width / 5, rc_height * 1.3 / 10);

  canvas.DrawAnnulus(
    {static_cast<int>(rc_width * 3 / 5) + static_cast<int>(rc_height) / 2,
     static_cast<int>(rc_height * 75 / 200)},
    static_cast<int>(rc_height) * 18 / 100, static_cast<int>(rc_height) * 26 / 100,
    -basic.track + Angle::Degrees(50), -basic.track + Angle::Degrees(310));

  canvas.DrawAnnulus(
    {static_cast<int>(rc_width * 3 / 5) + static_cast<int>(rc_height) / 2,
     static_cast<int>(rc_height * 75 / 200)},
    static_cast<int>(rc_height) * 18 / 100, static_cast<int>(rc_height) * 26 / 100,
    -basic.track - Angle::Degrees(8), -basic.track + Angle::Degrees(8));

  next_arrow.DrawArrowScale(canvas, pixelrect_next_arrow, bearing_diff, 21);
}

void
NavigatorRenderer::DrawProgressTask(
  const TaskSummary &summary, Canvas &canvas, const PixelRect &rc,
  const NavigatorLook &look, const TaskLook &look_task, bool inverse) noexcept {
  const int rc_height = rc.GetHeight();
  const int rc_width = rc.GetWidth();

  // render the progress bar
  PixelRect r{rc_height * 5 / 24, rc_height - rc_height * 5 / 48,
              rc_width - rc_height * 10 / 48, rc_height - rc_height * 1 / 48};

  bool task_has_started =
    CommonInterface::Calculated().task_stats.start.task_started;
  bool task_is_finished = CommonInterface::Calculated().task_stats.task_finished;

  unsigned int progression{};

  if (task_has_started && !task_is_finished)
    progression = 100 * (1 - summary.p_remaining);
  else if (task_has_started && task_is_finished)
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

    const PixelPoint position_waypoint(
      p * (rc_width - 10 / 24.0 * rc_height) + 5 / 24.0 * rc_height,
      rc_height - static_cast<int>(1.5 / 24.0 * rc_height));

    int w = Layout::Scale(2);

    /* search for the next Waypoint to reach and draw two horizontal lines
     * left and right if one Waypoint has been missed, the two lines are also
     * drawn
     */
    if (!it->achieved && target) {
      canvas.Select(pen_f);
      canvas.DrawLine(position_waypoint.At(-w, 0.5 * w),
                      position_waypoint.At(-2 * w, 0.5 * w));
      canvas.DrawLine(position_waypoint.At(w, 0.5 * w),
                      position_waypoint.At(2 * w, 0.5 * w));

      canvas.DrawLine(position_waypoint.At(-w, -0.5 * w),
                      position_waypoint.At(-2 * w, -0.5 * w));
      canvas.DrawLine(position_waypoint.At(w, -0.5 * w),
                      position_waypoint.At(2 * w, -0.5 * w));

      target = false;
    }

    if (i == summary.active) {
      // search for the Waypoint on which the user is looking for and draw
      // two vertical lines left and right
      canvas.Select(pen_fi);
      canvas.DrawLine(position_waypoint.At(-1.1 * w, w),
                      position_waypoint.At(-1.1 * w, -w));
      canvas.DrawLine(position_waypoint.At(1.2 * w, w),
                      position_waypoint.At(1.2 * w, -w));

      canvas.Select(pen_f);
      canvas.DrawLine(position_waypoint.At(-2 * w, w),
                      position_waypoint.At(-2 * w, -w));
      canvas.DrawLine(position_waypoint.At(2 * w, w),
                      position_waypoint.At(2 * w, -w));

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
NavigatorRenderer::DrawWaypointsIconsTitle(
  Canvas &canvas, WaypointPtr waypoint_before, WaypointPtr waypoint_current,
  unsigned task_size, [[maybe_unused]] const NavigatorLook &look,
  [[maybe_unused]] bool inverse) noexcept {

  const int rc_height = canvas.GetHeight();
  const int rc_width = canvas.GetWidth();

  const WaypointRendererSettings &waypoint_settings =
    CommonInterface::GetMapSettings().waypoint;
  const WaypointLook &waypoint_look = UIGlobals::GetMapLook().waypoint;

  WaypointIconRenderer waypoint_icon_renderer{
    waypoint_settings, waypoint_look, canvas};
  const PixelPoint position_waypoint_left{rc_width * 7 / 200, rc_height * 1 / 2};
  // const PixelPoint position_waypoint_centered{rc_width*42/200,
  // rc_height*1/2};
  const PixelPoint position_waypoint_right{rc_width * 190 / 200, rc_height * 1 / 3};

  // CALCULATE REACHABILITY
  WaypointReachability wr_before{WaypointReachability::UNREACHABLE};
  WaypointReachability wr_current{WaypointReachability::UNREACHABLE};


  ///////////
  /// \TODO: transfert costly calculation to CommonInterface::Calculated()?
  // // WaypointPtr waypoint_before;
  // // WaypointPtr waypoint_current;

  // unsigned task_size{};
  // unsigned i{};

  // if (protected_task_manager != nullptr) {
  //   ProtectedTaskManager::Lease lease(*protected_task_manager);

  //   const OrderedTask &task = lease->GetOrderedTask();

  //   task_size = task.TaskSize();
  //   waypoint_current = task.GetActiveTaskPoint()->GetWaypointPtr();
  //   i = task.GetActiveIndex();

  //   if (i == 0)
  //     waypoint_before = task.GetPoint(0).GetWaypointPtr();
  //   else
  //     waypoint_before = task.GetPoint(i - 1).GetWaypointPtr();

  //   // std::cout << "\nhas started? " << task.TaskStarted() << "\nhas
  //   // finished? " << task.GetStats().task_finished << std::endl;
  // }
  //
  //
  //   const auto &basic = CommonInterface::Basic();
  //   const ComputerSettings &settings_computer =
  //     CommonInterface::GetComputerSettings();
  //   const TaskBehaviour &task_behaviour = settings_computer.task;
  //   const auto &calculated = CommonInterface::Calculated();
  //   const GlidePolar &glide_polar =
  //     CommonInterface::GetComputerSettings().polar.glide_polar_task;
  //   const MacCready mac_cready(task_behaviour.glide, glide_polar);

  //   for (auto wp : {waypoint_before, waypoint_current}) {
  //     const auto elevation_wp_before = wp->elevation +
  //       task_behaviour.safety_height_arrival;
  //     const GlideState state_wp_before{
  //       GeoVector(basic.location, wp->location), elevation_wp_before,
  //       basic.nav_altitude, calculated.GetWindOrZero()};
  //     const GlideResult result_wp_before = mac_cready.SolveStraight(state_wp_before);

  //     if (result_wp_before.pure_glide_altitude_difference > 0) {
  //       if (wp == waypoint_before) {
  //         wr_before = WaypointReachability::TERRAIN;
  //       } else if (wp == waypoint_current) {
  //         wr_current = WaypointReachability::TERRAIN;
  //       }
  //     } else {
  //       if (wp == waypoint_before) {
  //         wr_before = WaypointReachability::UNREACHABLE;
  //       } else if (wp == waypoint_current) {
  //         wr_current = WaypointReachability::UNREACHABLE;
  //       }
  //     }
  //   }

  //   // std::cout << "ainddex" << i << "yepyep " << waypoint_current->name << "
  //   // rtrtr :" << waypoint_current->elevation << std::endl; std::cout << "cqscn
  //   // "
  //   // << waypoint_before->name << "  rtrtr :" << waypoint_before->elevation <<
  //   // std::endl; WaypointReachability wr = WaypointReachability::UNREACHABLE;
  /////////////////////////////////

  if (protected_task_manager != nullptr && task_size > 1) {
    if (waypoint_before != nullptr)
      waypoint_icon_renderer.Draw(
        *waypoint_before, position_waypoint_left, wr_before, true);
    // waypoint_icon_renderer.Draw(*waypoint_current,
    // position_waypoint_centered, wr , true);
    if (waypoint_current != nullptr)
      waypoint_icon_renderer.Draw(
        *waypoint_current, position_waypoint_right, wr_current, true);
  }
}
