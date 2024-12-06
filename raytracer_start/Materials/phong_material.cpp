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

 // A simple Phong based lighting model.

#include "phong_material.h"

#include <math.h>
#include <random>
#include "../Core/light.h"


// The compute_once() method supplies the ambient term.
Colour Phong::compute_once(Ray& viewer, Hit& hit, int recurse)
{
	Colour result;
	float ambient_intensity = 0.5f;
	float k = 0.1f; // ambient coefficient
	result.r = ambient_colour.r * k * ambient_intensity;
	result.g = ambient_colour.g * k * ambient_intensity;
	result.b = ambient_colour.b * k * ambient_intensity;
	return result;
}

// The compute_per_light() method supplies the diffuse and specular terms.
Colour Phong::compute_per_light(Vector& viewer, Hit& hit, Vector& ldir)
{

	// set constants
	float n = 20;
	float dK = 0.5f; //diffuse coeffiecient
	float sK = 0.5f; //specular coeffiecient 

	// Calculate diffuse lighting
	Vector surface_normal = hit.normal;
	surface_normal.normalise();
	Vertex hit_location = hit.position;
	hit_location.normalise();
	float diffuse = dK * (surface_normal.dot(-ldir));

	// Calculate specular lighting
	Vector reflection = ldir - 2 * ldir.dot(surface_normal) * surface_normal;
	reflection.normalise();
	Vector v = viewer;
	v.normalise();
	float specular = sK * (pow(reflection.dot(viewer), n));
	
	Colour result;
	result.r += diffuse * diffuse_colour.r;
	result.g += diffuse * diffuse_colour.g;
	result.b += diffuse * diffuse_colour.b;
	result.r += specular * specular_colour.r;
	result.g += specular * specular_colour.g;
	result.b += specular * specular_colour.b;
	
	return result;
	
}

bool Phong::is_caustic() {
	return false;
}

Photon Phong::compute_photon_reflect(Photon photon, Hit& hit)
	{
	// phong material have a small chance of reflecting (reflect chance)
	float reflect_chance = 0.135;
	std::default_random_engine generator;
	std::uniform_real_distribution<double> roulette_distribution(0, 1.0);
	std::uniform_real_distribution<double> direction_distribution(-1.0, 1.0);
	float x = roulette_distribution(generator);
	if (x > reflect_chance) {
		return Photon();
	}
	// random direction for reflec to mimic sub surface scattering.
	float x1 = direction_distribution(generator);
	float y1 = direction_distribution(generator);
	float z1 = direction_distribution(generator);
	Vector new_direction = Vector(x1, y1, z1);
	new_direction.normalise();
	if (new_direction.dot(hit.normal) < 0) {
		//inverse new_direction if it would hit wall
		new_direction.negate();
	}
	// return a new reflected photon
	Photon new_photon = Photon(hit.position + EPSILON * new_direction, new_direction, "reflect", 1);
	Colour new_colour = photon.colour * specular_colour;

	// set colour
	new_photon.set_colour(new_colour);
	return new_photon;
}

