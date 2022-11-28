#include "..\Public\Light.h"
#include "Shader.h"
#include "VIBuffer_Rect.h"
#include "Frustum.h"

CLight::CLight(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: m_pDevice(pDevice)
	, m_pContext(pContext)
{
	//Safe_AddRef(m_pDevice);
	//Safe_AddRef(m_pContext);
}

HRESULT CLight::Initialize()
{


	return S_OK;
}

HRESULT CLight::Render(CShader * pShader, CVIBuffer_Rect * pVIBuffer, CFrustum* _pFrustum)
{
	
	return S_OK;
}



void CLight::Free()
{
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
