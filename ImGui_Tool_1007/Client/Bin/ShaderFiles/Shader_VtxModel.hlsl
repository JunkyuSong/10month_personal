#include "Client_Shader_Defines.hpp"
matrix		g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D	g_DiffuseTexture;
texture2D	g_NormalTexture;

matrix		g_LightViewInverse;

vector		g_CamPosition;

float		g_fAlpha;

float		g_fY;

sampler DefaultSampler = sampler_state {

	filter = min_mag_mip_linear;
	/*minfilter = linear;
	magfilter = linear;
	mipfilter = linear;*/
	AddressU = WRAP;
	AddressV = WRAP;
};

sampler PointSampler = sampler_state {

	filter = min_mag_mip_point;
	/*minfilter = linear;
	magfilter = linear;
	mipfilter = linear;*/
	AddressU = WRAP;
	AddressV = WRAP;
};

struct VS_IN
{
	float3		vPosition : POSITION;
	float3		vNormal : NORMAL;
	float2		vTexUV : TEXCOORD0;
	float3		vTangent : TANGENT;
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
};


VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT		Out = (VS_OUT)0;

	matrix		matWV, matWVP;

	matWV = mul(g_WorldMatrix, g_ViewMatrix);
	matWVP = mul(matWV, g_ProjMatrix);

	Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
	Out.vNormal = normalize(mul(float4(In.vNormal, 0.f), g_WorldMatrix)).xyz;
	Out.vTangent = normalize(mul(float4(In.vTangent, 0.f), g_WorldMatrix)).xyz;
	Out.vBinormal = normalize(cross(Out.vNormal, Out.vTangent));
	Out.vTexUV = In.vTexUV;
	Out.vProjPos = Out.vPosition;
	Out.vWorldPosition = mul(float4(In.vPosition, 1.f), g_WorldMatrix);

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

struct PS_OUT_BLEND
{
	float4		vDiffuse : SV_TARGET0;
	float4		vDepth : SV_TARGET1;
	float4		vDistosion : SV_TARGET2;
	
};

struct PS_SHADOW_OUT
{
	float4		vShadowDepth : SV_TARGET0;
};

struct PS_OUT_NONLIGHT
{
	float4		vDiffuse : SV_TARGET0;
	float4		vDepth : SV_TARGET1;
};

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = (vector)1.f;

	Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);
	Out.vDiffuse = pow(Out.vDiffuse, 2.2f);

	vector		vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexUV);

	float3		vNormal = vNormalDesc.xyz * 2.f - 1.f;

	float3x3	WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);

	vNormal = normalize(mul(vNormal, WorldMatrix));

	if (0 >= Out.vDiffuse.a)
		discard;

	// -1 ~ 1
	//  0 ~ 1
	Out.vNormal = vector(vNormal.xyz * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w, 0.0f, 0.0f);


	return Out;
}

PS_SHADOW_OUT PS_SHADOW(PS_IN In)
{
	PS_SHADOW_OUT		Out = (PS_SHADOW_OUT)0;

	vector vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	if (0 == vDiffuse.a)
		discard;

	Out.vShadowDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 300.f, 0.0f, 0.0f);


	return Out;
}

PS_OUT_NONLIGHT PS_NONLIGHT(PS_IN In)
{
	PS_OUT_NONLIGHT		Out = (PS_OUT_NONLIGHT)0;

	Out.vDiffuse = (vector)1.f;

	Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);
	Out.vDiffuse = pow(Out.vDiffuse, 2.2f);

	if (0 >= Out.vDiffuse.a)
		discard;

	// -1 ~ 1
	//  0 ~ 1
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w, 0.0f, 0.0f);


	return Out;
}

PS_OUT PS_BALLOON(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	if (In.vWorldPosition.y < g_fY + 4.f)
		discard;

	Out.vDiffuse = (vector)1.f;

	Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);
	Out.vDiffuse = pow(Out.vDiffuse, 2.2f);
	vector		vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexUV);

	float3		vNormal = vNormalDesc.xyz * 2.f - 1.f;

	float3x3	WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);

	vNormal = normalize(mul(vNormal, WorldMatrix));

	if (0 >= Out.vDiffuse.a)
		discard;

	// -1 ~ 1
	//  0 ~ 1
	Out.vNormal = vector(vNormal.xyz * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w, 0.0f, 0.0f);


	return Out;
}

PS_OUT PS_MAIN_2(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = (vector)1.f;

	Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);
	Out.vDiffuse = pow(Out.vDiffuse, 2.2f);
	if (0 == Out.vDiffuse.a)
		discard;

	// -1 ~ 1
	//  0 ~ 1
	Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w, 0.0f, 0.0f);


	return Out;
}

