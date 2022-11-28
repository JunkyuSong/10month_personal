#include "..\Public\Light_Manager.h"
#include "Light.h"
#include "DirLight.h"
#include "StaticPointLight.h"
#include "DynamicPointLight.h"
#include "Frustum.h"

IMPLEMENT_SINGLETON(CLight_Manager)

CLight_Manager::CLight_Manager()
	: m_pFrustum(CFrustum::Get_Instance())
{
}

HRESULT CLight_Manager::Initialize(_uint iLv)
{
	m_DynamicPointLights = new vector<class CDynamicPointLight*>[iLv];
	m_DeadDynamicPointLights = new vector<class CDynamicPointLight*>[iLv];

	m_StaticPointLights = new vector<class CStaticPointLight*>[iLv];
	m_DeadStaticPointLights = new vector<class CStaticPointLight*>[iLv];

	m_DirLights = new vector<class CDirLight*>[iLv];
	m_DeadDirLights = new vector<class CDirLight*>[iLv];

	m_iNumLevels = iLv;
	return S_OK;
}

void CLight_Manager::Tick(_float fTimeDelta)
{
	for (_uint i = 0; i < m_iNumLevels; ++i)
	{
		for (_uint j = 0 ; j < m_DynamicPointLights[i].size(); ++j)
		{
			if (m_DynamicPointLights[i][j] && m_DynamicPointLights[i][j]->Tick(fTimeDelta) == false)
			{
				if (m_DeadDynamicPointLights[i][j] == nullptr)
				{
					m_DeadDynamicPointLights[i][j] = m_DynamicPointLights[i][j];
					m_DynamicPointLights[i][j] = nullptr;
				}
			}

		}
	}
	
}

DIRLIGHTDESC * CLight_Manager::Get_DirLightDesc(_uint iLv,_uint iIndex)
{
	if (iIndex >= m_DirLights[iLv].size() || m_DirLights[iLv][iIndex] == nullptr)
		return nullptr;
	return m_DirLights[iLv][iIndex]->Get_LightDesc();
}

POINTLIGHTDESC * CLight_Manager::Get_PointLightDesc(_uint iLv, LIGHTTYPE eLightType, _uint iIndex)
{

	switch (eLightType)
	{
	case Engine::CLight_Manager::DYNAMICPOINTLIHGT:
		if (iIndex >= m_DynamicPointLights[iLv].size() || m_DynamicPointLights[iLv][iIndex] == nullptr)
			return nullptr;
		return m_DynamicPointLights[iLv][iIndex]->Get_LightDesc();
	case Engine::CLight_Manager::STATICPOINTLIHGT:
		if (iIndex >= m_StaticPointLights[iLv].size() || m_StaticPointLights[iLv][iIndex] == nullptr)
			return nullptr;
		return m_StaticPointLights[iLv][iIndex]->Get_LightDesc();
	}

	return nullptr;
}

_uint CLight_Manager::Add_Light(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, _uint iLv, const DIRLIGHTDESC & LightDesc)
{
	CDirLight*		pLight = CDirLight::Create(pDevice, pContext, LightDesc);
	if (nullptr == pLight)
		return -1;

	m_DirLights[iLv].push_back(nullptr);
	m_DeadDirLights[iLv].push_back(pLight);

	return m_DeadDirLights[iLv].size() - 1;
}

_uint CLight_Manager::Add_Light(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, _uint iLv, LIGHTTYPE eLightType, const POINTLIGHTDESC & LightDesc, _float fSpeed, _float fTime)
{
	switch (eLightType)
	{
	case Engine::CLight_Manager::DYNAMICPOINTLIHGT:
		{
			CDynamicPointLight*		pLight = CDynamicPointLight::Create(pDevice, pContext, LightDesc, fSpeed, fTime);
			if (nullptr == pLight)
				return -1;

			m_DynamicPointLights[iLv].push_back(nullptr);
			m_DeadDynamicPointLights[iLv].push_back(pLight);
			return m_DeadDynamicPointLights[iLv].size() - 1;
		}
	break;
	case Engine::CLight_Manager::STATICPOINTLIHGT:
		{
			CStaticPointLight*		pLight = CStaticPointLight::Create(pDevice, pContext, LightDesc);
			if (nullptr == pLight)
				return -1;

			m_StaticPointLights[iLv].push_back(nullptr);
			m_DeadStaticPointLights[iLv].push_back(pLight);
			return m_DeadStaticPointLights[iLv].size() - 1;
		}
	break;
	}

	return -1;
}

HRESULT CLight_Manager::Render(CShader * pShader, CVIBuffer_Rect * pVIBuffer)
{
	for (_uint i = 0; i < m_iNumLevels; ++i)
	{
		for (auto& pLight : m_DirLights[i])
		{
			if (pLight)
				pLight->Render(pShader, pVIBuffer, m_pFrustum);
		}
		for (auto& pLight : m_StaticPointLights[i])
		{
			if (pLight)
				pLight->Render(pShader, pVIBuffer, m_pFrustum);
		}
		for (auto& pLight : m_DynamicPointLights[i])
		{
			if (pLight)
				pLight->Render(pShader, pVIBuffer, m_pFrustum);
		}
	}

	return S_OK;
}

