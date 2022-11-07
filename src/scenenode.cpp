#include "scenenode.h"
#include "application.h"
#include "texture.h"
#include "utils.h"

unsigned int SceneNode::lastNameId = 0;
unsigned int mesh_selected = 0;

SceneNode::SceneNode()
{
	this->name = std::string("Node" + std::to_string(lastNameId++));
}

SceneNode::SceneNode(const char * name)
{
	this->name = name;
}

SceneNode::~SceneNode()
{

}

void SceneNode::render(Camera* camera)
{
	if (material)
		material->render(mesh, model, camera);
}

void SceneNode::renderWireframe(Camera* camera)
{
	WireframeMaterial mat = WireframeMaterial();
	mat.render(mesh, model, camera);
}

void SceneNode::renderInMenu()
{
	//Model edit
	if (ImGui::TreeNode("Model")) 
	{
		float matrixTranslation[3], matrixRotation[3], matrixScale[3];
		ImGuizmo::DecomposeMatrixToComponents(model.m, matrixTranslation, matrixRotation, matrixScale);
		ImGui::DragFloat3("Position", matrixTranslation, 0.1f);
		ImGui::DragFloat3("Rotation", matrixRotation, 0.1f);
		ImGui::DragFloat3("Scale", matrixScale, 0.1f);
		ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, model.m);
		ImGui::TreePop();
		
	}


	//Material
	if (material && ImGui::TreeNode("Material"))
	{
		material->renderInMenu();
		ImGui::TreePop();
	}

	//Geometry
	if (mesh && ImGui::TreeNode("Geometry"))
	{
		bool changed = false;
		changed |= ImGui::Combo("Mesh", (int*)&mesh_selected, "SPHERE\0");
		

		ImGui::TreePop();
	}
}

#pragma region SUPLEMENTARY_SWAP_FUNCTION
void SceneNode::swapVolume(int volume_selected) {
	Volume* volume = new Volume();
	switch (volume_selected) {
		case 0:
			volume->loadPVM("data/volumes/CT-Abdomen.pvm");
			break;
		case 1:
			volume->loadPVM("data/volumes/Daisy.pvm");
			break;
		case 2:
			volume->loadPVM("data/volumes/Orange.pvm");
			break;
		case 3:
			volume->loadPNG("data/volumes/bonsai_16_16.png");
			break;
		case 4:
			volume->loadPNG("data/volumes/foot_16_16.png");
			break;
		case 5:
			volume->loadPNG("data/volumes/teapot_16_16.png");
			break;
	}
	Texture* tex = new Texture();

	tex->create3DFromVolume(volume, GL_CLAMP_TO_EDGE );
	material->texture = tex;
	model.scale(1, 1, 1);
	model.setScale(1, (volume->height * volume->heightSpacing) / (volume->width * volume->widthSpacing), (volume->depth * volume->depthSpacing) / (volume->width * volume->widthSpacing));

}
#pragma endregion SUPLEMENTARY_SWAP_FUNCTION