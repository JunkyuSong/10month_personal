#include "stdafx.h"
#include "..\Public\Camera_CutScene_Enter.h"
#include "GameInstance.h"
#include "Cell.h"
#include "HierarchyNode.h"

CCamera_CutScene_Enter::CCamera_CutScene_Enter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CClient_Camere(pDevice, pContext)
	, m_pTarget(nullptr)
{
}

CCamera_CutScene_Enter::CCamera_CutScene_Enter(const CCamera_CutScene_Enter & rhs, CTransform::TRANSFORMDESC * pArg)
	: CClient_Camere(rhs, pArg)
	, m_pTarget(rhs.m_pTarget)
{

}

HRESULT CCamera_CutScene_Enter::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;
	return S_OK;
}

HRESULT CCamera_CutScene_Enter::Initialize(void * pArg)
{
	/*if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;*/
	memcpy(&m_CameraDesc, pArg, sizeof(CAMERADESC));

	AUTOINSTANCE(CGameInstance, pGameInstance);
	//m_pTarget = static_cast<CPlayer*>(pGameInstance->Get_Player());
	m_pTarget = static_cast<CTransform*>(pGameInstance->Get_Player()->Get_ComponentPtr(TEXT("Com_Transform")));
	Safe_AddRef(m_pTarget);
	m_pModel = static_cast<CAnimModel*>(pGameInstance->Get_Player()->Get_ComponentPtr(TEXT("Com_Model")));
	Safe_AddRef(m_pModel);
	m_pCameraBone = m_pModel->Get_HierarchyNode("camera");
	Safe_AddRef(m_pCameraBone);
	

		switch (g_eCurLevel)
		{
		case Client::LEVEL_GAMEPLAY:
			
			break;
		case Client::LEVEL_STAGE_02_1:
			
			break;
		case Client::LEVEL_STAGE_02:
			m_CamSpeed = 20.f;
			break;

		}
	return S_OK;
}

void CCamera_CutScene_Enter::Tick(_float fTimeDelta)
{
	

	switch (g_eCurLevel)
	{
	case Client::LEVEL_GAMEPLAY:
		Enter_Lv_Stage_01(fTimeDelta);
		break;
	case Client::LEVEL_STAGE_02_1:
		End_Lv_Stage_02_1(fTimeDelta);
		break;
	case Client::LEVEL_STAGE_02:
		Enter_Lv_Stage_02(fTimeDelta);
		break;
	
	}

	__super::Tick(fTimeDelta);

}

void CCamera_CutScene_Enter::LateTick(_float fTimeDelta)
{
	
}

HRESULT CCamera_CutScene_Enter::Render()
{
	return S_OK;
}

void CCamera_CutScene_Enter::Set_Target(CGameObject * _pTarget)
{
	Safe_Release(m_pTarget);
	Safe_Release(m_pModel);
	Safe_Release(m_pCameraBone);


	m_pTarget = static_cast<CTransform*>(_pTarget->Get_ComponentPtr(TEXT("Com_Transform")));
	Safe_AddRef(m_pTarget);
	m_pModel = static_cast<CAnimModel*>(_pTarget->Get_ComponentPtr(TEXT("Com_Model")));
	Safe_AddRef(m_pModel);
	m_pCameraBone = m_pModel->Get_HierarchyNode("camera");
	Safe_AddRef(m_pCameraBone);
}

