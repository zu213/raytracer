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

#ifndef EPSILON
#define EPSILON 1e-4
#endif // !EPSILON

#define trumbone
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "polymesh_object.h"
#include "sphere_object.h"

using namespace std;


PolyMesh::PolyMesh(char* file, bool smooth)
{
	this->smooth = smooth;
    next = 0;
    triangle_count = 0;
	read_in_file(file);

}



Hit* PolyMesh::intersection(Ray ray)
{
    Hit* hits = 0;
	// Test bounding volume here
	Hit* bounding_sphere = sp->intersection(ray);
	if (bounding_sphere == 0) {
		// doesnt intersect bounding sphere
		return 0;
	}

	Vector u_normal, v_normal, w_normal;
	for (int i = 0; i < triangle_count; i++) {
		// iterate through all the triangles checking if they are hit
		int triangle_temp[3];
		for (int j = 0; j < 3; j++) {
			triangle_temp[j] = triangle[i][j];
		}
		int triangle_normal_temp[3];
		for (int k = 0; k < 3; k++) {
			triangle_normal_temp[k] = triangle_normals[i][k];
		}

		Vector p1 = vertex[triangle[i][0]];
		Vector p2 = vertex[triangle[i][1]];
		Vector p3 = vertex[triangle[i][2]];

		//Perform muller-trumbone for if defined
#ifdef trumbone
		Vector v1, v2, N, pvec, tvec, qvec;
		float t, det, idet, u, v;

		// get two plane vectors
		v1 = p2 - p1;
		v2 = p3 - p1;

		// work out determinant and inverse
		ray.direction.cross(v2, pvec);
		det = v1.dot(pvec);
		idet = 1 / det;

		// if the det is really small we skip
		if (fabsf(det) < EPSILON) {
			continue;
		}

		tvec = ray.position - p1;
		u = tvec.dot(pvec) * idet;

		if (u < 0 || u > 1) {
			continue;
		}

		tvec.cross(v1, qvec);
		v = ray.direction.dot(qvec) * idet;

		if (v < 0 || u + v > 1) {
			continue;
		}

		t = v2.dot(qvec) * idet;

		// create a hit
		Hit* hit = new Hit();
		hit->t = t;
		hit->what = this;
		hit->position = ray.position + ray.direction * t;
		if (smooth) {
			// if smoothing is on fetch the vertex normals, if they don't exist get the generated ones
			if (triangle_normal_temp[2] != -1) {
				u_normal = vertex_normals[triangle_normal_temp[1]];
			}
			else {
				u_normal = generated_vertex_normals[triangle[i][1]];
			}
			if (triangle_normal_temp[2] != -1) {
				v_normal = vertex_normals[triangle_normal_temp[2]];
			}
			else {
				v_normal = generated_vertex_normals[triangle[i][2]];
			}
			if (triangle_normal_temp[2] != -1) {
				w_normal = vertex_normals[triangle_normal_temp[0]];
			}
			else {
				w_normal = generated_vertex_normals[triangle[i][0]];
			}

			// calculate intersect normals from vertex normals
			Vector temp =
				u * u_normal +
				v * v_normal +
				(1-u-v) * w_normal;

			temp.normalise();

			hit->normal = temp;
		}
		else {
			// get normals by crossing two plane vectors
			v1.cross(v2, N);
			hit->normal = -N;
		}

		// add hit to list
		if (hits == 0) {
			hits = hit;
		}
		else {
			// redorder hits list if need be putting first hit at front of list
			if (hits->t > t) {
				Hit* temp_hits = hits;
				hits = hit;
				hits->next = temp_hits;

			}
			else {
				hits->next = hit;
			}
		}
#else
		// Normal raytracing
		float area, u_area, v_area, w_area, parallel_check, d, t;
		Vertex intersect_point;
		Vector N, a1, a2, a3;
		Vector v1 = p2 - p1;
		Vector v2 = p3 - p2;
		Vector v3 = p1 - p3;
		
		// calculate N
		v1.cross(v2, N);
		// Calculate area of triangle
		area = N.length() / 2;
		N.normalise();

		// check if the ray and plane are parallel.
		parallel_check = N.dot(ray.direction);
		if (parallel_check == 0) {
			continue;
		}

		// compute d parameter
		d = N.dot(p1);

		// compute t
		t = (d - N.dot(ray.position)) /
			(N.dot(ray.direction));

		//check if triangle is behind
		if (t < 0) {
			continue;
		}

		// compute intersect
		intersect_point = ray.position + ray.direction * t;

		// Check if it intersects by checking they all go the same direction and skipping if they don't
		a1 = intersect_point - p1;
		v1.cross(a1);
		u_area = (v1.length() / 2) /area;
		if (N.dot(v1) < 0) {
			continue;
		}
		a2 = intersect_point - p2;
		v2.cross(a2);
		v_area = (v2.length() / 2) / area;
		if (N.dot(v2) < 0) {
			continue;
		}
		a3 = intersect_point - p3;
		v3.cross(a3);
		w_area = (v3.length() / 2) / area;
		if (N.dot(v3) < 0) {
			continue;
		}

		// create a new hit
		Hit* hit = new Hit();
		hit->t = t;
		hit->what = this;
		hit->position = intersect_point;
		if (smooth) {
			// if smoothing is on fetch the vertex normals, if they don't exist get the generated ones
			if (triangle_normal_temp[2] != -1) {
				u_normal = vertex_normals[triangle_normal_temp[2]];
			}
			else {
				u_normal = generated_vertex_normals[triangle[i][2]];
			}
			if (triangle_normal_temp[2] != -1) {
				v_normal = vertex_normals[triangle_normal_temp[0]];
			}
			else {
				v_normal = generated_vertex_normals[triangle[i][0]];
			}
			if (triangle_normal_temp[2] != -1) {
				w_normal = vertex_normals[triangle_normal_temp[1]];
			}
			else {
				w_normal = generated_vertex_normals[triangle[i][1]];
			}

			// calculate intersect normals from vertex normals
			Vector temp = 
			u_area * u_normal +
			v_area * v_normal +
			w_area * w_normal;
								
			temp.normalise();
							
			hit->normal = temp;
		}
		else {
			hit->normal = -N;
		}

		// add hit to list
		if (hits == 0) {
			hits = hit;
		}
		else {
			// redorder hits list if need be putting first hit at front of list
			if (hits->t > t) {
				Hit* temp_hits = hits;
				hits = hit;
				hits->next = temp_hits;

			}
			else {
				hits->next = hit;
			}
		}

#endif
	}
    return hits;
}

