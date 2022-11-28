#include "Client_Shader_Defines.hpp"

matrix		g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D	g_DiffuseTexture;
texture2D	g_NoiseTexture;

float3		g_Right, g_Up, g_Look;
float		g_Width;
float3		g_vPos_1;
float3		g_vPos_2;

float3		g_Center;
float2		g_Size;
float		g_Alpha;

vector		g_vCamPosition;

int			g_iMaxY;
int			g_iMaxX;
int			g_iCurY;
int			g_iCurX;

float		g_fTime;
float		g_fUV_x;

float4		g_Color;

struct POINTLIST
{
	vector	vPos[2];
};

POINTLIST	g_PointList;

sampler DefaultSampler = sampler_state {

	filter = min_mag_mip_linear;
	/*minfilter = linear;
	magfilter = linear;
	mipfilter = linear;*/
};

sampler PointSampler = sampler_state {

	filter = min_mag_mip_point;
	/*minfilter = linear;
	magfilter = linear;
	mipfilter = linear;*/
};

struct VS_IN
{
	float4		vPosition : POSITION;
	float2		vSize : PSIZE;
};

struct VS_OUT
{
	float4		vPosition : POSITION;
	float2		vSize : PSIZE;
};


VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT		Out = (VS_OUT)0;
	
	return Out;
}

VS_OUT VS_PARTICLE(VS_IN In)
{
	VS_OUT		Out = (VS_OUT)0;

	Out.vPosition = mul(In.vPosition, g_WorldMatrix);

	return Out;
}


struct GS_IN
{
	float4		vPosition : POSITION;
};

struct GS_OUT
{
	float4		vPosition : SV_POSITION;
	float2		vTexUV : TEXCOORD0;
};

[maxvertexcount(12)]
void GS_MAIN(point GS_IN In[1], inout TriangleStream<GS_OUT> GeometryStream)
{
	GS_OUT			Out[8];
	matrix			matVP = mul(g_ViewMatrix, g_ProjMatrix);

	Out[0].vPosition = mul(vector(g_vPos_2 - (g_Right * g_Width), 1.f), matVP);
	Out[0].vTexUV = float2(0.f, 1.f);

	Out[1].vPosition = mul(vector(g_vPos_2 + (g_Right * g_Width), 1.f), matVP);
	Out[1].vTexUV = float2(0.f, 0.f);

	Out[2].vPosition = mul(vector(g_vPos_1 + (g_Right * g_Width), 1.f), matVP);
	Out[2].vTexUV = float2(1.f, 0.f);

	Out[3].vPosition = mul(vector(g_vPos_1 - (g_Right * g_Width), 1.f), matVP);
	Out[3].vTexUV = float2(1.f, 1.f);

	GeometryStream.Append(Out[0]);
	GeometryStream.Append(Out[1]);
	GeometryStream.Append(Out[2]);
	GeometryStream.RestartStrip();

	GeometryStream.Append(Out[0]);
	GeometryStream.Append(Out[2]);
	GeometryStream.Append(Out[3]);
	GeometryStream.RestartStrip();



	Out[4].vPosition = mul(vector(g_vPos_1 + (g_Up * g_Width), 1.f), matVP);
	Out[4].vTexUV = float2(0.f, 1.f);

	Out[5].vPosition = mul(vector(g_vPos_2 + (g_Up * g_Width), 1.f), matVP);
	Out[5].vTexUV = float2(0.f, 0.f);

	Out[6].vPosition = mul(vector(g_vPos_2 - (g_Up * g_Width), 1.f), matVP);
	Out[6].vTexUV = float2(1.f, 0.f);

	Out[7].vPosition = mul(vector(g_vPos_1 - (g_Up * g_Width), 1.f), matVP);
	Out[7].vTexUV = float2(1.f, 1.f);

	//Out[4].vPosition = mul(vector(g_PointList.vPos[0] + vector(g_Up * g_Width,1.f)), matVP);
	//Out[4].vTexUV = float2(0.f, 0.f);

	//Out[5].vPosition = mul(vector(g_PointList.vPos[1] + vector(g_Up * g_Width,1.f)), matVP);
	//Out[5].vTexUV = float2(1.f, 0.f);

	//Out[6].vPosition = mul(vector(g_PointList.vPos[1] - vector(g_Up * g_Width,1.f)), matVP);
	//Out[6].vTexUV = float2(1.f, 1.f);

	//Out[7].vPosition = mul(vector(g_PointList.vPos[0] - vector(g_Up * g_Width,1.f)), matVP);
	//Out[7].vTexUV = float2(0.f, 1.f);

	GeometryStream.Append(Out[4]);
	GeometryStream.Append(Out[5]);
	GeometryStream.Append(Out[6]);
	GeometryStream.RestartStrip();


	GeometryStream.Append(Out[4]);
	GeometryStream.Append(Out[6]);
	GeometryStream.Append(Out[7]);
	GeometryStream.RestartStrip();
}

