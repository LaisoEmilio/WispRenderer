/*
The mipmapping implementation used in this framework is a ported version of
MiniEngine's implementation.
*/
/*
The MIT License(MIT)

Copyright(c) 2013 - 2015 Microsoft

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once

#include "../resource_pool_texture.hpp"
#include "d3d12_enums.hpp"
#include "d3d12_texture_resources.hpp"
#include <DirectXMath.h>


namespace wr
{
	struct MipMapping_CB
	{
		uint32_t src_mip_level;			// Texture level of source mip
		uint32_t num_mip_levels;		// Number of OutMips to write: [1-4]
		uint32_t src_dimension;			// Width and height of the source texture are even or odd.
		uint32_t padding;				// Pad to 16 byte alignment.
		DirectX::XMFLOAT2 texel_size;	// 1.0 / OutMip1.Dimensions
	};

	class D3D12RenderSystem;
	class DescriptorAllocator;

	class D3D12TexturePool : public TexturePool
	{
	public:
		explicit D3D12TexturePool(D3D12RenderSystem& render_system);
		~D3D12TexturePool() final;

		void Evict() final;
		void MakeResident() final;
		void Stage(CommandList* cmd_list) final;
		void PostStageClear() final;
		void ReleaseTemporaryResources() final;

		d3d12::TextureResource* GetTextureResource(TextureHandle handle) final;

		//! Loads a texture from file.
		/*!
		  \param path std::string that contains a path to the texture file location.
		  \param srgb Defines if the texture should be loaded as srgb texture or not (currently not supported). 
		  \param generate_mips Defines if mipmaps should be created.
		  \return The texture handle to the loaded texture.
		  \sa wr::TextureHandle
		*/
		[[nodiscard]] TextureHandle LoadFromFile(std::string_view path, bool srgb, bool generate_mips) final;
		[[nodiscard]] TextureHandle LoadFromMemory(unsigned char* data, size_t width, size_t height, TextureFormat type, bool srgb, bool generate_mips) final;
		[[nodiscard]] TextureHandle CreateCubemap(std::string_view name, uint32_t width, uint32_t height, uint32_t mip_levels, Format format, bool allow_render_dest) final;
		[[nodiscard]] TextureHandle CreateTexture(std::string_view name, uint32_t width, uint32_t height, uint32_t mip_levels, Format format, bool allow_render_dest) final;

		DescriptorAllocator* GetAllocator(DescriptorHeapType type);
		DescriptorAllocator* GetMipmappingAllocator() { return m_mipmapping_allocator; }

		void MarkForUnload(TextureHandle& handle, unsigned int frame_idx) final;
		void UnloadTextures(unsigned int frame_idx) final;

		void GenerateMips_Cubemap(d3d12::TextureResource* texture, CommandList* cmd_list, unsigned int array_slice);

	protected:

		void MoveStagedTextures(unsigned int frame_idx);
		void GenerateMips(d3d12::TextureResource* texture, CommandList* cmd_list);

		void GenerateMips_UAV(d3d12::TextureResource* texture, CommandList* cmd_list);
		void GenerateMips_BGR(d3d12::TextureResource* texture, CommandList* cmd_list);
		void GenerateMips_SRGB(d3d12::TextureResource* texture, CommandList* cmd_list);

		//Unstaged textures are stored as pairs in a map. This removes the necessity of having a ScratchImage
		//pointer in the Texture struct. Once the textures are staged the ScratchImages are deleted.
		using UnstagedTextures = std::unordered_map<uint64_t, std::pair<Texture*, DirectX::ScratchImage*>>;
		UnstagedTextures m_unstaged_textures;

		using StagedTextures = std::unordered_map<uint64_t, Texture*>;
		StagedTextures m_staged_textures;
		std::vector<StagedTextures> m_staging_textures;

		D3D12RenderSystem& m_render_system;

		//CPU only visible heaps used for staging of descriptors.
		//Renderer will copy the descriptor it needs to the GPU visible heap used for rendering.
		DescriptorAllocator* m_allocators[static_cast<size_t>(DescriptorHeapType::DESC_HEAP_TYPE_NUM_TYPES)];

		DescriptorAllocator* m_mipmapping_allocator;

		//Track resources that are created in one frame and destroyed after
		std::array<std::vector<d3d12::TextureResource*>, d3d12::settings::num_back_buffers> m_temporary_textures;
		std::array< std::vector<d3d12::Heap<wr::HeapOptimization::BIG_STATIC_BUFFERS>*>, d3d12::settings::num_back_buffers> m_temporary_heaps;

		//Resources marked for deletion
		std::array<std::vector<d3d12::TextureResource*>, d3d12::settings::num_back_buffers> m_marked_for_unload;

		//Default UAV used for padding
		DescriptorAllocation m_default_uav;
	};




}