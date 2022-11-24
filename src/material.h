#ifndef MATERIAL_H
#define MATERIAL_H

#include "framework.h"
#include "shader.h"
#include "camera.h"
#include "mesh.h"
#include "extra/hdre.h"


typedef struct sPlane {
	float a; 
	float b; 
	float c; 
	float d; 
}Plane;

class Material {
public:

	Shader* shader = NULL;
	Texture* texture = NULL;
	Texture* noise_texture = NULL;
	Texture* tf_texture = NULL;
	vec4 color;
	Vector3 light_position;
	float brightness = 1.0f;
	float ray_step = 0.05f;
	float alpha_filter = 0.01f;
	float tf_filter = 0.01f;
	int method = 1;
	bool jitter = false;
	bool jitterMethodb = false;
	bool tf = false;
	bool vc = false;
	bool iso = false;
	float h_value = 0.0;
	float light_intensity = 0.1;
	float iso_threshold = 0.98;
	Plane plane;

	virtual void setUniforms(Camera* camera, Matrix44 model) = 0;
	virtual void render(Mesh* mesh, Matrix44 model, Camera * camera) = 0;
	virtual void renderInMenu() = 0;
};

class StandardMaterial : public Material {
public:

	StandardMaterial();
	~StandardMaterial();

	void setUniforms(Camera* camera, Matrix44 model);
	void render(Mesh* mesh, Matrix44 model, Camera * camera);
	void renderInMenu();
	void swapTF(int selected);
};

class WireframeMaterial : public StandardMaterial {
public:

	WireframeMaterial();
	~WireframeMaterial();

	void render(Mesh* mesh, Matrix44 model, Camera * camera);
};

// TODO: Derived class VolumeMaterial

#endif