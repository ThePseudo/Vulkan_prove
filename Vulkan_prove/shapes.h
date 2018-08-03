#pragma once

/*
Shapes implementation
*/

#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/geometric.hpp>


using namespace glm;

template <class T>
class Shape
{
public:
	Shape();
	~Shape();
};

class Triangle : public Shape<Triangle>
{
public:
	Triangle(vec3 a = vec3(0.0f,0.0f,0.0f), 
		vec3 b = vec3(0.0f, 0.0f, 0.0f),
		vec3 c = vec3(0.0f, 0.0f, 0.0f));
	vec3 operator[](int i);
private:
	inline vec3* asVecArray();
	vec3 A;
	vec3 B;
	vec3 C;
};

class Circle : public Shape<Circle>
{
public:
	Circle(float32_t x = 0.0f, float32_t y = 0.0f,
		float32_t z = 0.0f, float32_t r = 0.0f);
	inline vec3 center();
	inline float32_t radius();
private:
	vec4 center_radius;
};