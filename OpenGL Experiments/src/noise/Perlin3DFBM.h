#pragma once

#include "Perlin3D.h"

template<class T>
class Perlin3DFBM : public Noise<T>
{
public:
  Perlin3DFBM(uint res_x, uint res_y, uint res_z, uint scale_x, uint scale_y, uint scale_z, uint nr_oct);
private:
  Perlin3D<T> p;

  uint scale_x, scale_y, scale_z;
  uint nr_oct;

  void gen_img() override;
};

template<class T>
inline Perlin3DFBM<T>::Perlin3DFBM(uint res_x, uint res_y, uint res_z, uint scale_x, uint scale_y, uint scale_z, uint nr_oct)
  : Noise<T>(res_x, res_y, res_z, 1), scale_x{ scale_x }, scale_y{ scale_y }, scale_z{ scale_z }, nr_oct{nr_oct}, p {Perlin3D<T>(res_x, res_y, res_z, scale_x, scale_y, scale_z)}
{
  gen_img();
}

template<class T>
inline void Perlin3DFBM<T>::gen_img()
{
	this->img_data = p.get_vec();
	this->scale(0.5);

	float a = 0.5;
	float f = 2.0;

	glm::vec3 o = glm::vec3(100, 100, 100);

	for (uint i = 1; i < nr_oct; i++) {
		scale_x *= f;
		scale_y *= f;
		scale_z *= f;
		p.regenerate(scale_x, scale_y, scale_z, o);

		p.scale(a);
		this->add(p.get_vec());
		
		o += glm::vec3(100, 100, 100);
		a *= 0.5;
	}
}

template class Perlin3DFBM<float>;