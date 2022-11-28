#include "stdafx.h"
#include "..\Public\Scythe.h"
#include "GameInstance.h"


#include "Trail_Obj.h"

#include "Effect_Mgr.h"
#include "Fire.h"
#include "DisappearWeapon.h"
#include "CameraMgr.h"
#include "Camera.h"

CScythe::CScythe(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CWeapon(pDevice, pContext)
{
}

CScythe::CScythe(const CScythe & rhs)
	: CWeapon(rhs)
{
}

HRESULT CScythe::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CScythe::Initialize(void * pArg)
{
	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pTransformCom->Set_Scale(XMVectorSet(0.01f, 0.01f, 0.01f, 1.f));
	m_pTransformCom->Rotation(XMVectorSet(1.f, 0.f, 0.f, 0.f), XMConvertToRadians(-90.0f));





	return S_OK;
}

void CScythe::Tick(_float fTimeDelta)
{
	m_pTrailCom->Tick(fTimeDelta, m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix());

	if (m_bColliderOn)
		m_pColliderCom->Update(m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix());
	
}

void CScythe::Tick(_float fTimeDelta, CGameObject * _pUser)
{
	if(!m_pTrailCom->Get_On())
		TrailOn();
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

	if (m_bFire)
		Fire();
	Compute_CamZ(m_pTransformCom->Get_State(CTransform::STATE_POSITION));
}

void CScythe::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;


}

HRESULT CScythe::Render()
{
	if (nullptr == m_pModelCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	CGameObject* pTarget = m_pColliderCom->Get_Target();

	if (pTarget)
	{
		// 라이트는 패링되었을때, 그 외엔 피 이펙트(시간 멈추는 타이밍에)!
		//Light_On();
		Hit();
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
	//if (FAILED(m_pShaderCom->Set_RawValue("g_CamPosition", &pGameInstance->Get_CamPosition(), sizeof(_float4))))
	//	return E_FAIL;

	RELEASE_INSTANCE(CGameInstance);
	_uint		iNumMeshes = m_pModelCom->Get_NumMesh();

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(m_pModelCom->SetUp_OnShader(m_pShaderCom, m_pModelCom->Get_MaterialIndex(i), aiTextureType_DIFFUSE, "g_DiffuseTexture")))
			return E_FAIL;
		if (FAILED(m_pModelCom->SetUp_OnShader(m_pShaderCom, m_pModelCom->Get_MaterialIndex(i), aiTextureType_NORMALS, "g_NormalTexture")))
		return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(1)))
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

void CScythe::Clear(void * pArg)
{
	m_fFireTime = 0.f;
	m_bFire = *(_bool*)pArg;
}
void CScythe::End()
{
	CDisappearWeapon::DISAPPEARWEAPON _tInfo;
	_tInfo.pModel = m_pModelCom;
	XMStoreFloat4x4(&_tInfo.World, m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix());
	_tInfo.HighAndLow = m_pTrailCom->Get_HighAndLow(m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix());
	_tInfo.fSpeed = 0.6f;
	_tInfo.iPass = 4;
	CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_DISAPPEAR, &_tInfo);
}


HRESULT CScythe::Ready_Components()
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
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Weapon_Scythe"), TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	/* For.Com_Trail */
	CTrail::TRAILINFO _tInfo;
	_tInfo._Color = CLIENT_RGB(0.f,0.f,0.f);
	_tInfo._HighAndLow.vHigh = _float3(120.0f, 0.f, 0.f);
	_tInfo._HighAndLow.vLow = _float3(-135.f, 0.f, 0.f);
	//_tInfo._eOption = CTrail::TRAILOPTION_TEXTURE;
	//_tInfo._szTexture = TEXT("../Bin/Resources/Textures/Trail_Flame.png");
	AUTOINSTANCE(CGameInstance, _pInstance);
	m_pTrailCom = static_cast<CTrail_Obj*>(_pInstance->Clone_GameObject(TEXT("Prototype_GameObject_Trail"), &_tInfo));
	if (m_pTrailCom == nullptr)
		return E_FAIL;


	/* For.Com_OBB */
	CCollider::COLLIDERDESC		ColliderDesc;
	ZeroMemory(&ColliderDesc, sizeof(CCollider::COLLIDERDESC));

	ColliderDesc.vSize = _float3(100.0f, 130.f, 40.f);
	ColliderDesc.vCenter = _float3(100.f, ColliderDesc.vSize.y * 0.5f - 110.f, 0.f);
	ColliderDesc.vRotation = _float3(0.f, XMConvertToRadians(0.f), 0.f);
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Collider_OBB"), TEXT("Com_OBB"), (CComponent**)&m_pColliderCom, &ColliderDesc)))
		return E_FAIL;


	return S_OK;
}