[maxvertexcount(12)]
void GS_ARROW(point GS_IN In[1], inout TriangleStream<GS_OUT> GeometryStream)
{
	GS_OUT			Out[8];
	matrix			matVP = mul(g_ViewMatrix, g_ProjMatrix);

	Out[0].vPosition = mul(vector(g_vPos_2 - (g_Right * g_Width), 1.f), matVP);
	Out[0].vTexUV = float2(0.f, 1.f);

	Out[1].vPosition = mul(vector(g_vPos_2 + (g_Right * g_Width), 1.f), matVP);
	Out[1].vTexUV = float2(0.f, 0.f);

	Out[2].vPosition = mul(vector(g_vPos_1 + (g_Right * g_Width), 1.f), matVP);
	Out[2].vTexUV = float2(1.f, 0.f);

	Out[3].vPosition = mul(vector(g_vPos_1 - (g_Right * g_Width), 1.f), matVP);
	Out[3].vTexUV = float2(1.f, 1.f);

	GeometryStream.Append(Out[0]);
	GeometryStream.Append(Out[1]);
	GeometryStream.Append(Out[2]);
	GeometryStream.RestartStrip();

	GeometryStream.Append(Out[0]);
	GeometryStream.Append(Out[2]);
	GeometryStream.Append(Out[3]);
	GeometryStream.RestartStrip();



	//Out[4].vPosition = mul(vector(g_vPos_1 + (g_Up * g_Width), 1.f), matVP);
	//Out[4].vTexUV = float2(0.f, 1.f);

	//Out[5].vPosition = mul(vector(g_vPos_2 + (g_Up * g_Width), 1.f), matVP);
	//Out[5].vTexUV = float2(0.f, 0.f);

	//Out[6].vPosition = mul(vector(g_vPos_2 - (g_Up * g_Width), 1.f), matVP);
	//Out[6].vTexUV = float2(1.f, 0.f);

	//Out[7].vPosition = mul(vector(g_vPos_1 - (g_Up * g_Width), 1.f), matVP);
	//Out[7].vTexUV = float2(1.f, 1.f);

	////Out[4].vPosition = mul(vector(g_PointList.vPos[0] + vector(g_Up * g_Width,1.f)), matVP);
	////Out[4].vTexUV = float2(0.f, 0.f);

	////Out[5].vPosition = mul(vector(g_PointList.vPos[1] + vector(g_Up * g_Width,1.f)), matVP);
	////Out[5].vTexUV = float2(1.f, 0.f);

	////Out[6].vPosition = mul(vector(g_PointList.vPos[1] - vector(g_Up * g_Width,1.f)), matVP);
	////Out[6].vTexUV = float2(1.f, 1.f);

	////Out[7].vPosition = mul(vector(g_PointList.vPos[0] - vector(g_Up * g_Width,1.f)), matVP);
	////Out[7].vTexUV = float2(0.f, 1.f);

	//GeometryStream.Append(Out[4]);
	//GeometryStream.Append(Out[5]);
	//GeometryStream.Append(Out[6]);
	//GeometryStream.RestartStrip();


	//GeometryStream.Append(Out[4]);
	//GeometryStream.Append(Out[6]);
	//GeometryStream.Append(Out[7]);
	//GeometryStream.RestartStrip();
}

