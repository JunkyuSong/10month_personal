#include "stdafx.h"
#include "Claw.h"
#include "GameInstance.h"

#include "Effect_Mgr.h"

CClaw::CClaw(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CEffect(pDevice, pContext)
{
}

CClaw::CClaw(const CClaw & rhs)
	: CEffect(rhs)
{
}

CClaw::~CClaw()
{
}

HRESULT CClaw::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CClaw::Initialize(void * pArg)
{

	if (pArg == nullptr)
	{
		return E_FAIL;
	}

	m_tClaw = *(CLAW_DESC*)pArg;

	if (m_bDead == false)
	{
		if (FAILED(Ready_Components()))
			return E_FAIL;
	}

	m_bDead = false;
	m_pTransformCom->Set_WorldFloat4x4(m_tClaw.TargetMatrix);
	

	_vector _vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	_vector _vRight = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_RIGHT));
	_vector _vLook = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));
	_vector _vUp = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_UP));

	switch (m_tClaw.eClaw)
	{
	case CLAWTYPE_CENTER:
		m_pTransformCom->Set_Scale(XMVectorSet(0.0105f, 0.01f, 0.0105f, 1.f));
		_vPos += _vLook * 0.5f + _vUp * 1.3f;
		m_fCurTime = 0.1f;
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vPos);

		//m_pTransformCom->Rotation(XMVectorSet(1.f, 0.f, 0.f, 0.f), XMConvertToRadians(-90.0f));
		//m_fAngle = _float3(, -1.65f, )
		m_pTransformCom->Turn_Angle(_vRight, XMConvertToRadians(-2.f));
		//m_pTransformCom->Turn_Angle(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(-f));
		m_pTransformCom->Turn_Angle(_vLook, XMConvertToRadians(140.12f));
		break;
	case CLAWTYPE_RIGHT:
		m_pTransformCom->Set_Scale(XMVectorSet(0.01f, 0.01f, 0.01f, 1.f));
		_vPos += _vLook * 0.5f + _vUp * 1.43f + _vRight * 0.2f;
		m_fCurTime = 0.1f;
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vPos);

		//m_pTransformCom->Rotation(XMVectorSet(1.f, 0.f, 0.f, 0.f), XMConvertToRadians(-90.0f));
		//m_fAngle = _float3(, -1.65f, )
		m_pTransformCom->Turn_Angle(_vRight, XMConvertToRadians(3.f));
		//m_pTransformCom->Turn_Angle(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(-f));
		m_pTransformCom->Turn_Angle(_vLook, XMConvertToRadians(145.12f));
		break;
	case CLAWTYPE_LEFT:
		m_pTransformCom->Set_Scale(XMVectorSet(0.01f, 0.01f, 0.01f, 1.f));

		_vPos += _vLook * 0.5f + _vUp * 1.1f - _vRight * 0.22f;
		

		m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vPos);

		//m_pTransformCom->Rotation(XMVectorSet(1.f, 0.f, 0.f, 0.f), XMConvertToRadians(-90.0f));
		//m_fAngle = _float3(, -1.65f, )
		m_pTransformCom->Turn_Angle(_vRight, XMConvertToRadians(-7.f));
		//m_pTransformCom->Turn_Angle(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(-f));
		m_pTransformCom->Turn_Angle(_vLook, XMConvertToRadians(130.12f));
		break;
	}
	

	//행렬을 받아와서 세팅해준다



	return S_OK;
}

const _bool& CClaw::Update(_float fTimeDelta)
{
	if (m_bDead)
	{
		m_fCurTime = 0.f;
		m_iPass = 0;
		m_fEffectTime = 0.f;
		m_bCreate = false;
		return false;
	}
		
	m_fCurTime += fTimeDelta;

	if (m_fCenterMaxTime < m_fCurTime && !m_bCreate && m_tClaw.eClaw == CLAWTYPE_LEFT)
	{
		CClaw::CLAW_DESC _tInfo;
		_tInfo.TargetMatrix = m_tClaw.TargetMatrix;
		_tInfo.eClaw = CClaw::CLAWTYPE_RIGHT;
		CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_CLAW, &_tInfo);
		_tInfo.eClaw = CClaw::CLAWTYPE_CENTER;
		CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_CLAW, &_tInfo);

		m_bCreate = true;
	}

	if (m_fCurTime > m_fMaxTime)
	{
		m_iPass = 2;
		m_fEffectTime = (m_fCurTime - m_fMaxTime) * 1.1f;
		_float3 _vScale = m_pTransformCom->Get_Scale();
		_vScale.y *= 1.05f;
		m_pTransformCom->Set_Scale(XMLoadFloat3(&_vScale));
	}
	
	Compute_CamZ(m_pTransformCom->Get_State(CTransform::STATE_POSITION));
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_ALPHABLEND, this);

	return true;
}

HRESULT CClaw::Render()
{
	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);
	if (FAILED(m_pShaderCom->Set_RawValue("g_fColor", &CLIENT_RGB(119.f, 245.f, 130.f), sizeof(_float4))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4))))
		return E_FAIL;

	if (m_iPass == 2)
	{
		if (FAILED(m_pShaderCom->Set_RawValue("g_fTime", &(m_fEffectTime), sizeof(_float))))
			return E_FAIL;
		if (m_fEffectTime > 1.f)
			m_bDead = true;
	}

	RELEASE_INSTANCE(CGameInstance);
	_uint		iNumMeshes = m_pModelCom->Get_NumMesh();

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(m_pModelCom->SetUp_OnShader(m_pShaderCom, m_pModelCom->Get_MaterialIndex(i), aiTextureType_DIFFUSE, "g_DiffuseTexture")))
			return E_FAIL;
		

		if (FAILED(m_pShaderCom->Begin(m_iPass)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CClaw::Ready_Components()
{

	/* For.Com_Transform */
	CTransform::TRANSFORMDESC	_Desc;
	_Desc.fRotationPerSec = XMConvertToRadians(90.f);
	//_Desc.fSpeedPerSec = 1.5f;
	_Desc.fSpeedPerSec = 2.5f;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform"), (CComponent**)&m_pTransformCom, &_Desc)))
		return E_FAIL;
	
	/* For.Com_Renderer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_Effect"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_STAGE_LOBBY, TEXT("Prototype_Component_Model_Effect_Claw"), TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

CClaw * CClaw::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CClaw*		pInstance = new CClaw(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CClaw"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CClaw::Clone(void * pArg)
{
	CClaw*		pInstance = new CClaw(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CMonster"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CClaw::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
}
