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

#include "scene.h"
#include <random>
#include "photon.h"
#include "../kd_tree/src/tree.h"
#include "../Materials/global_material.h"
#include <iostream>
#define _USE_MATH_DEFINES
#include <cmath>
#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif


using namespace std;

void Progress_bar_bake(float progress, string prefix) {
	int barWidth = 90;

	cout << prefix << " Baking: [";
	int pos = (int)(barWidth * progress);
	for (int i = 0; i < barWidth; i++) {
		if (i < pos) {
			cout << "=";
		}
		else if (i == pos) {
			cout << ">";
		}
		else {
			cout << " ";
		}
	}
	cout << "] " << float(progress * 100.0) << " %\r";
	cout.flush();
}

Scene::Scene()
{
	object_list = 0;
	light_list = 0;

	Point min(-99, -99, -99);
	Point max(99, 99, 99);
	kdtree = new KD::Tree<CORE>(min, max);
	caustic_kdtree = new KD::Tree<CORE>(min, max);
	max_col = 0;
}

bool Scene::shadowtrace(Ray ray, float limit = 1000)
{
	Object *objects = this->object_list;

	while (objects != 0)
	{
		Hit *hit = this->select_first(objects->intersection(ray));
		if (hit != 0)
		{
		  if ((hit->t > 0.00000001f) &&( hit->t < limit))
		    {
			  delete hit;
		      return true;
		    }
		  delete hit;
		}

		objects = objects->next;
	}

	return false;
}

Hit *Scene::trace(Ray ray)
{
	Hit *best_hit = 0;

	Object *objects = this->object_list;

	while (objects != 0)
	{
		Hit *hit = this->select_first(objects->intersection(ray));


		if (hit != 0)
		{

			if (best_hit == 0)
			{
				best_hit = hit;

			} else if (hit->t < best_hit->t)
			{
				delete best_hit;
				best_hit = hit;
			}
			else
			{
				delete hit;
			}
		}

		objects = objects->next;
	}
	return best_hit;
}

Hit* Scene::select_first(Hit* list)
{
	Hit* result = 0;

	while (list != 0)
	{
		if (list->t >= 0.0f)
		{
			result = list;
			list = list->next;
			break;
		}

		Hit* temp = list;
		list = list->next;
		delete temp;
	}

	while (list != 0)
	{
		Hit* temp = list;
		list = list->next;
		delete temp;
	}

	return result;
}

void Scene::raytrace(Ray ray, int recurse, Colour &colour, float &depth)
{
  Object *objects = object_list;
  Light *lights = light_list;
	
  // a default colour if we hit nothing.
  colour.r = 0.0f;
  colour.g = 0.0f;
  colour.b = 0.0f;
  colour.a = 0.0f;
  depth = 0.0f;

  // first step, find the closest primitive
  Hit *best_hit = this->trace(ray);

  // if we found a primitive then compute the colour we should see
  if (best_hit != 0)
  {
	  depth = best_hit->t;
	  colour = colour + best_hit->what->material->compute_once(ray, *best_hit, recurse); // this will be the global components such as ambient or reflect/refract

	  // next, compute the light contribution for each light in the scene.
	  Light* light = light_list;
	  while (light != (Light*)0)
	  {
		  Vector viewer;
		  Vector ldir;

		  viewer = -best_hit->position;
		  viewer.normalise();

		  bool lit;
		  lit = light->get_direction(best_hit->position, ldir);
		  bool point = light->get_type();

		  if (ldir.dot(best_hit->normal) > 0)
		  {
			  lit = false;//light is facing wrong way.
		  }

		  // Put the shadow check here, if lit==true and in shadow, set lit=false
		  if (lit)
		  {
			  Vertex hit_pos = best_hit->position - EPSILON * ldir;
			  // need length for point light
			  float length = ldir.length();
			  ldir.normalise();
			  Ray shadowRay = Ray(hit_pos, -ldir);
			  if (!point) {
				  if (shadowtrace(shadowRay)) {
					  lit = false;
				  }
			  }
			  else {
				  // if a point light make sure ray only trace to where light is otherwise plane on bottom will
				  // cause fake shadows!
				  if (shadowtrace(shadowRay, length)) {
					  lit = false;
					  bool test = shadowtrace(shadowRay, ldir.length());
				  }
			  }
		  }

		  if (lit)
		  {
			  Colour intensity;

			  light->get_intensity(best_hit->position, intensity);
			  colour = colour + intensity * best_hit->what->material->compute_per_light(viewer, *best_hit, ldir); // this is the per light local contrib e.g. diffuse, specular
		  }

		  light = light->next;
	  }

	  delete best_hit;
  }
}


