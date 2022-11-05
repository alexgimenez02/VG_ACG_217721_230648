varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform vec3 u_camera_position;
uniform mat4 u_viewprojection;
uniform mat4 u_model;
uniform float u_time;
uniform vec4 u_color;

uniform sampler3D u_texture;
void main() {
	//1. Setup ray
	// init variables to use in algorithm
	vec3 dir = normalize(v_position - u_camera_position);
	float ray_step = 0.05;
	vec3 sample_pos = v_position;
	vec4 final_color = vec4(0, 0, 0, 0);
	//for loop
	for (int i = 0; i < 1000; i++) {
		//2. Get information from 3D texture
		float d = texture3D(u_texture, (sample_pos + 1.0)/2.0).x;
		//3. Obtain color from density obtained
		vec4 sample_color = vec4(d, d, d, d);

		sample_color.rgb *= sample_color.a;
		//4. Composition of final_color
		final_color += ray_step * (1.0 - final_color.a) * sample_color;
		//5. Next Sample and Exit Conditions
		sample_pos = sample_pos + dir*ray_step;
		if (sample_pos.x > 1.0 || sample_pos.y > 1.0 || sample_pos.z > 1.0) {
			break;
		}
		if (final_color.a >= 1.0) {
			break;
		}
	}

	if (final_color.a <= 0.01) {
		discard;
	}
	gl_FragColor = final_color;
}