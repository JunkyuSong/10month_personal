#include "Client_Shader_Defines.hpp"
matrix		g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
float		g_fAlpha;
float		g_fTime;

matrix		g_LightViewInverse;

float4		g_fColor;

float4			g_vCamPosition;
float			g_fFogRange = 10.f;

struct tagBoneMatrices
{
	matrix		BoneMatrix[256];
};

tagBoneMatrices		g_BoneMatrices;

texture2D	g_DiffuseTexture;
texture2D	g_NormalTexture;

texture2D	g_DissolveTexture;

sampler DefaultSampler = sampler_state {

	filter = min_mag_mip_linear;
	/*minfilter = linear;
	magfilter = linear;
	mipfilter = linear;*/
};

struct VS_IN
{
	float3		vPosition : POSITION;
	float3		vNormal : NORMAL;
	float2		vTexUV : TEXCOORD0;
	float3		vTangent : TANGENT;
	uint4		vBlendIndex : BLENDINDEX;
	float4		vBlendWeight : BLENDWEIGHT;
};

struct VS_OUT
{
	float4		vPosition : SV_POSITION;
	float3		vNormal : NORMAL;
	float2		vTexUV : TEXCOORD0;
	float4		vProjPos : TEXCOORD1;
	float3		vTangent : TANGENT;
	float3		vBinormal : BINORMAL;
	float4		vWorldPosition : TEXCOORD2;
	float4		vLightViewPosition : TEXCOORD3;
};


VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT		Out = (VS_OUT)0;

	matrix		matWV, matWVP;

	matWV = mul(g_WorldMatrix, g_ViewMatrix);
	matWVP = mul(matWV, g_ProjMatrix);

	float		fWeightW = 1.f - (In.vBlendWeight.x + In.vBlendWeight.y + In.vBlendWeight.z);

	float4x4	BoneMatrix = g_BoneMatrices.BoneMatrix[In.vBlendIndex.x] * In.vBlendWeight.x +
		g_BoneMatrices.BoneMatrix[In.vBlendIndex.y] * In.vBlendWeight.y +
		g_BoneMatrices.BoneMatrix[In.vBlendIndex.z] * In.vBlendWeight.z +
		g_BoneMatrices.BoneMatrix[In.vBlendIndex.w] * fWeightW;

	vector		vPosition = mul(vector(In.vPosition, 1.f), BoneMatrix);
	Out.vNormal = mul(vector(In.vNormal, 0.f), BoneMatrix).xyz;
	Out.vTangent = mul(vector(In.vTangent, 0.f), BoneMatrix).xyz;
	
	Out.vWorldPosition = mul(vPosition, g_WorldMatrix);
	Out.vPosition = mul(vPosition, matWVP);

	Out.vNormal = normalize(mul(float4(Out.vNormal, 0.f), g_WorldMatrix)).xyz;
	Out.vTangent = normalize(mul(float4(Out.vTangent, 0.f), g_WorldMatrix)).xyz;
	Out.vBinormal = normalize(cross(Out.vNormal, Out.vTangent));

	Out.vTexUV = In.vTexUV;
	Out.vProjPos = Out.vPosition;
	

	return Out;
}

struct PS_IN
{
	float4		vPosition : SV_POSITION;
	float3		vNormal : NORMAL;
	float2		vTexUV : TEXCOORD0;
	float4		vProjPos : TEXCOORD1;
	float3		vTangent : TANGENT;
	float3		vBinormal : BINORMAL;
	float4		vWorldPosition : TEXCOORD2;
};

struct PS_OUT
{
	float4		vDiffuse : SV_TARGET0;
	float4		vNormal : SV_TARGET1;
	float4		vDepth : SV_TARGET2;

};

struct PS_SHADOW_OUT
{
	float4		vShadowDepth : SV_TARGET0;
};

struct PS_OUT_NONLIGHT
{
	float4		vDiffuse : SV_TARGET0;
	float4		vDepth : SV_TARGET2;

};

struct PS_OUT_TRAIL
{
	float4		vDiffuse : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;
	
	

	Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);
	Out.vDiffuse = pow(Out.vDiffuse, 2.2f);
	if (0 == Out.vDiffuse.a)
		discard;

	vector		vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexUV);
	float3		vNormal = vNormalDesc.xyz * 2.f - 1.f;
	float3x3	WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);
	vNormal = normalize(mul(vNormal, WorldMatrix));

	Out.vNormal = vector(vNormal.xyz * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w , 0.0f, 0.0f);


	return Out;
}

PS_SHADOW_OUT PS_SHADOW(PS_IN In)
{
	PS_SHADOW_OUT		Out = (PS_SHADOW_OUT)0;

	vector vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	if (0 == vDiffuse.a)
		discard;

	Out.vShadowDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 300.f, 0.f, 0.0f);


	return Out;
}

