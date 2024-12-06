/****************************************************************************
*
* krt - Ken's Raytracer - Coursework Edition. (C) Copyright 1993-2022.
*
* I've put a lot of time and effort into this code. For the last decade
* it's been used to introduce hundreds of students at multiple universities
* to raytracing. It forms the basis of your coursework but you are free
* to continue using/developing forever more. However, I ask that you don't
* share the code or your derivitive versions publicly. In order to continue
* to be used for coursework and in particular assessment it's important that
* versions containing solutions are not searchable on the web or easy to
* download.
*
* If you want to show off your programming ability, instead of releasing
* the code, consider generating an incredible image and explaining how you
* produced it.
*/

// Material is the base class for materials.

#pragma once

#include "colour.h"
#include "ray.h"
#include "hit.h"
#include "photon.h"


class Material {
public:

	Colour ambient_colour;
	Colour diffuse_colour;
	Colour specular_colour;
	Photon photons[2];


	// compute_once is called once per intersection
	virtual Colour compute_once(Ray &viewer, Hit &hit, int recurse)
	{
		Colour result;
		result.r = 0.0f;
		result.g = 0.0f;
		result.b = 0.0f;
		return result;
	}

	// compute_once is called once per intersection
	virtual Colour compute_once_photon(Ray &viewer, Hit& hit, int recurse)
	{
		Colour result;
		result.r = 0.0f;
		result.g = 0.0f;
		result.b = 0.0f;
		return result;
	}

	// compute_per_light is called for each light that reaches a surface
	virtual Colour compute_per_light(Vector &viewer, Hit &hit, Vector &ldir)
	{
		Colour result;
		result.r = 0.0f;
		result.g = 0.0f;
		result.b = 0.0f;
		return result;
	}

	// compute_per_light is called for each light that reaches a surface
	virtual Colour compute_per_light_photon(Vector& viewer, Hit& hit, Vector& ldir)
	{
		Colour result;
		result.r = 0.0f;
		result.g = 0.0f;
		result.b = 0.0f;
		return result;
	}

	// You will need additional material methods to support Photon-mapping.
	virtual Photon compute_photon_reflect(Photon i_photon, Hit& hit)
	{
		Photon photon;
		return photon;
	}

	virtual Photon* compute_photon_refract(Photon i_photon, Hit& hit)
	{
		return photons;
	}

	virtual bool is_caustic() {
		return false;
	}

	virtual Colour brdf(Vector incoming, Vector outgoing) {
		return Colour(1.0f,1.0f,1.0f);
	}

	virtual Colour get_reflect_weight() {
		return Colour();
	}

	virtual Colour get_refract_weight() {
		return Colour();
	}
};
