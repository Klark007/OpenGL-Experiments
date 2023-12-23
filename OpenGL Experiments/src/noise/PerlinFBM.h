#pragma once

#include "Perlin.h"

template<class T>
class PerlinFBM : public Noise<T>
{
public: 
	PerlinFBM(uint res_x, uint res_y, uint scale_x, uint scale_y, uint nr_oct);
private:
	Perlin<T> p;

	uint scale_x, scale_y;
	uint nr_oct;

	void gen_img() override;
};

template<class T>
inline PerlinFBM<T>::PerlinFBM(uint res_x, uint res_y, uint scale_x, uint scale_y, uint nr_oct)
	: Noise<T>(res_x, res_y, 1), scale_x {scale_x}, scale_y{scale_y}, nr_oct{ nr_oct }, p {Perlin<T>(res_x,res_y,scale_x,scale_y)}
{
	gen_img();
}

template<class T>
inline void PerlinFBM<T>::gen_img()
{
	this->img_data = p.get_vec();
	this->scale(0.5);

	float a = 0.5;
	float f = 2.0;

	glm::vec2 o = glm::vec2(100, 100);

	for (uint i = 1; i < nr_oct; i++) {
		scale_x *= f;
		scale_y *= f;
		p.regenerate(scale_x, scale_y, o);
		
		p.scale(a);
		this->add(p.get_vec());
		
		o += glm::vec2(100, 100);
		a *= 0.5;
	}
}

template class PerlinFBM<float>;