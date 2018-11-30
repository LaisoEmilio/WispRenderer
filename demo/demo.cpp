#include <memory>
#include <algorithm>
#include "wisp.hpp"
#include "render_tasks/d3d12_test_render_task.hpp"
#include "render_tasks/d3d12_imgui_render_task.hpp"
#include "render_tasks/d3d12_deferred_main.hpp"
#include "render_tasks/d3d12_deferred_composition.hpp"

#include "engine_interface.hpp"
#include "scene_viknell.hpp"
#include "resources.hpp"
#include "scene_cubes.hpp"

#define SCENE viknell_scene

std::unique_ptr<wr::D3D12RenderSystem> render_system;
std::shared_ptr<wr::SceneGraph> scene_graph;

void RenderEditor()
{
	engine::RenderEngine(render_system.get(), scene_graph.get());
}

void WispEntry()
{
	// ImGui Logging
	util::log_callback::impl = [&](std::string const & str)
	{
		engine::debug_console.AddLog(str.c_str());
	};

	render_system = std::make_unique<wr::D3D12RenderSystem>();
	auto window = std::make_unique<wr::Window>(GetModuleHandleA(nullptr), "D3D12 Test App", 1280, 720);

	window->SetKeyCallback([](int key, int action, int mods)
	{
		if (action == WM_KEYUP && key == 0xC0)
		{
			engine::open_console = !engine::open_console;
			engine::debug_console.EmptyInput();
		}
		if (action == WM_KEYUP && key == VK_F1)
		{
			engine::show_imgui = !engine::show_imgui;
		}
	});

	render_system->Init(window.get());	

	resources::CreateResources(render_system.get());

	scene_graph = std::make_shared<wr::SceneGraph>(render_system.get());

	SCENE::CreateScene(scene_graph.get(), window.get());

	render_system->InitSceneGraph(*scene_graph.get());

	wr::FrameGraph frame_graph;
	frame_graph.AddTask(wr::GetDeferredMainTask());
	frame_graph.AddTask(wr::GetDeferredCompositionTask());
	frame_graph.AddTask(wr::GetImGuiTask(&RenderEditor));
	frame_graph.Setup(*render_system);

	window->SetResizeCallback([&](std::uint32_t width, std::uint32_t height)
	{
		render_system->WaitForAllPreviousWork();
		frame_graph.Resize(*render_system.get(), width, height);
		render_system->Resize(width, height);
	});

	while (window->IsRunning())
	{
		window->PollEvents();

		SCENE::UpdateScene();

		auto texture = render_system->Render(scene_graph, frame_graph);
	}

	render_system->WaitForAllPreviousWork(); // Make sure GPU is finished before destruction.
	frame_graph.Destroy();
	render_system.reset();
}

WISP_ENTRY(WispEntry)
