#version 330 core
in vec2 TexCoords;
uniform sampler2D uTextureSampler;

void main()
{
    /*
    if(texture(uTextureSampler, TexCoords).a < 0.5) {
        discard;
    }
    */
}
