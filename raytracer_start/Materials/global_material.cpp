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

#include "global_material.h"
#include <random>
#include <math.h>

GlobalMaterial::GlobalMaterial(Environment* &p_env, Colour p_reflect_weight, Colour p_refract_weight, Colour p_ambient_colour, Colour p_diff_colour, Colour p_spec_colour, float p_ior)
{
	environment = p_env;
	reflect_weight = p_reflect_weight;
	refract_weight = p_refract_weight;
	ior = p_ior;
	ambient_colour = p_ambient_colour;
	diffuse_colour = p_diff_colour;
	specular_colour = p_spec_colour;
}


// reflection and recursion computation
Colour GlobalMaterial::compute_once(Ray& viewer, Hit& hit, int recurse)
{
	Colour result;

	// Ambient
	float k = 0.05f; // ambient coefficient
	result.r = k * ambient_colour.r;
	result.g = k * ambient_colour.g;
	result.b = k * ambient_colour.b;

	// reflection
	if(recurse > 0 && (reflect_weight.r > 0 || reflect_weight.g > 0 || reflect_weight.b > 0)){
		Colour new_colour = reflect(viewer, hit, recurse);

		result.r += reflect_weight.r * new_colour.r;
		result.g += reflect_weight.g * new_colour.g;
		result.b += reflect_weight.b * new_colour.b;
	}

	// refraction
	if (recurse > 0 && (refract_weight.r > 0 || refract_weight.g > 0 || refract_weight.b > 0)) {
		//calculate refraction index
		float n;
		if (hit.entering) {
			n = ior;
		}
		else {
			n = 1 / ior;
		}
		// Work out constants
		float cos_i = fabs(hit.normal.dot(viewer.direction));
		float cos_t = sqrt(1 - (1 / (pow(n, 2))) * (1 - pow(cos_i, 2)));

		// Fresnel terms
		float rpar = (n * cos_i - cos_t) / (n * cos_i + cos_t);
		float rper = (cos_i - n * cos_t) / (cos_i + n * cos_t);
		float kr = 0.5f * (pow(rpar, 2) + pow(rper, 2));
		float kt = 1 - kr;

		// Calculate refraction ray if need be
		if (kt > 0) {
			Vector new_ray_direction = 1 / n * (viewer.direction) - (cos_t - (1 / n) * cos_i) * hit.normal;
			new_ray_direction.normalise();
			Ray new_ray = Ray(hit.position + EPSILON * new_ray_direction, new_ray_direction);
			Colour new_colour = Colour();
			float depth;
			environment->raytrace(new_ray, recurse - 1, new_colour, depth);
			// retrieve colour on the way back
			result.r += kt * refract_weight.r * new_colour.r;
			result.g += kt * refract_weight.g * new_colour.g;
			result.b += kt * refract_weight.b * new_colour.b;
		}

		// Calculate new reflection ray if need be
		if (kr > 0) {
			Colour new_colour_reflect = reflect(viewer, hit, recurse);
			result.r += kr * refract_weight.r * new_colour_reflect.r;
			result.g += kr * refract_weight.g * new_colour_reflect.g;
			result.b += kr * refract_weight.b * new_colour_reflect.b;
		}
	}

	return result;
}

Colour GlobalMaterial::compute_once_photon(Ray& ray, Hit& hit, int recurse)
{
	// very similar to compute once, main difference is we call photon ray trace
	Colour result;

	// reflection
	if (recurse > 0 && (reflect_weight.r > 0 || reflect_weight.g > 0 || reflect_weight.b > 0)) {
		//calculate reflected ray
		Vector new_ray_direction = ray.direction - 2 * (hit.normal.dot(ray.direction)) * hit.normal;
		new_ray_direction.normalise();
		Photon new_photon = Photon(hit.position + EPSILON * new_ray_direction, new_ray_direction, "normal");

		// Create new colour and depth to pass in
		Colour new_colour = Colour();
		float depth;
		environment->photon_raytrace(new_photon, recurse - 1, new_colour, depth);

		result.r += reflect_weight.r * new_colour.r;
		result.g += reflect_weight.g * new_colour.g;
		result.b += reflect_weight.b * new_colour.b;
	}

	// refraction
	if (recurse > 0 && (refract_weight.r > 0 || refract_weight.g > 0 || refract_weight.b > 0)) {
		//calculate refraction index
		float n;
		if (hit.entering) {
			n = ior;
		}
		else {
			n = 1 / ior;
		}
		// Work out constants
		float cos_i = fabs(hit.normal.dot(ray.direction));
		float cos_t = sqrt(1 - (1 / (pow(n, 2))) * (1 - pow(cos_i, 2)));

		// Fresnel terms
		float rpar = (n * cos_i - cos_t) / (n * cos_i + cos_t);
		float rper = (cos_i - n * cos_t) / (cos_i + n * cos_t);
		float kr = 0.5f * (pow(rpar, 2) + pow(rper, 2));
		float kt = 1 - kr;

		// Calculate refraction ray if need be
		if (kt > 0) {
			Vector new_ray_direction = 1 / n * (ray.direction) - (cos_t - (1 / n) * cos_i) * hit.normal;
			new_ray_direction.normalise();
			Ray new_ray = Ray(hit.position + EPSILON * new_ray_direction, new_ray_direction);
			Colour new_colour = Colour();
			float depth;
			environment->photon_raytrace(new_ray, recurse - 1, new_colour, depth);
			// retrieve colour on the way back
			result.r += kt * refract_weight.r * new_colour.r;
			result.g += kt * refract_weight.g * new_colour.g;
			result.b += kt * refract_weight.b * new_colour.b;
		}

		// Calculate new reflection ray if need be
		if (kr > 0) {
			//calculate reflected ray
			Vector new_ray_direction = ray.direction - 2 * (hit.normal.dot(ray.direction)) * hit.normal;
			new_ray_direction.normalise();
			Photon new_photon = Photon(hit.position + EPSILON * new_ray_direction, new_ray_direction, "normal");

			// Create new colour and depth to pass in
			Colour new_colour_reflect = Colour();
			float depth;
			environment->photon_raytrace(new_photon, recurse - 1, new_colour_reflect, depth);
			result.r += kr * refract_weight.r * new_colour_reflect.r;
			result.g += kr * refract_weight.g * new_colour_reflect.g;
			result.b += kr * refract_weight.b * new_colour_reflect.b;
		}
	}

	return result;
}


