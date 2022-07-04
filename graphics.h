#pragma once
#include <string>
#include <boost/serialization/version.hpp>
#include <boost/asio/buffer.hpp>
#include <entt/entt.hpp>
#include <SDL.h>
#include <boost/serialization/array_wrapper.hpp>
#include "utility.h"

namespace mirage::graphics
{
	using Icon = entt::id_type;
	using Scale = std::pair<float, float>;
	using Transform = utils::Vec3<float>;
	using Filter = boost::variant<Transform, Scale>;

	struct SurfaceWrapper
	{
		SDL_Surface* surface;
	};

	struct IconResource
	{
		SDL_Surface* surface{nullptr};
		float scale = 1.f;
		Icon id;

		void save(auto& ar, const unsigned) const
		{
			ar & SurfaceWrapper{surface};
			ar & scale;
			ar & id;
		}

		void load(auto& ar, const unsigned version)
		{
			SurfaceWrapper sw{};
			ar & sw;
			surface = sw.surface;
			ar & scale;
			ar & id;
		}

		BOOST_SERIALIZATION_SPLIT_MEMBER()
	};

	
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

namespace boost::serialization
{
	void save(auto& ar, const mirage::graphics::SurfaceWrapper& surfacew, unsigned int)
	{
		auto* const& surface = surfacew.surface;
		ar & surface->w;
		ar & surface->h;
		ar & surface->format->BitsPerPixel;
		ar & surface->format->format;
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

		uint8_t depth;
		ar & depth;

		uint32_t format;
		ar & format;

		bool shouldInitialize = !surface;
		if(shouldInitialize)
			surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, depth, format);	
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
