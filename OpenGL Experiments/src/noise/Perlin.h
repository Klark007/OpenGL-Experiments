#pragma once

typedef unsigned int uint;

// uses glm perlin http://glm.g-truc.net/0.9.5/api/a00177.html
template<class T>
class Perlin
{
public:
  Perlin(uint res_x, uint res_y, uint scale_x, uint scale_y);
  ~Perlin();
private:
  T* img_data;
public:
  inline T* get_data() const;
};

template<class T>
inline T* Perlin<T>::get_data() const
{
  return img_data;
}