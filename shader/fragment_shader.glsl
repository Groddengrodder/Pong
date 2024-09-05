#version 330 core

in vec4 input_color;
out vec4 color;

void main(){
   color = vec4(input_color.r, input_color.g, input_color.b, 1.0);
}
