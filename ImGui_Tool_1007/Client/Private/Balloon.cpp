#include "stdafx.h"
#include "Balloon.h"
#include "Obj_NonAnim.h"
#include "Renderer.h"
#include "Shader.h"
#include "NonAnimModel.h"
#include "Transform.h"
#include "GameInstance.h"
#include "Status.h"
#include "CollisionMgr.h"
#include "ImGuiMgr.h"

CBalloon::CBalloon(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CMonster(pDevice, pContext)
{
}

CBalloon::CBalloon(const CBalloon & rhs)
	: CMonster(rhs)
{
}

HRESULT CBalloon::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBalloon::Initialize(void * pArg)
{
	if (FAILED(Ready_Components()))
		return E_FAIL;

	AUTOINSTANCE(CGameInstance, _pInstance);

	m_pTransformCom->Set_Scale(XMVectorSet(0.015f, 0.015f, 0.015f, 0.f));
	//좌표에 따라서 인덱스 받고
	// 그 인덱스에 따라서 파괴되었을때 저어기 몇몇개에 몬스터 생성
	if (pArg)
	{
		MONSTERINFO _tInfo = *static_cast<MONSTERINFO*>(pArg);

		m_pTransformCom->Set_State(CTransform::STATE_POSITION, _tInfo._vPos);
		
		memcpy(&m_vOriginPos, pArg, sizeof(_float4));

		if (XMVector3Equal(_tInfo._vPos, XMVectorSet(53.674f, 25.997f, 77.647f, 1.f)))
		{
			m_iPointIndex = 0;
		}
		else if (XMVector3Equal(_tInfo._vPos, XMVectorSet(88.44f, 32.59f - 4.f, 52.921f, 1.f)))
		{
			m_iPointIndex = 1;
		}
		else if (XMVector3Equal(_tInfo._vPos, XMVectorSet(51.764f, 35.997f - 4.f, 13.47f, 1.f)))
		{
			m_iPointIndex = 2;
		}
		else if (XMVector3Equal(_tInfo._vPos, XMVectorSet(1.025f, 38.967f - 4.f, 43.205f, 1.f)))
		{
			m_iPointIndex = 3;
		}
	}

	m_bCollision[COLLISION_PUSH] = true;
	m_bCollision[COLLISION_BODY] = true;

	_float fRot = _pInstance->Rand_Float(0.f,359.f);
	m_pTransformCom->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(fRot));

	return S_OK;
}

void CBalloon::Tick( _float fTimeDelta)
{
	if (!m_bDead && m_bHit)
	{
		m_fShakeCurTime += fTimeDelta;

		//흔들려
		if (m_fShakeCurTime > m_fShakeTime)
		{
			m_bHit = false;
			m_fShakeCurTime = 0.f;
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMLoadFloat4(&m_vOriginPos));
			m_bCollision[COLLISION_BODY] = true;
		}
		else
		{
			if ((_uint(m_fShakeCurTime*20.f) % 2) == 0)
			{
				_vector _vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
				_vPos -= XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_RIGHT) + m_pTransformCom->Get_State(CTransform::STATE_LOOK)) * fTimeDelta;
				m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vPos);
			}
			else
			{
				_vector _vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
				_vPos += XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_RIGHT) + m_pTransformCom->Get_State(CTransform::STATE_LOOK)) * fTimeDelta * 1.5f;
				m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vPos);
			}		
		}
	}

	Update_Collider();
}

