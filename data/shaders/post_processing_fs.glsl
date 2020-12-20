#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;



void main()
{ 
    //vec4 col = texture(screenTexture, TexCoords);
    
    //FragColor = col; //normal
    
    //FragColor = vec4(vec3(1.0 - col), 1.0); //Invert Colors

    
    //FragColor = vec4(vec3(col.g, col.r, col.b), 1.0);// Swap colors
    //FragColor = vec4(vec3(col.b, col.g, col.r), 1.0);// Swap colors
    //FragColor = vec4(vec3(col.r, col.b, col.g), 1.0);// Swap colors


    //float average = 0.2126 * col.r + 0.7152 * col.g + 0.0722 * col.b; //Gray Scale
    //FragColor = vec4(average, average, average, 1.0); //Gray Scale


    //Kernel filters
    float offsetx = 1.0 / 1920.0;
    float offsety = 1.0 / 1080.0;
    vec2 offsets[9] = vec2[](
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

//     Sharpening
    float kernel[9] = float[](
        -1/8.0, -0, 1/8.0,
        -2/8.0,  0, 2/8.00,
        -1/8.0, -0, 1/8.0
    );
    

    //Gaussian blur
//    float kernel[9] = float[](
//    1.0 / 16, 2.0 / 16, 1.0 / 16,
//    2.0 / 16, 4.0 / 16, 2.0 / 16,
//    1.0 / 16, 2.0 / 16, 1.0 / 16  
//    );

    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];

    if(length(col) > 0.1)
    {
        col = vec3(0, 0, 0);
        FragColor = vec4(col, 1.0);
    }
    else
    {
        FragColor = texture(screenTexture, TexCoords.st);
    }
    
    


}