HRESULT CLight_Manager::Light_On(_uint iLv, LIGHTTYPE eLightType, _uint _iIndex)
{
	switch (eLightType)
	{
	case Engine::CLight_Manager::DIRLIGHT:
		if (_iIndex >= m_DeadDirLights[iLv].size() || iLv >= m_iNumLevels)
			return E_FAIL;
		if (m_DirLights[iLv][_iIndex] == nullptr)
		{
			m_DirLights[iLv][_iIndex] = m_DeadDirLights[iLv][_iIndex];
			m_DeadDirLights[iLv][_iIndex] = nullptr;
		}
		break;
	case Engine::CLight_Manager::DYNAMICPOINTLIHGT:
		if (_iIndex >= m_DeadDynamicPointLights[iLv].size() || iLv >= m_iNumLevels)
			return E_FAIL;
		if (m_DynamicPointLights[iLv][_iIndex] == nullptr)
		{
			m_DynamicPointLights[iLv][_iIndex] = m_DeadDynamicPointLights[iLv][_iIndex];
			m_DeadDynamicPointLights[iLv][_iIndex] = nullptr;
			
		}
		static_cast<CDynamicPointLight*>(m_DynamicPointLights[iLv][_iIndex])->Reset_Time();
		break;
	case Engine::CLight_Manager::STATICPOINTLIHGT:
		if (_iIndex >= m_DeadStaticPointLights[iLv].size() || iLv >= m_iNumLevels)
			return E_FAIL;
		if (m_StaticPointLights[iLv][_iIndex] == nullptr)
		{
			m_StaticPointLights[iLv][_iIndex] = m_DeadStaticPointLights[iLv][_iIndex];
			m_DeadStaticPointLights[iLv][_iIndex] = nullptr;
		}
		break;
	}
	return S_OK;
}

HRESULT CLight_Manager::Light_Off(_uint iLv, LIGHTTYPE eLightType, _uint _iIndex)
{
	switch (eLightType)
	{
	case Engine::CLight_Manager::DIRLIGHT:
		if (_iIndex >= m_DirLights[iLv].size() || iLv >= m_iNumLevels)
			return E_FAIL;
		if (m_DeadDirLights[iLv][_iIndex] == nullptr)
		{
			m_DeadDirLights[iLv][_iIndex] = m_DirLights[iLv][_iIndex];
			m_DirLights[iLv][_iIndex] = nullptr;
		}
		break;
	case Engine::CLight_Manager::DYNAMICPOINTLIHGT:
		if (_iIndex >= m_DynamicPointLights[iLv].size() || iLv >= m_iNumLevels)
			return E_FAIL;
		if (m_DeadDynamicPointLights[iLv][_iIndex] == nullptr)
		{
			m_DeadDynamicPointLights[iLv][_iIndex] = m_DynamicPointLights[iLv][_iIndex];
			m_DynamicPointLights[iLv][_iIndex] = nullptr;
		}
		break;
	case Engine::CLight_Manager::STATICPOINTLIHGT:
		if (_iIndex >= m_StaticPointLights[iLv].size() || iLv >= m_iNumLevels)
			return E_FAIL;
		if (m_DeadStaticPointLights[iLv][_iIndex] == nullptr)
		{
			m_DeadStaticPointLights[iLv][_iIndex] = m_StaticPointLights[iLv][_iIndex];
			m_StaticPointLights[iLv][_iIndex] = nullptr;
		}
		break;
	}

	

	return S_OK;
}

HRESULT CLight_Manager::Clear(_uint iLv)
{
	for (auto& pLight : m_DirLights[iLv])
		Safe_Release(pLight);
	m_DirLights[iLv].clear();
	for (auto& pLight : m_DeadDirLights[iLv])
		Safe_Release(pLight);
	m_DeadDirLights[iLv].clear();

	for (auto& pLight : m_StaticPointLights[iLv])
		Safe_Release(pLight);
	m_StaticPointLights[iLv].clear();
	for (auto& pLight : m_DeadStaticPointLights[iLv])
		Safe_Release(pLight);
	m_DeadStaticPointLights[iLv].clear();

	for (auto& pLight : m_DynamicPointLights[iLv])
		Safe_Release(pLight);
	m_DynamicPointLights[iLv].clear();
	for (auto& pLight : m_DeadDynamicPointLights[iLv])
		Safe_Release(pLight);
	m_DeadDynamicPointLights[iLv].clear();
	return S_OK;
}

void CLight_Manager::Free()
{
	for (_uint iLv = 0; iLv < m_iNumLevels; ++iLv)
	{
		for (auto& pLight : m_DirLights[iLv])
			Safe_Release(pLight);
		m_DirLights[iLv].clear();
		for (auto& pLight : m_DeadDirLights[iLv])
			Safe_Release(pLight);
		m_DeadDirLights[iLv].clear();

		for (auto& pLight : m_StaticPointLights[iLv])
			Safe_Release(pLight);
		m_StaticPointLights[iLv].clear();
		for (auto& pLight : m_DeadStaticPointLights[iLv])
			Safe_Release(pLight);
		m_DeadStaticPointLights[iLv].clear();

		for (auto& pLight : m_DynamicPointLights[iLv])
			Safe_Release(pLight);
		m_DynamicPointLights[iLv].clear();
		for (auto& pLight : m_DeadDynamicPointLights[iLv])
			Safe_Release(pLight);
		m_DeadDynamicPointLights[iLv].clear();
	}

	Safe_Delete_Array(m_DynamicPointLights);
	Safe_Delete_Array(m_DeadDynamicPointLights);

	Safe_Delete_Array(m_StaticPointLights);
	Safe_Delete_Array(m_DeadStaticPointLights);

	Safe_Delete_Array(m_DirLights);
	Safe_Delete_Array(m_DeadDirLights);
}
