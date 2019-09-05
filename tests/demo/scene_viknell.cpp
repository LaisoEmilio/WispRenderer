/*!
 * Copyright 2019 Breda University of Applied Sciences and Team Wisp (Viktor Zoutman, Emilio Laiso, Jens Hagen, Meine Zeinstra, Tahar Meijs, Koen Buitenhuis, Niels Brunekreef, Darius Bouma, Florian Schut)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include "scene_viknell.hpp"

ViknellScene::ViknellScene() :
	Scene(256, 2_mb, 2_mb),
	m_cube_model(nullptr),
	m_material_1(),
	m_material_2(),
	m_skybox({}),
	m_time(0)
{
	m_lights_path = "resources/viknell_lights.json";
}

ViknellScene::~ViknellScene()
{
	if (m_cube_model)
	{
		m_model_pool->Destroy(m_cube_model);
		m_material_pool->DestroyMaterial(m_material_1);
		m_material_pool->DestroyMaterial(m_material_2);
	}
}

void ViknellScene::LoadResources()
{
	// Models
	m_cube_model = m_model_pool->Load<wr::Vertex>(m_material_pool.get(), m_texture_pool.get(), "resources/models/cube.fbx");

	// Textures
	wr::TextureHandle bamboo_albedo = m_texture_pool->LoadFromFile("resources/materials/bamboo/bamboo-wood-semigloss-albedo.png", true, true);

	m_skybox = m_texture_pool->LoadFromFile("resources/materials/Barce_Rooftop_C_3k.hdr", false, false);

	// Materials
	m_material_1 = m_material_pool->Create(m_texture_pool.get());
	wr::Material* mat_1_int = m_material_pool->GetMaterial(m_material_1);
	mat_1_int->SetConstant<wr::MaterialConstant::ROUGHNESS>(0.05f);
	mat_1_int->SetConstant<wr::MaterialConstant::METALLIC>(1.0f);
	mat_1_int->SetConstant<wr::MaterialConstant::COLOR>({ 1, 1, 1 });

	m_material_2 = m_material_pool->Create(m_texture_pool.get());
	wr::Material* mat_2_int = m_material_pool->GetMaterial(m_material_2);
	mat_2_int->SetTexture(wr::TextureType::ALBEDO, bamboo_albedo);
	mat_2_int->SetConstant<wr::MaterialConstant::ROUGHNESS>(0.99f);
	mat_2_int->SetConstant<wr::MaterialConstant::METALLIC>(0.01f);
}

void ViknellScene::BuildScene(unsigned int width, unsigned int height, void* extra)
{
	m_camera = m_scene_graph->CreateChild<DebugCamera>(nullptr, 90.f, (float)width / (float)height);
	m_camera->SetPosition({ 0, 20, 20 });
	m_camera->SetSpeed(20);

	m_camera_spline_node = m_scene_graph->CreateChild<SplineNode>(nullptr, "Camera Spline", false);

	auto skybox = m_scene_graph->CreateChild<wr::SkyboxNode>(nullptr, m_skybox);

	// Geometry
	auto floor = m_scene_graph->CreateChild<wr::MeshNode>(nullptr, m_cube_model);
	floor->SetScale({ 10, 1, 300 });
	floor->SetMaterials({ m_material_2 });

	auto cube_1 = m_scene_graph->CreateChild<wr::MeshNode>(nullptr, m_cube_model);
	cube_1->SetPosition({ 8, 10, 0 });
	cube_1->SetMaterials({ m_material_1 });

	auto cube_2 = m_scene_graph->CreateChild<wr::MeshNode>(nullptr, m_cube_model);
	cube_2->SetPosition({ -8, 10, 0 });
	cube_2->SetRotation({ 0.0f, 45.0_deg, 0.0f});
	cube_2->SetMaterials({ m_material_1 });

	for (int i = 0; i < 200; ++i)
	{
		auto cube = m_scene_graph->CreateChild<wr::MeshNode>(nullptr, m_cube_model);

		float rand_x = float(rand()) / RAND_MAX; rand_x = rand_x * 16.0f - 8.0f;
		float rand_z = float(rand()) / RAND_MAX; rand_z *= -300.0f;

		cube->SetPosition({ rand_x, 10, rand_z });
		cube->SetMaterials({ m_material_1 });
	}

	// Lights
	/*auto point_light_0 = m_scene_graph->CreateChild<wr::LightNode>(nullptr, wr::LightType::DIRECTIONAL, DirectX::XMVECTOR{ 1, 1, 1 });
	point_light_0->SetRotation({ 20.950f, 0.98f, 0.f });
	point_light_0->SetPosition({ -0.002f, 0.080f, 1.404f });

	auto point_light_1 = m_scene_graph->CreateChild<wr::LightNode>(nullptr, wr::LightType::POINT, DirectX::XMVECTOR{ 1, 0, 0 });
	point_light_1->SetRadius(5.0f);
	point_light_1->SetPosition({ 0.5f, 0.f, -0.3f });

	auto point_light_2 = m_scene_graph->CreateChild<wr::LightNode>(nullptr, wr::LightType::POINT, DirectX::XMVECTOR{ 0, 0, 1 });
	point_light_2->SetRadius(5.0f);
	point_light_2->SetPosition({ -0.5f, 0.5f, -0.3f });*/

	LoadLightsFromJSON();
}

void ViknellScene::Update(float delta)
{
	m_camera->Update(delta);
	m_camera_spline_node->UpdateSplineNode(ImGui::GetIO().DeltaTime, m_camera);
}