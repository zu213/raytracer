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

#pragma once

#include "colour.h"
#include "ray.h"
#include "../Core/object.h"
#include "../Core/light.h"
#include "../Lights/point_light.h"
#include "hit.h"
#include "environment.h"
#include "../kd_tree/src/tree.h"
#include "photon.h"

struct Point
{
	float data[3];
	Photon photon;

	Point() {}
	Point(float x, float y, float z) {
		data[0] = x;
		data[1] = y;
		data[2] = z;

	}

	Point(float x, float y, float z, Photon p) {
		data[0] = x;
		data[1] = y;
		data[2] = z;
		photon = p;

	}

	float operator [] (int i) const {
		return data[i];
	}

	bool operator == (const Point& p) const {
		return data[0] == p[0] && data[1] == p[1] && data[2] == p[2];
	}

	friend std::ostream& operator << (std::ostream& s, const Point& p) {
		return s << '(' << p[0] << ", " << p[1] << ", " << p[2] << ')';
	}
};

// Scene is a class that is used to build a scene database of objects
// and lights and then trace a ray through it.

class Scene: public Environment {
public:

  Object *object_list;
  Light *light_list;

  typedef KD::Core<3, Point> CORE;
  KD::Tree<CORE> *kdtree;
  KD::Tree<CORE> *caustic_kdtree;
  float max_col = 0;

  Scene();

  // Filter the list of returned hits to the closest +ve
  Hit* select_first(Hit* list);

  // Trace a ray through the scene and find the closest if any object
  // intersection in front of the ray.
  Hit *trace(Ray ray);
	
  // Trace a ray through the scene and return its colour. This function
  // is the one that should recurse down the reflection/refraction tree within a material
  void raytrace(Ray ray, int recurse, Colour& colour, float& depth);

  void photon_raytrace(Ray ray, int recurse, Colour& colour, float& depth);
  void photon_bake(int photons);

  // raytrace a shadow ray.
  bool shadowtrace(Ray, float limit);

  void add_object(Object *obj);
  void add_light(Light *light);

  void fire_photons_from_light(PointLight* light, float max_depth, int photons, int recurse);

  void fire_photon(Photon &photon, float max_depth, int recurse);

  void fire_caustic_photon(Photon &photon, float max_depth, int recurse, int &count);

};
