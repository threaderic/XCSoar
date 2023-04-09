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

#include "NavigatorWidget.hpp"
#include "InfoBoxes/InfoBoxWindow.hpp"
#include "Input/InputEvents.hpp"
#include "Interface.hpp"
#include "Renderer/NavigatorRenderer.hpp"
#include "Task/Points/TaskWaypoint.hpp"
#include "Task/TaskType.hpp"
#include "Task/Unordered/GotoTask.hpp"
#include "ui/canvas/Canvas.hpp"
#include <iostream>

NavigatorWindow::NavigatorWindow(const NavigatorLook &_look,
                                 const TaskLook &_look_task, const bool _inverse) noexcept
  : look(_look), look_task(_look_task), inverse(_inverse), dragging(false) {}

void
NavigatorWindow::ReadBlackboard(const AttitudeState _attitude) noexcept {
  attitude = _attitude;
  Invalidate();
}

void
NavigatorWindow::OnPaint(Canvas &canvas) noexcept {
  if (inverse)
    canvas.Clear(COLOR_BLACK);
  else
    canvas.ClearWhite();

  TaskType tp{};
  WaypointPtr waypoint_before;
  WaypointPtr wp_current;

  // bool task_valid =
  //   CommonInterface::Full().Calculated().ordered_task_stats.task_valid;
  unsigned task_size{};
  unsigned i{};

  if (protected_task_manager != nullptr) {
    ProtectedTaskManager::Lease lease(*protected_task_manager);

    const OrderedTask &task = lease->GetOrderedTask();

    const auto &activeNextPoint = lease->GetActiveTaskPoint();

    task_size = task.TaskSize();
    tp = lease->GetMode();

    if (lease->IsMode(TaskType::ORDERED)) {
      i = task.GetActiveIndex();
      wp_current = task.GetActiveTaskPoint()->GetWaypointPtr();
      if (i == 0)
        waypoint_before = task.GetPoint(0).GetWaypointPtr();
      else
        waypoint_before = task.GetPoint(i - 1).GetWaypointPtr();
    } else if (lease->IsMode(TaskType::GOTO) || lease->IsMode(TaskType::ABORT)) {
      wp_current = activeNextPoint->GetWaypointPtr();
    } else if (lease->IsMode(TaskType::NONE)) {
      wp_current = nullptr;
    }


    // std::cout << "\nhas started? " << task.TaskStarted() << "\nhas
    // finished? " << task.GetStats().task_finished << std::endl;
  }

  const PixelRect frame_navigator = canvas.GetRect().WithPadding(Layout::Scale(1));

  const int fnw_height = canvas.GetHeight();
  const int fnw_width = canvas.GetWidth();
  const PixelRect frame_navigator_waypoint{
    {fnw_width * 18 / 100, fnw_height * 1 / 10},
    {fnw_width * 8 / 10, fnw_height * 5 / 10}};

  NavigatorRenderer::DrawFrame(canvas, frame_navigator, look);
  NavigatorRenderer::DrawFrame(canvas, frame_navigator_waypoint, look);

  if (tp == TaskType::ORDERED)
    NavigatorRenderer::DrawProgressTask(
      CommonInterface::Calculated().common_stats.ordered_summary, canvas,
      canvas.GetRect(), look, look_task, false);

  if (wp_current != nullptr)
    NavigatorRenderer::DrawText(
      canvas, tp, *wp_current, canvas.GetRect(), look, inverse);

  // if (task_valid)
  NavigatorRenderer::DrawWaypointsIconsTitle(
    canvas, waypoint_before, wp_current, task_size, look, inverse);
}

void
NavigatorWindow::StopDragging() {
  // std::cout << "on gesture stop dragging" << std::endl;
  if (!dragging)
    return;

  dragging = false;
  ReleaseCapture();
}


bool
NavigatorWindow::OnGesture(const TCHAR *gesture) {
  {

    if (StringIsEqual(gesture, _T("U"))) {
      InputEvents::ShowMenu();
      return true;
    }
    if (StringIsEqual(gesture, _T("D"))) {
      InputEvents::ShowMenu();
      return true;
    }
    if (StringIsEqual(gesture, _T("L"))) {
      // InputEvents::eventTaskTransition(const TCHAR *misc);
      InputEvents::eventAdjustWaypoint("previouswrap");
      // InputEvents::ShowMenu();
      return true;
    }
    if (StringIsEqual(gesture, _T("R"))) {
      InputEvents::eventAdjustWaypoint("nextwrap");
      // InputEvents::ShowMenu();
      return true;
    }
    if (StringIsEqual(gesture, _T("UD"))) {
      InputEvents::ShowMenu();
      return true;
    }
    if (StringIsEqual(gesture, _T("DR"))) {
      InputEvents::ShowMenu();
      return true;
    }
    if (StringIsEqual(gesture, _T("RL"))) {
      InputEvents::ShowMenu();
      return true;
    }
    if (gesture) {
      // std::cout << "on gesture mouse simple" << std::endl;
      InputEvents::ShowMenu();
      return true;
    }

    return true;
    // return InputEvents::processGesture(gesture);
  }
}

