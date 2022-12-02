#include "stdafx.h"
#include "..\Public\Level_Client.h"

#include "GameInstance.h"
#include "GameObject.h"

#include "ReleaseMgr.h"
#include "Obj_Plus.h"
#include "InstancingObj.h"

CLevel_Client::CLevel_Client(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel(pDevice, pContext)
{	
}
void CLevel_Client::NowLevel(_uint _CurLevel)
{
	g_eCurLevel = LEVEL(_CurLevel);
	
	return;
}
HRESULT CLevel_Client::Loading(const LEVEL& _eLevel)
{
	AUTOINSTANCE(CGameInstance, _Instance);

	_tchar szFullPath[MAX_PATH] = TEXT("../Bin/Map/"); //여기에 넣을 예정
	_tchar szFileName[MAX_PATH] = TEXT(""); //파일 이름

	switch (g_eCurLevel)
	{
	case Client::LEVEL_STATIC:
		break;
	case Client::LEVEL_LOADING:
		return S_OK;
	case Client::LEVEL_LOGO:
		return S_OK;
	case Client::LEVEL_GAMEPLAY:
		lstrcpy(szFileName, TEXT("LEVEL_GAMEPLAY"));
		break;
	case Client::LEVEL_STAGE_02:
		lstrcpy(szFileName, TEXT("LEVEL_STAGE_02"));
		break;
	case Client::LEVEL_STAGE_LAST:
		lstrcpy(szFileName, TEXT("LEVEL_STAGE_LAST"));
		break;
	case Client::LEVEL_STAGE_LOBBY:
		lstrcpy(szFileName, TEXT("LEVEL_STAGE_LOBBY"));
		break;
	case Client::LEVEL_END:
		return S_OK;
	default:
		return S_OK;
	}

	_tchar*		_szFileName = nullptr;


	lstrcat(szFullPath, szFileName);
	//파일 이름 넣고
	lstrcat(szFullPath, TEXT(".dat"));
	//경로 넣고

	//맵에 경로 넣고 벡터 넣어서 만든다


	HANDLE		hFile = CreateFile(szFullPath,
		GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	DWORD		dwByte = 0;
	map<_tchar*, vector<_float4x4>> _Instancing;
	CObj_Plus::OBJ_DESC	_tInfo;
	ZeroMemory(&_tInfo, sizeof(CObj_Plus::OBJ_DESC));
	while (true)
	{
		ZeroMemory(&_tInfo, sizeof(CObj_Plus::OBJ_DESC));
		ReadFile(hFile, &_tInfo, sizeof(CObj_Plus::OBJ_DESC), &dwByte, nullptr);
		if (dwByte == 0)
			break;

		/*if (!lstrcmp(_tInfo.szModelTag, TEXT("Prototype_Component_Model_Light02")))
		{
		if (FAILED(_Instance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Map_StreetLight"), g_eCurLevel, TEXT("Layer_Map"), &_tInfo)))
		{
		MSG_BOX(TEXT("FAILED LOAD MAP"));
		CloseHandle(hFile);
		return;
		}
		}
		else if (FAILED(_Instance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Obj_NonAnim"), g_eCurLevel, TEXT("Layer_Map"), &_tInfo)))
		{
		MSG_BOX(TEXT("FAILED LOAD MAP"));
		CloseHandle(hFile);
		return;
		}*/

		_bool	Is = true;
		for (auto& iter = _Instancing.begin(); iter != _Instancing.end(); ++iter)
		{

			if (!lstrcmp(iter->first, _tInfo.szModelTag))
			{
				iter->second.push_back(_tInfo.matWorld);
				Is = false;
				break;
			}
		}
		if (Is || _Instancing.size() == 0)
		{
			vector<_float4x4> Temp;
			Temp.push_back(_tInfo.matWorld);
			_tchar* szTemp = new _tchar[260];
			//ZeroMemory(&szTemp,sizeof(_tchar)* 260);
			lstrcpy(szTemp, _tInfo.szModelTag);
			CReleaseMgr::Get_Instance()->Add_Tchar(szTemp);

			_Instancing.emplace(szTemp, Temp);
		}
	}

	for (map<_tchar*, vector<_float4x4>>::iterator iter = _Instancing.begin(); iter != _Instancing.end(); ++iter)
	{
		if (iter->second.size() < 2)
		{
			ZeroMemory(&_tInfo, sizeof(CObj_Plus::OBJ_DESC));
			_tInfo.matWorld = iter->second[0];
			for (int i = 0; (iter->first)[i] != NULL; ++i)
			{
				_tInfo.szModelTag[i] = (iter->first)[i];
			}
			if (FAILED(_Instance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Obj_NonAnim"), g_eCurLevel, TEXT("Layer_Map"), &_tInfo)))
			{
				MSG_BOX(TEXT("FAILED LOAD MAP"));
				CloseHandle(hFile);

				return E_FAIL;
			}
		}
		else if (!lstrcmp(iter->first, TEXT("Prototype_Component_Model_Light02")))
		{

			for (auto& streetlight : iter->second)
			{
				ZeroMemory(&_tInfo, sizeof(CObj_Plus::OBJ_DESC));
				for (int i = 0; (iter->first)[i] != NULL; ++i)
				{
					_tInfo.szModelTag[i] = (iter->first)[i];
				}
				_tInfo.matWorld = streetlight;
				if (FAILED(_Instance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Map_StreetLight"), g_eCurLevel, TEXT("Layer_Map"), &_tInfo)))
				{
					MSG_BOX(TEXT("FAILED LOAD MAP"));
					CloseHandle(hFile);

					return E_FAIL;
				}
			}
		}
		else
		{
			CGameObject* _pObj = nullptr;

			if (FAILED(_Instance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Obj_Instancing"), g_eCurLevel, TEXT("Layer_Instancing"), nullptr, &_pObj)))
			{
				MSG_BOX(TEXT("FAILED LOAD MAP"));
				CloseHandle(hFile);

				return E_FAIL;
			}

			static_cast<CInstancingObj*>(_pObj)->Set_Instancing(iter->first, &(iter->second));
		}
	}

	/*if (!lstrcmp(_tInfo.szModelTag, TEXT("Prototype_Component_Model_Light02")))
	{
	if (FAILED(_Instance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Map_StreetLight"), g_eCurLevel, TEXT("Layer_Map"), &_tInfo)))
	{
	MSG_BOX(TEXT("FAILED LOAD MAP"));
	CloseHandle(hFile);

	return;
	}
	}*/

	CloseHandle(hFile);

	return S_OK;
}