void CScythe::Fire()
{
	TRAILPOS HL = m_pTrailCom->Get_HighAndLow();
	CFire::FIRE_DESC _tInfo;
	XMStoreFloat4(&_tInfo.vPos, XMVectorSetW(XMLoadFloat3(&HL.vHigh), 1.f));
	//high지점에 불 하나 놓고
	// high - low 방향으로 나간다
	if (m_pTrailCom->Get_On())
	{
		m_fFireTime += 0.3f;

		AUTOINSTANCE(CGameInstance, _pInstance);

		_tInfo.vColor = CLIENT_RGB(100.f, 255.f, 100.f);
		_tInfo.fLifeTime = m_fFireTime;

		_float vSize = _pInstance->Rand_Float(0.7f, 1.5f);
		_tInfo.vSize = { vSize, vSize };
		_tInfo.iChance = 1;
		XMStoreFloat3(&_tInfo.vDirect, XMVector3Normalize(XMLoadFloat3(&HL.vHigh) - XMLoadFloat3(&HL.vLow)));

		_float fDis = _pInstance->Rand_Float(-0.2f, 0.3f);
		XMStoreFloat4(&_tInfo.vPos, XMLoadFloat4(&_tInfo.vPos) + XMVectorSetW(fDis * XMLoadFloat3(&_tInfo.vDirect), 0.f));

		_float _fTerm = _pInstance->Rand_Float(0.12f, 0.18f);
		if (fabs(XMVector3Length(XMLoadFloat4(&m_vPreFirePos) - XMLoadFloat4(&_tInfo.vPos)).m128_f32[0]) > _fTerm)
			CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_FIRE, &_tInfo);
		_float4 _vPos = _tInfo.vPos;
		_float3 _vDir = _tInfo.vDirect;
		if (fabs(XMVector3Length(XMLoadFloat4(&m_vPreFirePos) - XMLoadFloat4(&_tInfo.vPos)).m128_f32[0]) > 1.2f)
		{
			for (_float _fRatio = 0.f; _fRatio < 1.f; _fRatio += 0.33f)
			{
				vSize = _pInstance->Rand_Float(0.7f, 1.5f);
				_tInfo.vSize = { vSize, vSize };
				XMStoreFloat4(&_tInfo.vPos, XMVectorSetW(XMVectorLerp(XMLoadFloat4(&m_vPreFirePos), XMLoadFloat4(&_vPos), _fRatio), 1.f));
				XMStoreFloat3(&_tInfo.vDirect, XMVector3Normalize(XMVectorLerp(XMLoadFloat3(&m_vPreFireDir), XMLoadFloat3(&_vDir), _fRatio)));
				CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_FIRE, &_tInfo);
			}
		}

	}
	m_vPreFirePos = _tInfo.vPos;
	m_vPreFireDir = _tInfo.vDirect;
}

void CScythe::Hit()
{
	_vector LightPos, vHigh, vLow;
	vHigh = XMVectorSet(100.0f, 0.f, 0.f, 1.f);
	vLow = XMVectorSet(-5.f, 0.f, 0.f, 1.f);


	vHigh = XMVector3TransformCoord(vHigh, m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix());
	vLow = XMVector3TransformCoord(vLow, m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix());

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

CScythe * CScythe::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CScythe*		pInstance = new CScythe(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CScythe"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CScythe::Clone(void * pArg)
{
	CScythe*		pInstance = new CScythe(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CMonster"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CScythe::Free()
{
	__super::Free();
}
