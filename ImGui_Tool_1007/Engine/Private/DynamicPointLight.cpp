#include "..\Public\DynamicPointLight.h"
#include "Shader.h"
#include "VIBuffer_Rect.h"
#include "Frustum.h"

CDynamicPointLight::CDynamicPointLight(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CPointLight(pDevice, pContext)
{

}

HRESULT CDynamicPointLight::Initialize(const POINTLIGHTDESC & LightDesc, _float fSpeed, _float fTime)
{
	m_LightDesc = LightDesc;
	m_fTime = fTime;
	m_fSpeed = fSpeed;

	return S_OK;
}

_bool CDynamicPointLight::Tick(_float TimeDelta)
{
	m_fCurTime += TimeDelta;
	if (m_fCurTime > m_fTime)
	{
		m_fCurTime = 0.f;
		return false;
	}

	m_LightDesc.fRange += m_fSpeed;

	return true;
}

HRESULT CDynamicPointLight::Render(CShader * pShader, CVIBuffer_Rect * pVIBuffer, CFrustum* _pFrustum)
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

CDynamicPointLight * CDynamicPointLight::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const POINTLIGHTDESC & LightDesc, _float fSpeed, _float fTime)
{
	CDynamicPointLight*			pInstance = new CDynamicPointLight(pDevice, pContext);

	if (FAILED(pInstance->Initialize(LightDesc, fSpeed, fTime)))
	{
		MSG_BOX(TEXT("Failed To Created : CDynamicPointLight"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CDynamicPointLight::Free()
{
}