PS_OUT PS_RIMLIGHT(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;



	Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);
	Out.vDiffuse = pow(Out.vDiffuse, 2.2f);
	if (0 == Out.vDiffuse.a)
		discard;

	vector		vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexUV);
	float3		vNormal = vNormalDesc.xyz * 2.f - 1.f;
	float3x3	WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);
	vNormal = normalize(mul(vNormal, WorldMatrix));

	float4 ToCamera = normalize(g_vCamPosition - In.vWorldPosition);

	float fRimColor = smoothstep(0.5f, 1.f, 1.f - max(0, dot(ToCamera, normalize(vector(vNormal, 0.f)))));

	//if (dot(ToCamera, normalize(vector(vNormal, 0.f))) < 0.5f)
	Out.vDiffuse *= 0.1f;
		Out.vDiffuse += vector(0.466f,0.96f,0.78f,1.f) * fRimColor;


	Out.vNormal = vector(vNormal.xyz * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w, 0.0f, 0.0f);

	return Out;
}

PS_OUT PS_MAIN2(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = (vector)1.f;

	Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	Out.vDiffuse.a *= (g_fAlpha);
	//Out.vDiffuse = pow(Out.vDiffuse, 2.2f);
	if (0.f >= Out.vDiffuse.a)
		discard;

	return Out;
}
PS_OUT_NONLIGHT PS_MAIN3(PS_IN In)
{
	PS_OUT_NONLIGHT		Out = (PS_OUT_NONLIGHT)0;

	Out.vDiffuse = (vector)1.f;

	Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);
	Out.vDiffuse = pow(Out.vDiffuse, 2.2f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w, 0.0f, 0.0f);


	if (0.f >= Out.vDiffuse.a)
		discard;

	return Out;
}

PS_OUT_TRAIL PS_MAIN_TRAIL(PS_IN In)
{
	PS_OUT_TRAIL		Out = (PS_OUT_TRAIL)0;

	//Out.vColor = (vector)1.f;

	Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	Out.vDiffuse *= vector(0.5f, 0.2f, 0.8f, g_fAlpha);

	//Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w, 0.0f, 0.0f);


	if (0 >= Out.vDiffuse.a)
		discard;
	//Out.vDiffuse = pow(Out.vDiffuse, 2.2f);
	return Out;
}

PS_OUT_NONLIGHT PS_BOW(PS_IN In)
{
	PS_OUT_NONLIGHT		Out = (PS_OUT_NONLIGHT)0;

	Out.vDiffuse = g_fColor;

	vector Mask = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	Out.vDiffuse *= Mask.r;

	if (0 >= Out.vDiffuse.a)
		discard;
	Out.vDiffuse = pow(Out.vDiffuse, 2.2f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w, 0.0f, 0.0f);

	return Out;
}

PS_OUT PS_DISSOLVE(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	float fDissolve = length( g_DissolveTexture.Sample(DefaultSampler, In.vTexUV));
	fDissolve = smoothstep(0.f, 4.f, fDissolve);

	Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	if (fDissolve <= g_fTime)
		discard;

	if (fDissolve <= g_fTime + 0.03f)
	{
		/*Out.vDiffuse.r = 1.f;
		Out.vDiffuse.g *= 1.f - (g_fTime - fDissolve) ;
		Out.vDiffuse.b *= 1.f - (g_fTime - fDissolve) ;*/

		Out.vDiffuse.r = 119.f / 255.f;
		Out.vDiffuse.g = 245.f / 255.f;
		Out.vDiffuse.b = 200.f / 255.f;

	}

	if (0 == Out.vDiffuse.a)
		discard;

	vector		vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexUV);
	float3		vNormal = vNormalDesc.xyz * 2.f - 1.f;
	float3x3	WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);
	vNormal = normalize(mul(vNormal, WorldMatrix));

	Out.vNormal = vector(vNormal.xyz * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w, 0.0f, 0.0f);

	Out.vDiffuse = pow(Out.vDiffuse, 2.2f);

	return Out;
}

PS_OUT PS_COLOR_DISSOLVE(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	float fDissolve = length(g_DissolveTexture.Sample(DefaultSampler, In.vTexUV));
	fDissolve = smoothstep(0.f, 4.f, fDissolve);

	Out.vDiffuse = g_fColor;

	if (fDissolve >= g_fTime)
		discard;

	if (fDissolve >= g_fTime + 0.03f)
	{
		Out.vDiffuse.r = 1.f;
	}

	if (0 == Out.vDiffuse.a)
		discard;

	vector		vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexUV);
	float3		vNormal = vNormalDesc.xyz * 2.f - 1.f;
	float3x3	WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);
	vNormal = normalize(mul(vNormal, WorldMatrix));

	Out.vNormal = vector(vNormal.xyz * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w, 0.0f, 0.0f);

	Out.vDiffuse = pow(Out.vDiffuse, 2.2f);

	return Out;
}

technique11 DefaultTechnique
{
	pass DefaultPass
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}

	pass DefaultPass2
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN2();
	}

	pass MotionTrail
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_TRAIL();
	}

	pass DefaultPass3
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN3();
	}

	pass Dissolve
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_DISSOLVE();
	}

	pass RIMLIGHT
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_RIMLIGHT();
	}

	pass EFFECT_BOW
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_BOW();
	}

	pass COLOR_DISSOLVE
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_COLOR_DISSOLVE();
	}

	pass PASS_SHADOW
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_SHADOW();
	}
}