#pragma once

#include <glm/glm.hpp>
#include "./Worley-Noise/Worley Noise/Noise.h"

template<class T>
class Perlin3D : public Noise<T>
{
public:
  Perlin3D(uint res_x, uint res_y, uint res_z, uint scale_x, uint scale_y, uint scale_z);
private:
  uint scale_x, scale_y, scale_z;
  glm::vec3 offset;
  void gen_img() override;
public: 
  inline void regenerate(uint scale_x, uint scale_y, uint scale_z, const glm::vec3& off);
};

template<class T>
inline void Perlin3D<T>::regenerate(uint scale_x, uint scale_y, uint scale_z, const glm::vec3& off)
{
  this->scale_x = scale_x;
  this->scale_y = scale_y;
  this->scale_z = scale_z;
  offset = off;
  gen_img();
}
