#version 140
#extension GL_ARB_compatibility: enable

in vec2 pixel_position;

uniform sampler2D tex_neuron; //16bit
uniform sampler2D tex_structure; // rgba
uniform sampler2D tex_mito; //16bit
uniform sampler2D tex_mitoLabel; //rgba
uniform sampler2D tex_correction; //rgba
uniform sampler2D tex_subset; //rgba
uniform sampler2D tex_connection; // rgba
uniform sampler2D tex_mitoLabel_original; //8bit


uniform sampler2D color_table_neuron;
uniform sampler2D color_table_mito;

uniform vec4 background_color;

uniform vec2 shift;

uniform vec2 window_size;
uniform float scale;

uniform vec2 imageSize;

uniform float mito_thresh;
uniform float mito_alpha;

uniform vec2 mousePos;
uniform int brushSize;
uniform vec3 brushColor;

uniform int structure_prev;
uniform float structure_thresh;

uniform int thresh_control;

uniform bool grayscale;

uniform int work_type;

void main()
{

    vec2 p_screen=(pixel_position+vec2(1,1))*0.5*window_size;
    vec2 p_render=p_screen-(window_size-imageSize*scale)/2.0-shift*scale;
    vec2 p_tex=p_render/(imageSize*scale);
    vec2 p_image=p_render/scale;

    if(p_tex.x>=0 && p_tex.x<1 && p_tex.y>=0 && p_tex.y<1){
        float value1=texture2D(tex_neuron,p_tex).x;
        float value2=texture2D(tex_mito,p_tex).x;
        //value2=sqrt(value2);
        vec4 value3=texture2D(tex_structure,p_tex);
        vec4 value4=texture2D(tex_mitoLabel,p_tex);
        vec4 value5=texture2D(tex_correction,p_tex);

        vec4 value6=texture2D(tex_subset,p_tex);

        vec4 value7=texture2D(tex_connection,p_tex);

        float value8=texture2D(tex_mitoLabel_original,p_tex).x;


        float alpha=0;
        vec4 cur_color=vec4(0,0,0,0);

        float value1_y=float(int(value1*65535)/255)/255;
        float value1_x=float(int(value1*65535)%255)/255;
        value1=texture2D(color_table_neuron,vec2(value1_x,value1_y)).x;


        float value2_y=float(int(value2*65535)/255)/255;
        float value2_x=float(int(value2*65535)%255)/255;
        value2=texture2D(color_table_mito,vec2(value2_x,value2_y)).x;


//        if(thresh_control==1){
//            if(value1<structure_thresh){
//                cur_color=vec4(1,0,0,0.3);
//                alpha=0.3;
//            }
//            else{
//                cur_color=vec4(0,0,0.5,0.3);
//                alpha=0.3;
//            }
//        }
//        else if(thresh_control==2){
//            if(value2<structure_thresh){
//                cur_color=vec4(1,0,0,0.3);
//                alpha=0.3;
//            }
//            else{
//                cur_color=vec4(0,0,0.5,0.3);
//                alpha=0.3;
//            }
//        }

//        if(structure_prev==1){
//            if(length(p_image-mousePos)<brushSize){
//                if(value1>=structure_thresh){
//                    cur_color=cur_color+vec4(brushColor,1)*0.7*(1-alpha);
//                    alpha=alpha+0.7*(1-alpha);
//                }
//            }
//        }
//        else if(structure_prev==2){
//            if(length(p_image-mousePos)<brushSize){
//                if(value1<structure_thresh){
//                    cur_color=cur_color+vec4(brushColor,1)*0.7*(1-alpha);
//                    alpha=alpha+0.7*(1-alpha);
//                }
//            }
//        }
//        else{
        if(length(p_image-mousePos)<brushSize){
            cur_color=cur_color+vec4(brushColor,1)*0.5*(1-alpha);
            alpha=alpha+0.5*(1-alpha);
        }
//        }

        cur_color=cur_color+value6*value6.w*(1-alpha);
        alpha=alpha+value6.w*(1-alpha);

        cur_color=cur_color+value7*value7.w*(1-alpha);
        alpha=alpha+value7.w*(1-alpha);





//        if(value4.x>0){
//            cur_color=cur_color+vec4(1,0,0,1)*mito_alpha*(1-alpha);
//            alpha=alpha+mito_alpha*(1-alpha);
//        }

        if(work_type!=3){

            if(thresh_control==0){
                if(value4.w>0){
                    cur_color=cur_color+value4*value4.w*(1-alpha);
                    alpha=alpha+value4.w*(1-alpha);
                }

                if(value8>0 && value8<0.6){
                    cur_color=cur_color+vec4(1,0,0,1)*mito_alpha*(1-alpha);
                    alpha=alpha+mito_alpha*(1-alpha);
                }
                else{
                    cur_color=cur_color+vec4(1,0,0,1)*value8*0.5*mito_alpha*(1-alpha);
                    alpha=alpha+value8*0.5*mito_alpha*(1-alpha);
                }
            }
        }


        if(work_type!=4){
            cur_color=cur_color+value3*value3.w*(1-alpha);
            alpha=alpha+value3.w*(1-alpha);
        }


        cur_color=cur_color+value5*value5.w*(1-alpha);
        alpha=alpha+value5.w*(1-alpha);


        if(work_type!=3){
            vec3 mitoColor=vec3(1,1,1);
            if(grayscale==false){
                if(value2<0.25){
                    mitoColor=vec3(0,0,0)*(0.25-value2)*4 + vec3(0,0,1)*value2*4;
                }
                else if(value2<0.75){
                    mitoColor=vec3(0,0,1)*(0.75-value2)*2 + vec3(1,1,0)*(value2-0.25)*2;
                }
                else mitoColor=vec3(1,1,0);
            }
            cur_color=cur_color+vec4(mitoColor.r,mitoColor.g,mitoColor.b,1)*value2*(1-alpha);
            alpha=alpha+value2*(1-alpha);
        }


        if(work_type==4){
            value1*=0.3;
        }
        cur_color=cur_color+vec4(31.0/255.0,255.0/255.0,0,1)*value1*(1-alpha);
        alpha=alpha+value1*(1-alpha);
        gl_FragColor=cur_color;
//        gl_FragColor=vec4(value2,value2,value2,1);
        //if(value2>0)gl_FragColor=vec4(1,0,0,1);
        //gl_FragColor=value4*value4.a;
    }
    else{
        discard;
    }
}
