#pragma once

#include "./Worley-Noise/Worley Noise/Noise.h"

template<class T>
class Perlin3D : public Noise<T>
{
public:
  Perlin3D(uint res_x, uint res_y, uint res_z, uint scale_x, uint scale_y, uint scale_z);
private:
  uint scale_x, scale_y, scale_z;
  void gen_img() override;
};

