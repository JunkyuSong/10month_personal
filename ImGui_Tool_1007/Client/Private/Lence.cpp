#include "stdafx.h"
#include "..\Public\Lence.h"
#include "GameInstance.h"


#include "Trail_Obj.h"

#include "Effect_Mgr.h"
#include "DisappearWeapon.h"
#include "CameraMgr.h"
#include "Camera.h"

CLence::CLence(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CWeapon(pDevice, pContext)
{
}

CLence::CLence(const CLence & rhs)
	: CWeapon(rhs)
{
}

HRESULT CLence::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLence::Initialize(void * pArg)
{
	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pTransformCom->Set_Scale(XMVectorSet(0.01f, 0.01f, 0.01f, 1.f));
	m_pTransformCom->Rotation(XMVectorSet(1.f, 0.f, 0.f, 0.f), XMConvertToRadians(-90.0f));





	return S_OK;
}

void CLence::Tick(_float fTimeDelta)
{
	m_pTrailCom->Tick(fTimeDelta, m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix());

	if (m_bColliderOn)
		m_pColliderCom->Update(m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix());
}

void CLence::Tick(_float fTimeDelta, CGameObject * _pUser)
{
	m_pTrailCom->Tick(fTimeDelta, m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix());

	if (m_bColliderOn)
	{
		m_pColliderCom->Update(m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix());
		CCollisionMgr::Get_Instance()->Add_CollisoinList(CCollisionMgr::TYPE_PLAYER_WEAPON, m_pColliderCom, _pUser);
	}
	m_fCurDelayTime += fTimeDelta / CGameInstance::Get_Instance()->Get_TimeSpeed(TEXT("Timer_Main"));
	if (m_bHitTime && m_fCurDelayTime > m_fDelayTime)
	{
		m_fCurStopTime += fTimeDelta / CGameInstance::Get_Instance()->Get_TimeSpeed(TEXT("Timer_Main"));
		if (m_fCurStopTime > m_fStopTime)
		{
			CGameInstance::Get_Instance()->Set_TimeSpeed(TEXT("Timer_Main"), DEFAULTTIME);
			m_bHitTime = false;
			m_fCurStopTime = 0.f;
		}
	}
	Compute_CamZ(m_pTransformCom->Get_State(CTransform::STATE_POSITION));
}

void CLence::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;


}

HRESULT CLence::Render()
{
	if (nullptr == m_pModelCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;
	CGameObject* pTarget = m_pColliderCom->Get_Target();

	if (pTarget)
	{
		// 라이트는 패링되었을때, 그 외엔 피 이펙트(시간 멈추는 타이밍에)!
		//Light_On();
		Hit(pTarget);
		CCameraMgr::Get_Instance()->Get_Cam(CCameraMgr::CAMERA_PLAYER)->ZoomIn(50.f, 80.f, 0.3f);
		//CCameraMgr::Get_Instance()->Get_Cam(CCameraMgr::CAMERA_PLAYER)->Shake_On(0.1f, 0.5f);

		CGameInstance::Get_Instance()->Set_TimeSpeed(TEXT("Timer_Main"), 0.1f);
		m_fCurStopTime = 0.f;
		m_fCurDelayTime = 0.f;
		m_bHitTime = true;


	}
	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);

	if (FAILED(m_pShaderCom->Set_RawValue("g_fColor", &CLIENT_RGB(119.f, 245.f, 200.f), sizeof(_float4))))
		return E_FAIL;

	_float4x4		WorldMatrix;

	XMStoreFloat4x4(&WorldMatrix, XMMatrixTranspose(m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix()));

	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &WorldMatrix, sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4))))
		return E_FAIL;

	RELEASE_INSTANCE(CGameInstance);
	_uint		iNumMeshes = m_pModelCom->Get_NumMesh();

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(m_pModelCom->SetUp_OnShader(m_pShaderCom, m_pModelCom->Get_MaterialIndex(i), aiTextureType_DIFFUSE, "g_DiffuseTexture")))
			return E_FAIL;
		if (FAILED(m_pModelCom->SetUp_OnShader(m_pShaderCom, m_pModelCom->Get_MaterialIndex(i), aiTextureType_NORMALS, "g_NormalTexture")))
		return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(m_iPass)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	m_pTrailCom->Render();

#ifdef _DEBUG
	//if (nullptr != m_pColliderCom && m_bColliderOn)
	//	m_pColliderCom->Render();
#endif

	return S_OK;

}

void CLence::Clear(void * pArg)
{

}

void CLence::End()
{
	CDisappearWeapon::DISAPPEARWEAPON _tInfo;
	_tInfo.pModel = m_pModelCom;
	XMStoreFloat4x4(&_tInfo.World, m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix());
	_tInfo.HighAndLow = m_pTrailCom->Get_HighAndLow(m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix());
	_tInfo.fSpeed = 1.7f;
	CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_DISAPPEAR, &_tInfo);
}



