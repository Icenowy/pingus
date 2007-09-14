//  $Id$
//
//  Pingus - A free Lemmings clone
//  Copyright (C) 2005 Ingo Ruhnke <grumbel@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "SDL_image.h"
#include <sstream>
#include <iostream>
#include "surface.hpp"

class SurfaceImpl
{
public:
  SurfaceImpl(SDL_Surface* surface = NULL) : surface(surface) {}
  ~SurfaceImpl() {
    SDL_FreeSurface(surface);
  }
  SDL_Surface* surface;
};

Surface::Surface()
{
}

Surface::Surface(const Pathname& pathname)
  : impl(new SurfaceImpl())
{
  impl->surface = IMG_Load(pathname.get_sys_path().c_str());
  if (!impl->surface)
    std::cout << "XXXXXX Failed to load: " << pathname.str() << std::endl;
  ///else
  //std::cout << "Loaded surface: " << name << ": " << surface->w << "x" << surface->h << std::endl;

}

Surface::Surface(int width, int height, SDL_Palette* palette, int colorkey)
  : impl(new SurfaceImpl())
{
  if (colorkey == -1)
    {
      impl->surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 8,
                                           0, 0, 0 ,0);
    }
  else
    {
      impl->surface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCCOLORKEY, width, height, 8,
                                           0, 0, 0 ,0);
      SDL_SetColorKey(impl->surface, SDL_SRCCOLORKEY, colorkey);
    }

  SDL_SetColors(impl->surface, palette->colors, 0, palette->ncolors);
}

Surface::Surface(int width, int height)
  : impl(new SurfaceImpl())
{
  impl->surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32,
                                       0x000000ff,
                                       0x0000ff00,
                                       0x00ff0000,
                                       0xff000000);
  //SDL_FillRect(surface, NULL, SDL_MapRGBA(surface->format, 0, 0, 0, 0));
}

Surface::Surface(SDL_Surface* surface)
  : impl(new SurfaceImpl(surface))
{
}

Surface::~Surface()
{
}

void
Surface::blit(const Surface& src, int x, int y)
{
  if (!get_surface())
    {
      std::cout << "Surface: Trying to blit to empty surface" << std::endl;
    }
  else if (!src.get_surface())
    {
      std::cout << "Surface: Trying to blit with an empty surface" << std::endl;
    }
  else
    {
      SDL_Rect dstrect;

      dstrect.x = x;
      dstrect.y = y;

      SDL_BlitSurface(src.get_surface(), NULL, get_surface(), &dstrect);
    }
}

void
Surface::lock()
{
  SDL_LockSurface(get_surface());
}

void
Surface::unlock()
{
  SDL_UnlockSurface(get_surface());
}

uint8_t*
Surface::get_data() const
{
  return static_cast<uint8_t*>(get_surface()->pixels);
}

int
Surface::get_width()  const
{
  if (get_surface())
    return get_surface()->w;
  else
    return 0;
}

int
Surface::get_height() const
{
  if (get_surface())
    return get_surface()->h;
  else
    return 0;
}

int
Surface::get_pitch() const
{
  if (get_surface())
    return get_surface()->pitch;
  else
    return 0;
}

SDL_Surface* 
Surface::get_surface() const
{
  return impl ? impl->surface : 0;
}

Surface::operator bool() const
{
  return impl ? impl->surface != NULL : false;
}

Color
Surface::get_pixel(int x, int y) const
{
  Uint8 *p = (Uint8 *)get_surface()->pixels + y * get_surface()->pitch + x * get_surface()->format->BytesPerPixel;
  Uint32 pixel;

  switch(get_surface()->format->BytesPerPixel)
    {
    case 1:
      pixel = *p;
    case 2: /* This will cause some problems ... */
      pixel = *(Uint16 *)p;
    case 3:
      if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
	pixel = p[0] << 16 | p[1] << 8 | p[2];
      else
	pixel = p[0] | p[1] << 8 | p[2] << 16;
    case 4:
      pixel = *(Uint32 *)p;
    default:
      pixel = 0;       /* shouldn't happen, but avoids warnings */
    } 

  Color color;
  SDL_GetRGBA(pixel, get_surface()->format, &color.r, &color.g, &color.b, &color.a);
  return color;
}

/* EOF */