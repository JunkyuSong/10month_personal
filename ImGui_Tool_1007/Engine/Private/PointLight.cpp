#include "..\Public\PointLight.h"
#include "Shader.h"
#include "VIBuffer_Rect.h"
#include "Frustum.h"

CPointLight::CPointLight(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CLight(pDevice, pContext)
{
}

HRESULT CPointLight::Initialize()
{
	return S_OK;
}

HRESULT CPointLight::Render(CShader * pShader, CVIBuffer_Rect * pVIBuffer, CFrustum* _pFrustum)
{
	return S_OK;
}


void CPointLight::Free()
{
}
