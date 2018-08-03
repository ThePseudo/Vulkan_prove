#pragma once
#include <iostream>

class Resolution
{
public:
	Resolution(int width = 0, int height = 0);
	~Resolution();
	uint32_t width;
	uint32_t height;
};

