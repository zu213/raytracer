#include "point_light.h"

PointLight::PointLight()
{
	Light();
}

PointLight::PointLight(Vertex pos, Colour col)
{
	Light();

	position = pos;
	intensity = col;
}

PointLight::PointLight(Vertex pos, Colour col, Vector dir)
{
	Light();

	position = pos;
	intensity = col;
	// this direction is used in photon mapping to make it more accurate
	direction = dir;
}

bool PointLight::get_direction(Vertex& surface, Vector& dir)
{
	// get direction
	dir = surface - position;
	dir.normalise();
	return true;
}

void PointLight::get_intensity(Vertex& surface, Colour& level)
{
	level = intensity;
}

bool PointLight::get_type() {
	return point;
}
