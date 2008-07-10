//  Pingus - A free Lemmings clone
//  Copyright (C) 1999 Ingo Ruhnke <grumbel@gmx.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <iostream>
#include <algorithm>
#include "../math.hpp"
#include "../fonts.hpp"
#include "../string_util.hpp"
#include "../globals.hpp"
#include "../server.hpp"
#include "../game_session.hpp"
#include "../display/drawing_context.hpp"
#include "button_panel.hpp"

using namespace Actions;

ButtonPanel::ButtonPanel(GameSession* s, const Vector2i& pos)
  : RectComponent(Rect()),
    session(s),
    background("core/buttons/buttonbackground"),
    highlight("core/buttons/buttonbackgroundhl"),
    current_button(0)
{
  ActionHolder* aholder = session->get_server()->get_action_holder();

  std::vector<ActionName> actions = aholder->get_available_actions();

  set_rect(Rect(Vector2i(pos.x, pos.y - (actions.size() * 38)/2),
                Size(60, actions.size() * 38)));

  // Sort the action so that they always have the same order in the panel
  std::sort(actions.begin(), actions.end());

  for(std::vector<ActionName>::size_type i = 0; i < actions.size(); ++i)
    {
      ActionButton button;
      button.name   = actions[i];
      button.sprite = Sprite("pingus/player0/" + action_to_string(button.name) + "/right");
      button.sprite.set_play_loop(true);
      buttons.push_back(button);
    }
}

ButtonPanel::~ButtonPanel()
{
}

void
ButtonPanel::draw(DrawingContext& gc)
{
  ActionHolder* aholder = session->get_server()->get_action_holder();

  for(std::vector<ActionButton>::size_type i = 0; i < buttons.size(); ++i)
    {
      if (current_button == i)
        gc.draw(highlight, rect.left, rect.top + 38*i);
      else
        gc.draw(background, rect.left, rect.top + 38*i);

      gc.draw(buttons[i].sprite, rect.left + 20, rect.top + 38*i + 32);

      std::string str = StringUtil::to_string(aholder->get_available(buttons[i].name));
      gc.print_center(Fonts::pingus_small, rect.left + 46, rect.top + 5 + 38*i, str);
    }
}

void
ButtonPanel::update (float delta)
{
  for(std::vector<ActionButton>::size_type i = 0; i < buttons.size(); ++i)
    if (i == current_button)
      buttons[i].sprite.update(delta);
    else
      buttons[i].sprite.set_frame(0);
}

ActionName
ButtonPanel::get_action_name()
{
  return buttons[current_button].name;
}

void
ButtonPanel::set_button(int n)
{
  if (n >= 0 || n < static_cast<int>(buttons.size()))
    {
      current_button = n;
    }
  else
    {
      // FIXME: Play 'boing' sound here
    }
}

void
ButtonPanel::next_action()
{
  current_button = (current_button + 1) + int(buttons.size()) % int(buttons.size());
}

void
ButtonPanel::previous_action()
{
  current_button = (current_button - 1) + int(buttons.size()) % int(buttons.size());
}

void
ButtonPanel::on_primary_button_press(int x, int y)
{
  int action = (y - rect.top) / 38;
  current_button = Math::clamp(0, action, int(buttons.size()-1));
}

void
ButtonPanel::on_primary_button_release(int x, int y)
{
}

void
ButtonPanel::set_pos(const Vector2i& pos)
{
  set_rect(Rect(Vector2i(pos.x, pos.y - (buttons.size() * 38)/2),
                Size(60, buttons.size() * 38)));
}

/* EOF */
