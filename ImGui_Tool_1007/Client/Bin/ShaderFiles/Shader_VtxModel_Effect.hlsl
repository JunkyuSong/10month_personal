#include "Client_Shader_Defines.hpp"
matrix		g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D	g_DiffuseTexture;
float		g_fAlpha;

struct tagBoneMatrices
{
	matrix		BoneMatrix[256];
};
tagBoneMatrices		g_BoneMatrices;

float4		g_fColor;

float		g_fTime;
float		g_fAppear;

//119.f, 215.f, 223.f
//187.f, 229.f,217.f
// 119.f, 245.f, 200.f


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
	float4		vDepth : SV_TARGET1;
};

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = g_fColor;

	vector Mask = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	Out.vDiffuse *= Mask.r;

	if (0 >= Out.vDiffuse.a)
		discard;

	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w, 0.0f, 0.0f);
	Out.vDiffuse = pow(Out.vDiffuse, 2.2f);
	return Out;
}

PS_OUT PS_BAT_CLAW(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;


	vector Mask = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	Out.vDiffuse = Mask;
	Out.vDiffuse.a = Mask.r;
	if (0 >= Out.vDiffuse.a)
		discard;

	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w, 0.0f, 0.0f);
	//Out.vDiffuse = pow(Out.vDiffuse, 2.2f);
	return Out;
}

PS_OUT PS_TWIST(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	vector Mask = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	Out.vDiffuse = Mask * 5.f;
	Out.vDiffuse.a = Mask.g;
	if (0.2 >= Out.vDiffuse.a)
		discard;

	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w, 0.0f, 0.0f);
	//Out.vDiffuse = pow(Out.vDiffuse, 2.2f);
	return Out;
}

PS_OUT PS_DISAPEARTWIST(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	vector Mask = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	Out.vDiffuse = Mask * 5.f;
	Out.vDiffuse.a = Mask.g;
		
	Out.vDiffuse.a -= g_fTime;

	if (0.2 >= Out.vDiffuse.a)
		discard;

	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w, 0.0f, 0.0f);
	//Out.vDiffuse = pow(Out.vDiffuse, 2.2f);
	return Out;

}

PS_OUT PS_MAIN_WIND(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = g_fColor;

	vector Mask = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	Out.vDiffuse = Mask;
	Out.vDiffuse.a *= 0.7f;
	if (0 >= Out.vDiffuse.a)
		discard;

	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w, 0.0f, 0.0f);
//	Out.vDiffuse = pow(Out.vDiffuse, 2.2f);
	return Out;
}

PS_OUT PS_SKILLWEAPON(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = g_fColor;

	vector Mask = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	Out.vDiffuse *= Mask.r;

	Out.vDiffuse.a = 1.f;

	if (0 >= Out.vDiffuse.a)
		discard;

	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w, 0.0f, 0.0f);
	Out.vDiffuse = pow(Out.vDiffuse, 2.2f);
	return Out;
}

PS_OUT PS_SKILLWEAPON2(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = g_fColor;

	vector Mask = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	Out.vDiffuse *= Mask.r;

	//Out.vDiffuse.a = 1.f;

	

	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w, 0.0f, 0.0f);
	Out.vDiffuse = pow(Out.vDiffuse, 2.2f);
	return Out;
}

PS_OUT PS_CLAWDEAD(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = g_fColor;

	vector Mask = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	Out.vDiffuse *= Mask.r;
	Out.vDiffuse.g += g_fTime;
	Out.vDiffuse.a -= g_fTime;

	if (0 >= Out.vDiffuse.a)
		discard;

	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w, 0.0f, 0.0f);
	Out.vDiffuse = pow(Out.vDiffuse, 2.2f);
	return Out;
}

PS_OUT PS_BAT_CLAW_DEAD(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	vector Mask = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	Out.vDiffuse = Mask;
	Out.vDiffuse.a = Mask.r;
	Out.vDiffuse.a -= g_fTime;

	if (0 >= Out.vDiffuse.a)
		discard;

	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w, 0.0f, 0.0f);
	//Out.vDiffuse = pow(Out.vDiffuse, 2.2f);
	return Out;
}

