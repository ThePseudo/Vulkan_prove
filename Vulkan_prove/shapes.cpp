#include "shapes.h"

template<class T>
inline Shape<T>::Shape()
{
}

template<class T>
inline Shape<T>::~Shape()
{
}


Triangle::Triangle(vec3 a, vec3 b, vec3 c)
{
	A = a;
	B = b;
	C = c;
}

vec3 Triangle::operator[](int i)
{
	assert(i < 3);
	return asVecArray()[i];
}

inline vec3* Triangle::asVecArray()
{
	return (vec3*)this;
}

Circle::Circle(float32_t x, float32_t y, float32_t z, float32_t r)
{
	center_radius = vec4(x, y, z, r);
}

vec3 Circle::center()
{
	return (vec3)center_radius;
}

float32_t Circle::radius()
{
	return center_radius[3];
}