Colour GlobalMaterial::compute_per_light(Vector& viewer, Hit& hit, Vector& ldir)
{
	Colour result;
	result.r = 0.0f;
	result.g = 0.0f;
	result.b = 0.0f;
	return result;
}

Colour GlobalMaterial::reflect(Ray& viewer, Hit& hit, int recurse) {
	//calculate reflected ray
	Vector new_ray_direction = viewer.direction - 2 * (hit.normal.dot(viewer.direction)) * hit.normal;
	new_ray_direction.normalise();
	Ray new_ray = Ray(hit.position + EPSILON * new_ray_direction, new_ray_direction);

	// Create new colour and depth to pass in
	Colour new_colour = Colour();
	float depth;
	environment->raytrace(new_ray, recurse - 1, new_colour, depth);
	return new_colour;
}

Photon GlobalMaterial::compute_photon_reflect(Photon photon, Hit& hit) {
	if (reflect_weight.r <= 0 && reflect_weight.g <= 0 && reflect_weight.b <= 0) {
		return Photon();
	}
	std::default_random_engine generator;
	std::uniform_real_distribution<double> roulette_distribution(0, 1.0);
	float x = roulette_distribution(generator);
	if (x > ((reflect_weight.r + reflect_weight.g + reflect_weight.b) / 3)) {
		return Photon();
	}
	// return a new reflected photon
	Vector new_photon_direction = photon.direction - 2 * (hit.normal.dot(photon.direction)) * hit.normal;
	new_photon_direction.normalise();
	Photon new_photon = Photon(hit.position + EPSILON * new_photon_direction, new_photon_direction, "reflect", 1);
	Colour new_colour = photon.colour;

	new_photon.set_colour(new_colour);
	return new_photon;
}


Photon* GlobalMaterial::compute_photon_refract(Photon photon, Hit& hit) {
	// return a array of length two with refract/reflect from Fresnel
	std::default_random_engine generator;
	std::uniform_real_distribution<double> roulette_distribution(0, 1.0);
	float x = roulette_distribution(generator);
	photons[0].type = "";
	photons[1].type = "";
	float n;
	if (hit.entering) {
		n = ior;
	}
	else {
		n = 1 / ior;
	}
	// Work out constants
	float cos_i = fabs(hit.normal.dot(photon.direction));
	float cos_t = sqrt(1 - (1 / (pow(n, 2))) * (1 - pow(cos_i, 2)));

	// Fresnel terms
	float rpar = (n * cos_i - cos_t) / (n * cos_i + cos_t);
	float rper = (cos_i - n * cos_t) / (cos_i + n * cos_t);
	float kr = 0.5f * (pow(rpar, 2) + pow(rper, 2));
	float kt = 1 - kr;

	if (x < kt) {
		Vector new_ray_direction = 1 / n * (photon.direction) - (cos_t - (1 / n) * cos_i) * hit.normal;
		new_ray_direction.normalise();
		Photon refract_photon = Photon(hit.position + EPSILON * new_ray_direction, new_ray_direction, "refract");
		Colour new_colour = photon.colour;
		refract_photon.set_colour(new_colour);
		photons[0] = refract_photon;
	}
	if (x < kr) {
		Photon reflect_photon = compute_photon_reflect(photon, hit);
		Colour new_colour = photon.colour;
		reflect_photon.set_colour(new_colour);
		photons[1] = reflect_photon;
	}
	return photons;
}

bool GlobalMaterial::is_caustic() {
	if (refract_weight.r > 0 || refract_weight.g > 0 || refract_weight.b > 0) {
		return true;
	}
	return false;
}

Colour GlobalMaterial::get_reflect_weight() {
	return reflect_weight;
}

Colour GlobalMaterial::get_refract_weight() {
	// this is used in the caustic photons baking.
	return refract_weight;
}

