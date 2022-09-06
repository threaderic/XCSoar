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

#include "Blackboard/BlackboardListener.hpp"
#include "Components.hpp"
#include "Engine/Task/Ordered/OrderedTask.hpp"
#include "Engine/Task/Ordered/Points/OrderedTaskPoint.hpp"
#include "Engine/Task/TaskManager.hpp"
#include "Engine/Task/Unordered/AlternateList.hpp"
#include "Input/InputEvents.hpp"
#include "Interface.hpp"
#include "Look/Look.hpp"
#include "Look/NavigatorLook.hpp"
#include "MainWindow.hpp"
#include "Screen/Layout.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "UIGlobals.hpp"
#include "UIUtil/GestureManager.hpp"
#include "Widget/ContainerWidget.hpp"
#include "Widget/WindowWidget.hpp"
#include "ui/window/AntiFlickerWindow.hpp"
/**
 * A Window which renders a Navigator
 */
class NavigatorWindow : public PaintWindow {

  const NavigatorLook &look;
  const TaskLook &look_task;
  const InfoBoxLook &look_infobox;

  const bool inverse;

  AttitudeState attitude;

  GestureManager gestures;
  bool dragging{false};
  bool ignore_single_click{false};

  PeriodClock mouse_down_clock;

public:
  /**
   * Constructor. Initializes most class members.
   */
  NavigatorWindow(const NavigatorLook &_look, const TaskLook &_look_task,
                  const InfoBoxLook &_look_infobox, const bool _inverse) noexcept;

  void ReadBlackboard(const AttitudeState _attitude) noexcept;

protected:
  /* virtual methods from AntiFlickerWindow */
  void OnPaint(Canvas &canvas) noexcept override;

private:
  void StopDragging();

public:
  bool OnGesture(const TCHAR *gesture);
  bool OnMouseDouble([[maybe_unused]] PixelPoint p) noexcept override;
  bool OnMouseDown(PixelPoint p) noexcept override;
  bool OnMouseUp([[maybe_unused]] PixelPoint p) noexcept override;
  bool OnMouseMove(PixelPoint p, [[maybe_unused]] unsigned keys) noexcept override;
  void OnCancelMode() noexcept override;
  bool OnKeyDown(unsigned key_code) noexcept override;
};


class NavigatorWidget final : public NullWidget, private NullBlackboardListener {

  // LiveBlackboard &blackboard;
  // const NavigatorLook &look;

  std::unique_ptr<NavigatorWindow> NavWindow;

protected:
  bool enable_auto_zoom = true;
  unsigned zoom = 2;

public:
  // NavigatorWidget() noexcept;

  ~NavigatorWidget() noexcept = default;

  /* virtual methods from class Widget */
  void Prepare(ContainerWindow &parent, const PixelRect &rc) noexcept override;
  void Show(const PixelRect &rc) noexcept override;
  void Hide() noexcept override;
  void Move(const PixelRect &rc) noexcept override;
  bool SetFocus() noexcept override;

  PixelSize GetMinimumSize() const noexcept override;

  NavigatorWindow* GetWindow() noexcept {
    return NavWindow.get();
  }
private:
  void Update(const MoreData &basic) noexcept;
  void UpdateLayout() noexcept;

  /* virtual methods from class BlackboardListener */
  void OnGPSUpdate(const MoreData &basic) noexcept override;
};
