#include <float.h>
#include <math.h>
#include <ray.h>
#include <shapes.h>

// https://raytracing.github.io/books/RayTracingInOneWeekend.html#rays,asimplecamera,andbackground/therayclass
hitinfo ray_sphere_intersection(ray r, sphere s) {
	vec3 oc = s.position - r.origin;
    float a = v3_dot(r.direction, r.direction);
    float b = -2.0 * v3_dot(r.direction, oc);
    float c = v3_dot(oc, oc) - s.radius*s.radius;
    float discriminant = b*b - 4*a*c;

	hitinfo out;
    if (discriminant < 0) {
        out.hit = 0;
    } else {
        out.distance =  (-b - sqrt(discriminant) ) / (2.0*a);
		out.normal = v3_norm(r.direction * out.distance + r.origin - s.position);
		out.position = r.direction * out.distance + r.origin;
		out.hit = 1;
    }
    out.hit *= out.distance > 0;
	return out;
}

// https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
hitinfo ray_tri_intersection(ray r, tri tri) {
	hitinfo out;
	vec3 edge1 = tri.vertex_b - tri.vertex_a;
	vec3 edge2 = tri.vertex_c - tri.vertex_a;

	vec3 normal = v3_cross(edge1, edge2);

	if(v3_dot(normal, r.direction) > 0) {
		out.hit = 0;
		return out;
	}

	vec3 ray_cross_e2 = v3_cross(r.direction, edge2);

	float det = v3_dot(edge1, ray_cross_e2);

	if(fabs(det) < FLT_EPSILON) {
		out.hit = 0;
		return out;
	}

	float inv_det = 1/det;

	vec3 s = r.origin - tri.vertex_a;

	float u = inv_det * v3_dot(s, ray_cross_e2);

	if(u < -FLT_EPSILON || u-1 > FLT_EPSILON) {
		out.hit = 0;
		return out;
	}

	vec3 s_cross_e1 = v3_cross(s, edge1);

	float v = inv_det * v3_dot(r.direction, s_cross_e1);

	if (v < -FLT_EPSILON || u + v - 1 > FLT_EPSILON) {
		out.hit = 0;
		return out;
	}
	
	float t = inv_det * v3_dot(edge2, s_cross_e1);

	if(t > FLT_EPSILON) {
		out.hit = 1;
		out.normal = v3_norm(normal);
		out.distance = t;
		out.position = r.direction * t + r.origin;
		return out;
	}

	out.hit = 0;
	return out;
}
