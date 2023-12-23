#include "Perlin.h"

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>

template<class T>
Perlin<T>::Perlin(uint res_x, uint res_y, uint scale_x, uint scale_y)
  : Noise<T>(res_x, res_y, 1), scale_x {scale_x}, scale_y {scale_y}
{
    offset = glm::vec2(0, 0);
  gen_img();
}

template<class T>
void Perlin<T>::gen_img()
{
  glm::vec2 position;

  const glm::vec2 tiling = glm::vec2(scale_x, scale_y);

  for (uint y = 0; y < this->res_y; y++) {
    for (uint x = 0; x < this->res_x; x++) {
      position = glm::vec2((float)x / this->res_x * scale_x, (float)y / this->res_y * scale_y);
      float perlin = (glm::perlin(position+offset, tiling) + 1.0) / 2;
      T value = this->distance_to_val(perlin);
      this->img_data.at(this->idx(x, y, 0)) = value;
    }
  }
}

template class Perlin<unsigned char>;
template class Perlin<float>;