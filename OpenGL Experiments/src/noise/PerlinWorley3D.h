#pragma once

#include "Perlin3DFBM.h"
#include "Worley-Noise/Worley Noise/Worley3DFBM.h"

#define NR_OCT 4

template<class T>
class PerlinWorley3D : public Noise<T> {
public:
  PerlinWorley3D(uint res_x, uint res_y, uint res_z, uint scale_x, uint scale_y, uint scale_z, std::vector<std::tuple<uint, uint, uint> > grid_res);

  Worley3DFBM<T> w;
  Perlin3DFBM<T> p;
private:
  void gen_img() override;
  static inline T combine(T perlin_value, T worley_value, float worley_weight);
};

template<class T>
inline PerlinWorley3D<T>::PerlinWorley3D(uint res_x, uint res_y, uint res_z, uint scale_x, uint scale_y, uint scale_z, std::vector<std::tuple<uint, uint, uint>> grid_res)
  : Noise<T>(res_x, res_y, res_z, WORLEY_NR_CHANNELS), w {Worley3DFBM<T>(res_x,res_y,res_z, grid_res,NR_OCT-1)}, p {Perlin3DFBM<T>(res_x,res_y,res_z,scale_x,scale_y,scale_z,NR_OCT)}
{
  gen_img();
}

template<class T>
void PerlinWorley3D<T>::gen_img() {
  // change first channel into perlin-worley
  p.normalize();

  w.invert();
  w.normalize();

  p.normalize();

  this->img_data = w.get_vec();

  std::vector<T> perlin_data = p.get_vec();

  uint c = 0;
  for (uint z = 0; z < this->res_z; z++) {
    for (uint y = 0; y < this->res_y; y++) {
      for (uint x = 0; x < this->res_x; x++) {
        T perlin_value = perlin_data.at(p.idx(x, y, z, c));
        T worley_value = this->img_data.at(w.idx(x, y, z, c));

        this->img_data.at(w.idx(x,y,z,c)) = PerlinWorley3D<T>::combine(perlin_value, worley_value, 0.15);
      }
    }
  }
}

// according to https://github.com/sixthsurge/volume-noise-generator/blob/main/src/main.cpp#L21
template<class T>
inline T PerlinWorley3D<T>::combine(T perlin_value, T worley_value, float worley_weight)
{
  float curve = 0.75;
  if (worley_weight < 0.5) {
    worley_weight *= 2;
    T noise = perlin_value + worley_value * worley_weight;
    return noise * glm::mix(1.0, 0.5, glm::pow(worley_weight, curve));
  }
  else {
    worley_weight = (worley_weight - 0.5) * 2;
    T noise = worley_value + perlin_value * (1.0 - worley_weight);
    return noise * glm::mix(0.5, 1.0, glm::pow(worley_weight, 1.0 / curve));
  }
  
}

template class PerlinWorley3D<float>;