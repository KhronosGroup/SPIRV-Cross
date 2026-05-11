#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) flat in mediump uint src;
layout(location = 0) out mediump uint dst_mp;
layout(location = 1) out uint dst_hp;

void main()
{
    mediump uint copy_mp = src;
    uint copy_hp = src;
    dst_mp = copy_mp;
    dst_hp = copy_hp << 16u;
}