void Scene::photon_raytrace(Ray ray, int recurse, Colour& colour, float& depth)
{
	Object* objects = object_list;
	// a default colour if we hit nothing.
	colour.r = 0.0f;
	colour.g = 0.0f;
	colour.b = 0.0f;
	colour.a = 0.0f;
	depth = 0.0f;
	// now find the closest primitive
	Hit* best_hit = this->trace(ray);

	// if we found a primitive then compute the colour we should see
	if (best_hit != 0)
	{
		//some constants
		depth = best_hit->t;
		float r = 1.0f;
		float k = 1.1;
		float w = 1;// / (best_hit->t / 3);
		float cone_filter = 1 - 2 / (3 * k);
		// first get point we hit
		Point closest = Point(best_hit->position.x, best_hit->position.y, best_hit->position.z);
		std::vector<Point> temp;
		
		// work out global lighting
		//first search with tiny radius, if no. of results is too low search wider
		temp = kdtree->within(closest, r/2);
		if (temp.size() > 1) {
			for (size_t i = 0; i < temp.size(); i++)
			{
				// calculate brdf
				Colour brdf = best_hit->what->material->brdf(ray.direction, temp[i].photon.direction);

				// k = 1.1 here for now this bit is for the cone filter
				Vector photon_hit_location = Vector(temp[i].data[0], temp[i].data[1], temp[i].data[2]);
				float wpc = 1 - ((photon_hit_location - best_hit->position).length()) / (k * r);

				Colour contribution = brdf * temp[i].photon.intensity * wpc;
				contribution.r = fabs(contribution.r);
				contribution.g = fabs(contribution.g);
				contribution.b = fabs(contribution.b);

				colour += (temp[i].photon.colour * contribution);
			}
			float divisor = M_PI * (pow(r, 2)) * cone_filter;
			colour /= divisor;
		}
		

		// then we work out caustics
		temp = caustic_kdtree->within(closest, r/10.0f);
		Colour caustic_colour = Colour();
		if (temp.size() > 1) {
			for (size_t i = 0; i < temp.size(); i++)
			{
				// calculate brdf
				Colour brdf = best_hit->what->material->brdf(ray.direction, temp[i].photon.direction);

				// k = 1.1 here for now this bit is for the cone filter
				Vector photon_hit_location = Vector(temp[i].data[0], temp[i].data[1], temp[i].data[2]);
				float wpc = 1 - ((photon_hit_location - best_hit->position).length()) / (k * r);

				Colour contribution = brdf * temp[i].photon.intensity * wpc;
				contribution.r = fabs(contribution.r);
				contribution.g = fabs(contribution.g);
				contribution.b = fabs(contribution.b);

				caustic_colour += (temp[i].photon.colour * contribution);

			}
			float divisor = M_PI * (pow(r, 2)) *cone_filter;
			caustic_colour /= divisor;
		}

		// then we use normal raytracing for specular
		Light* light = light_list;
		Colour specular_colour = Colour();
		while (light != (Light*)0)
		{
			Vector viewer;
			Vector ldir;

			viewer = -best_hit->position;
			viewer.normalise();

			bool lit;
			lit = light->get_direction(best_hit->position, ldir);

			if (lit)
			{
				Vertex hit_pos = best_hit->position - EPSILON * ldir;
				// need length for point light
				float length = ldir.length();
				ldir.normalise();
				Ray shadowRay = Ray(hit_pos, -ldir);
				// always point light
				// cause fake shadows!
				if (shadowtrace(shadowRay, length)) {
					lit = false;
					bool test = shadowtrace(shadowRay, ldir.length());
				}
			}
			if (lit)
			{
				Colour intensity;
				light->get_intensity(best_hit->position, intensity);
				if (best_hit->what->material->compute_per_light(viewer, *best_hit, ldir).r > 1) {
					float c = 7;
				}
				specular_colour = specular_colour + (intensity * best_hit->what->material->compute_per_light(viewer, *best_hit, ldir)) * w; // this is the per light local contrib e.g. diffuse, specular
			}

			light = light->next;
		}
		// some scaling so they are roughly the same
		colour *= 0.0001f;
		// good value for general scene
		caustic_colour *= 0.000005f;
		specular_colour *=0.03f;

		Colour recurse_col = best_hit->what->material->compute_once_photon(ray, *best_hit, recurse);
		// then we recurse down with rays to deal with reflection and refraction
		colour += recurse_col;
		colour += caustic_colour;
		colour += specular_colour;
		
		delete best_hit;
	}
}

void Scene::photon_bake(int photons){
	Light* light = light_list;

	while (light != (Light*)0)
	{
		if (light->get_type()) {
			fire_photons_from_light((PointLight*)light, 100, photons, 10);
		}
		light = light->next;
	}
}

void Scene::add_object(Object *obj)
{
  obj->next = this->object_list;
  this->object_list = obj;
}

void Scene::add_light(Light *light)
{
  light->next = this->light_list;
  this->light_list = light;
}

