/***************************************************************************
 *
 * krt - Kens Raytracer - Coursework Edition. (C) Copyright 1997-2018.
 *
 * Do what you like with this code as long as you retain this comment.
 */

// Phong is a child class of Material and implement the simple Phong
// surface illumination model.

#pragma once

#include "../Core/material.h"
#include "../Core/environment.h"

class Phong : public Material {
public:

	Phong(Colour ambient, Colour specular, Colour diffuse)
	{
		this->ambient_colour = ambient;
		this->specular_colour = specular;
		this->diffuse_colour = diffuse;

	}

	Colour compute_once(Ray& viewer, Hit& hit, int recurse);
	Colour compute_per_light(Vector& viewer, Hit& hit, Vector& ldir);
	Photon compute_photon_reflect(Photon photon, Hit& hit);

	bool is_caustic();

};
