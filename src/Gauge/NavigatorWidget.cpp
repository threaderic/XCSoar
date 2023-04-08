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
#include "Components.hpp"
#include "Engine/Task/Ordered/OrderedTask.hpp"
#include "Engine/Task/Ordered/Points/OrderedTaskPoint.hpp"
#include "Engine/Task/TaskManager.hpp"
#include "Engine/Task/Unordered/AlternateList.hpp"
#include "InfoBoxes/InfoBoxWindow.hpp"
#include "Input/InputEvents.hpp"
#include "Interface.hpp"
#include "Look/Look.hpp"
#include "Renderer/NavigatorRenderer.hpp"
#include "Screen/Layout.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "UIGlobals.hpp"
#include "UIUtil/GestureManager.hpp"
#include "ui/canvas/Canvas.hpp"
#include "ui/window/AntiFlickerWindow.hpp"
#include "ui/window/DoubleBufferWindow.hpp"
#include "ui/window/PaintWindow.hpp"

#include <iostream>

/**
 * A Window which renders a Navigator
 */
class NavigatorWindow : public AntiFlickerWindow {
  const NavigatorLook &look;
  const TaskLook &look_task;
  const bool inverse;
  AttitudeState attitude;

  GestureManager gestures;
  bool dragging;

public:
  /**
   * Constructor. Initializes most class members.
   */
  NavigatorWindow(const NavigatorLook &_look, const TaskLook &_look_task,
                  const bool _inverse) noexcept
    : look(_look), look_task(_look_task), inverse(_inverse), dragging(false) {}

  void ReadBlackboard(const AttitudeState _attitude) noexcept {
    attitude = _attitude;
    Invalidate();
  }

protected:
  /* virtual methods from AntiFlickerWindow */
  void OnPaintBuffer(Canvas &canvas) noexcept override {
    if (inverse)
      canvas.Clear(COLOR_BLACK);
    else
      canvas.ClearWhite();

    WaypointPtr waypoint_before;
    WaypointPtr wp_current;

    unsigned task_size{};
    unsigned i{};

    if (protected_task_manager != nullptr) {
      ProtectedTaskManager::Lease lease(*protected_task_manager);

      const OrderedTask &task = lease->GetOrderedTask();

      task_size = task.TaskSize();
      wp_current = task.GetActiveTaskPoint()->GetWaypointPtr();
      i = task.GetActiveIndex();

      if (i == 0)
        waypoint_before = task.GetPoint(0).GetWaypointPtr();
      else
        waypoint_before = task.GetPoint(i - 1).GetWaypointPtr();

      // std::cout << "\nhas started? " << task.TaskStarted() << "\nhas
      // finished? " << task.GetStats().task_finished << std::endl;
    }

    const PixelRect frame_navigator =
      canvas.GetRect().WithPadding(Layout::Scale(1));

    const int fnw_height = canvas.GetHeight();
    const int fnw_width = canvas.GetWidth();
    const PixelRect frame_navigator_waypoint{
      {fnw_width * 18 / 100, fnw_height * 1 / 10},
      {fnw_width * 8 / 10, fnw_height * 5 / 10}};

    NavigatorRenderer::DrawFrame(canvas, frame_navigator, look);
    NavigatorRenderer::DrawFrame(canvas, frame_navigator_waypoint, look);
    NavigatorRenderer::DrawProgressTask(
      CommonInterface::Calculated().common_stats.ordered_summary, canvas,
      canvas.GetRect(), look, look_task, false);

    NavigatorRenderer::DrawText(
      canvas, *wp_current, canvas.GetRect(), look, inverse);

    bool task_valid =
      CommonInterface::Full().Calculated().ordered_task_stats.task_valid;
    if (task_valid)
      NavigatorRenderer::DrawWaypointsIconsTitle(
        canvas, waypoint_before, wp_current, task_size, look, inverse);
  }

private:
  void StopDragging() {
    if (!dragging)
      return;

    dragging = false;
    ReleaseCapture();
  }

protected:
  /** from class Window */
  // void OnCreate() override{

  // }
  // void OnDestroy() noexcept override{

  // }
  // void OnResize([[maybe_unused]] PixelSize new_size) noexcept override{

  // }
  // bool OnMouseWheel(PixelPoint p, int delta) noexcept override;

  bool OnGesture(const TCHAR *gesture) {
    {
      
      if (StringIsEqual(gesture, _T("U"))) {
        InputEvents::ShowMenu();
        return true;
      }
      if (StringIsEqual(gesture, _T("D"))) {
        InputEvents::ShowMenu();
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

      return InputEvents::processGesture(gesture);
    }
  }

  bool OnMouseDouble([[maybe_unused]] PixelPoint p) noexcept override {
    StopDragging();
    InputEvents::ShowMenu();
    return true;
  }

  bool OnMouseDown(PixelPoint p) noexcept override {
    std::cout << "on gesture" << std::endl;
    if (!dragging) {
      dragging = true;
      SetCapture();
      gestures.Start(p, Layout::Scale(1));
    }

    return true;
  }

  bool OnMouseUp([[maybe_unused]] PixelPoint p) noexcept override {
    if (dragging) {
      StopDragging();

      const TCHAR *gesture = gestures.Finish();
      if (gesture && OnGesture(gesture))
        return true;
    }

    return false;
  }

  bool OnMouseMove(PixelPoint p, [[maybe_unused]] unsigned keys) noexcept override {
    if (dragging)
      gestures.Update(p);

    return true;
  }

  void OnCancelMode() noexcept override {
#ifndef USE_WINUSER
    ReleaseCapture();
#endif
    StopDragging();
  }

  bool OnKeyDown(unsigned key_code) noexcept override {
    return InputEvents::processKey(key_code);
  }
};

void
NavigatorWidget::Update(const MoreData &basic) noexcept {
  NavigatorWindow &w = (NavigatorWindow &)GetWindow();
  w.ReadBlackboard(basic.attitude);
  w.Invalidate();
}

void
NavigatorWidget::Prepare(ContainerWindow &parent, const PixelRect &rc) noexcept {
  const Look &look = UIGlobals::GetLook();

  WindowStyle style;
  style.Hide();
  style.Disable();

  auto w = std::make_unique<NavigatorWindow>(
    look.navigator, look.map.task, look.info_box.inverse);
  w->Create(parent, rc, style);
  SetWindow(std::move(w));
}

void
NavigatorWidget::Show(const PixelRect &rc) noexcept {
  Update(CommonInterface::Basic());
  CommonInterface::GetLiveBlackboard().AddListener(*this);

  WindowWidget::Show(rc);
}

void
NavigatorWidget::Hide() noexcept {
  WindowWidget::Hide();

  CommonInterface::GetLiveBlackboard().RemoveListener(*this);
}

void
NavigatorWidget::OnGPSUpdate(const MoreData &basic) noexcept {
  Update(basic);
}
