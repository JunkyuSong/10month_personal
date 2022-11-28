#include "stdafx.h"
#include "..\Public\Camera_CutScene.h"
#include "GameInstance.h"
#include "Cell.h"
#include "HierarchyNode.h"

CCamera_CutScene::CCamera_CutScene(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CClient_Camere(pDevice, pContext)
	, m_pTarget(nullptr)
{
}

CCamera_CutScene::CCamera_CutScene(const CCamera_CutScene & rhs, CTransform::TRANSFORMDESC * pArg)
	: CClient_Camere(rhs, pArg)
	, m_pTarget(rhs.m_pTarget)
{

}

HRESULT CCamera_CutScene::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;
	return S_OK;
}

HRESULT CCamera_CutScene::Initialize(void * pArg)
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
	return S_OK;
}

void CCamera_CutScene::Tick(_float fTimeDelta)
{
	AUTOINSTANCE(CGameInstance, pGameInstance);

	
	_float4x4	_matWorld;
	XMStoreFloat4x4(&_matWorld, m_pCameraBone->Get_OffSetMatrix()* m_pCameraBone->Get_CombinedTransformation()*XMLoadFloat4x4(&m_pModel->Get_PivotMatrix())*m_pTarget->Get_WorldMatrix());
	m_pTransformCom->Set_WorldFloat4x4(_matWorld);
	if (m_pLookAtBone)
	{
		_matrix _LookWorld = m_pLookAtBone->Get_OffSetMatrix()* m_pLookAtBone->Get_CombinedTransformation()*XMLoadFloat4x4(&m_pModel->Get_PivotMatrix())*m_pTarget->Get_WorldMatrix();
		_LookWorld.r[3].m128_f32[1] += 0.7f;
		m_pTransformCom->LookAt(_LookWorld.r[3]);
		//_vector 
	}
	else
	{
		_vector _vTargetPos = m_pTarget->Get_State(CTransform::STATE_POSITION);
		_vTargetPos.m128_f32[1] += 1.f;
		XMStoreFloat4(&m_CameraDesc.vAt, _vTargetPos);
		m_pTransformCom->LookAt(_vTargetPos);
	}
	__super::Tick(fTimeDelta);
}

void CCamera_CutScene::LateTick(_float fTimeDelta)
{
	
}

HRESULT CCamera_CutScene::Render()
{
	return S_OK;
}

HRESULT CCamera_CutScene::LookAt(char * _szBone)
{
	Safe_Release(m_pLookAtBone);

	m_pLookAtBone = m_pModel->Get_HierarchyNode(_szBone);

	if (m_pLookAtBone == nullptr)
	{
		return E_FAIL;
	}
	Safe_AddRef(m_pLookAtBone);
	return S_OK;
}

CCamera_CutScene * CCamera_CutScene::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCamera_CutScene*		pInstance = new CCamera_CutScene(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CCamera_CutScene"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CCamera_CutScene::Clone(void* pArg)
{
	CCamera::CAMERADESC*		pCameraDesc = (CCamera::CAMERADESC*)pArg;

	CCamera_CutScene*		pInstance = new CCamera_CutScene(*this, &pCameraDesc->TransformDesc);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CTerrain"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCamera_CutScene::Free()
{
	__super::Free();

	Safe_Release(m_pTarget);
	Safe_Release(m_pModel);
	Safe_Release(m_pCameraBone);
	Safe_Release(m_pLookAtBone);
}