HRESULT CLence::Ready_Components()
{
	/* For.Com_Transform */
	CTransform::TRANSFORMDESC	_Desc;
	_Desc.fRotationPerSec = XMConvertToRadians(90.f);
	//_Desc.fSpeedPerSec = 1.5f;
	_Desc.fSpeedPerSec = 2.5f;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform"), (CComponent**)&m_pTransformCom, &_Desc)))
		return E_FAIL;

	/* For.Com_ParentTransform */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_ParentTransform"), (CComponent**)&m_pParentTransformCom)))
		return E_FAIL;



	/* For.Com_Renderer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_Effect"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Weapon_Lence"), TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	/* For.Com_Trail */
	CTrail::TRAILINFO _tInfo;
	_tInfo._Color = _float4(1.f, 1.f, 1.f, 1.f);
	_tInfo._HighAndLow.vHigh = _float3(210.0f, 0.f, 0.f);
	_tInfo._HighAndLow.vLow = _float3(-5.f, 0.f, 0.f);
	//_tInfo._eOption = CTrail::TRAILOPTION_DITORTION;
	AUTOINSTANCE(CGameInstance, _pInstance);
	m_pTrailCom = static_cast<CTrail_Obj*>(_pInstance->Clone_GameObject(TEXT("Prototype_GameObject_Trail"), &_tInfo));
	if (m_pTrailCom == nullptr)
		return E_FAIL;


	/* For.Com_OBB */
	CCollider::COLLIDERDESC		ColliderDesc;
	ZeroMemory(&ColliderDesc, sizeof(CCollider::COLLIDERDESC));

	ColliderDesc.vSize = _float3(420.0f, 50.f, 50.f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vSize.y * 0.5f, 0.f);
	ColliderDesc.vRotation = _float3(0.f, XMConvertToRadians(0.f), 0.f);
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Collider_OBB"), TEXT("Com_OBB"), (CComponent**)&m_pColliderCom, &ColliderDesc)))
		return E_FAIL;


	return S_OK;
}

void CLence::Hit(CGameObject* _pTarget)
{
	_vector LightPos, vHigh, vLow;
	vHigh = XMVectorSet(100.0f, 0.f, 0.f, 1.f);
	vLow = XMVectorSet(-5.f, 0.f, 0.f, 1.f);

	//맞았을때 창까지의 방향벡터랑 창의 방향이랑 내적해서
	// 음수라면 창대에 맞은것, 양수라면 창날에 맞은것.
	
	vHigh = XMVector3TransformCoord(vHigh, m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix());
	vLow = XMVector3TransformCoord(vLow, m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix());

	_vector _vTargetPos = static_cast<CTransform*>(_pTarget->Get_ComponentPtr(TEXT("Com_Transform")))->Get_State(CTransform::STATE_POSITION);
	_vector _vMyPos = (m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix()).r[3];

	_vector _TargetToMeDir = XMVector3Normalize( _vMyPos - _vTargetPos);
	_vector	_LowToHighDir = XMVector3Normalize(vHigh - vLow);
	_float _fDotResult = XMVector3Dot(_TargetToMeDir, _LowToHighDir).m128_f32[0];
	
	if (_fDotResult > 0.f)
	{
		vHigh = XMVectorSet(5.0f, 0.f, 0.f, 1.f);
		vLow = XMVectorSet(-100.f, 0.f, 0.f, 1.f);

		vHigh = XMVector3TransformCoord(vHigh, m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix());
		vLow = XMVector3TransformCoord(vLow, m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix());
	}

	LightPos = XMVectorSetW((vHigh + vLow) * 0.5f, 1.f);

	CEffect_Particle::OPTION _tOption;
	//_tOption.Center = _float3(45.f, 2.f, 45.f);
	_vector	_vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	_float4x4	_vWorld = m_pTransformCom->Get_WorldFloat4x4();
	//_tOption.Center = _float3(_vPos.m128_f32[0], _vPos.m128_f32[1] + 1.f, _vPos.m128_f32[2]);

	XMStoreFloat3(&_tOption.Center, LightPos);

	_tOption.eType = CEffect_Particle::PARTICLETYPE::TYPE_STRIGHT;
	_tOption.fAccSpeed = 0.99f;
	_tOption.fSpeed = { 1.5f, 3.3f };
	_tOption.fGravity = 0.f;
	_tOption.fLifeTime = 0.f;
	_tOption.fRange = _float3(5.f, 5.f, 1.f);
	_tOption.iNumParticles = 50;
	_tOption.Size = _float2(0.2f, 0.2f);
	_tOption.Spread = CEffect_Particle::SPREAD::SPREAD_EDGE;
	_tOption.szMaskTag = TEXT("Prototype_Component_Texture_Mask_Blood");
	_tOption.szTextureTag = TEXT("Prototype_Component_Texture_Diffuse_Blood");
	//_tOption.vColor = CLIENT_RGB(119.f, 245.f, 200.f);
	//_tOption.vColor = CLIENT_RGB(255.f, 9.f, 4.f);
	_tOption.vColor = CLIENT_RGB(255.f, 0.f, 0.f);
	_tOption.bPlayerDir = true;
	_tOption.fSpead_Angle = _float3(0.f, 25.f, 25.f);
	_tOption.vStart_Dir = _float3(1.f, 0.f, 0.f);
	_tOption.eDiffuseType = CEffect_Particle::DIFFUSE_COLOR;
	_tOption.eDirType = CEffect_Particle::DIR_TYPE::DIR_ANGLE;
	_tOption.eStartType = CEffect_Particle::START_CENTER;
	_tOption.fMaxDistance = { 0.4f, 1.2f };
	XMStoreFloat4x4(&_tOption.matPlayerAxix, m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix());

	if (nullptr == CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_PARTICLE, &_tOption))
	{
		MSG_BOX(TEXT("effect_blood"));
		return;
	}
}

CLence * CLence::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CLence*		pInstance = new CLence(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CLence"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CLence::Clone(void * pArg)
{
	CLence*		pInstance = new CLence(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CMonster"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLence::Free()
{
	__super::Free();
}
