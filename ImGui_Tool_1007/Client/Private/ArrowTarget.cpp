#include "stdafx.h"
#include "ArrowTarget.h"
#include "GameInstance.h"

#include "Effect_Mgr.h"

CArrowTarget::CArrowTarget(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CEffect(pDevice, pContext)
{
}

CArrowTarget::CArrowTarget(const CArrowTarget & rhs)
	: CEffect(rhs)
{
}

CArrowTarget::~CArrowTarget()
{
}

HRESULT CArrowTarget::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CArrowTarget::Initialize(void * pArg)
{

	if (pArg == nullptr)
	{
		return E_FAIL;
	}

	


	if (m_bDead == false)
	{
		if (FAILED(Ready_Components()))
			return E_FAIL;
	}

	m_bDead = false;
	_matrix _matWorld = *(_matrix*)pArg;
	m_pTransformCom->Set_WorldMatrix(_matWorld);
	m_pTransformCom->Set_Scale(XMVectorSet(0.01f, 0.01f, 0.01f, 0.f));
	m_pTransformCom->Turn_Angle(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(90.f));
	return S_OK;
}

const _bool& CArrowTarget::Update(_float fTimeDelta)
{
	if (m_bDead)
	{
		m_fCurTime = 0.f;
		m_fEffectTime = 0.f;
		m_fSpeed = 0.f;
		m_iPass = 10;
		m_fAccSpeed = 1.f;
		m_bLast = false;
		return false;
	}
	//m_fSpeed *= 1.03f;
	//m_pTransformCom->Turn_Angle(XMVectorSet(0.f, 1.f, 0.f, 0.f), -m_fSpeed * 3.f * fTimeDelta);
	m_fAccSpeed *= 1.01f;
	if (m_bLast)
	{
		m_fEffectTime -= fTimeDelta * m_fAccSpeed* 2.f;

	}
	else
	{
		m_fEffectTime += fTimeDelta * m_fAccSpeed* 1.2f;
	}
	m_fSpeed += fTimeDelta;
	//if (m_bLast) // 이건 퐉 퍼질때
	//{
	//	m_fCurTime += fTimeDelta;
	//	m_fEffectTime = (m_fCurTime) * 1.1f;
	//	_float3 _vScale = m_pTransformCom->Get_Scale();
	//	_vScale.x *= 1.05f;
	//	_vScale.y *= 1.01f;
	//	_vScale.z *= 1.05f;
	//	m_pTransformCom->Set_Scale(XMLoadFloat3(&_vScale));
	//	
	//	m_iPass = 6;
	//}
	//else
	//{
	//	AUTOINSTANCE(CGameInstance, _pGame);
	//	_float _fRand = _pGame->Rand_Float(-0.003f, 0.003f);
	//	_float _fRandY = _pGame->Rand_Float(-0.001f, 0.001f);
	//	_float3 _vScale = m_pTransformCom->Get_Scale();
	//	_vScale.x += _fRand;
	//	//_vScale.y += _fRandY;
	//	_vScale.z += _fRand;
	//	m_pTransformCom->Set_Scale(XMLoadFloat3(&_vScale));
	//}
	//
	//Compute_CamZ(m_pTransformCom->Get_State(CTransform::STATE_POSITION));
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_ALPHABLEND, this);

	return true;
}

HRESULT CArrowTarget::Render()
{
	AUTOINSTANCE(CGameInstance, pGameInstance);

	if (FAILED(m_pShaderCom->Set_RawValue("g_fColor", &(CLIENT_RGB(119.f, 245.f, 100.f)), sizeof(_float4))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4))))
		return E_FAIL;

	
	if (FAILED(m_pShaderCom->Set_RawValue("g_fAppear", &(m_fEffectTime), sizeof(_float))))
		return E_FAIL; 
	if (FAILED(m_pShaderCom->Set_RawValue("g_fTime", &(m_fSpeed), sizeof(_float))))
		return E_FAIL;

	if (m_fEffectTime > 1.f)
	{
		m_bLast = true;
	}
	if (m_fEffectTime < 0.f)
		m_bDead = true;

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

void CArrowTarget::Setting(_vector _vPos, _bool _bLast)
{
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vPos);
	m_bLast = _bLast;
}

HRESULT CArrowTarget::Ready_Components()
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
	if (FAILED(__super::Add_Component(LEVEL_STAGE_LOBBY, TEXT("Prototype_Component_Model_Effect_Target"), TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

CArrowTarget * CArrowTarget::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CArrowTarget*		pInstance = new CArrowTarget(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CArrowTarget"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CArrowTarget::Clone(void * pArg)
{
	CArrowTarget*		pInstance = new CArrowTarget(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CMonster"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CArrowTarget::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
}
