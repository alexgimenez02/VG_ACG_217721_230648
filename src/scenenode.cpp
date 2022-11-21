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
	Application* app = Application::instance;
	vector<SceneNode*> nd_list = app->node_list;
	if (nd_list.size() - 1 > 1) {
		if (ImGui::TreeNode("Geometry"))
		{
			bool changed = false;
			changed |= ImGui::Combo("Volume", (int*)&mesh_selected, "CT-ABDOMEN\0DAISY\0ORANGE\0BONSAI\0FOOT\0TEAPOT");
			if (changed) {
				swapVolume(mesh_selected);
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Properties")) {
			{ //Part 1
				ImGui::SliderFloat("Brightness", &app->brightness, 0.0f, 15.0f);
				ImGui::SliderFloat("Step vector", &app->ray_step, 0.001f, 1.00f);
				ImGui::SliderFloat("Alpha filter", &app->alpha_filter, 0.01f, 0.1f);

			}
			//Part 2
			if (ImGui::TreeNode("Jittering")) {

				ImGui::Checkbox("Activate", (bool*)&app->jitter);
				ImGui::Combo("Method", (int*)&app->method, "Blue Noise Texture\0Pseudorandom-looking\0");
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Transfer Function")) {
				ImGui::Checkbox("Activate", (bool*)&app->tf);
				ImGui::Combo("Texture", (int*)&app->tf_selected, "TF1\0TF2\0TF3\0TF4\0");
				ImGui::SliderFloat("Filter", (float*)&app->tf_filter, 0.01, 1.0);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Volume clipping")) {
				ImGui::Checkbox("Activate", (bool*)&app->vc);
				float planeVector[4];
				planeVector[0] = app->pl.a;
				planeVector[1] = app->pl.b;
				planeVector[2] = app->pl.c;
				planeVector[3] = app->pl.d;
				ImGui::SliderFloat4("Plane values", planeVector, -5.0, 5.0);
				app->pl = { planeVector[0] , planeVector[1] , planeVector[2]  , planeVector[3] };
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Isosurfaces")) {
				ImGui::Checkbox("Activate", (bool*)&app->iso);
				ImGui::SliderFloat("Threshold", (float*)&app->threshold, 0.01, 0.99f);
				ImGui::SliderFloat("H value", (float*)&app->h_value, 0.005, 0.1);
				if (ImGui::TreeNode("Light values")) {
					ImGui::Checkbox("Show", (bool*)&app->show_light);
					float lightPos[3];
					lightPos[0] = app->light_position.x;
					lightPos[1] = app->light_position.y;
					lightPos[2] = app->light_position.z;
					ImGui::DragFloat3("Position", lightPos, 0.05f);
					app->light_position = Vector3(lightPos[0], lightPos[1], lightPos[2]);
					ImGui::SliderFloat("Light intensity", (float*)&app->light_intensity, 0.1, 2.5);
					ImGui::TreePop();
				}
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
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