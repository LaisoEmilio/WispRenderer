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

#include "../frame_graph/frame_graph.hpp"
#include "../d3d12/d3d12_renderer.hpp"
#include "../d3d12/d3d12_functions.hpp"
#include "../d3d12/d3d12_constant_buffer_pool.hpp"
#include "../pipeline_registry.hpp"
#include "../engine_registry.hpp"

#include "../scene_graph/camera_node.hpp"

namespace wr
{
	struct ShadowMappingSettings
	{
		struct Runtime
		{
			float m_far_plane = 2000.0f;
			uint32_t m_light_id = 0;
			DirectX::XMUINT2 m_resolution = { settings::app_width, settings::app_height };
		};

		Runtime m_runtime;
	};

	struct ShadowMappingTaskData
	{
		d3d12::PipelineState* in_pipeline;
		std::shared_ptr<ConstantBufferPool> camera_cb_pool;
		D3D12ConstantBufferHandle* cb_handle = nullptr;
	};

	struct ShadowMappingCameraData
	{
		DirectX::XMMATRIX m_view = DirectX::XMMatrixIdentity();
		DirectX::XMMATRIX m_projection = DirectX::XMMatrixIdentity();
	};


	namespace internal
	{

		inline void SetupShadowMappingTask(RenderSystem& rs, FrameGraph& fg, RenderTaskHandle handle, bool resized)
		{
			auto& data = fg.GetData<ShadowMappingTaskData>(handle);
			auto& n_render_system = static_cast<D3D12RenderSystem&>(rs);

			auto& ps_registry = PipelineRegistry::Get();

			if (!resized)
			{
				// Camera constant buffer
				data.camera_cb_pool = rs.CreateConstantBufferPool(2);
				data.cb_handle = static_cast<D3D12ConstantBufferHandle*>(data.camera_cb_pool->Create(sizeof(ShadowMappingCameraData)));

				data.in_pipeline = (d3d12::PipelineState*)ps_registry.Find(pipelines::shadow_mapping);
			}
		}

		inline void ExecuteShadowMappingTask(RenderSystem& rs, FrameGraph& fg, SceneGraph& scene_graph, RenderTaskHandle handle)
		{
			auto& n_render_system = static_cast<D3D12RenderSystem&>(rs);
			auto& data = fg.GetData<ShadowMappingTaskData>(handle);
			auto cmd_list = fg.GetCommandList<d3d12::CommandList>(handle);

			auto settings = fg.GetSettings<ShadowMappingTaskData, ShadowMappingSettings>();

			if (n_render_system.m_render_window.has_value())
			{
				size_t width = settings.m_runtime.m_resolution.x;
				size_t height = settings.m_runtime.m_resolution.y;
				size_t light_id = settings.m_runtime.m_light_id;
				float far_plane = settings.m_runtime.m_far_plane;

				const auto viewport = d3d12::CreateViewport(width, height);
				const auto frame_idx = n_render_system.GetRenderWindow()->m_frame_idx;

				d3d12::BindViewport(cmd_list, viewport);
				d3d12::BindPipeline(cmd_list, data.in_pipeline);
				d3d12::SetPrimitiveTopology(cmd_list, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				//Get light
				auto& lights = scene_graph.GetLightNodes();
				auto camera = scene_graph.GetActiveCamera();

				ShadowMappingCameraData cam_data;

				if (!lights.empty())
				{
					cam_data.m_view = lights[light_id]->GetView();
					cam_data.m_projection = lights[light_id]->GetProjection(width, height, 0.01f, far_plane);
					//TODO: inverted depth buffer for greater precision
				}
				else
				{
					cam_data.m_view = camera->m_view;
					cam_data.m_projection = camera->m_projection;
				}

				data.cb_handle->m_pool->Update(data.cb_handle, sizeof(ShadowMappingCameraData), 0, frame_idx, (uint8_t*)& cam_data);
				d3d12::BindConstantBuffer(cmd_list, data.cb_handle->m_native, 0, frame_idx);

				scene_graph.Render(cmd_list, scene_graph.GetActiveCamera().get());
			}
		}
	}

	inline void AddShadowMappingTask(FrameGraph& fg, std::optional<unsigned int> target_width, std::optional<unsigned int> target_height)
	{
		std::wstring name(L"ShadowMapping Pass");
		
		RenderTargetProperties rt_properties
		{
			RenderTargetProperties::IsRenderWindow(false),
			RenderTargetProperties::Width(target_width),
			RenderTargetProperties::Height(target_height),
			RenderTargetProperties::ExecuteResourceState(ResourceState::RENDER_TARGET),
			RenderTargetProperties::FinishedResourceState(ResourceState::NON_PIXEL_SHADER_RESOURCE),
			RenderTargetProperties::CreateDSVBuffer(true),
			RenderTargetProperties::DSVFormat(Format::D32_FLOAT),
			RenderTargetProperties::RTVFormats({ Format::R16G16B16A16_FLOAT, Format::R16G16B16A16_FLOAT, Format::R16G16B16A16_FLOAT }),
			RenderTargetProperties::NumRTVFormats(3),
			RenderTargetProperties::Clear(true),
			RenderTargetProperties::ClearDepth(true),
			RenderTargetProperties::ResolutionScalar(1.0f)
		};

		RenderTaskDesc desc;
		desc.m_setup_func = [](RenderSystem& rs, FrameGraph& fg, RenderTaskHandle handle, bool resized) {
			internal::SetupShadowMappingTask(rs, fg, handle, resized);
		};
		desc.m_execute_func = [](RenderSystem& rs, FrameGraph& fg, SceneGraph& sg, RenderTaskHandle handle) {
			internal::ExecuteShadowMappingTask(rs, fg, sg, handle);
		};
		desc.m_destroy_func = [](FrameGraph&, RenderTaskHandle, bool) {
			// Nothing to destroy
		};

		desc.m_properties = rt_properties;
		desc.m_type = RenderTaskType::DIRECT;
		desc.m_allow_multithreading = true;

		fg.AddTask<ShadowMappingTaskData>(desc, L"ShadowMapping Pass");

		ShadowMappingSettings settings;

		if (target_width.has_value() || target_height.has_value())
		{
			settings.m_runtime.m_resolution = { static_cast<uint32_t>(target_width.value()), static_cast<uint32_t>(target_height.value()) };
		}

		fg.UpdateSettings<ShadowMappingTaskData>(settings);
	}

} /* wr */
