//  Pingus - A free Lemmings clone
//  Copyright (C) 2000 Ingo Ruhnke <grumbel@gmx.de>
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

#include <assert.h>
#include <iostream>
#include "display/display.hpp"
#include "pingus/fonts.hpp"
#include "pingus/path_manager.hpp"
#include "pingus/stat_manager.hpp"
#include "pingus/savegame_manager.hpp"
#include "util/system.hpp"
#include "pingus/resource.hpp"
#include "pingus/globals.hpp"
#include "sound/sound.hpp"
#include "pingus/pingus_error.hpp"
#include "gettext.h"
#include "pingus/globals.hpp"
#include "util/sexpr_file_reader.hpp"
#include "pingus/debug.hpp"
#include "lisp/parser.hpp"
#include "lisp/lisp.hpp"
#include "worldmap/worldmap.hpp"
#include "worldmap/worldmap_story.hpp"
#include "worldmap/worldmap_screen.hpp"
#include "worldmap/pingus.hpp"
#include "worldmap/drawable_factory.hpp"
#include "worldmap/drawable.hpp"
#include "worldmap/dot.hpp"
#include "worldmap/level_dot.hpp"
#include "worldmap/path_graph.hpp"
#include "math/math.hpp"
#include "pingus/stat_manager.hpp"

#include "pingus/story_screen.hpp"
#include "screen/screen_manager.hpp"

namespace WorldmapNS {

Worldmap* Worldmap::current_ = 0; 

Worldmap::Worldmap(const std::string& arg_filename)
  : filename(arg_filename),
    mouse_x(0),
    mouse_y(0)
{
  current_ = this;

  worldmap = PingusWorldmap(Pathname(filename, Pathname::DATA_PATH));

  // Create all objects
  const std::vector<FileReader>& object_reader = worldmap.get_objects();
  for(std::vector<FileReader>::const_iterator i = object_reader.begin(); i != object_reader.end(); ++i)
    {
      Drawable* drawable = DrawableFactory::create(*i);
      if (drawable)
        {
          objects.push_back(drawable);
          drawables.push_back(drawable);
        }
      else
        {
          std::cout << "Worldmap::parse_objects: Parse Error" << std::endl;
        }
    }

  FileReader path_graph_reader = worldmap.get_graph();
  path_graph = new PathGraph(this, path_graph_reader);

  default_node = path_graph->lookup_node(worldmap.get_default_node());
  final_node   = path_graph->lookup_node(worldmap.get_final_node());

  pingus = new Pingus(path_graph);
  set_starting_node();
  add_drawable(pingus);

  gc_state.set_limit(Rect(Vector2i(0, 0), Size(worldmap.get_width(), worldmap.get_height())));
}

Worldmap::~Worldmap()
{
  for (DrawableLst::iterator i = drawables.begin (); i != drawables.end (); ++i)
      delete (*i);
  
  delete path_graph;
}

void
Worldmap::draw(DrawingContext& gc)
{
  Vector2i pingu_pos(static_cast<int>(pingus->get_pos().x), 
                     static_cast<int>(pingus->get_pos().y));
  int min, max;
  int width  = worldmap.get_width();
  int height = worldmap.get_height();

  if (width >= gc.get_width())
    {
      min = gc.get_width()/2;
      max = width - gc.get_width()/2;
    }
  else
    {
      min = width - gc.get_width()/2;
      max = gc.get_width()/2;
    }
  pingu_pos.x = Math::clamp(min, pingu_pos.x, max);

  if (height >= gc.get_height())
    {
      min = gc.get_height()/2;
      max = height - gc.get_height()/2;
    }
  else
    {
      min = height - gc.get_height()/2;
      max = gc.get_height()/2;
    }
  pingu_pos.y = Math::clamp(min, pingu_pos.y, max);

  gc_state.set_pos(Vector2i(pingu_pos.x, pingu_pos.y));
	
  gc_state.push(gc);
  
  
  for (DrawableLst::iterator i = drawables.begin (); i != drawables.end (); ++i)
    (*i)->draw(gc);

  Vector2f mpos = gc_state.screen2world(Vector2i(mouse_x, mouse_y));
  Dot* dot = path_graph->get_dot(mpos.x, mpos.y);
  if (dot)
    dot->draw_hover(gc);
  
  gc_state.pop(gc);
}

void
Worldmap::update(float delta)
{
  for (DrawableLst::iterator i = drawables.begin (); i != drawables.end (); ++i)
    {
      (*i)->update (delta);
    }
}

void
Worldmap::on_startup()
{
  Sound::PingusSound::play_music(worldmap.get_music());
  update_locked_nodes();
}

void
Worldmap::add_drawable(Drawable* drawable)
{
  drawables.push_back(drawable);
}

void
Worldmap::on_pointer_move(int x, int y)
{
  mouse_x = x;
  mouse_y = y;
}

void
Worldmap::on_primary_button_press(int x, int y)
{
  Vector2f click_pos = gc_state.screen2world(Vector2i(x, y));

  if (pingus_debug_flags & PINGUS_DEBUG_WORLDMAP)
    {
      std::cout
        << "\n<leveldot>\n"
        << "  <dot>\n"
        << "    <name>leveldot_X</name>\n"
        << "    <position>\n"
        << "      <x>" << (int)click_pos.x << "</x>\n"
        << "      <y>" << (int)click_pos.y << "</y>\n"
        << "      <z>0</z>\n"
        << "    </position>\n"
        << "  </dot>\n"
        << "  <levelname>level10.pingus</levelname>\n"
        << "</leveldot>\n" << std::endl;
    }

  Dot* dot = path_graph->get_dot(click_pos.x, click_pos.y);
  if (dot)
    {
      if (maintainer_mode)
        std::cout << "Worldmap: Clicked on: " << dot->get_name() << std::endl;

      if (path_graph->lookup_node(dot->get_name()) == pingus->get_node())
        {
          if (maintainer_mode)
            std::cout << "Worldmap: Pingu is on node, issue on_click()" << std::endl;
          dot->on_click();
        }
      else
        {
          if (dot->accessible())
            {
              if (!pingus->walk_to_node(path_graph->lookup_node(dot->get_name())))
                {
                  if (maintainer_mode)
                    std::cout << "Worldmap: NO PATH TO NODE FOUND!" << std::endl;
                }
              else
                {
                  StatManager::instance()->set_string(worldmap.get_short_name() + "-current-node", dot->get_name());
                }
            }
          else
            {
              Sound::PingusSound::play_sound("chink");
            }
        }
    }
}

void
Worldmap::on_secondary_button_press(int x, int y)
{
  if (maintainer_mode)
    {
      Vector3f click_pos = gc_state.screen2world(Vector2i(x, y));

      Dot* dot = path_graph->get_dot(click_pos.x, click_pos.y);
      if (dot)
        { // FIXME: Dot NodeID missmatch...
          NodeId id = path_graph->get_id(dot);
          pingus->set_position(id);
        }
    }
}

void
Worldmap::enter_level()
{
  NodeId node = get_pingus()->get_node();

  if (node != NoNode)
    {
      Dot* dot = path_graph->get_dot(node);
      if (dot)
        {
          dot->on_click();
        }
    }
  else
    {
      if (maintainer_mode)
        std::cout << "Worldmap: Pingus not on level" << std::endl;
    }
}

struct unlock_nodes
{
  PathGraph* path_graph;
  unlock_nodes(PathGraph* g)
  {
    path_graph = g;
  }

