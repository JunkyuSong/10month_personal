#include "stdafx.h"
#include "Arrow.h"
#include "GameInstance.h"

#include "Status.h"
#include "Effect_Mgr.h"
#include "ArrowTwist.h"



CArrow::CArrow(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CEffect(pDevice, pContext)
{
}

CArrow::CArrow(const CArrow & rhs)
	: CEffect(rhs)
{
}

HRESULT CArrow::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CArrow::Initialize(void * pArg)
{
	m_eTypeObj = CGameObject::TYPE_BULLET;
	if (!m_bDead)
	{
		if (FAILED(Ready_Components()))
			return E_FAIL;
	}

	if (!pArg)
	{
		return E_FAIL;
	}

	m_tInfo = *(ARROW*)pArg;

	_matrix _Start;
	m_tInfo.StartMatrix._42 += 1.1f;

	m_pTransformCom->Set_WorldFloat4x4(m_tInfo.StartMatrix);
	_Start = XMLoadFloat4x4(&m_tInfo.StartMatrix);
	XMStoreFloat4(&m_vStartPos, _Start.r[3]);
	m_vPos.clear();

	_float3 _fvPos;
	XMStoreFloat3(&_fvPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION) - m_pTransformCom->Get_State(CTransform::STATE_LOOK));
	m_vPos.push_back(_fvPos);
	XMStoreFloat3(&_fvPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION) + m_pTransformCom->Get_State(CTransform::STATE_LOOK));
	m_vPos.push_back(_fvPos);

	m_pArrowTwist.push_back(static_cast<CArrowTwist*>(CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_TWIST, &m_pTransformCom->Get_WorldMatrix())));
	m_pTransformCom->Turn_Angle(m_pTransformCom->Get_State(CTransform::STATE_LOOK), XMConvertToRadians(90.f));
	m_pArrowTwist.push_back(static_cast<CArrowTwist*>(CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_TWIST, &m_pTransformCom->Get_WorldMatrix())));

	for (auto& iter : m_pArrowTwist)
	{
		Safe_AddRef(iter);
	}
	m_bDead = false;
	m_fCurTime = 0.f;
	m_fTick = 0.f;

	m_tOption.eType = CEffect_Particle::PARTICLETYPE::TYPE_STRIGHT;
	m_tOption.fAccSpeed = 0.99f;
	m_tOption.fSpeed = { 0.5f, 1.3f };
	m_tOption.fGravity = 0.f;
	m_tOption.fLifeTime = 0.5f;
	m_tOption.fRange = _float3(5.f, 5.f, 1.f);
	m_tOption.iNumParticles = 1;
	m_tOption.Size = _float2(0.1f, 0.1f);
	m_tOption.Spread = CEffect_Particle::SPREAD::SPREAD_EDGE;
	m_tOption.szMaskTag = TEXT("Prototype_Component_Texture_Mask_Blood");
	m_tOption.szTextureTag = TEXT("Prototype_Component_Texture_Diffuse_Blood");
	m_tOption.vColor = CLIENT_RGB(179.f, 245.f, 150.f);
	m_tOption.fSpead_Angle = _float3(0.f, 10.f, 10.f);
	m_tOption.vStart_Dir = _float3(0.f, 0.f, -1.f);
	m_tOption.eDiffuseType = CEffect_Particle::DIFFUSE_COLOR;
	m_tOption.eDirType = CEffect_Particle::DIR_TYPE::DIR_ANGLE;
	m_tOption.eStartType = CEffect_Particle::START_CENTER;
	m_tOption.fMaxDistance = { 0.4f, 1.2f };
	m_tOption.eDissapear = CEffect_Particle::DISSAPEAR_ALPHA;

	m_tOption.bPlayerDir = true;
	XMStoreFloat4x4(&m_tOption.matPlayerAxix, m_pTransformCom->Get_WorldMatrix());

	m_pStatusCom->Set_Attack(m_fUseStealSkill * 25.f);

	return S_OK;
}

void CArrow::Tick(_float fTimeDelta)
{
}

void CArrow::LateTick(_float fTimeDelta)
{

}

