#version 450

layout(std430, binding = 0) buffer StorageBuffer
{
    int foo;
} storageBuffer;

layout(binding = 1, rgba8) uniform image2D storageImage;

layout(vertices = 1) out;

void writeToBuffer()
{
    storageBuffer.foo = 0;
}

void writeToImage()
{
    imageStore(storageImage, ivec2(0, 0), vec4(0));
}

void main()
{
    gl_TessLevelInner[0] = 8.9;
    gl_TessLevelInner[1] = 6.9;
    gl_TessLevelOuter[0] = 8.9;
    gl_TessLevelOuter[1] = 6.9;
    gl_TessLevelOuter[2] = 3.9;
    gl_TessLevelOuter[3] = 4.9;
    writeToBuffer();
    writeToImage();
}
