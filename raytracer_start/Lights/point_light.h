

#pragma once
#include "../Core/light.h"

class PointLight : public Light {
public:

	Vertex position;
	Colour intensity;
	Vector direction = Vector();
	bool point = true;

	PointLight();
	PointLight(Vertex pos, Colour col);
	PointLight(Vertex pos, Colour col, Vector dir);

	bool get_direction(Vertex& surface, Vector& dir);
	void get_intensity(Vertex& surface, Colour& intensity);
	bool get_type();

};