void PolyMesh::apply_transform(Transform& trans)
{
	// create bounding volume here
	float max_x, max_y, max_z, min_x, min_y, min_z;
	max_x = max_y = max_z = -1e15;
	min_x = min_y = min_z = 1e15;
	// need centre + radius of bounding sphere
	for (int i = 0; i < vertex_count; i++) {
		// transform vertices
		trans.apply(vertex[i]);

		// bunch of ifs to get max and min of x,y,z
		if (vertex[i].x > max_x) {
			max_x = vertex[i].x;
		}
		if (vertex[i].y > max_y) {
			max_y = vertex[i].y;
		}
		if (vertex[i].z > max_z) {
			max_z = vertex[i].z;
		}
		if (vertex[i].x < min_x) {
			min_x = vertex[i].x;
		}
		if (vertex[i].y < min_y) {
			min_y = vertex[i].y;
		}
		if (vertex[i].z < min_z) {
			min_z = vertex[i].z;
		}

	}
	Vertex centre = Vertex((max_x + min_x) / 2, (max_y + min_y) / 2, (max_z + min_z) / 2);
	float radius = pow(( pow((max_x - centre.x), 2) + pow((max_y - centre.y), 2) + pow((max_z - centre.z), 2) ), 0.5);
	sp = new Sphere(centre, radius);
	sp->set_material(this->material);

	// transform normals
	for (int j = 0; j < vertex_normals_count; j++) {
		trans.apply(vertex_normals[j]);
		vertex_normals[j].normalise();
	}
}

