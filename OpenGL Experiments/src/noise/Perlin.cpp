#include "Perlin.h"

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>

#include <iostream>

template<class T>
Perlin<T>::Perlin(uint res_x, uint res_y, uint scale_x, uint scale_y)
{
  img_data = new T[res_x * res_y];

  glm::vec2 position;

  const glm::vec2 tiling = glm::vec2(scale_x,scale_y);

  std::cout << "Start perlin generation" << std::endl;

  for (uint y = 0; y < res_y; y++) {
    for (uint x = 0; x < res_x; x++) {
      position = glm::vec2((float) x / res_x * scale_x, (float) y / res_y * scale_y);
      float perlin = (glm::perlin(position, tiling)+1.0)/2;
      T value = static_cast <T> (perlin * std::numeric_limits<T>::max());
      img_data[y * res_x + x] = value;
    }
  }

  std::cout << "Finished perlin generation" << std::endl;
}

template<class T>
Perlin<T>::~Perlin()
{
  delete[] img_data;
}

template class Perlin<unsigned char>;