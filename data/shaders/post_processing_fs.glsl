#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;
uniform int render_style;
uniform bool render_outlines;
uniform sampler2D screenTexture;

/*-- RENDER STYLES -----------*/
#define RENDER_NORMAL 0
#define RENDER_NEGATIVE 1
#define RENDER_COLOR_SWAP_1 2
#define RENDER_COLOR_SWAP_2 3
#define RENDER_COLOR_SWAP_3 4
#define RENDER_GRAY_SCALE 5
/*----------------------------*/

const vec4 edge_color = vec4(0.0, 0.0, 0.0, 1.0);

const float offsetx = 1.0 / 1920.0;
const float offsety = 1.0 / 1080.0;
const vec2 offsets[9] = vec2[](
    vec2(-offsetx,  offsety), // top-left
    vec2( 0.0f,    offsety), // top-center
    vec2( offsetx,  offsety), // top-right
    vec2(-offsetx,  0.0f),   // center-left
    vec2( 0.0f,    0.0f),   // center-center
    vec2( offsetx,  0.0f),   // center-right
    vec2(-offsetx, -offsety), // bottom-left
    vec2( 0.0f,   -offsety), // bottom-center
    vec2( offsetx, -offsety)  // bottom-right    
);

void main()
{ 
    vec4 col = texture(screenTexture, TexCoords);
    
    if (render_style == RENDER_NEGATIVE)
    {
        FragColor = vec4(vec3(1.0 - col), 1.0); //Invert Colors
    }
    else if (render_style == RENDER_COLOR_SWAP_1)
    {
        FragColor = vec4(vec3(col.g, col.r, col.b), 1.0);// Swap colors
    }
    else if (render_style == RENDER_COLOR_SWAP_2)
    {
        FragColor = vec4(vec3(col.b, col.g, col.r), 1.0);// Swap colors
    }
    else if (render_style == RENDER_COLOR_SWAP_3)
    {
        FragColor = vec4(vec3(col.r, col.b, col.g), 1.0);// Swap colors
    }
    else if (render_style == RENDER_GRAY_SCALE)
    {
        float average = 0.2126 * col.r + 0.7152 * col.g + 0.0722 * col.b; //Gray Scale
        FragColor = vec4(average, average, average, 1.0); //Gray Scale
    }
    else
    {
        FragColor = col; //normal
    }


    if (render_outlines)
    {
        //Kernel filters
    

        // Sharpening
        float kernel[9] = float[](
            -1/8.0, -0, 1/8.0,
            -2/8.0,  0, 2/8.00,
            -1/8.0, -0, 1/8.0
        );

        vec3 sampleTex[9];
        for(int i = 0; i < 9; i++)
        {
            sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
        }
        vec3 edge_strength = vec3(0.0);
        for(int i = 0; i < 9; i++)
            edge_strength += sampleTex[i] * kernel[i];

        if(length(edge_strength) > 0.1)
        {
            FragColor = edge_color;
        }

    
        //Gaussian blur
    //    float kernel[9] = float[](
    //    1.0 / 16, 2.0 / 16, 1.0 / 16,
    //    2.0 / 16, 4.0 / 16, 2.0 / 16,
    //    1.0 / 16, 2.0 / 16, 1.0 / 16  
    //    );

    }

}