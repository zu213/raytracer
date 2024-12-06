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

#include "quadratic_object.h"
#include <math.h>

using namespace std;

Quadratic::Quadratic(float p_a, float p_b, float p_c, float p_d, float p_e, float p_f, float p_g, float p_h, float p_i, float p_j)
{
  a = p_a;
  b = p_b;
  c = p_c;
  d = p_d;
  e = p_e;
  f = p_f;
  g = p_g;
  h = p_h;
  i = p_i;
  j = p_j;


  next = (Object *)0;

}

Hit *Quadratic::intersection(Ray ray)
{
	float t0 = NULL;
	float t1 = NULL;
	Vertex intersect_point0, intersect_point1;
	// F(x, y, z) = Ax2 + By2 + Cz2 + Dxy + Exz + Fyz + Gx + Hy + Iz + J = 0
	// Get out constants, this makes it easier to work out Aq, Bq and Cq
	float x = ray.position.x;
	float y = ray.position.y;
	float z = ray.position.z;
	float x_d = ray.direction.x;
	float y_d = ray.direction.y;
	float z_d = ray.direction.z;

	// Work out Aq, Bq and Cq
	float Aq = (a * pow(x_d, 2)) + (b * pow(y_d,2)) + (c * pow(z_d, 2)) + (d * x_d * y_d) + (e * x_d * z_d) + (f * y_d * z_d);
	float Bq = 2 * (a * x * x_d) + 2 * (b * y * y_d) + 2 *  (c * z * z_d) + d * (x * y_d + y * x_d) + e * (x * z_d + z * x_d)
		+ f * (y * z_d + y_d * z) + g * x_d + h * y_d + i * z_d;
	float Cq = (a * pow(x,2)) + (b * pow(y,2)) + (c* pow(z,2)) + (d * x * y) + (e * x * z) + (f * y * z) + (g * x) + (h * y) 
		+ (i * z) + j;

	float discriminant = pow(Bq, 2) - 4 * Aq * Cq;
	// if discriminant <0 it never hits
	if(discriminant < 0){
		return 0;
	}

	// if Aq is 0 we don't want to divide by 0
	if (Aq == 0.0f) {
		t0 = -Cq / Bq;
		t1 = -Cq / Bq;
	}
	else {
		// otherwise use quadratic formula to work out t1 and t0
		t0 = (-Bq - pow(discriminant, 0.5)) / (2 * Aq);
		if (t0 <= 0) {
			t1 = (-Bq + pow(discriminant, 0.5)) / (2 * Aq);
		}
	}

	// Get first intersection
	intersect_point0 = ray.position + ray.direction * t0;
	Vector normal;
	// get normal of intercept
	float x_i = intersect_point0.x;
	float y_i = intersect_point0.y;
	float z_i = intersect_point0.z;
	normal.x = 2 * a * x_i + d * y_i + e * z_i * i + g;
	normal.y = 2 * b * y_i + d * x_i + f * z_i * i + h;
	normal.z = 2 * c * z_i + e * x_i + f * y_i * i + i;
	normal.normalise();

	// Add it to hit list
	Hit* hit1 = new Hit();
	hit1->t = t0;
	hit1->what = this;
	hit1->position = intersect_point0;
	hit1->normal = normal;

	// If hits a second time add that aswell
	if (t1 != NULL) {
		intersect_point1 = ray.position + ray.direction * t1;
		// get normal of intercept
		x_i = intersect_point1.x;
		y_i = intersect_point1.y;
		z_i = intersect_point1.z;
		normal.x = 2 * a * x_i + d * y_i + e * z_i * i + g;
		normal.y = 2 * b * y_i + d * x_i + f * z_i * i + h;
		normal.z = 2 * c * z_i + e * x_i + f * y_i * i + i;
		normal.normalise();

		// Add it to hit list
		Hit* hit2 = new Hit();
		hit2->entering = false;
		hit2->t = t1;
		hit2->what = this;
		hit2->position = intersect_point1;
		hit2->normal = normal;

		hit1->next = hit2;
	}

	return hit1;
}


void Quadratic::apply_transform(Transform& trans)
{
	// To be implemented
	Transform temp = Transform(
		a, d/2, e/2, g/2,
		d/2, b, f/2, h/2,
		e/2, f/2, c, i/2,
		g/2, h/2, i/2, j);
	trans.inverse();
	Transform temp2 = trans.transpose();

	temp = temp2*temp*trans;
	a = temp.matrix[0][0];
	b = temp.matrix[1][1] ;
	c = temp.matrix[2][2];
	d = temp.matrix[0][1] * 2;
	e = temp.matrix[0][2] * 2;
	f = temp.matrix[1][2] * 2;
	g = temp.matrix[0][3] * 2;
	h = temp.matrix[1][3] * 2;
	i = temp.matrix[2][3] * 2;
	j = temp.matrix[3][3];
}