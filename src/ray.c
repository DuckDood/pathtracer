#include <math.h>
#include <ray.h>
#include <stdio.h>

// https://raytracing.github.io/books/RayTracingInOneWeekend.html#rays,asimplecamera,andbackground/therayclass
float ray_sphere_intersection(ray r, vec3 center, float radius) { 
	vec3 oc = center - r.origin;
    float a = v3_dot(r.direction, r.direction);
    float b = -2.0 * v3_dot(r.direction, oc);
    float c = v3_dot(oc, oc) - radius*radius;
    float discriminant = b*b - 4*a*c;

    if (discriminant < 0) {
        return -1.0;
    } else {
        return (-b - sqrt(discriminant) ) / (2.0*a);
    }
}