void CBalloon::LateTick( _float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;
	AUTOINSTANCE(CGameInstance, _pInstance);

	CGameObject* _pTarget = nullptr;

	if (_pTarget = m_pColliderCom[COLLISION_PUSH]->Get_Target())
	{
		_vector _vDir = XMLoadFloat3(&(static_cast<CCapsule*>(m_pColliderCom[COLLISION_PUSH])->Get_Dir()));
		
		_float	_vDis = (static_cast<CCapsule*>(m_pColliderCom[COLLISION_PUSH])->Get_Dis());
		CTransform* _pTargetTrans = static_cast<CTransform*>(_pTarget->Get_ComponentPtr(TEXT("Com_Transform")));
		//_vector _vDir = XMVector3Normalize(_pTargetTrans->Get_State(CTransform::STATE_LOOK));
		_vector _vPos = _pTargetTrans->Get_State(CTransform::STATE_POSITION) - XMVector3Normalize(_vDir) *(_vDis);
		_bool		isMove = true;

		

		isMove = static_cast<CNavigation*>(_pTarget->Get_ComponentPtr(TEXT("Com_Navigation")))->isMove(_vPos, nullptr);

		if (true == isMove)
			_pTargetTrans->Set_State(CTransform::STATE_POSITION, _vPos);
		else
		{
			_vDir = XMVector3Normalize(_pTargetTrans->Get_State(CTransform::STATE_LOOK));
			_vPos = _pTargetTrans->Get_State(CTransform::STATE_POSITION) - XMVector3Normalize(_vDir) * fabs(_vDis);
			_pTargetTrans->Set_State(CTransform::STATE_POSITION, _vPos);
		}
	}
	if (_pTarget = m_pColliderCom[COLLISION_BODY]->Get_Target())
	{
		//흔들리고
		if (false == m_pStatusCom->Damage(static_cast<CStatus*>(_pTarget->Get_ComponentPtr(TEXT("Com_Status")))->Get_Attack()))
		{
			Safe_Release(m_pNonAnimModelCom);
			Safe_Release(m_Components[TEXT("Com_Model")]);
			m_pNonAnimModelCom = static_cast<CNonAnimModel*>(_pInstance->Clone_Component(LEVEL_STAGE_LAST, TEXT("Prototype_Component_Model_balloonDamaged01")));
			m_Components[TEXT("Com_Model")] = m_pNonAnimModelCom;
			Safe_AddRef(m_Components[TEXT("Com_Model")]);
			m_bDead = true;
			m_bCollision[COLLISION_PUSH] = false;
			m_bCollision[COLLISION_BODY] = false;
			_vector _vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
			_vPos .m128_f32[1] += 3.f;
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vPos);
			m_iPass = 0;
			Die();
		}
		m_bHit = true;
		m_bCollision[COLLISION_BODY] = false;
	}
	
	_bool		isDraw = _pInstance->isIn_Frustum_WorldSpace(m_pTransformCom->Get_State(CTransform::STATE_POSITION), 5.f);
	if (isDraw)
	{
		m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, this);
	}
	
}

HRESULT CBalloon::Render()
{
	if (nullptr == m_pNonAnimModelCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;



	SetUp_ShaderResources();

	_uint		iNumMeshes = m_pNonAnimModelCom->Get_NumMesh();//메쉬 갯수를 알고 메쉬 갯수만큼 렌더를 할 것임. 여기서!

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(m_pNonAnimModelCom->SetUp_OnShader(m_pShaderCom, m_pNonAnimModelCom->Get_MaterialIndex(i), aiTextureType_DIFFUSE, "g_DiffuseTexture")))
			return E_FAIL;
		if (FAILED(m_pNonAnimModelCom->SetUp_OnShader(m_pShaderCom, m_pNonAnimModelCom->Get_MaterialIndex(i), aiTextureType_NORMALS, "g_NormalTexture")))
			return E_FAIL;

		if (FAILED(m_pNonAnimModelCom->SetUp_OnShader(m_pShaderCom, m_pNonAnimModelCom->Get_MaterialIndex(i), aiTextureType_NORMALS, "g_NormalTexture")))
			return E_FAIL;
		
		if (FAILED(m_pShaderCom->Begin(m_iPass)))
			return E_FAIL;

		if (FAILED(m_pNonAnimModelCom->Render(i)))
			return E_FAIL;
	}

	//if (nullptr != m_pColliderCom[COLLISION_BODY])
		m_pColliderCom[COLLISION_BODY]->Render();


	return S_OK;
}


