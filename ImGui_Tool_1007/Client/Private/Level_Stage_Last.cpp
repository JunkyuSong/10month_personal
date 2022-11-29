#include "stdafx.h"
#include "..\Public\Level_Stage_Last.h"

#include "GameInstance.h"
#include "Camera_Free.h"
#include "Camera_Player.h"
#include "CameraMgr.h"
#include "UI_Mgr.h"
#include "Monster.h"


CLevel_Stage_Last::CLevel_Stage_Last(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel_Client(pDevice, pContext)
{
}

HRESULT CLevel_Stage_Last::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;
	if (FAILED(Ready_Lights()))
		return E_FAIL;


	if (FAILED(Ready_Layer_BackGround(TEXT("Layer_BackGround"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_Player(TEXT("Layer_Player"))))
		return E_FAIL;

	/*if (FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
		return E_FAIL;*/

	if (FAILED(Ready_Layer_Monster(TEXT("Layer_Monster"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_Effect(TEXT("Layer_Effect"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_UI(TEXT("Layer_UI"))))
		return E_FAIL;

	CCameraMgr::Get_Instance()->Initialize();
	CCameraMgr::Get_Instance()->Change_Camera(CCameraMgr::CAMERA_PLAYER);
	AUTOINSTANCE(CGameInstance, _pInstance);
	_pInstance->Set_TimeSpeed(TEXT("Timer_Main"), DEFAULTTIME);
	
	return S_OK;
}

void CLevel_Stage_Last::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);
}

HRESULT CLevel_Stage_Last::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;


	SetWindowText(g_hWnd, TEXT("Last Boss"));

	return S_OK;
}

HRESULT CLevel_Stage_Last::Ready_Lights()
{
	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);

	DIRLIGHTDESC			LightDesc;
	ZeroMemory(&LightDesc, sizeof(DIRLIGHTDESC));

	XMStoreFloat4(&LightDesc.vDirection,
		XMVectorSet(45.491f, 0.165f, 45.676f, 1.f) - XMVectorSet(73.739f, 50.28f, 44.589f, 1.f));

	LightDesc.vPosition = _float4(73.739f, 50.28f, 44.589f, 1.f);
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vAmbient = _float4(0.3f, 0.3f, 0.3f, 1.1f);
	LightDesc.vSpecular = _float4(0.2f, 0.2f, 0.2f, 0.2f);
	LightDesc.LightViewMatrix = new _float4x4;
	LightDesc.LightProjMatrix = new _float4x4;
	XMStoreFloat4x4(LightDesc.LightProjMatrix, XMMatrixTranspose(XMMatrixPerspectiveFovLH(XMConvertToRadians(60.f), 1280.f / 720.f, 0.2f, 300.f)));

	if (FAILED(pGameInstance->Add_Light(m_pDevice, m_pContext, LEVEL_STAGE_LAST, LightDesc)))
		return E_FAIL;
	//pGameInstance->Light_On(LEVEL_STAGE_LAST, CLight_Manager::DIRLIGHT, 0);

	RELEASE_INSTANCE(CGameInstance);

	return S_OK;
}

HRESULT CLevel_Stage_Last::Ready_Layer_Camera(const _tchar * pLayerTag)
{
	AUTOINSTANCE(CGameInstance, pGameInstance);

	CCamera::CAMERADESC			CameraDesc;

	CameraDesc.vEye = _float4(0.f, 10.f, -10.f, 1.f);
	CameraDesc.vAt = _float4(0.f, 0.f, 0.f, 1.f);
	CameraDesc.fFovy = XMConvertToRadians(60.0f);
	CameraDesc.fAspect = (_float)g_iWinSizeX / g_iWinSizeY;
	CameraDesc.fNear = 0.2f;
	CameraDesc.fFar = 300.0f;

	CameraDesc.TransformDesc.fSpeedPerSec = 5.f;
	CameraDesc.TransformDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Camera_Free"), LEVEL_GAMEPLAY, pLayerTag, &CameraDesc)))
		return E_FAIL;



	return S_OK;
}

HRESULT CLevel_Stage_Last::Ready_Layer_Player(const _tchar * pLayerTag)
{
	AUTOINSTANCE(CGameInstance, pGameInstance);

	if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Player"), LEVEL_STAGE_LAST, pLayerTag)))
		return E_FAIL;



	return S_OK;
}

HRESULT CLevel_Stage_Last::Ready_Layer_BackGround(const _tchar * pLayerTag)
{
	AUTOINSTANCE(CGameInstance, pGameInstance);

	if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Sky"), LEVEL_STAGE_LAST, pLayerTag)))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Terrain"), LEVEL_STAGE_LAST, pLayerTag)))
		return E_FAIL;

	if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Stage_Last"), LEVEL_STAGE_LAST, TEXT("Layer_Stage"))))
		return E_FAIL;

	//if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Stage_Test"), LEVEL_STAGE_LAST, TEXT("Layer_Test"))))
	//	return E_FAIL;
