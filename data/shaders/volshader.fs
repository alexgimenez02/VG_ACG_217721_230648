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

//Transfer function
uniform bool u_tf;
uniform float u_tf_filter;
uniform sampler2D u_tf_texture;

//Volume clipping
uniform bool u_vc;
uniform vec4 u_plane;

//Isosurfaces
uniform bool u_iso;
uniform float u_h_value;
uniform vec3 u_local_light_position;
uniform vec3 u_local_camera_position;
uniform float u_light_intensity;
uniform vec4 u_light_color;
uniform float u_threshold;


//Part 2 Jittering pseudo-random
float rand(vec2 co) {
	return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
	//1. Setup ray (part 1)
	// init variables to use in algorithm (part 1)

	// Dirección de la cámara al objeto
	vec3 dir = normalize(v_position - (u_inverse_model * vec4(u_camera_position, 1.0)).xyz);

	//Inicializa sample position en la primera posición del vertex shader
	vec3 sample_pos = v_position;

	// Otras variables que se usan en el loop
	vec4 final_color;
	float d;
	vec4 sample_color;
	float offset;

	//Part 2 jittering
	if (u_jitter) {
		if(u_method) {
			// Calcula el offset
			// Rand devuelve un float como resultado del fractal
			offset = rand(gl_FragCoord.xy);
		}
		else {
			// Se saca una posición y se escala al tamaño de la textura de ruido
			vec2 uv_noise = gl_FragCoord.xy / u_texture_width;
			// Calcula el offset
			offset = texture2D(u_noise_texture, uv_noise).x;
		}
		// Calcula la sample position
		sample_pos += offset * dir * u_ray_step;
	}

	//for loop (part 1)
	for (int i = 0; i < 10000; i++) {
		//Part 2 volume clipping
		if (u_vc) {
			// Se guarda la ecuación del plano
			float pl_eq = sample_pos.x * u_plane.x + sample_pos.y * u_plane.y + sample_pos.z * u_plane.z + u_plane.a;
			if (pl_eq > 0) {
				// Se pasa al siguiente step
				sample_pos += dir*u_ray_step;

				// Se comprueba si la siguiente posición está fuera del volumen
				if (sample_pos.x > 1.0 || sample_pos.y > 1.0 || sample_pos.z > 1.0 
					|| sample_pos.x < -1.0 || sample_pos.y < -1.0 || sample_pos.z < -1.0) {
					break;
				}
				// O si la opacidad es mayor o igual a 1
				if (final_color.a >= 1.0) {
					break;
				}
				// En ambos casos se sale del bucle
				
				continue;
			}
		}

		//2. Get information from 3D texture (part 1)
		// De la textura se saca la densidad (.x)
		d = texture(u_texture, (sample_pos + 1.0)/2.0).x;

		//3. Obtain color from density obtained (part 1)
		//Part 2 Transfer function
		if (u_tf) {
			// Se mira si la densidad del volumen es mayor que el filtro
			if (d > u_tf_filter) {
				// Se saca el valor del color del png en la posición de densidad
				sample_color = texture(u_tf_texture, vec2(d*1.5, 1.0));
				// Se multiplica la opacidad por dos veces la densidad
				sample_color.a *= d * 2.0;
			}
		}
		// Sacar el sample color creando un vector de 4 valores y poniendo la densidad en cada posición
		else {
			sample_color = vec4(d, d, d, d); //Part 1
		}
		// Multiplicar los valores RGB por la opacidad 
		sample_color.rgb *= sample_color.a;

		//4. Composition of final_color (Part 1)

		//Part 2 Isosurfaces
		if (u_iso) {
			if (d > u_threshold) {
				// Dirección de la luz al objeto
				vec3 L = u_local_light_position - sample_pos;
				// Dirección de la cámara al objeto
				vec3 V = u_local_camera_position - sample_pos;

				// Se calcula la normal con el gradiente
				float posdx = texture3D(u_texture, ((sample_pos+1.0)/2.0 + vec3(u_h_value,0,0))).x - texture3D(u_texture, ((sample_pos+1.0)/2.0 - vec3(u_h_value,0,0))).x;
				float posdy = texture3D(u_texture, ((sample_pos+1.0)/2.0 + vec3(0,u_h_value,0))).x - texture3D(u_texture, ((sample_pos+1.0)/2.0 - vec3(0,u_h_value,0))).x;
				float posdz = texture3D(u_texture, ((sample_pos+1.0)/2.0 + vec3(0,0,u_h_value))).x - texture3D(u_texture, ((sample_pos+1.0)/2.0 - vec3(0,0,u_h_value))).x;
				vec3 N = -1*vec3(posdx, posdy, posdz) / (2*u_h_value);

				// Se normalizan los vectores
				N = normalize(N);
				L = normalize(L);
				V = normalize(V);

				// Se calcula el rayo reflejado
				vec3 R = reflect(-L, N);
				
				// Phong 
				vec4 Ip = vec4(sample_color.rgb * (clamp(dot(N,L),0.0,1.0) + pow(clamp(dot(R,V),0.0,1.0), u_light_intensity)), 1.0); 
				
				final_color = Ip;
				break;
			}
		}
		else {
			if (!u_tf) { //Part 2
				// Calcular el color final (se le añade u_color (color que se le pasa desde el framework))
				final_color += u_ray_step * (1.0 - final_color.a) * sample_color * u_color; //Part 1
			}
			else {
				final_color += u_ray_step * (1.0 - final_color.a) * sample_color;
			}	
		}
		//5. Next Sample and Exit Conditions (Part 1)
		// Calcular la siguiente posición
		sample_pos = sample_pos + dir*u_ray_step; //Part 1
		
		//Part 1
		// Antes de seguir, se comprueba si la siguiente posición está fuera del volumen
		if (sample_pos.x > 1.0 || sample_pos.y > 1.0 || sample_pos.z > 1.0 
			|| sample_pos.x < -1.0 || sample_pos.y < -1.0 || sample_pos.z < -1.0) {
			break;
		}
		// O si la opacidad es mayor o igual a 1
		if (final_color.a >= 1.0) {
			break;
		}
		// En ambos casos se sale del bucle
		
	}

	//Part 1
	// Filtro para quitar los valores más oscuros
	if (final_color.a <= u_alpha_filter) {
		discard;
	}

	// Se pasa el color final multiplicado por el brillo al OpenGL
	gl_FragColor = final_color * u_brightness;
}