void CCamera_CutScene_Enter::End_Lv_Stage_02_1(_float fTimeDelta)
{
	AUTOINSTANCE(CGameInstance, pGameInstance);
	switch (m_iCutScene)
	{
	case 0:
		// 플레이어 위치와 방향을 받아서 거기서 6만큼 떨어진 위치에 라이트로 40도 정도 올리고 공전 1회
	{
		_float4x4 _PlayerWorld;
		XMStoreFloat4x4(&_PlayerWorld, static_cast<CTransform*>(pGameInstance->Get_Player()->Get_ComponentPtr(TEXT("Com_Transform")))->Get_WorldMatrix());
		_PlayerWorld._42 += 0.5f;

		m_pTransformCom->Set_WorldFloat4x4(_PlayerWorld);

		m_pTransformCom->Turn_Angle(m_pTransformCom->Get_State(CTransform::STATE_RIGHT), XMConvertToRadians(40.f));
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_pTransformCom->Get_State(CTransform::STATE_POSITION) - (XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK)) * 4.f));
		m_pTransformCom->LookAt(static_cast<CTransform*>(pGameInstance->Get_Player()->Get_ComponentPtr(TEXT("Com_Transform")))->Get_State(CTransform::STATE_POSITION));
		// 그게 카메라 최초 위치
	}
	break;
	case 1:
		// 플레이어를 계속 바라보고
		m_pTransformCom->LookAt(static_cast<CTransform*>(pGameInstance->Get_Player()->Get_ComponentPtr(TEXT("Com_Transform")))->Get_State(CTransform::STATE_POSITION));
		break;
	case 2:
		// 점프 시작할때 위치를 y축 기준으로만 싸악 돌리면서 플레이어 공전
	{
		m_fAngleY += fTimeDelta / pGameInstance->Get_TimeSpeed(TEXT("Timer_Main"));
		if (XMConvertToRadians(90.f) >= m_fAngleY)
		{
			m_pTransformCom->LookAt(XMVectorSet(60.512f, 1.916f, 28.185f, 1.f));
			_vector _vDis = XMVectorSetW(m_pTransformCom->Get_State(CTransform::STATE_POSITION) - XMVectorSet(60.512f, 1.916f, 28.185f, 1.f), 1.f);

			m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(60.512f, 1.916f, 28.185f, 1.f));

			m_pTransformCom->Turn_Angle(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta / pGameInstance->Get_TimeSpeed(TEXT("Timer_Main")));

			m_pTransformCom->Set_State(CTransform::STATE_POSITION,
				XMVectorSet(60.512f, 1.916f, 28.185f, 1.f) - XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK)) * XMVector3Length(_vDis).m128_f32[0]
			);
			m_pTransformCom->LookAt(static_cast<CTransform*>(pGameInstance->Get_Player()->Get_ComponentPtr(TEXT("Com_Transform")))->Get_State(CTransform::STATE_POSITION)); // 플레이어 바라본다
		}


	}

	break;
	case 3:
		// 플레이어가 가장 정점에 있을때(40) 속도가 엄청 느려지고 얼마 있다가

		break;
	case 4:
		// 플레이어 떨어지기 시작할때 빠르게 카메라를 플레이어 공전으로 라이트 90도에서 바라보게
	{
		m_fAngleX += fTimeDelta * 2.f;
		if (XMConvertToRadians(80.f) >= m_fAngleX)
		{
			//m_pTransformCom->LookAt(XMVectorSet(60.512f, 1.916f, 28.185f, 1.f));
			_vector _vDis = XMVectorSetW(m_pTransformCom->Get_State(CTransform::STATE_POSITION) - XMVectorSet(60.512f, 1.916f, 28.185f, 1.f), 1.f);

			m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(60.512f, 1.916f, 28.185f, 1.f));

			m_pTransformCom->Turn_Angle(m_pTransformCom->Get_State(CTransform::STATE_RIGHT), fTimeDelta * 2.f);

			m_pTransformCom->Set_State(CTransform::STATE_POSITION,
				XMVectorSet(60.512f, 1.916f, 28.185f, 1.f) - XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK)) * XMVector3Length(_vDis).m128_f32[0]
			);
		}
		else
		{
			CTransform* _pTrans = static_cast<CTransform*>(pGameInstance->Get_Player()->Get_ComponentPtr(TEXT("Com_Transform")));
			_vector _vPos = _pTrans->Get_State(CTransform::STATE_POSITION);

			_vector _vDis = _vPos - m_pTransformCom->Get_State(CTransform::STATE_POSITION);


			m_pTransformCom->Go_Dir(XMVector3Normalize(_vDis), fTimeDelta*0.6f);

		}

	}

	break;

	case 5:
	{
		CTransform* _pTrans = static_cast<CTransform*>(pGameInstance->Get_Player()->Get_ComponentPtr(TEXT("Com_Transform")));
		_vector _vPos = _pTrans->Get_State(CTransform::STATE_POSITION);
		//m_pTransformCom->LookAt(_vPos);
		_vector _vDis = _vPos - m_pTransformCom->Get_State(CTransform::STATE_POSITION);


		m_pTransformCom->Go_Dir(XMVector3Normalize(_vDis), fTimeDelta*0.6f);


	}
	break;
	}

}

