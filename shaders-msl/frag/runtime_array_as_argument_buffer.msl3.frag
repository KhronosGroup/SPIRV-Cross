#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in  flat uint inputId;

layout(binding = 0) uniform sampler2D textures[];
layout(binding = 1, std430) readonly buffer Ssbo { uint val;  } ssbo[];
layout(binding = 2, std140) uniform         Ubo  { uint val;  } ubo[];

void implicit_texture() {
  vec4 d = textureLod(textures[nonuniformEXT(inputId)],vec2(0,0),0);
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

void explicit_texture(in sampler2D tex) {
  vec4 d = textureLod(tex,vec2(0,0),0);
  if(d.a>0.5)
    discard;
  }

void main() {
  implicit_texture();
  implicit_ssbo();
  implicit_ubo();

  explicit_texture(textures[nonuniformEXT(inputId)]);
  }