HRESULT CBalloon::Ready_Components()
{
	/* For.Com_Transform */
	CTransform::TRANSFORMDESC	_Desc;
	_Desc.fRotationPerSec = XMConvertToRadians(90.f);
	//_Desc.fSpeedPerSec = 1.5f;
	_Desc.fSpeedPerSec =0.5f;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform"), (CComponent**)&m_pTransformCom, &_Desc)))
		return E_FAIL;

	/* For.Com_Renderer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_Model"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	AUTOINSTANCE(CGameInstance, _pInstance);
	if (_pInstance->Rand_Int(0, 1) == 1)
	{
		if (FAILED(__super::Add_Component(LEVEL_STAGE_LAST, TEXT("Prototype_Component_Model_Balloon01"), TEXT("Com_Model"), (CComponent**)&m_pNonAnimModelCom)))
			return E_FAIL;
	}
	else
	{
		if (FAILED(__super::Add_Component(LEVEL_STAGE_LAST, TEXT("Prototype_Component_Model_Balloon02"), TEXT("Com_Model"), (CComponent**)&m_pNonAnimModelCom)))
			return E_FAIL;
	}

	/* For.Com_Status */
	CStatus::STATUS _tStatus;
	_tStatus.fMaxHp = 100.f;
	_tStatus.fAttack = 0.f;
	_tStatus.fHp = _tStatus.fMaxHp;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Status"), TEXT("Com_Status"), (CComponent**)&m_pStatusCom, &_tStatus)))
		return E_FAIL;


	CCollider::COLLIDERDESC		ColliderDesc;
	ZeroMemory(&ColliderDesc, sizeof(CCollider::COLLIDERDESC));
	ColliderDesc.vSize = _float3(450.f, 250.f, 450.f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vSize.y * 0.5f + 300.f, 0.f);
	ColliderDesc.vRotation = _float3(0.f, 0.f, 0.f);
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Collider_OBB"), TEXT("Com_OBB"), (CComponent**)&m_pColliderCom[COLLISION_BODY], &ColliderDesc)))
		return E_FAIL;

	ZeroMemory(&ColliderDesc, sizeof(CCollider::COLLIDERDESC));
	ColliderDesc.vSize = _float3(6.5f, 6.5f, 6.5f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vSize.y * 0.5f + 1.8f, 0.f);
	ColliderDesc.vRotation = _float3(0.f, 0.f, 0.f);
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Collider_Capsule"), TEXT("Com_Capsule"), (CComponent**)&m_pColliderCom[COLLISION_PUSH], &ColliderDesc)))
		return E_FAIL;


	return S_OK;
}

HRESULT CBalloon::SetUp_ShaderResources()
{
	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);
	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_CamPosition", &pGameInstance->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Set_RawValue("g_fY", &(m_pTransformCom->Get_WorldMatrix().r[3].m128_f32[1]), sizeof(_float))))
		return E_FAIL;

	RELEASE_INSTANCE(CGameInstance);

	return S_OK;
}

void CBalloon::Update_Collider()
{
	if (m_bCollision[COLLISION_PUSH])
	{
		m_pColliderCom[COLLISION_PUSH]->Update(m_pTransformCom->Get_WorldMatrix());
		CCollisionMgr::Get_Instance()->Add_CollisoinList(CCollisionMgr::TYPE_MONSTER_PUSH, m_pColliderCom[COLLISION_PUSH], this);
	}
	if (m_bCollision[COLLISION_BODY])
	{
 		m_pColliderCom[COLLISION_BODY]->Update(m_pTransformCom->Get_WorldMatrix());
		CCollisionMgr::Get_Instance()->Add_CollisoinList(CCollisionMgr::TYPE_MONSTER_BODY, m_pColliderCom[COLLISION_BODY], this);
	}
}