bool
NavigatorWindow::OnMouseDouble([[maybe_unused]] PixelPoint p) noexcept {
  // std::cout << "on gesture mouse double" << std::endl;
  StopDragging();
  ignore_single_click = true;
  InputEvents::ShowMenu();
  return true;
}

bool
NavigatorWindow::OnMouseDown(PixelPoint p) noexcept {
  // std::cout << "on gesture" << std::endl;
  // Ignore single click event if double click detected
  if (ignore_single_click)
    return true;

  mouse_down_clock.Update();

  if (!dragging) {
    dragging = true;
    SetCapture();
    gestures.Start(p, Layout::Scale(20));
  }

  return true;
}

bool
NavigatorWindow::OnMouseUp([[maybe_unused]] PixelPoint p) noexcept {
  // Ignore single click event if double click detected
  if (ignore_single_click) {
    ignore_single_click = false;
    return true;
  }

  const auto click_time = mouse_down_clock.Elapsed();
  mouse_down_clock.Reset();

  if (dragging) {
    StopDragging();

    const TCHAR *gesture = gestures.Finish();
    if (gesture && OnGesture(gesture))
      return true;

    if (click_time > std::chrono::milliseconds(400) &&
        click_time < std::chrono::milliseconds(1000))
      // on gesture mouse up == simpleClick
      // std::cout << "on current Task" << std::endl;
      InputEvents::eventAnalysis("AnalysisPage::TASK");

    else if (click_time > std::chrono::milliseconds(1000) &&
             click_time < std::chrono::milliseconds(3000))
      InputEvents::eventSetup("Task");

    else
      return false;
  }


  return false;
}

bool
NavigatorWindow::OnMouseMove(PixelPoint p, [[maybe_unused]] unsigned keys) noexcept {
  if (dragging)
    gestures.Update(p);

  return true;
}

void
NavigatorWindow::OnCancelMode() noexcept {
#ifndef USE_WINUSER
  ReleaseCapture();
#endif
  StopDragging();
}

bool
NavigatorWindow::OnKeyDown(unsigned key_code) noexcept {
  return InputEvents::processKey(key_code);
}
////// ------------------------------------------------------------------//////


void
NavigatorWidget::Update([[maybe_unused]] const MoreData &basic) noexcept {
  // NavigatorWindow &w = (NavigatorWindow &)GetWindow();

  NavWindow->ReadBlackboard(basic.attitude);
  NavWindow->Invalidate();
}

void
NavigatorWidget::Prepare(ContainerWindow &parent, const PixelRect &rc) noexcept {
  const Look &look = UIGlobals::GetLook();

  WindowStyle style;
  style.Hide();
  style.Disable();

  NavWindow = std::make_unique<NavigatorWindow>(
    look.navigator, look.map.task, look.info_box.inverse);
  NavWindow->Create(parent, rc, style);
  // SetWindow(std::move(NavWindow));
}

void
NavigatorWidget::Show([[maybe_unused]] const PixelRect &rc) noexcept {
  Update(CommonInterface::Basic());
  UpdateLayout();
  CommonInterface::GetLiveBlackboard().AddListener(*this);

  // WindowWidget::Show(rc);
  NavWindow->Show();
}

void
NavigatorWidget::Hide() noexcept {
  // WindowWidget::Hide();
  NavWindow->Hide();

  CommonInterface::GetLiveBlackboard().RemoveListener(*this);
}

void
NavigatorWidget::Move(const PixelRect &rc) noexcept {
  NavWindow->Move(rc);

  UpdateLayout();
}

bool
NavigatorWidget::SetFocus() noexcept {
  return false;
}

void
NavigatorWidget::OnGPSUpdate(const MoreData &basic) noexcept {
  Update(basic);
}

void
NavigatorWidget::UpdateLayout() noexcept {
  const PixelRect rc = NavWindow->GetClientRect();
  NavWindow->Move(rc);
}