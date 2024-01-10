#pragma once

#include <glm/glm.hpp>
#include "./Worley-Noise/Worley Noise/Noise.h"

// uses glm perlin http://glm.g-truc.net/0.9.5/api/a00177.html
template<class T>
class Perlin : public Noise<T>
{
public:
  Perlin(uint res_x, uint res_y, uint scale_x, uint scale_y);
private:
  uint scale_x, scale_y;
  glm::vec2 offset;
  void gen_img() override;
public:
	inline void regenerate(uint scale_x, uint scale_y, const glm::vec2& off);
};

template<class T>
inline void Perlin<T>::regenerate(uint scale_x, uint scale_y, const glm::vec2& off)
{
	this->scale_x = scale_x;
	this->scale_y = scale_y;
	offset = off;
	gen_img();
}