#include "..\Public\StaticPointLight.h"
#include "Shader.h"
#include "VIBuffer_Rect.h"
#include "Frustum.h"

CStaticPointLight::CStaticPointLight(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CPointLight(pDevice, pContext)
{

}

HRESULT CStaticPointLight::Initialize(const POINTLIGHTDESC & LightDesc)
{
	m_LightDesc = LightDesc;

	return S_OK;
}

HRESULT CStaticPointLight::Render(CShader * pShader, CVIBuffer_Rect * pVIBuffer, CFrustum* _pFrustum)
{
	

	_uint		iPassIndex = 2;

	
	if (!_pFrustum->isIn_WorldSpace(XMLoadFloat4(&m_LightDesc.vPosition), m_LightDesc.fRange))
	{
		return S_OK;
	}

	if (FAILED(pShader->Set_RawValue("g_vLightPos", &m_LightDesc.vPosition, sizeof(_float4))))
		return E_FAIL;

	if (FAILED(pShader->Set_RawValue("g_fLightRange", &m_LightDesc.fRange, sizeof(_float))))
		return E_FAIL;

	if (FAILED(pShader->Set_RawValue("g_vLightDiffuse", &m_LightDesc.vDiffuse, sizeof(_float4))))
		return E_FAIL;

	if (FAILED(pShader->Set_RawValue("g_vLightAmbient", &m_LightDesc.vAmbient, sizeof(_float4))))
		return E_FAIL;

	if (FAILED(pShader->Set_RawValue("g_vLightSpecular", &m_LightDesc.vSpecular, sizeof(_float4))))
		return E_FAIL;

	pShader->Begin(iPassIndex);

	pVIBuffer->Render();

	return S_OK;
}

CStaticPointLight * CStaticPointLight::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const POINTLIGHTDESC & LightDesc)
{
	CStaticPointLight*			pInstance = new CStaticPointLight(pDevice, pContext);

	if (FAILED(pInstance->Initialize(LightDesc)))
	{
		MSG_BOX(TEXT("Failed To Created : CStaticPointLight"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CStaticPointLight::Free()
{
}