void PolyMesh::read_in_file(string filename) {
	std::ifstream infile(filename);
	std::string line;
	while (std::getline(infile, line))
	{
		strip_file_info(line);
	}

	// Create lists based on counts
	vertex = (Vertex*)malloc(sizeof(Vertex) * vertex_count);
	vertex_normals = (Vector*)malloc(sizeof(Vector) * vertex_normals_count);
	vertex_textures = (Vertex*)malloc(sizeof(Vertex) * textures_count);
	generated_vertex_normals = (Vector*)malloc(sizeof(Vector) * vertex_count);
	triangle = (int**)malloc(sizeof(int[3]) * 3 * triangle_count);
	triangle_normals = (int**)malloc(sizeof(int[3]) * 3 * triangle_count);

	for (int i = 0; i < triangle_count; i++) {
		triangle[i] = new int[3];
		triangle_normals[i] = new int[3];
	}

	// reset counts and file
	vertex_count = 0;
	vertex_normals_count = 0;
	textures_count = 0;
	triangle_count = 0;
	infile.clear();
	infile.seekg(0);

	// loop through file again
	while (std::getline(infile, line))
	{
		int len = line.length() - 1;

		int gap1 = -1;
		int gap2 = -1;
		int gap3 = -1;
		
		// if the line is a vertex coord
		if (line[0] == 'v' && line[1] ==' ') {
			// extract spaces
			for (int i = 2; i < len; i++) {
				if (line[i] == ' ') {
					if (gap1 > 0) {
						gap2 = i;
					}
					else {
						gap1 = i;
					}
				}
			}
			// add sub strings to array
			if (gap1 > 0 && gap2 > 0) {
				vertex[vertex_count].x = std::stof(line.substr(2, gap1-2));
				vertex[vertex_count].y = std::stof(line.substr(gap1, gap2-gap1));
				vertex[vertex_count].z = std::stof(line.substr(gap2, len-gap2));
				vertex[vertex_count].w = 1;

				// create a generates vertex in case
				generated_vertex_normals[vertex_count] = Vector(0, 0, 0);
				vertex_count++;

			}
			else {
				throw("Invalid vertex details in .obj file");
			}
		}

		// if the line is a vertex normal
		else if (line[0] == 'v' && line[1] == 'n') {
			// extract spaces
			for (int i =3; i < len; i++) {
				if (line[i] == ' ') {
					if (gap1 > 0) {
						gap2 = i;
					}
					else {
						gap1 = i;
					}
				}
			}
			// add sub strings to array
			if (gap1 > 0 && gap2 > 0) {
				vertex_normals[vertex_normals_count].x = std::stof(line.substr(2, gap1 - 2));
				vertex_normals[vertex_normals_count].y = std::stof(line.substr(gap1, gap2 - gap1));
				vertex_normals[vertex_normals_count].z = std::stof(line.substr(gap2, len - gap2));
				vertex_normals_count++;

			}
			else {
				throw("Invalid vertex normal details in .obj file");
			}
		}

		// if the line is vertex texture
		else if (line[0] == 'v' && line[1] == 't') {
			// to be implemented
		}

		// if the line is  face
		else if (line[0] == 'f') {
			// extract spaces
			for (int i = 2; i < len; i++) {
				if (line[i] == ' ') {
					if (gap2 > 0 && gap3 < 0) {
						gap3 = i;
					}
					if (gap1 > 0 && gap2 < 0) {
						gap2 = i;
					}
					if(gap1 < 0){
						gap1 = i;
					}
				}
			}

			if (gap1 > 0 && gap2 > 0) {
				int * vertex1 = extract_face_info(line.substr(2, gap1 - 2));
				int * vertex2 = extract_face_info(line.substr(gap1, gap2 - gap1));
				int * vertex3 = extract_face_info(line.substr(gap2, gap3 - gap2));

				// generate normals in case they aren't in the file
				Vector v1 = vertex[vertex2[0]] - vertex[vertex1[0]];
				Vector v2 = vertex[vertex3[0]] - vertex[vertex1[0]];
				v1.cross(v2);
				v1.normalise();
				v1.negate();
				// by doing this we can normalise later and it'll average the planes normals from around a vertex
				generated_vertex_normals[vertex1[0]] += v1;
				generated_vertex_normals[vertex2[0]] += v1;
				generated_vertex_normals[vertex3[0]] += v1;

				// add vertices
				triangle[triangle_count][0] = vertex1[0];
				triangle[triangle_count][1] = vertex2[0];
				triangle[triangle_count][2] = vertex3[0];

				// add normals
				triangle_normals[triangle_count][0] = vertex1[2];
				triangle_normals[triangle_count][1] = vertex2[2];
				triangle_normals[triangle_count][2] = vertex3[2];
				triangle_count++;

				// if there is a third gap its a polygon and needs to be split into 2
				if (gap3 > 0) {
					int * vertex4 = extract_face_info(line.substr(gap3, len - gap3));
					// add second traingle vertices
					triangle[triangle_count][0] = vertex1[0];
					triangle[triangle_count][1] = vertex3[0];
					triangle[triangle_count][2] = vertex4[0];

					// add fourth generated normal if polygon
					generated_vertex_normals[vertex4[0]] += v1;

					//add second triangle normals
					triangle_normals[triangle_count][0] = vertex1[2];
					triangle_normals[triangle_count][1] = vertex3[2];
					triangle_normals[triangle_count][2] = vertex4[2];
					triangle_count ++;
				}
			}
			else {
				throw("Invalid face details in .obj file");
			}

		}

	}
	// loop through at the end and normalise the generated normals
	for (int i = 0; i < vertex_count; i++) {
		generated_vertex_normals[i].normalise();
	}

	return;
}

