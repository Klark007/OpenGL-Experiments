#include "Perlin3D.h"

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>

#include <numeric> // for iota
#include <execution> // for parallel for_each

template<class T>
inline Perlin3D<T>::Perlin3D(uint res_x, uint res_y, uint res_z, uint scale_x, uint scale_y, uint scale_z)
  : Noise<T>(res_x, res_y, res_z, 1), scale_x{ scale_x }, scale_y{ scale_y }, scale_z { scale_y }
{
  offset = glm::vec3(0,0,0);
  gen_img();
}

template<class T>
void Perlin3D<T>::gen_img()
{
  auto image_lambda = [&](uint z) {
    glm::vec3 position;
    const glm::vec3 tiling = glm::vec3(scale_x, scale_y, scale_z);

    for (uint y = 0; y < this->res_y; y++) {
      for (uint x = 0; x < this->res_x; x++) {
        position = glm::vec3(
          (float)x / this->res_x * scale_x,
          (float)y / this->res_y * scale_y,
          (float)z / this->res_z * scale_z
        );
        float perlin = (glm::perlin(position + offset, tiling) + 1.0) / 2;
        T value = this->distance_to_val(perlin);
        this->img_data.at(this->idx(x, y, z, 0)) = value;
      }
    }
  };

  std::vector<uint> z_range = std::vector<uint>(this->res_z);
  std::iota(z_range.begin(), z_range.end(), 0);

  std::for_each(
    std::execution::par,
    z_range.begin(),
    z_range.end(),
    image_lambda
  );

  /*
  for (uint z = 0; z < this->res_z; z++) {
    for (uint y = 0; y < this->res_y; y++) {
      for (uint x = 0; x < this->res_x; x++) {
        position = glm::vec3(
          (float)x / this->res_x * scale_x, 
          (float)y / this->res_y * scale_y,
          (float)z / this->res_z * scale_z
         );
        float perlin = (glm::perlin(position+offset, tiling) + 1.0) / 2;
        T value = this->distance_to_val(perlin);
        this->img_data.at(this->idx(x, y, z, 0)) = value;
      }
    }
  }
  */
  
}

template class Perlin3D<unsigned char>;
template class Perlin3D<float>;