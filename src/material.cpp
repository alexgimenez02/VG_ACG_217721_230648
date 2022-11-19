#include "material.h"
#include "texture.h"
#include "application.h"
#include "extra/hdre.h"

StandardMaterial::StandardMaterial()
{
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
}

StandardMaterial::~StandardMaterial()
{

}

void StandardMaterial::setUniforms(Camera* camera, Matrix44 model)
{

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//upload node uniforms
	Matrix44 inv_model = model;
	inv_model.inverse();
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_camera_position", camera->eye);
	shader->setUniform("u_model", model);
	shader->setUniform("u_inverse_model", inv_model);
	shader->setUniform("u_time", Application::instance->time);
	shader->setUniform("u_color", color);
	{ //Added
		{//Part 1
			shader->setUniform("u_brightness", brightness);
			shader->setUniform("u_ray_step", ray_step);
			shader->setUniform("u_alpha_filter", alpha_filter);
		}
		{//Part 2
			//Jittering
			shader->setUniform("u_method", jitterMethodb);
			shader->setUniform("u_jitter", jitter);
			//Transfer function
			shader->setUniform("u_tf", tf);
			shader->setUniform("u_tf_filter", tf_filter);
			//Volume clipping
			shader->setUniform("u_vc", vc);
			shader->setUniform("u_plane",Vector4(plane.a, plane.b, plane.c, plane.d));

			//Isosurface
			shader->setUniform("u_iso", iso);
			shader->setUniform("u_h_value", h_value);
		}
		
	}

	if (texture)
		shader->setUniform("u_texture", texture, 0);
	if (noise_texture) {
		shader->setUniform("u_texture_width", noise_texture->width);
		shader->setUniform("u_noise_texture", noise_texture, 1);
	}
	if (tf_texture)
		shader->setUniform("u_tf_texture", tf_texture, 2);
}

void StandardMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
{
	if (mesh && shader)
	{
		//enable shader
		shader->enable();

		//upload uniforms
		setUniforms(camera, model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		//disable shader
		shader->disable();
	}
}

void StandardMaterial::renderInMenu()
{
	ImGui::ColorEdit3("Color", (float*)&color); // Edit 3 floats representing a color
}

void StandardMaterial::swapTF(int selected)
{
	string filename = "data/images/";
	switch (selected) {
		case 0:
			filename += "TF1.png";
			break;
		case 1:
			filename += "TF2.png";
			break;
		case 2:
			filename += "TF3.png";
			break;
		case 3:
			filename += "TF4.png";
			break;
	}
	tf_texture = Texture::Get(filename.c_str());

}

WireframeMaterial::WireframeMaterial()
{
	color = vec4(1.f, 1.f, 1.f, 1.f);
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
}

WireframeMaterial::~WireframeMaterial()
{

}

void WireframeMaterial::render(Mesh* mesh, Matrix44 model, Camera* camera)
{
	if (shader && mesh)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		//enable shader
		shader->enable();

		//upload material specific uniforms
		setUniforms(camera, model);

		//do the draw call
		mesh->render(GL_TRIANGLES);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}