const _bool & CArrow::Update(_float _fTimeDelta)
{
	if (m_bDead)
	{
		m_vPos.clear();
		for (auto& iter : m_pArrowTwist)
		{
			Safe_Release(iter);
		}
		m_pArrowTwist.clear();
		return false;
	}
		

	AUTOINSTANCE(CGameInstance, _pGameInstance);

	m_fCurTime += _fTimeDelta/* / _pGameInstance->Get_TimeSpeed(TEXT("Timer_Main"))*/;
	m_fTick += _fTimeDelta / _pGameInstance->Get_TimeSpeed(TEXT("Timer_Main"));
	_vector _vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	_float	_fDis = fabs(XMVector3Length(_vPos - XMLoadFloat4(&m_vStartPos)).m128_f32[0]);
	if (_fDis < 130.f)
	{		
		m_pTransformCom->Go_Straight(m_fSpeed * _fTimeDelta /*/ _pGameInstance->Get_TimeSpeed(TEXT("Timer_Main"))*/);
		XMStoreFloat3(&m_vPos[0], XMLoadFloat3(&m_vPos[0]) + m_fSpeed *
			( m_pTransformCom->Get_State(CTransform::STATE_LOOK)) * _fTimeDelta /*/ _pGameInstance->Get_TimeSpeed(TEXT("Timer_Main"))*/ );

		for (auto& iter : m_pArrowTwist)
		{
			iter->Move_Effect(m_pTransformCom->Get_State(CTransform::STATE_POSITION));
		}
	}
	
	if (m_fCurTime > m_fMaxTime)
	{
		XMStoreFloat3(&m_vPos[1], XMLoadFloat3(&m_vPos[1]) +  7.f * _fTimeDelta * m_pTransformCom->Get_State(CTransform::STATE_LOOK));
		//m_fCurTime = 0.f;

		m_tOption.Center = m_vPos[1];
		m_tOption.Size = _float2(_pGameInstance->Rand_Float(0.03f, 0.13f), _pGameInstance->Rand_Float(0.03f, 0.13f));
		if (nullptr == CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_PARTICLE, &m_tOption))
		{
			MSG_BOX(TEXT("fail effect"));
			return false;
		}
	}

	if (m_fTick > 4.f)
	{
		m_bDead = true;
		for (auto& iter : m_pArrowTwist)
		{
			Safe_Release(iter);
		}
		m_pArrowTwist.clear();
	}

	Compute_CamZ(m_pTransformCom->Get_State(CTransform::STATE_POSITION));
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_ALPHABLEND, this) ;
	m_pColliderCom->Update(m_pTransformCom->Get_WorldMatrix());
	CCollisionMgr::Get_Instance()->Add_CollisoinList(CCollisionMgr::GAMEOBJ_TYPE::TYPE_PLAYER_BULLET, m_pColliderCom, this);
	return true;
}

