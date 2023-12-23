#pragma once

#include "./Worley-Noise/Worley Noise/Noise.h"

// uses glm perlin http://glm.g-truc.net/0.9.5/api/a00177.html
template<class T>
class Perlin : public Noise<T>
{
public:
  Perlin(uint res_x, uint res_y, uint scale_x, uint scale_y);
private:
  uint scale_x, scale_y;
  void gen_img() override;
};