void CCamera_CutScene_Enter::Enter_Lv_Stage_01(_float fTimeDelta)
{
	_float4x4	_matWorld;
	XMStoreFloat4x4(&_matWorld, m_pCameraBone->Get_OffSetMatrix()* m_pCameraBone->Get_CombinedTransformation()*XMLoadFloat4x4(&m_pModel->Get_PivotMatrix())*m_pTarget->Get_WorldMatrix());
	m_pTransformCom->Set_WorldFloat4x4(_matWorld);
	m_pTransformCom->Set_Scale(XMVectorSet(1.f, 1.f, 1.f, 1.f));
	_vector _vTargetPos = XMLoadFloat4(&m_vLookAt);
	m_CameraDesc.fNear = 0.05f;
	XMStoreFloat4(&m_CameraDesc.vAt, _vTargetPos);
	m_pTransformCom->LookAt(_vTargetPos);
}

void CCamera_CutScene_Enter::Enter_Lv_Stage_02(_float fTimeDelta)
{
	switch (m_iCutScene)
	{
	case 0:
		//플레이어 그 자리만 쳐다봄
		{
			_vector vLookPoint = XMVectorSet(44.083f, 0.5f, 17.580f, 1.f);
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, vLookPoint);
			m_pTransformCom->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(40.f));
			m_pTransformCom->Set_State(CTransform::STATE_POSITION,
				m_pTransformCom->Get_State(CTransform::STATE_POSITION) + m_pTransformCom->Get_State(CTransform::STATE_LOOK)*4.f
			);
			m_pTransformCom->LookAt(vLookPoint);
		}
		
		break;
	case 1:
		//박쥐만 쳐다봄
	{
		_vector vLookPoint = XMVectorSet(30.672f, 5.502f, 50.622f, 1.f);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, vLookPoint);
		m_pTransformCom->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(-180.f));
		m_pTransformCom->Set_State(CTransform::STATE_POSITION,
			m_pTransformCom->Get_State(CTransform::STATE_POSITION) + m_pTransformCom->Get_State(CTransform::STATE_LOOK)*m_CamSpeed
		);
		m_pTransformCom->LookAt(vLookPoint);
		m_CamSpeed += fTimeDelta*1.f;
	}
		break;
	case 2:
		//박쥐만 쳐다봄 > 0
	{
		_vector vLookPoint = XMVectorSet(30.672f, 5.502f, 50.622f, 1.f);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, vLookPoint);
		m_pTransformCom->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(-180.f));
		m_pTransformCom->Set_State(CTransform::STATE_POSITION,
			m_pTransformCom->Get_State(CTransform::STATE_POSITION) + m_pTransformCom->Get_State(CTransform::STATE_LOOK)*m_CamSpeed
		);
		m_pTransformCom->LookAt(vLookPoint);
		m_CamSpeed += fTimeDelta*1.f;
	}
	break;
	}
}

CCamera_CutScene_Enter * CCamera_CutScene_Enter::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCamera_CutScene_Enter*		pInstance = new CCamera_CutScene_Enter(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CCamera_CutScene_Enter"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CCamera_CutScene_Enter::Clone(void* pArg)
{
	CCamera::CAMERADESC*		pCameraDesc = (CCamera::CAMERADESC*)pArg;

	CCamera_CutScene_Enter*		pInstance = new CCamera_CutScene_Enter(*this, &pCameraDesc->TransformDesc);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CTerrain"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCamera_CutScene_Enter::Free()
{
	__super::Free();
	
	Safe_Release(m_pTarget);
	Safe_Release(m_pModel);
	Safe_Release(m_pCameraBone);
}
