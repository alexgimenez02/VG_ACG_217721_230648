varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform sampler3D u_texture;
uniform vec3 u_camera_position;
uniform mat4 u_viewprojection;
uniform mat4 u_model;
uniform mat4 u_inverse_model;
uniform float u_time;
uniform vec4 u_color;
//Part 1
uniform float u_ray_step;
uniform float u_brightness;
uniform float u_alpha_filter;
//Part 2 
//Jitter
uniform bool u_method;
uniform bool u_jitter;
uniform sampler2D u_noise_texture;
uniform float u_texture_width;

float rand( vec2 co )
{
	return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
	//1. Setup ray
	// init variables to use in algorithm
	vec3 dir = normalize(v_position - (u_inverse_model * vec4(u_camera_position, 1.0)).xyz );
	vec3 sample_pos = v_position;
	vec4 final_color;
	float d;
	vec4 sample_color;
	float offset;
	//for loop
	if(u_jitter){
		if(u_method){
			offset = rand(gl_FragCoord.xy);
		}else{
			vec2 uv_noise = gl_FragCoord.xy / u_texture_width;
			offset = texture2D(u_noise_texture, uv_noise).x;
		}
		sample_pos += offset * dir * u_ray_step;
	}
	for (int i = 0; i < 10000; i++) {
		//2. Get information from 3D texture
		d = texture(u_texture, (sample_pos + 1.0)/2.0).x;
		//3. Obtain color from density obtained
		sample_color = vec4(d, d, d, d);
		sample_color.rgb *= sample_color.a;
		//4. Composition of final_color
		final_color += u_ray_step * (1.0 - final_color.a) * sample_color;
		//5. Next Sample and Exit Conditions
		sample_pos = sample_pos + dir*u_ray_step;
		if (sample_pos.x > 1.0 || sample_pos.y > 1.0 || sample_pos.z > 1.0 
			|| sample_pos.x < -1.0 || sample_pos.y < -1.0 || sample_pos.z < -1.0) {
			break;
		}
		if (final_color.a >= 1.0) {
			break;
		}
	}

	if (final_color.a <= u_alpha_filter) {
		discard;
	}
	gl_FragColor = final_color * u_brightness * u_color;
}