void CBalloon::Die()
{
	CMonster::MONSTERINFO _tInfo;
	AUTOINSTANCE(CGameInstance, pGameInstance);
	ZeroMemory(&_tInfo, sizeof(CMonster::MONSTERINFO));
	_tchar* pLayerTag = TEXT("Layer_Monster");
	
	switch (m_iPointIndex)
	{
	case 0:
		_tInfo._vPos = XMVectorSet(69.364f, 30.837f, 74.11f, 1.f);
		_tInfo._iIndex = 413;
		if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Monster_Extra01_Last"), LEVEL_STAGE_LAST, pLayerTag, &_tInfo)))
			return;
		_tInfo._vPos = XMVectorSet(79.457f, 31.437f, 70.647f, 1.f);
		_tInfo._iIndex = 430;
		if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Monster_Extra01_Last"), LEVEL_STAGE_LAST, pLayerTag, &_tInfo)))
			return;
		_tInfo._vPos = XMVectorSet(84.248f, 31.917f, 63.999f, 1.f);
		_tInfo._iIndex = 446;
		if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Monster_Extra01_Last"), LEVEL_STAGE_LAST, pLayerTag, &_tInfo)))
			return;
	case 1:
		_tInfo._vPos = XMVectorSet(90.44f, 33.357f, 39.826f, 1.f);
		_tInfo._iIndex = 489;
		if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Monster_Extra01_Last"), LEVEL_STAGE_LAST, pLayerTag, &_tInfo)))
			return;
		_tInfo._vPos = XMVectorSet(84.067f, 34.077f, 28.841f, 1.f);
		_tInfo._iIndex = 513;
		if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Monster_Extra02_Last"), LEVEL_STAGE_LAST, pLayerTag, &_tInfo)))
			return;
		_tInfo._vPos = XMVectorSet(72.572f, 35.157f, 14.989f, 1.f);
		_tInfo._iIndex = 549;
		if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Monster_Extra02_Last"), LEVEL_STAGE_LAST, pLayerTag, &_tInfo)))
			return;
		break;
	case 2:
		_tInfo._vPos = XMVectorSet(37.136f, 35.997f, 11.874f, 1.f);
		_tInfo._iIndex = 578;
		if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Monster_Extra01_Last"), LEVEL_STAGE_LAST, pLayerTag, &_tInfo)))
			return;
		_tInfo._vPos = XMVectorSet(23.298f, 36.564f, 12.622f, 1.f);
		_tInfo._iIndex = 594;
		if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Monster_Extra01_Last"), LEVEL_STAGE_LAST, pLayerTag, &_tInfo)))
			return;
		_tInfo._vPos = XMVectorSet(10.544f, 37.557f, 22.196f, 1.f);
		_tInfo._iIndex = 623;
		if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Monster_Extra02_Last"), LEVEL_STAGE_LAST, pLayerTag, &_tInfo)))
			return;
		_tInfo._vPos = XMVectorSet(1.573f, 38.397f, 33.37f, 1.f);
		_tInfo._iIndex = 659;
		if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Monster_Extra01_Last"), LEVEL_STAGE_LAST, pLayerTag, &_tInfo)))
			return;
		break;
	case 3:
		_tInfo._vPos = XMVectorSet(1.753f, 39.717f, 56.157f, 1.f);
		_tInfo._iIndex = 703;
		if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Monster_Extra01_Last"), LEVEL_STAGE_LAST, pLayerTag, &_tInfo)))
			return;
		_tInfo._vPos = XMVectorSet(10.876f, 40.557f, 67.592f, 1.f);
		_tInfo._iIndex = 728;
		if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Monster_Extra02_Last"), LEVEL_STAGE_LAST, pLayerTag, &_tInfo)))
			return;
		_tInfo._vPos = XMVectorSet(14.003f, 40.910f, 71.534f, 1.f);
		_tInfo._iIndex = 740;
		if (FAILED(pGameInstance->Add_GameObjectToLayer(TEXT("Prototype_GameObject_Monster_Extra01_Last"), LEVEL_STAGE_LAST, pLayerTag, &_tInfo)))
			return;
		break;
	}
}

CBalloon * CBalloon::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CBalloon*		pInstance = new CBalloon(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CBalloon"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CBalloon::Clone(void * pArg)
{
	CBalloon*		pInstance = new CBalloon(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CBalloon"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBalloon::Free()
{
	__super::Free();

	Safe_Release(m_pNonAnimModelCom);

	Safe_Release(m_pColliderCom[COLLISION_BODY]);
	Safe_Release(m_pColliderCom[COLLISION_PUSH]);
}