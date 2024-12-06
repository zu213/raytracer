/****************************************************************************
*
* krt - Ken's Raytracer - Coursework Edition. (C) Copyright 1993-2023.
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

#include "full_camera.h"
#include "../Core/transform.h"
#include "../Core/photon.h"


void Progress_bar(float progress) {
	int barWidth = 90;

	cout << "Tracing [";
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

FullCamera::FullCamera()
{
	fov = 0.5;
	FullCamera::height = 0;
	FullCamera::width = 0;
}

FullCamera::FullCamera(float f, Vertex p_position, Vector p_lookat, Vector p_up)
{
	fov = f;
	FullCamera::position = p_position;
	FullCamera::lookat = p_lookat;
	FullCamera::up = p_up;

}


FullCamera::FullCamera(float f, Vertex p_position, Vector p_lookat, Vector p_up, bool focus)
{
	fov = f;
	FullCamera::position = p_position;
	FullCamera::lookat = p_lookat;
	FullCamera::up = p_up;
	FullCamera::focus = focus;
	FullCamera::focal_vector = lookat - position;

}

void FullCamera::get_ray_offset(int p_x, int p_y, float p_ox, float p_oy, Ray& ray)
{
}

void FullCamera::get_sub_ray(float p_ox, float p_oy, Ray& ray)
{
	// Similar to get ray pixel, going from x,y space to u,v,w, difference is this function creates an offset sub ray
	float fx = p_ox / 2.0f;
	float fy = p_oy / 2.0f;

	Vector position_change = Vector(
		((fx - 0.5f) * u.x + (0.5f - fy) * v.x - fov * w.x),
		((fx - 0.5f) * u.y + (0.5f - fy) * v.y - fov * w.y),
		((fx - 0.5f) * u.z + (0.5f - fy) * v.z - fov * w.z)
	);

	ray.position = centre_ray.position + position_change;
	ray.direction = centre_ray.direction - position_change;
	ray.direction.normalise();
}

void FullCamera::get_ray_pixel(int p_x, int p_y, Ray& ray)
{
		// Going from x,y space to u,v,w
		float fx = ((float)p_x + 0.5f) / (float)width;
		float fy = ((float)p_y + 0.5f) / (float)height;

		ray.position.x = position.x;
		ray.position.y = position.y;
		ray.position.z = position.z;
		ray.position.w = 1.0f;

		Vector direction = Vector(
			((fx - 0.5f) * u.x + (0.5f - fy) * v.x - fov * w.x),
			((fx - 0.5f) * u.y + (0.5f - fy) * v.y - fov * w.y),
			((fx - 0.5f) * u.z + (0.5f - fy) * v.z - fov * w.z)
		);

		ray.direction.x = direction.x;
		ray.direction.y = direction.y;
		ray.direction.z = direction.z;
		ray.direction.normalise();

}

void FullCamera::render(Environment& env, FrameBuffer& fb)
{
	width = fb.width;
	height = fb.height;
	//check if the up angle is at a right angle to look at before next step
	Vector direction_vector = lookat - position;
	float error_check = direction_vector.dot(up);
	if (error_check == 0) {
		// Calculate w,u and v
		w = position - lookat;
		w.normalise();

		u = position - lookat;

		u.cross(up);
		u.normalise();

		v = position - lookat;
		v.cross(u);
		v.normalise();
		v.negate();

		if (env.photon_mapping) {
			env.photon_bake(100000);
		}
	

		for (int y = 0; y < height; y += 1)
		{
			for (int x = 0; x < width; x += 1)
			{
				Ray ray;

				get_ray_pixel(x, y, ray);

				Colour colour;
				float depth;

				// if we are focusing camera fire off more rays and combine the colours to blur out of focus ones
				if (focus) {
					int lens_height = 10;
					int samples = 10;
					// create the centre ray
					centre_ray = ray;
					float focal_length = focal_vector.length();
					centre_ray.direction = centre_ray.direction * focal_length;

					for (int c = 0; c < samples; c += 1)
					{
						float f_c = ((float)c + 0.5f) / (float)lens_height;
						for (int d = 0; d < samples; d += 1)
						{
							float f_d = ((float)d + 0.5f) / (float)lens_height;
							
							Ray primary_ray;
							get_sub_ray(f_c, f_d, primary_ray);

							Colour new_colour;
							if (!env.photon_mapping) {
								env.raytrace(primary_ray, 10, new_colour, depth);
							}
							else {
								Photon photon = Photon(primary_ray.position, primary_ray.direction, "normal");
								env.photon_raytrace(photon, 10, new_colour, depth);
							}							
							colour += new_colour;
						}
					}
					colour = colour * (1/pow(lens_height, 2));

				}
				else {
					// Else we just do normal ray tracing
					if (!env.photon_mapping) {
						env.raytrace(ray, 10, colour, depth);
					}
					else {
						Photon photon = Photon( ray.position, ray.direction, "normal");
						env.photon_raytrace(photon, 10, colour, depth);
					}
				}

				fb.plotPixel(x, y, colour.r, colour.g, colour.b);
				fb.plotDepth(x, y, depth);
			}

			Progress_bar((float)y / (float)height);
		}
	}
	else {
		throw "error look at not perpendicular to up";
	}
}