HRESULT CArrow::Render()
{
	if (nullptr == m_pVIBufferCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	if (m_pColliderCom->Get_Target())
	{
		//맞은 이펙트 추가 하고 얘는 사라지고
		for (auto& iter : m_pArrowTwist)
		{
			iter->Set_Stop(true);
		}
		_matrix _World = m_pTransformCom->Get_WorldMatrix();
		CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_TARGET, &_World);

		m_bDead = true;

	}

	AUTOINSTANCE(CGameInstance, pGameInstance);

	{
	//up은 카메라를 보는 방향 넣고, 그리고 룩은 고정이니까 그걸로 넣어서 right구하고, 그걸로 제대로 된 up 구한다.
		_vector _vUp = XMLoadFloat4(&pGameInstance->Get_CamPosition()) - m_pTransformCom->Get_State(CTransform::STATE_POSITION);

		_vector _vRight = XMVector3Cross(_vUp, m_pTransformCom->Get_State(CTransform::STATE_LOOK));
		_vUp = XMVector3Cross(m_pTransformCom->Get_State(CTransform::STATE_LOOK), _vRight);

		m_pTransformCom->Set_State(CTransform::STATE_RIGHT, _vRight);
		m_pTransformCom->Set_State(CTransform::STATE_UP, _vUp);
		m_pTransformCom->Set_Scale(XMVectorSet(1.f, 1.f, 1.f, 0.f));
	}

	_float		_fWidth = 0.3f;

	m_pShaderCom->Set_RawValue("g_Width", &_fWidth, sizeof(_float));
	m_pShaderCom->Set_RawValue("g_fTime", &m_fTick, sizeof(_float));
	if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4))))
		return E_FAIL;
	m_pShaderCom->Set_RawValue("g_Color", &(CLIENT_RGB(119.f, 245.f, 100.f)), sizeof(_float4));
	//m_pTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture");

	
	
	{
		_float3 _vRight, _vUp, _vLook;

		XMStoreFloat3(&_vRight, m_pTransformCom->Get_State(CTransform::STATE_RIGHT));
		XMStoreFloat3(&_vUp, m_pTransformCom->Get_State(CTransform::STATE_UP));

		m_pShaderCom->Set_RawValue("g_Right", &(_vRight), sizeof(_float3));
		m_pShaderCom->Set_RawValue("g_Up", &(_vUp), sizeof(_float3));
		_fWidth = 1.5f;

		m_pArrowTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture");
		_float3 vPos[2];
		vPos[0] = m_vPos[1];
		vPos[1] = m_vPos[0];

		m_pShaderCom->Set_RawValue("g_vPos_1", &(vPos[0]), sizeof(_float3));
		m_pShaderCom->Set_RawValue("g_vPos_2", &(vPos[1]), sizeof(_float3));

		if (FAILED(m_pShaderCom->Begin(6)))
			return E_FAIL;

		if (FAILED(m_pVIBufferCom->Render()))
			return E_FAIL;


	}

	return S_OK;
}

CArrow * CArrow::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CArrow*		pInstance = new CArrow(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CUI_Plus"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CArrow::Clone(void * pArg)
{
	CGameObject*		pInstance = new CArrow(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CUI_Plus"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CArrow::Free()
{
	__super::Free();

	Safe_Release(m_pRendererCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pTransformCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pFlareTextureCom);
	Safe_Release(m_pArrowTextureCom);
	for (auto& iter : m_pArrowTwist)
	{
		Safe_Release(iter);
	}
	m_pArrowTwist.clear();
	Safe_Release(m_pColliderCom);
	Safe_Release(m_pStatusCom);	
}

HRESULT CArrow::Ready_Components()
{
	/* For.Com_Transform */
	CTransform::TRANSFORMDESC	_Desc;
	_Desc.fRotationPerSec = XMConvertToRadians(90.f);
	//_Desc.fSpeedPerSec = 1.5f;
	_Desc.fSpeedPerSec = 1.f;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform"), (CComponent**)&m_pTransformCom, &_Desc)))
		return E_FAIL;

	m_pTransformCom->Set_Scale(XMVectorSet(1.f, 1.f, 1.f, 0.f));
	/* For.Com_Renderer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_Point"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Point"), TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	/* For.Com_Status */
	CStatus::STATUS _tStatus;
	_tStatus.fMaxHp = 0.f;
	_tStatus.fAttack = 25.f;
	_tStatus.fHp = _tStatus.fMaxHp;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Status"), TEXT("Com_Status"), (CComponent**)&m_pStatusCom, &_tStatus)))
		return E_FAIL;

	/* For.Com_ArrowTexture */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_Mask_Arrow"), TEXT("Com_ArrowTexture"), (CComponent**)&m_pArrowTextureCom)))
		return E_FAIL;

	/* For.Com_OBB */
	CCollider::COLLIDERDESC		ColliderDesc;
	ZeroMemory(&ColliderDesc, sizeof(CCollider::COLLIDERDESC));

	ColliderDesc.vSize = _float3(0.3f, 0.3f, 0.3f);
	ColliderDesc.vCenter = _float3(0.f, 0.f, 0.f);
	ColliderDesc.vRotation = _float3(0.f, 0.f, 0.f);
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Collider_OBB"), TEXT("Com_Claw"), (CComponent**)&m_pColliderCom, &ColliderDesc)))
		return E_FAIL;

	return S_OK;
}
