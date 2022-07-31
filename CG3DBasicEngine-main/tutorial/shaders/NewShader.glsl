#version 330

in vec3 first_pos;


uniform int Dots;
uniform vec4[7] all_dots; 

out vec4 Color_of_points;


void main()
{

	vec3 pos = first_pos;
	bool flag= true; 
	int i =0; 
	while (i< Dots && flag){
		if ((first_pos.s - (all_dots[i]*2.4).x)*(first_pos.s-(all_dots[i]*2.4).x) + (first_pos.t-(all_dots[i]*2.4).y)*(first_pos.t-(all_dots[i]*2.4).y) < 0.04)
		{
         flag = false;
        }
		i++;
    }
    if(flag)
		discard;
    else 
      Color_of_points = vec4(0.4433, 1.0, 0.4392, 2.0);
}