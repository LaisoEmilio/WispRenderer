#include "resource_pool_texture.hpp"
#include "util/log.hpp"
#include "util/strings.hpp"

namespace wr
{

	TexturePool::TexturePool()
	{
#ifdef _DEBUG
		static uint16_t pool_count = 0u;
		
		m_name = "TexturePool_" + std::to_string(pool_count);

		pool_count++;
#endif
	}

	TextureHandle TexturePool::LoadFromCompressedMemory(char* data, size_t width, size_t height, const std::string& texture_extension, bool srgb, bool generate_mips)
	{
		std::string new_str = texture_extension;

		std::transform(texture_extension.begin(), texture_extension.end(), new_str.begin(), ::tolower);

		TextureType type;

		if (new_str == "png"|| new_str == "jpg"
			|| new_str == "jpeg" || new_str == "bmp")
		{
			type = TextureType::WIC;
		}
		else if (new_str.compare("dds") == 0)
		{
			type = TextureType::DDS;
		}
		else if (new_str.compare("hdr") == 0)
		{
			type = TextureType::HDR;
		}
		else
		{
			LOGC("[ERROR]: Texture format not supported.");
		}

		return LoadFromCompressedMemory(data, width, height, type, srgb, generate_mips);
	}

	TextureHandle TexturePool::GetDefaultAlbedo() const
	{
		return m_default_albedo;
	}

	TextureHandle TexturePool::GetDefaultNormal() const
	{
		return m_default_normal;
	}

	TextureHandle TexturePool::GetDefaultRoughness() const
	{
		return m_default_roughness;
	}

	TextureHandle TexturePool::GetDefaultMetalic() const
	{
		return m_default_metalic;
	}

	TextureHandle TexturePool::GetDefaultAO() const
	{
		return m_default_ao;
	}
}