[maxvertexcount(6)]
void GS_UIRECT(point GS_IN In[1], inout TriangleStream<GS_OUT> GeometryStream)
{
	GS_OUT			Out[4];

	

	matrix			matVP = mul(g_ViewMatrix, g_ProjMatrix);

	Out[0].vPosition = mul(vector(g_Center.x - g_Size.x * 0.5f, g_Center.y + g_Size.y * 0.5f, g_Center.z, 1.f), matVP);
	Out[0].vTexUV = float2(0.f, 0.f);

	Out[1].vPosition = mul(vector(g_Center.x + g_Size.x * 0.5f, g_Center.y + g_Size.y * 0.5f, g_Center.z, 1.f), matVP);
	Out[1].vTexUV = float2(1.f, 0.f);

	Out[2].vPosition = mul(vector(g_Center.x + g_Size.x * 0.5f, g_Center.y - g_Size.y * 0.5f, g_Center.z, 1.f), matVP);
	Out[2].vTexUV = float2(1.f, 1.f);

	Out[3].vPosition = mul(vector(g_Center.x - g_Size.x * 0.5f, g_Center.y - g_Size.y * 0.5f, g_Center.z, 1.f), matVP);
	Out[3].vTexUV = float2(0.f, 1.f);

	GeometryStream.Append(Out[0]);
	GeometryStream.Append(Out[1]);
	GeometryStream.Append(Out[2]);
	GeometryStream.RestartStrip();


	GeometryStream.Append(Out[0]);
	GeometryStream.Append(Out[2]);
	GeometryStream.Append(Out[3]);
	GeometryStream.RestartStrip();
}

[maxvertexcount(6)]
void GS_PARTICLE(point GS_IN In[1], inout TriangleStream<GS_OUT> GeometryStream)
{
	GS_OUT			Out[4];

	float3			vLook = (In[0].vPosition - g_vCamPosition).xyz;
	float3			vRight = normalize(cross(float3(0.f, 1.f, 0.f), vLook)) * g_Size.x * 0.5f;
	float3			vUp = normalize(cross(vLook, vRight)) * g_Size.y * 0.5f;

	matrix			matVP = mul(g_ViewMatrix, g_ProjMatrix);

	Out[0].vPosition = mul(vector(In[0].vPosition.xyz + vRight + vUp, 1.f), matVP);
	Out[0].vTexUV = float2(0.f, 0.f);

	Out[1].vPosition = mul(vector(In[0].vPosition.xyz - vRight + vUp, 1.f), matVP);
	Out[1].vTexUV = float2(1.f, 0.f);

	Out[2].vPosition = mul(vector(In[0].vPosition.xyz - vRight - vUp, 1.f), matVP);
	Out[2].vTexUV = float2(1.f, 1.f);

	Out[3].vPosition = mul(vector(In[0].vPosition.xyz + vRight - vUp, 1.f), matVP);
	Out[3].vTexUV = float2(0.f, 1.f);

	GeometryStream.Append(Out[0]);
	GeometryStream.Append(Out[1]);
	GeometryStream.Append(Out[2]);
	GeometryStream.RestartStrip();


	GeometryStream.Append(Out[0]);
	GeometryStream.Append(Out[2]);
	GeometryStream.Append(Out[3]);
	GeometryStream.RestartStrip();
}

struct PS_IN
{
	float4		vPosition : SV_POSITION;
	float2		vTexUV : TEXCOORD0;
};

struct PS_OUT
{
	float4		vColor : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vColor = float4( 1.f, 1.f, 0.f, 0.5f );
	if (0 >= Out.vColor.a)
		discard;
	//Out.vColor = pow(Out.vColor, 2.2f);
	return Out;	
}

PS_OUT PS_CROSSTRAIL(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vColor = g_Color;
	if (0 >= Out.vColor.a)
		discard;
	//Out.vColor = pow(Out.vColor, 2.2f);
	return Out;
}

PS_OUT PS_ARROW(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;
	float2 UV = In.vTexUV;
	//UV.x += g_fUV_x;
	float4 Mask = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);
	Out.vColor = g_Color * Mask.r * 4.f;
	Out.vColor.a = Mask.r* 2.f;
	if (0 >= Out.vColor.a)
		discard;
	//Out.vColor = pow(Out.vColor, 2.2f);
	return Out;
}