PS_OUT PS_Sky(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = (vector)1.f;

	Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);
	Out.vDiffuse = pow(Out.vDiffuse, 2.2f);
	if (0 == Out.vDiffuse.a)
		discard;

	// -1 ~ 1
	//  0 ~ 1
	Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w, 0.0f, 0.0f);
	Out.vDiffuse = pow(Out.vDiffuse, 2.2f);

	return Out;
}

PS_OUT PS_Stage(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = (vector)1.f;

	Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);
	Out.vDiffuse = pow(Out.vDiffuse, 2.2f);
	vector		vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexUV);

	float3		vNormal = vNormalDesc.xyz * 2.f - 1.f;

	float3x3	WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);

	vNormal = normalize(mul(vNormal, WorldMatrix));

	if (0 >= Out.vDiffuse.a)
		discard;

	// -1 ~ 1
	//  0 ~ 1
	Out.vNormal = vector(vNormal.xyz * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w, 0.0f, 0.0f);


	return Out;
}

PS_OUT_BLEND PS_MAIN_3(PS_IN In)
{
	PS_OUT_BLEND		Out = (PS_OUT_BLEND)0;

	Out.vDiffuse = (vector)1.f;

	vector			vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);
	Out.vDiffuse = vMtrlDiffuse;
	Out.vDiffuse.a *= g_fAlpha;

	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w, 0.0f, 0.0f);
	if (0 >= Out.vDiffuse.a)
		discard;
	
	return Out;
}

PS_OUT PS_TEST(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = (vector)1.f;

	vector			vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);
	Out.vDiffuse = vMtrlDiffuse;
	Out.vDiffuse.a = 0.5f;

	if (0 == Out.vDiffuse.a)
		discard;
	Out.vDiffuse = pow(Out.vDiffuse, 2.2f);
	return Out;
}

PS_OUT PS_MAIN_SEl(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = (vector)1.f;

	vector			vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);


	Out.vDiffuse = vMtrlDiffuse;
	//Out.vColor = (g_vLightDiffuse * vMtrlDiffuse) * saturate(In.fShade + (g_vLightAmbient * g_vMtrlAmbient)) +
	//	(g_vLightSpecular * g_vMtrlSpecular) * In.fSpecular + 0.8f;
	Out.vDiffuse.g = 1.f;
	Out.vDiffuse *= vector(0.8f, 1.f, 0.8f, 1.f);
	if (0 == Out.vDiffuse.a)
		discard;
	Out.vDiffuse = pow(Out.vDiffuse, 2.2f);
	return Out;
}

PS_OUT PS_MAIN_ADD(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = (vector)1.f;

	vector			vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);


	Out.vDiffuse = vMtrlDiffuse;
	//Out.vColor = (g_vLightDiffuse * vMtrlDiffuse) * saturate(In.fShade + (g_vLightAmbient * g_vMtrlAmbient)) +
	//	(g_vLightSpecular * g_vMtrlSpecular) * In.fSpecular + 0.8f;
	/*Out.vColor.g = Out.vColor.b;
	Out.vColor.r = Out.vColor.b;*/

	Out.vDiffuse.r = 1.f;
	Out.vDiffuse *= vector(1.f, 0.8f, 0.8f, 1.f);

	if (0 == Out.vDiffuse.a)
		discard;
	Out.vDiffuse = pow(Out.vDiffuse, 2.2f);
	return Out;
}

PS_OUT PS_MAIN4(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = (vector)1.f;

	Out.vDiffuse.r = 1.f;
	Out.vDiffuse.g = 1.f;
	Out.vDiffuse.b = 0.f;

	
	if (0 == Out.vDiffuse.a)
		discard;
	Out.vDiffuse = pow(Out.vDiffuse, 2.2f);
	return Out;
}
PS_OUT PS_SKILLWEAPON(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = (vector)1.f;


	vector			vMtrlDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	Out.vDiffuse = vMtrlDiffuse;
	Out.vDiffuse.a = 1.f;
	if (0 == Out.vDiffuse.a)
		discard;

	return Out;
}
technique11 DefaultTechnique
{

	/*pass DefaultPass
	{
	VertexShader = compile vs_5_0 VS_MAIN();
	GeometryShader = NULL;
	PixelShader = compile ps_5_0 PS_MAIN();
	}*/
	pass DefaultPass2
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}
	pass SelectPass
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_SEl();
	}

	pass AddPass
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_ADD();
	}

	pass MagicianWeapon
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_3();
	}

	pass MagicianCard
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN4();
	}

	pass Stage
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Default, 0);
		//BS_AlphaBlend
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_Stage();
	}

	pass SkyBox
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Skybox, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		/*	SetBlendState(DefaultBlendState);
		SetRasterizerState();
		*/

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_Sky();
	}


	pass TEST
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_TEST();
	}

	pass BALLOON
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_BALLOON();
	}

	pass NONLIGHT
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_NONLIGHT();
	}
	pass SKILLWEAPON
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_SKILLWEAPON();
	}

	pass Shadow
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_SHADOW();
	}

}