/*

	if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Sky"), LEVEL_GAMEPLAY, pLayerTag)))
		return E_FAIL;
*/


	return S_OK;
}

HRESULT CLevel_Stage_Last::Ready_Layer_Monster(const _tchar * pLayerTag)
{
	AUTOINSTANCE(CGameInstance, pGameInstance);

	//for (_uint i = 0; i < 3; ++i)
	//{
	//	if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Monster"), LEVEL_GAMEPLAY, pLayerTag)))
	//		return E_FAIL;

	//}

	if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Monster_Puppet"), LEVEL_STAGE_LAST, pLayerTag)))
		return E_FAIL;

	CMonster::MONSTERINFO _tInfo;

	ZeroMemory(&_tInfo, sizeof(CMonster::MONSTERINFO));
	/*_tInfo._vPos = XMVectorSet(61.113f, 30.302f, 76.070f, 1.f);
	_tInfo._iIndex = 396;
	if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Monster_Extra01_Last"), LEVEL_STAGE_LAST, pLayerTag, &_tInfo)))
		return E_FAIL;*/

	_tInfo._vPos = XMVectorSet(53.674f, 25.997f, 77.647f, 1.f);
	_tInfo._iIndex = 389;
	if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Monster_Balloon"), LEVEL_STAGE_LAST, TEXT("Layer_Balloon"), &_tInfo)))
		return E_FAIL;

	_tInfo._vPos = XMVectorSet(88.44f, 32.59f - 4.f, 52.921f, 1.f);
	_tInfo._iIndex = 464;
	if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Monster_Balloon"), LEVEL_STAGE_LAST, TEXT("Layer_Balloon"), &_tInfo)))
		return E_FAIL;

	_tInfo._vPos = XMVectorSet(51.764f, 35.997f - 4.f, 13.47f, 1.f);
	_tInfo._iIndex = 577;
	if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Monster_Balloon"), LEVEL_STAGE_LAST, TEXT("Layer_Balloon"), &_tInfo)))
		return E_FAIL;

	_tInfo._vPos = XMVectorSet(1.025f, 38.967f - 4.f, 43.205f, 1.f);
	_tInfo._iIndex = 678;
	if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Monster_Balloon"), LEVEL_STAGE_LAST, TEXT("Layer_Balloon"), &_tInfo)))
		return E_FAIL;

	
	
	
	
	
	return S_OK;
}

HRESULT CLevel_Stage_Last::Ready_Layer_UI(const _tchar * pLayerTag)
{
	AUTOINSTANCE(CGameInstance, pGameInstance);

	/*for (_uint i = 0; i < 1; ++i)
	{
		if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_UI"), LEVEL_GAMEPLAY, pLayerTag)))
			return E_FAIL;

	}

*/
	CGameObject* _pGameObj = nullptr;
	if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_UI_PlayerHpBar"), LEVEL_STAGE_LAST, pLayerTag, nullptr, &_pGameObj)))
		return E_FAIL;
	CUI_Mgr::Get_Instance()->Add_UI(TEXT("PLAYER_HP_BAR"), _pGameObj);

	if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_UI_SkillSlot"), LEVEL_STAGE_LOBBY, pLayerTag, nullptr, &_pGameObj)))
		return E_FAIL;
	CUI_Mgr::Get_Instance()->Add_UI(TEXT("PLAYER_SLOT"), _pGameObj);



	return S_OK;
}

HRESULT CLevel_Stage_Last::Ready_Layer_Effect(const _tchar * pLayerTag)
{
	AUTOINSTANCE(CGameInstance, pGameInstance);

	//for (_uint i = 0; i < 10; ++i)
	//{
	//	if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Effect"), LEVEL_GAMEPLAY, pLayerTag)))
	//		return E_FAIL;

	//}



	return S_OK;
}

CLevel_Stage_Last * CLevel_Stage_Last::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_Stage_Last*		pInstance = new CLevel_Stage_Last(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Created : CLevel_Stage_Last"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevel_Stage_Last::Free()
{
	__super::Free();

}


