#include "stdafx.h"
#include "..\Public\Monster.h"

#include "GameInstance.h"
#include "Status.h"

CMonster::CMonster(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject(pDevice, pContext)
{
	m_eTypeObj = CGameObject::TYPE_MONSTER;
}

CMonster::CMonster(const CMonster & rhs)
	: CGameObject(rhs)
{
}

void CMonster::LookPlayerSlow(_float _fRatio)
{
	AUTOINSTANCE(CGameInstance, _Intance);
	CTransform* _Trans =
		static_cast<CTransform*>(_Intance->Get_Player()->Get_ComponentPtr(TEXT("Com_Transform")));

	_vector _vLookStart = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
	_vector _vLookEnd = XMVector3Normalize((_Trans->Get_State(CTransform::STATE_POSITION)) - (m_pTransformCom->Get_State(CTransform::STATE_POSITION)));

	if (XMVector3Length(_vLookStart - _vLookEnd).m128_f32[0] < 0.03f)
	{
		m_pTransformCom->LookAt_ForLandObject(_Trans->Get_State(CTransform::STATE_POSITION));
	}
	else
	{
		m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE_LOOK), _vLookEnd, 0.75f);
	}

}

void CMonster::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRendererCom);
	Safe_Release(m_pTransformCom);
	Safe_Release(m_pStatusCom);
	Safe_Release(m_pNavigationCom);
}
