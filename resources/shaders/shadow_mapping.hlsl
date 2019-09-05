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
#ifndef __SHADOW_MAPPING_HLSL__
#define __SHADOW_MAPPING_HLSL__

//48 KiB; 48 * 1024 / sizeof(MeshNode)
//48 * 1024 / (4 * 4 * 4) = 48 * 1024 / 64 = 48 * 16 = 768
#define MAX_INSTANCES 768

#include "material_util.hlsl"

struct VS_INPUT
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float depth_ls : DEPTH_LS;
};

cbuffer CameraProperties : register(b0)
{
	float4x4 view;
	float4x4 projection;
};

struct ObjectData
{
	float4x4 model;
	float4x4 prev_model;
};

cbuffer ObjectProperties : register(b1)
{
	ObjectData instances[MAX_INSTANCES];
};

VS_OUTPUT main_vs(VS_INPUT input, uint instid : SV_InstanceId)
{
	VS_OUTPUT output;

	float3 pos = input.pos;

	ObjectData inst = instances[instid];

	//TODO: Use precalculated MVP or at least VP
	float4x4 vm = mul(view, inst.model);
	float4x4 mvp = mul(projection, vm);

	output.pos = mul(mvp, float4(pos, 1.0f));
	output.depth_ls = mul(vm, float4(pos, 1.0f)).z;
	
	return output;
}

struct PS_OUTPUT
{
	float4 output_buffer : SV_TARGET0;
};

Texture2D material_albedo : register(t0);
Texture2D material_normal : register(t1);
Texture2D material_roughness : register(t2);
Texture2D material_metallic : register(t3);
Texture2D material_ao : register(t4);
Texture2D material_emissive : register(t5);

SamplerState s0 : register(s0);

cbuffer MaterialProperties : register(b2)
{
	MaterialData data;
}


float4 main_ps(VS_OUTPUT input) : SV_TARGET0
{
	float4 debug_col = float4(0.0f, 0.0f, 0.0f, 1.0f);

	float depth = input.pos.z / input.pos.w;

	debug_col = float4(depth.xxx, 1.0f);

	return debug_col;
}

#endif //__SHADOW_MAPPING_HLSL__