int* PolyMesh::extract_face_info(string face)
{
	int len = face.length() - 1;

	int slash1 = -1;
	int slash2 = -1;

	// extract the /'s 
	for (int i = 0; i < len; i++) {
		if (face[i] == '/') {
			if (slash1 > 0) {
				slash2 = i;
			}
			else {
				slash1 = i;
			}
		}
	}
	// return a array [vertex, texture, normal]
	int* returnList = new int[3];
	returnList[0] = std::stoi(face.substr(0, slash1)) - 1;
	if (slash2 != -1) {
		returnList[1] = std::stoi(face.substr(slash1 + 1, slash2 - slash1 - 1)) - 1;
		returnList[2] = std::stoi(face.substr(slash2 + 1, len - slash2)) - 1;
	}
	//if there is no vertex normal return -1 this will be pickedmup later and the generated normals used.
	else {
		returnList[1] = std::stoi(face.substr(slash1 + 1, len - slash1 - 1)) - 1;
		returnList[2] = -1;
	}
	
	return returnList;
}

void PolyMesh::strip_file_info(std::string line)
{
	int gap;
	int vPos;
	// Strip information to create lists
	// Get vertices count
	if (line[0] == '#') {
		vPos = line.find("vertices");
		if (vPos != -1) {
			gap = vPos - 2;
			while (true) {
				if (line[gap] == ' ') {
					gap++;
					break;
				}
				else {
					gap--;
				}
			}
			vertex_count = std::stoi(line.substr(gap, vPos - gap));
		}

		// Get vertex normals count
		vPos = line.find("vertex normals");
		if (vPos != -1) {
			gap = vPos - 2;
			while (true) {
				if (line[gap] == ' ') {
					gap++;
					break;
				}
				else {
					gap--;
				}
			}
			vertex_normals_count = std::stoi(line.substr(gap, vPos - gap));
		}

		// Get texture count
		vPos = line.find("texture coords");
		if (vPos != -1) {
			gap = vPos - 2;
			while (true) {
				if (line[gap] == ' ') {
					gap++;
					break;
				}
				else {
					gap--;
				}
			}
			textures_count += (2 * std::stoi(line.substr(gap, vPos - gap)));
		}

		// Get triangles count
		vPos = line.find("triangles");
		if (vPos != -1) {
			gap = vPos - 2;
			while (true) {
				if (line[gap] == ' ') {
					gap++;
					break;
				}
				else {
					gap--;
				}
			}
			triangle_count += std::stoi(line.substr(gap, vPos - gap));
		}

		// Get polygons count
		vPos = line.find("polygons");
		if (vPos != -1) {
			gap = vPos - 2;
			while (true) {
				if (line[gap] == ' ') {
					gap++;
					break;
				}
				else {
					gap--;
				}
			}
			triangle_count += (2 * std::stoi(line.substr(gap, vPos - gap)));
		}

	}
}