void Scene::fire_photons_from_light(PointLight* light, float max_depth, int photons, int recurse)
{
	// Initial setup of photons
	std::default_random_engine generator;
	std::uniform_real_distribution<double> float_distribution(-1.0, 1.0);

	// fire the global photons
	for (int a = 0; a < photons; a++) {
		float x = float_distribution(generator);
		float y = float_distribution(generator);
		float z = float_distribution(generator);
		Vector dir = Vector(x, y, z);
		dir.normalise();
		if (!(light->direction.x == 0 && light->direction.y == 0 && light->direction.z == 0)) {
			// if there is a direction we only do it in the semi circle, if its out side we can just flip it
			if (dir.dot(light->direction) < 0) {
				dir.negate();
			}
		}
		Photon photon = Photon(light->position, dir, "normal");
		photon.set_colour(light->intensity);
		fire_photon(photon, max_depth, recurse);
		if (a % 1000 == 0) {
			Progress_bar_bake((float)a / (float)photons, "Global");
		}
	}

	// fire the caustic photons a portion at a time until the tree is reasonably full
	if (caustic) {
		int portion = photons / 100;
		int count = 0;
		if (photons < 100000) {
			portion = 100;
		}

		while (count < photons) {
			for (int a = 0; a < portion; a++) {
				float x = float_distribution(generator);
				float y = float_distribution(generator);
				float z = float_distribution(generator);
				Vector dir = Vector(x, y, z);
				dir.normalise();
				if (!(light->direction.x == 0 && light->direction.y == 0 && light->direction.z == 0)) {
					// if there is a direction we only do it in the semi circle, if its out side we can just flip it
					if (dir.dot(light->direction) < 0) {
						dir.negate();
					}
				}
				Photon photon = Photon(light->position, dir, "caustic");
				photon.intensity = 0.1f;
				photon.set_colour(light->intensity);
				fire_caustic_photon(photon, max_depth, recurse, count);
			}
			Progress_bar_bake((float)count / (((float)photons)), "Caustic");
		}
	}
}

void Scene::fire_photon(Photon &photon, float max_depth, int recurse){
	// when a photon hits bake the colour in the kd tree
	// dont process colourless rays
	if (photon.colour.r == 0 && photon.colour.g == 0 && photon.colour.b == 0) {
		return;
	}

	Hit* best_hit = this->trace(photon);
	if (best_hit != 0)
	{
		//first find resulting colour
		Colour new_colour = best_hit->what->material->ambient_colour;
		new_colour.r *= photon.colour.r;
		new_colour.g *= photon.colour.g;
		new_colour.b *= photon.colour.b;

		// Find if its reflective
		Photon reflect_photon = best_hit->what->material->compute_photon_reflect(photon, *best_hit);
		if (reflect_photon.type == "reflect" && recurse != 0) {
			// if reflective it picks up the colour, this is set in the reflect function
			fire_photon(reflect_photon, max_depth, recurse - 1);
		}

		// Find if its refractive
		Photon* refract_photons = best_hit->what->material->compute_photon_refract(photon, *best_hit);
		if (refract_photons != 0) {

			if (refract_photons[0].type == "refract" && recurse != 0) {
				refract_photons[0].set_colour(best_hit->what->material->get_refract_weight() * photon.colour);
				fire_photon(refract_photons[0], max_depth, recurse - 1);

			}
			if (refract_photons[1].type == "reflect" && recurse != 0) {
				refract_photons[1].set_colour(best_hit->what->material->get_refract_weight() * photon.colour);
				fire_photon(refract_photons[1], max_depth, recurse - 1);
			}
		}

		// Pick up the surface material
		photon.set_colour(new_colour);
		kdtree->insert(Point(best_hit->position.x, best_hit->position.y, best_hit->position.z, photon));
	}
}

void Scene::fire_caustic_photon(Photon &photon, float max_depth, int recurse, int &count) {
	// dont process colourless rays
	if (photon.colour.r == 0 && photon.colour.g == 0 && photon.colour.b == 0) {
		return;
	}

	Hit* best_hit = this->trace(photon);
	if (best_hit != 0 && best_hit->what->material->is_caustic())
	{

		// We only care if the material is refractive (it will be if caustic)
		Photon* refract_photons = best_hit->what->material->compute_photon_refract(photon, *best_hit);
		if (refract_photons != 0) {
			if (refract_photons[0].type == "refract" && recurse != 0) {
				refract_photons[0].set_colour(best_hit->what->material->get_refract_weight() * photon.colour);
				fire_caustic_photon(refract_photons[0], max_depth, recurse - 1, count);

			}
		}
		
	}
	if (best_hit != 0 && !best_hit->what->material->is_caustic() && photon.type == "refract") {
		caustic_kdtree->insert(Point(best_hit->position.x, best_hit->position.y, best_hit->position.z, photon));
		// increase the count
		count += 1;
	}
}

