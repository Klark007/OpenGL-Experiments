#pragma once

#include "Program.h"
#include <string>
#include <memory>

enum Texture_Filter
{
  linear=GL_LINEAR,
  nearest=GL_NEAREST,
  linear_mipmap=GL_LINEAR_MIPMAP_LINEAR,
  nearest_mipmap=GL_NEAREST_MIPMAP_NEAREST,
};
enum Texture_Wrap
{
  repeat=GL_REPEAT,
  clamp=GL_CLAMP_TO_BORDER,
};

class Texture
{
public:
  Texture(const std::string& name, unsigned int res_x, unsigned int res_y, const float* data, int internal_format, int format, Texture_Filter filter, Texture_Wrap wrap, bool mip_map=false);
  Texture(const std::string& name, unsigned int res_x, unsigned int res_y, const unsigned char* data, int internal_format, int format, Texture_Filter filter, Texture_Wrap wrap, bool mip_map=false);

  Texture(const std::string& name, unsigned int res_x, unsigned int res_y, unsigned int res_z, const float* data, int internal_format, int format, Texture_Filter filter, Texture_Wrap wrap);
  Texture(const std::string& name, unsigned int res_x, unsigned int res_y, unsigned int res_z, const unsigned char* data, int internal_format, int format, Texture_Filter filter, Texture_Wrap wrap);

  ~Texture();
private:
  unsigned int id;
  std::string name;

  GLenum type;
  int texture_unit;
public:
  // assuming program is current program
  void set_texture_unit(Program& program, int unit);
  void bind(); 
  void bind_texture_unit(Program& program, int unit);
};

// handle them in unique_ptr to avoid deconstuctor being called unnecessarily
typedef std::unique_ptr<Texture> TextureP;