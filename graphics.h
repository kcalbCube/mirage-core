#pragma once
#include <cstdint>
#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/serialization/version.hpp>
#include "boost/optional/optional.hpp"
#include "boost/variant/variant.hpp"
#include "utility.h"
#include <entt/entt.hpp>
#include <SDL.h>
namespace mirage::graphics
{
	using Color = SDL_Color;
	using Icon  = entt::id_type;

	struct SurfaceWrapper
	{
		SDL_Surface* surface;
	};

	struct IconResource
	{
		SDL_Surface* surface{nullptr};
		float scale = 1.f;

		void save(auto& ar, const unsigned)
		{
			ar & SurfaceWrapper{surface};
			ar & scale;
		}

		void load(auto& ar, const unsigned version)
		{
			SurfaceWrapper sw{};
			ar & sw;
			surface = sw.surface;
			ar & scale;
		}
	};
	
	using Transform = utils::Vec3<float>;
	using Scale 	= float;
	using Filter 	= boost::variant<Color, Transform, Scale>;
	
	struct Vertice
	{
		enum : uint32_t
		{
			clickable = 1,
			transparent = 2, // bypasses clicks
		};

		uint16_t // (0..1) * UINT16_MAX
			x = 0,
			y = 0;

		Icon icon;
		uint8_t flags = 0;

		uint16_t id = 0;
		
		int8_t 
			 layer = 0,
			 plane = 0;

		void serialize(auto& ar, const unsigned)
		{
			ar & x;
			ar & y;
			ar & icon;
			ar & flags;
			ar & layer;
			ar & plane;	
			ar & id;
		}
	};

	struct VerticeGroup
	{
		std::vector<Vertice> vertices;
		std::vector<Filter>  filters;

		void serialize(auto& ar, const unsigned)
		{
			ar & vertices;
			ar & filters;
		}
	};

}

BOOST_CLASS_VERSION(mirage::graphics::IconResource, 1);
BOOST_CLASS_VERSION(mirage::graphics::Vertice, 1);
BOOST_CLASS_VERSION(mirage::graphics::VerticeGroup, 1);

BOOST_SERIALIZATION_SPLIT_FREE(mirage::graphics::SurfaceWrapper);
BOOST_SERIALIZATION_SPLIT_FREE(mirage::graphics::IconResource);

namespace boost::serialization
{
	void save(auto& ar, const mirage::graphics::SurfaceWrapper& surfacew, unsigned int)
	{
		auto* const& surface = surfacew.surface;
		ar & surface->w;
		ar & surface->h;
		ar & surface->pitch;
		ar & serialization::make_array(static_cast<char*>(surface->pixels), surface->h * surface->pitch);
	}
	
	void load(auto& ar, mirage::graphics::SurfaceWrapper& surfacew, unsigned int)
	{
		auto*& surface = surfacew.surface;

		decltype(surface->w) 
			w, 
			h;
		ar & w;
		ar & h;

		bool shouldInitialize = !surface;
		if(shouldInitialize)
			surface = SDL_CreateRGBSurface(
				0, 
				w, 
				h, 
				32, 
				0, 
				0, 
				0, 
				0);
		else
			SDL_LockSurface(surface);


		ar & surface->pitch;
		ar & serialization::make_array(static_cast<char*>(surface->pixels), surface->h * surface->pitch);

		if(!shouldInitialize)
			SDL_UnlockSurface(surface);
	}

	void serialize(auto& ar, SDL_Color& color, unsigned int)
	{
		ar & color.r;
		ar & color.g;
		ar & color.b;
		ar & color.a;
	}
}
