#version 140
#extension GL_ARB_compatibility: enable

in vec2 pixel_position;

uniform sampler2D tex_background; //16bit
uniform sampler2D tex_label; // 8bit


uniform vec2 window_size;

uniform vec2 mousePos;
uniform int brushSize;
uniform vec3 brushColor;

uniform vec2 imageStart;
uniform vec2 imageEnd;


uniform sampler2D color_table;


void main()
{

    vec2 p_tex=(pixel_position+vec2(1,1))*0.5;
    vec2 p_screen=p_tex*window_size;
    vec2 p_image=imageStart+(imageEnd-imageStart)*p_tex;

    float shape_dis=sqrt(pixel_position.x*pixel_position.x+pixel_position.y*pixel_position.y);

    if(shape_dis<1.0 && shape_dis>0.98){
        gl_FragColor=vec4(1,1,0,1);
    }
    else if(p_tex.x>=0 && p_tex.x<1 && p_tex.y>=0 && p_tex.y<1
            && shape_dis<1.0){
        float value1=texture2D(tex_background,p_tex).x;
        float value2=texture2D(tex_label,p_tex).x;


        float alpha=0;
        vec4 cur_color=vec4(0,0,0,0);

        float value1_y=float(int(value1*65535)/255)/255;
        float value1_x=float(int(value1*65535)%255)/255;
        value1=texture2D(color_table,vec2(value1_x,value1_y)).x;

        if(length(p_image-mousePos)<brushSize){
            cur_color=cur_color+vec4(brushColor,1)*0.5*(1-alpha);
            alpha=alpha+0.5*(1-alpha);
        }



        if(value2>0 && value2<0.6){
            cur_color=cur_color+vec4(1,0,0,1)*(1-alpha);
            alpha=alpha+(1-alpha);
        }
        else if(value2>=0.6){
            cur_color=cur_color+vec4(1,0,0,1)*0.2*(1-alpha);
            alpha=alpha+0.2*(1-alpha);
        }


        vec3 mitoColor=vec3(1,1,0)*(max(value1,0.5)-0.5)*2
                + vec3(0,0,1)*(1-(max(value1,0.5)-0.5)*2);
        if(value1<0.25){
            mitoColor=vec3(0,0,0)*(0.25-value1)*4 + vec3(0,0,1)*value1*4;
        }
        else if(value1<0.75){
            mitoColor=vec3(0,0,1)*(0.75-value1)*2 + vec3(1,1,0)*(value1-0.25)*2;
        }
        else mitoColor=vec3(1,1,0);

        cur_color=cur_color+vec4(mitoColor.r,mitoColor.g,mitoColor.b,1)*value1*(1-alpha);
        alpha=alpha+value1*(1-alpha);

        cur_color=cur_color+vec4(1,1,1,1)*(1-alpha);
        alpha=1;


        gl_FragColor=cur_color;
    }
    else{
        discard;
    }
}
