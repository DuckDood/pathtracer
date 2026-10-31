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
