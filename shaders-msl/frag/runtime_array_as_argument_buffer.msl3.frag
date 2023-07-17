#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_shader_image_load_formatted : enable

layout(location = 0) in  flat uint inputId;

layout(binding = 0) uniform sampler2D smp_textures[];
layout(binding = 1) uniform sampler   smp[];
layout(binding = 2) uniform texture2D textures[];
layout(binding = 3, std430) readonly buffer Ssbo { uint val;  } ssbo[];
layout(binding = 4, std140) uniform         Ubo  { uint val;  } ubo[];
layout(binding = 5) uniform image2D images[];

void implicit_combined_texture() {
  vec4 d = textureLod(smp_textures[nonuniformEXT(inputId)],vec2(0,0),0);
  if(d.a>0.5)
    discard;
  }

void implicit_texture() {
  vec4 d = textureLod(sampler2D(textures[nonuniformEXT(inputId)], smp[nonuniformEXT(inputId+8)]),vec2(0,0),0);
  if(d.a>0.5)
    discard;
  }

void implicit_ssbo() {
  if(ssbo[nonuniformEXT(inputId)].val==2)
    discard;
  }

void implicit_ubo() {
  if(ubo[nonuniformEXT(inputId)].val==2)
    discard;
  }

void implicit_image() {
  vec4 d = imageLoad(images[nonuniformEXT(inputId)],ivec2(0,0));
  if(d.a>0.5)
    discard;
  }

void explicit_comb_texture(in sampler2D tex) {
  vec4 d = textureLod(tex,vec2(0,0),0);
  if(d.a>0.5)
    discard;
  }

void explicit_texture(in texture2D tex, in sampler smp) {
  vec4 d = textureLod(sampler2D(tex,smp),vec2(0,0),0);
  if(d.a>0.5)
    discard;
  }

void explicit_image(in image2D tex) {
  vec4 d = imageLoad(tex,ivec2(0,0));
  if(d.a>0.5)
    discard;
  }

void main() {
  implicit_combined_texture();
  implicit_texture();
  implicit_ssbo();
  implicit_ubo();
  implicit_image();

  explicit_comb_texture(smp_textures[nonuniformEXT(inputId)]);
  explicit_texture(textures[nonuniformEXT(inputId)], smp[nonuniformEXT(inputId)]);
  explicit_image(images[nonuniformEXT(inputId)]);
  }
