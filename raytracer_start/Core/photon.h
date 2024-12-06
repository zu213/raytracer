#pragma once

#include <iostream>
using namespace std;

#include "vector.h"
#include "vertex.h"
#include "ray.h"

class Photon: public Ray {
public:
	string type;
	float intensity = 1;
	Colour colour;

	Photon()
	{
		position = Vertex(0,0,0);
		direction = Vector(0,0,0);
		type = "";
	}

	Photon(Vertex p, Vector d, string t)
	{
		position = p;
		direction = d;
		type = t;
	}

	Photon(Vertex p, Vector d, string t, float i)
	{
		position = p;
		direction = d;
		type = t;
		intensity = i;
	}

	void set_colour(Colour c) {
		colour = c;
	}

	//A handy debug operator
	friend ostream& operator<<(ostream& os, const Ray& r);
};
