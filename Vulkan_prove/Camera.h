#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

class Camera
{
public:
	Camera();
	~Camera();

	inline void calculateMVP();

	//setters
	void projection(mat4 Projection);
	void view(mat4 View);
	void model(mat4 Model);
	void clip(mat4 Clip);
	inline void MVP(mat4 mvp) { _MVP = mvp; }

	//getters
	inline mat4 projection() { return _projection; }
	inline mat4 view() { return _view; }
	inline mat4 model() { return _model; }
	inline mat4 clip() { return _clip; }
	inline mat4 MVP() { return _MVP; }
private:

	mat4			_projection;	// from camera coordinates to homogeneous coordinates (from camera frustum to camera cube)
	mat4			_view;			// from world to camera coordinates
	mat4			_model;			// from model to world coordinates
	mat4			_clip;
	mat4			_MVP;			// Model - View - Projection
};