PS_OUT PS_TEXTURE(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vColor = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);
	
	Out.vColor.a = g_Alpha;
	if (0 >= Out.vColor.a)
		discard;
	//Out.vColor = pow(Out.vColor, 2.2f);
	return Out;
}

PS_OUT PS_PARTICLE(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	float4 _vNoise = g_NoiseTexture.Sample(DefaultSampler, In.vTexUV);

	In.vTexUV.y = (1.f / g_iMaxY * g_iCurY) + (1.f / g_iMaxY * In.vTexUV.y);
	In.vTexUV.x = (1.f / g_iMaxX * g_iCurX) + (1.f / g_iMaxX * In.vTexUV.x);

	

	Out.vColor = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV) * _vNoise.r;
	//Out.vColor.a = saturate( _vNoise.r + _vNoise.g + _vNoise.b);
	Out.vColor.a += g_fTime;

	if (0 >= Out.vColor.a)
		discard;



	//Out.vColor = pow(Out.vColor, 2.2f);
	return Out;
}

PS_OUT PS_PARTICLE_Color(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	float4 _vNoise = g_NoiseTexture.Sample(DefaultSampler, In.vTexUV);

	Out.vColor = g_Color * _vNoise.r;
	Out.vColor.a = saturate( _vNoise.r + _vNoise.g + _vNoise.b);
	
	Out.vColor.a -= g_fTime;
	if (0 >= Out.vColor.a)
		discard;
	//Out.vColor = pow(Out.vColor, 2.2f);
	return Out;
}
struct PS_OUT_FIRE
{
	float4		vColor : SV_TARGET0;
	float4		vDistortion : SV_TARGET2;
};
PS_OUT_FIRE PS_FIRE(PS_IN In)
{
	PS_OUT_FIRE		Out = (PS_OUT_FIRE)0;
	In.vTexUV.y = (1.f / g_iMaxY * g_iCurY) + (1.f / g_iMaxY * In.vTexUV.y);
	In.vTexUV.x = (1.f / g_iMaxX * g_iCurX) + (1.f / g_iMaxX * In.vTexUV.x);
	float4 _vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	Out.vColor = g_Color * _vDiffuse.r;
	Out.vDistortion = float4(1.f, 1.f, 1.f, 1.f);
	if (0 >= Out.vColor.a)
		discard;
	return Out;
}

PS_OUT PS_FLARE(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;
	
	float4 _vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	Out.vColor = _vDiffuse;
	Out.vColor.a = _vDiffuse.g - g_fTime * 1.5f;
	if (0 >= Out.vColor.a)
		discard;
	return Out;
}

technique11 DefaultTechnique
{
	pass DefaultPass
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = compile gs_5_0 GS_MAIN();
		PixelShader = compile ps_5_0 PS_MAIN();
	}

	pass DrawUIRect
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = compile gs_5_0 GS_UIRECT();
		PixelShader = compile ps_5_0 PS_TEXTURE();
	}

	pass DrawParticle
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_PARTICLE();
		GeometryShader = compile gs_5_0 GS_PARTICLE();
		PixelShader = compile ps_5_0 PS_PARTICLE();
	}

	pass DrawParticle_Color
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_PARTICLE();
		GeometryShader = compile gs_5_0 GS_PARTICLE();
		PixelShader = compile ps_5_0 PS_PARTICLE_Color();
	}

	pass CROSSTRAIL
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = compile gs_5_0 GS_MAIN();
		PixelShader = compile ps_5_0 PS_CROSSTRAIL();
	}

	pass FIRE
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_PARTICLE();
		GeometryShader = compile gs_5_0 GS_PARTICLE();
		PixelShader = compile ps_5_0 PS_FIRE();
	}

	pass ARROW
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = compile gs_5_0 GS_ARROW();
		PixelShader = compile ps_5_0 PS_ARROW();
	}

	pass FLARE
	{
		SetRasterizerState(RS_CullNone);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_PARTICLE();
		GeometryShader = compile gs_5_0 GS_PARTICLE();
		PixelShader = compile ps_5_0 PS_FLARE();
	}
}