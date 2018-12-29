#include <memory>
#include <algorithm>
#include "wisp.hpp"
#include "render_tasks/d3d12_imgui_render_task.hpp"
#include "render_tasks/d3d12_deferred_main.hpp"
#include "render_tasks/d3d12_deferred_composition.hpp"
#include "render_tasks/d3d12_deferred_render_target_copy.hpp"
#include "render_tasks/d3d12_raytracing_task.hpp"

#include "engine_interface.hpp"
#include "scene_viknell.hpp"
#include "resources.hpp"
#include "scene_cubes.hpp"
#include "scene_pbr.hpp"

#include "model_loader_assimp.hpp"

#define SCENE viknell_scene

constexpr bool do_raytracing = true;

std::unique_ptr<wr::D3D12RenderSystem> render_system;
std::shared_ptr<wr::SceneGraph> scene_graph;

std::shared_ptr<wr::TexturePool> texture_pool;

void RenderEditor()
{
	engine::RenderEngine(render_system.get(), scene_graph.get());
}

int WispEntry()
{
	/*// ImGui Logging
	util::log_callback::impl = [&](std::string const & str)
	{
		engine::debug_console.AddLog(str.c_str());
	};

	render_system = std::make_unique<wr::D3D12RenderSystem>();
	auto window = std::make_unique<wr::Window>("D3D12 Test App", 1280, 720);

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

	wr::ModelLoader* assimp_model_loader = new wr::AssimpModelLoader();

	render_system->Init(window.get());	

	resources::CreateResources(render_system.get());

	scene_graph = std::make_shared<wr::SceneGraph>(render_system.get());

	SCENE::CreateScene(scene_graph.get(), window.get());

	render_system->InitSceneGraph(*scene_graph.get());

	wr::FrameGraph frame_graph;
	if (do_raytracing)
	{
		frame_graph.AddTask<wr::DeferredMainTaskData>(wr::GetDeferredMainTask());
		wr::AddDeferredCompositionTask(frame_graph);
		frame_graph.AddTask<wr::RenderTargetCopyTaskData>(wr::GetRenderTargetCopyTask<wr::DeferredCompositionTaskData>());
	}
	else
	{
		frame_graph.AddTask<wr::RaytracingData>(wr::GetRaytracingTask());
		frame_graph.AddTask<wr::RenderTargetCopyTaskData>(wr::GetRenderTargetCopyTask<wr::RaytracingData>());
	}
	frame_graph.AddTask<wr::ImGuiTaskData>(wr::GetImGuiTask(&RenderEditor));
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

	delete assimp_model_loader;

	render_system->WaitForAllPreviousWork(); // Make sure GPU is finished before destruction.
	frame_graph.Destroy();
	render_system.reset();*/
	return 0;
}

class MyWindow : public wr::Window
{
	virtual void Init()
	{

	}

	virtual void Loop()
	{

	}

	virtual void OnResize(int width, int height)
	{

	}
};

//WISP_ENTRY(WispEntry)
//CREATE_GAME(MyWindow, 1280, 720, "Welcome back UWP");

int main(Platform::Array<Platform::String^>^) \
{ \
auto direct3DApplicationSource = ref new wr::Direct3DApplicationSource<MyWindow, 1280, 720>(); \
Windows::ApplicationModel::Core::CoreApplication::Run(direct3DApplicationSource); \
return 0; \
}
