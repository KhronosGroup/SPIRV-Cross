#version 330
uniform params {
    mat4 mvp;
    float psize;
};

in vec4 position;
in vec4 color0;
out vec4 color;

void main() {
    gl_Position = mvp * position;
    gl_PointSize = psize;
    color = color0;
}