  void operator()(Node<Dot*>& node)
  {
    if (node.data->finished())
      {
        //std::cout << "Unlocking neightbours of: " << node.data << std::endl;
        for (std::vector<EdgeId>::iterator i = node.next.begin(); i != node.next.end(); ++i)
          {
            Edge<Path*>& edge = path_graph->graph.resolve_edge(*i);

            // FIXME: This should be identical to node.data->unlock(), but not sure
            path_graph->graph.resolve_node(edge.source).data->unlock();
            path_graph->graph.resolve_node(edge.destination).data->unlock();
          }
      }
  }
};

void
Worldmap::update_locked_nodes()
{
  // FIXME: This shouldn't be a polling function
  path_graph->graph.for_each_node(unlock_nodes(path_graph));

#if 0
  bool credits_unlocked = false;
  StatManager::instance()->get_bool(worldmap.get_short_name() + "-endstory-seen", credits_unlocked);

  if (!credits_unlocked)
    {
      // See if the last level is finished
      Dot* dot = path_graph->get_dot(final_node);
      if (dot)
        {
          if (dot->finished())
            {
              ScreenManager::instance()->push_screen(new StoryScreen(worldmap.get_end_story()));
            }
        }
      else
        {
          std::cout << "Error: Worldmap: Last level missing" << std::endl;
        }
    }
#endif
}

// Determine starting node
void
Worldmap::set_starting_node()
{
  // See if the user has played this map before.  If not, use the <default-node>
  // tag from the XML file.
  NodeId id;
  std::string node_name;

  if (StatManager::instance()->get_string(worldmap.get_short_name() + "-current-node", node_name))
    {
      // Just in case that level doesn't exist, look it up.
      id = path_graph->lookup_node(node_name);
      if (id == NoNode)
        id = default_node;
    }
  else
    id = default_node;
		
  pingus->set_position(id);

  LevelDot* leveldot = dynamic_cast<LevelDot*>(path_graph->get_dot(id));
  leveldot->unlock();
}

bool
Worldmap::is_final_map()
{
  return pingus->get_node() == final_node;
}

std::string
Worldmap::get_levelname()
{
  if (pingus->get_node() != NoNode)
    {
      LevelDot* leveldot = dynamic_cast<LevelDot*>(path_graph->get_dot(pingus->get_node()));
      if (leveldot)
        return _(leveldot->get_plf().get_levelname());
      else 
        return "---";
    }
  else
    {
      return _("...walking...");
    }
}

int
Worldmap::get_width()  const
{
  return worldmap.get_width();
}

int
Worldmap::get_height() const
{
  return worldmap.get_height();
}

} // namespace WorldmapNS

/* EOF */
