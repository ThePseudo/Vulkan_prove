#include "Camera.h"



Camera::Camera()
{
}


Camera::~Camera()
{
}

inline void Camera::calculateMVP()
{
	_MVP = _clip * _projection * _view * _model;
}

//setters

void Camera::projection(mat4 Projection) 
{
	_projection = Projection;
	calculateMVP();
}

void Camera::view(mat4 View) 
{
	_view = View;
	calculateMVP();
}

void Camera::model(mat4 Model) 
{ 
	_model = Model;
	calculateMVP();
}

void Camera::clip(mat4 Clip) 
{ 
	_clip = Clip; 
	calculateMVP();
}
