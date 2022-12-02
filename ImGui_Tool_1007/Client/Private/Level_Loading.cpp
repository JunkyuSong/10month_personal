#include "stdafx.h"
#include "..\Public\Level_Loading.h"
#include "GameInstance.h"

#include "Level_Logo.h"
#include "Level_GamePlay.h"
#include "Level_Stage_02_1.h"
#include "Level_Stage_02.h"
#include "Level_Stage_Lobby.h"
#include "Level_Stage_Last.h"

#include "Loader.h"

#include "CameraMgr.h"
#include "TerrainMgr.h"
#include "UI_Mgr.h"


CLevel_Loading::CLevel_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel_Client(pDevice, pContext)
{
}

//HRESULT CLevel_Loading::Initialize(LEVEL eNextLevel)
//{
//	if (FAILED(__super::Initialize()))
//		return E_FAIL;
//
//	m_eNextLevel = eNextLevel;
//
//	m_pLoader = CLoader::Create(m_pDevice, m_pContext, eNextLevel);
//	if (nullptr == m_pLoader)
//		return E_FAIL;
//
//	return S_OK;
//}

HRESULT CLevel_Loading::Initialize(const _uint& _eNextLevel)
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;
	
	if (!m_OneReady)
	{
		if (FAILED(Ready_Component()))
			return E_FAIL;
		m_OneReady = true;
	}

	CCameraMgr::Get_Instance()->Free();
	CTerrainMgr::Get_Instance()->Free();
	CUI_Mgr::Get_Instance()->Free();

	m_eNextLevel = (LEVEL)_eNextLevel;


	switch (m_eNextLevel)
	{
	case LEVEL_LOGO:
		//m_LoadingMax = 21;
		break;

	case LEVEL_GAMEPLAY:
		//m_LoadingMax = 0;
		break;
	case LEVEL_STAGE_02:
		//pLoader->Loading_ForGamePlayLevel();
		break;
	case LEVEL_STAGE_LAST:
		//pLoader->Loading_ForGamePlayLevel();
		break;
	case LEVEL_STAGE_LOBBY:
		//pLoader->Loading_ForGamePlayLevel();
		break;
	case LEVEL_END:
		return S_OK;
	}

	m_pLoader = CLoader::Create(m_pDevice,m_pContext, (LEVEL)_eNextLevel);
	if (nullptr == m_pLoader)
		return E_FAIL;

	return S_OK;
}

void CLevel_Loading::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	if (true == m_pLoader->Get_Finished())
	{
		if (GetKeyState(VK_RETURN) & 0x8000)
		{

			AUTOINSTANCE(CGameInstance, pGameInstance);

			CLevel*		pNewLevel = nullptr;

			switch (m_eNextLevel)
			{
			case LEVEL_LOGO:
				pNewLevel = CLevel_Logo::Create(m_pDevice, m_pContext);
				break;

			case LEVEL_GAMEPLAY:
				pNewLevel = CLevel_GamePlay::Create(m_pDevice, m_pContext);
				break;

			case LEVEL_STAGE_02_1:
				pNewLevel = CLevel_Stage_02_1::Create(m_pDevice, m_pContext);
				break;

			case LEVEL_STAGE_02:
				pNewLevel = CLevel_Stage_02::Create(m_pDevice, m_pContext);
				break;

			case LEVEL_STAGE_LAST:
				pNewLevel = CLevel_Stage_Last::Create(m_pDevice, m_pContext);
				break;

			case LEVEL_STAGE_LOBBY:				
				pNewLevel = CLevel_Stage_Lobby::Create(m_pDevice, m_pContext);
				break;
			}

			if (nullptr == pNewLevel || ((FAILED(pGameInstance->Open_Level(m_eNextLevel, pNewLevel))) || true))
			{
				Safe_Release(m_pLoader);
				return;
			}
		}
	}
}

HRESULT CLevel_Loading::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	switch (m_eNextLevel)
	{
	case LEVEL_LOGO:
		m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4));
		m_pShaderCom->Set_RawValue("g_ViewMatrix", &m_ViewMatrix, sizeof(_float4x4));
		m_pShaderCom->Set_RawValue("g_ProjMatrix", &m_ProjMatrix, sizeof(_float4x4));
		m_pTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture", 5);
		break;
	case LEVEL_GAMEPLAY:
		m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4));
		m_pShaderCom->Set_RawValue("g_ViewMatrix", &m_ViewMatrix, sizeof(_float4x4));
		m_pShaderCom->Set_RawValue("g_ProjMatrix", &m_ProjMatrix, sizeof(_float4x4));
		m_pTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture", 1);
		break;
	case LEVEL_STAGE_02:
		m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4));
		m_pShaderCom->Set_RawValue("g_ViewMatrix", &m_ViewMatrix, sizeof(_float4x4));
		m_pShaderCom->Set_RawValue("g_ProjMatrix", &m_ProjMatrix, sizeof(_float4x4));
		m_pTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture", 3);
		break;
	case LEVEL_STAGE_02_1:
		m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4));
		m_pShaderCom->Set_RawValue("g_ViewMatrix", &m_ViewMatrix, sizeof(_float4x4));
		m_pShaderCom->Set_RawValue("g_ProjMatrix", &m_ProjMatrix, sizeof(_float4x4));
		m_pTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture", 2);
		break;
	case LEVEL_STAGE_LAST:
		m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4));
		m_pShaderCom->Set_RawValue("g_ViewMatrix", &m_ViewMatrix, sizeof(_float4x4));
		m_pShaderCom->Set_RawValue("g_ProjMatrix", &m_ProjMatrix, sizeof(_float4x4));
		m_pTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture", 4);
		break;
	case LEVEL_STAGE_LOBBY:
		m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4));
		m_pShaderCom->Set_RawValue("g_ViewMatrix", &m_ViewMatrix, sizeof(_float4x4));
		m_pShaderCom->Set_RawValue("g_ProjMatrix", &m_ProjMatrix, sizeof(_float4x4));
		m_pTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture",0);
		break;
	}

	SetWindowText(g_hWnd, m_pLoader->Get_LoadingText());

	if (FAILED(m_pShaderCom->Begin(0)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_Loading::Ready_Component()
{
	AUTOINSTANCE(CGameInstance, _pGame);
	m_pTransformCom = static_cast<CTransform*>(_pGame->Clone_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform")));
	m_pTextureCom = static_cast<CTexture*>(_pGame->Clone_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_Loading")));
	m_pShaderCom = static_cast<CShader*>(_pGame->Clone_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxTex")));
	m_pVIBufferCom =static_cast<CVIBuffer_Rect*>(_pGame->Clone_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect")));
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixTranspose(XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f)));
	m_pTransformCom->Set_Scale(XMVectorSet(1280.f, 720.f, 1.f, 0.f));

	return S_OK;
}

CLevel_Loading * CLevel_Loading::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID)
{
	CLevel_Loading*		pInstance = new CLevel_Loading(pDevice, pContext);

	if (FAILED(pInstance->Initialize(eNextLevelID)))
	{
		MSG_BOX(TEXT("Failed To Created : CMainApp"));
		Safe_Release(pInstance);
	}

	return pInstance;

}


void CLevel_Loading::Free()
{
	__super::Free();

	Safe_Release(m_pLoader);

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pTransformCom);
	Safe_Release(m_pVIBufferCom);

}