PS_OUT PS_WINDDEAD(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = g_fColor;

	vector Mask = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	Out.vDiffuse = Mask;
	Out.vDiffuse.a *= 0.7f;
	Out.vDiffuse.a -= g_fTime;

	if (0 >= Out.vDiffuse.a)
		discard;

	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w, 0.0f, 0.0f);
	Out.vDiffuse = pow(Out.vDiffuse, 2.2f);
	return Out;
}

PS_OUT PS_LENCEDEAD(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = g_fColor;

	vector Mask = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);
	float2 vMyUV = In.vTexUV;
	
	//남은게 전체에서 얼마나 되는 비율인가
	// 원래 y/전체 * 남은거 비율
	
	vMyUV.y = saturate(In.vTexUV.y - (g_fTime)) / (1 - g_fTime);

	vector Mask2 = g_DiffuseTexture.Sample(DefaultSampler, vMyUV);
	Out.vDiffuse *= Mask2.r;
	Out.vDiffuse.a = 1.f;

	if (In.vTexUV.y < g_fTime)
		Out.vDiffuse.a = 0.f;

	if (0 >= Out.vDiffuse.a)
		discard;

	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w, 0.0f, 0.0f);
	Out.vDiffuse = pow(Out.vDiffuse, 2.2f);
	return Out;
}

PS_OUT PS_REVERSEDEAD(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = g_fColor;

	vector Mask = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);
	float2 vMyUV = In.vTexUV;

	//남은게 전체에서 얼마나 되는 비율인가
	// 원래 y/전체 * 남은거 비율

	vMyUV.y = saturate(In.vTexUV.y - (g_fTime)) / (1 - g_fTime);

	vector Mask2 = g_DiffuseTexture.Sample(DefaultSampler, vMyUV);
	Out.vDiffuse *= Mask2.r;
	Out.vDiffuse.a = 1.f;

	if (1.f - In.vTexUV.y < g_fTime)
		Out.vDiffuse.a = 0.f;

	if (0 >= Out.vDiffuse.a)
		discard;

	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w, 0.0f, 0.0f);
	Out.vDiffuse = pow(Out.vDiffuse, 2.2f);
	return Out;
}


PS_OUT PS_DEAD_X(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = g_fColor;

	vector Mask = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);
	float2 vMyUV = In.vTexUV;

	//남은게 전체에서 얼마나 되는 비율인가
	// 원래 y/전체 * 남은거 비율

	vMyUV.x = saturate(In.vTexUV.x + (g_fTime)) / (g_fTime);

	vector Mask2 = g_DiffuseTexture.Sample(DefaultSampler, vMyUV);
	Out.vDiffuse *= Mask2.r;
	Out.vDiffuse.a = 1.f;

	if (1.f - In.vTexUV.x < g_fTime)
		Out.vDiffuse.a = 0.f;

	if (0 >= Out.vDiffuse.a)
		discard;

	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w, 0.0f, 0.0f);
	Out.vDiffuse = pow(Out.vDiffuse, 2.2f);
	return Out;
}

PS_OUT PS_TARGET(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	if (In.vTexUV.y < 1.f - g_fAppear)
		discard;


	Out.vDiffuse = g_fColor;

	float2 vMyUV = In.vTexUV;
	vMyUV.y += g_fTime;
	vector Mask = g_DiffuseTexture.Sample(DefaultSampler, vMyUV);


	Out.vDiffuse *= Mask.r*2.f;
	if (0 >= Out.vDiffuse.a)
		discard;

	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w, 0.0f, 0.0f);

	return Out;
}

technique11 DefaultTechnique
{
	pass Default
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Test, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}

	pass SkillWeapon
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_SKILLWEAPON();
	}

	pass CLAWDEAD
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_CLAWDEAD();
	}

	pass LENCEDEAD
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_LENCEDEAD();
	}

	pass REVERSEDEAD
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_REVERSEDEAD();
	}
	pass DEAD_X
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_DEAD_X();
	}

	pass WINDDEAD
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_WINDDEAD();
	}

	pass WIND
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Test, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_WIND();
	}

	pass TWIST
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Test, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_TWIST();
	}

	pass DISAPEARTWIST
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Test, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_DISAPEARTWIST();
	}

	pass ArrowTarget
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Test, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_TARGET();
	}

	pass BAT_CLAW
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Test, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_BAT_CLAW();
	}

	pass BAT_CLAW_DEAD
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_BAT_CLAW_DEAD();
	}
}