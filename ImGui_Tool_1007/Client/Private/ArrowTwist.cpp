#include "stdafx.h"
#include "ArrowTwist.h"
#include "GameInstance.h"

#include "Effect_Mgr.h"

CArrowTwist::CArrowTwist(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CEffect(pDevice, pContext)
{
}

CArrowTwist::CArrowTwist(const CArrowTwist & rhs)
	: CEffect(rhs)
{
}

CArrowTwist::~CArrowTwist()
{
}

HRESULT CArrowTwist::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CArrowTwist::Initialize(void * pArg)
{

	if (pArg == nullptr)
	{
		return E_FAIL;
	}
	if (!m_bDead)
	{
		if (FAILED(Ready_Components()))
			return E_FAIL;

	}
	//행렬 받아서 그대로 룩 만들고 거기에서 돌려가면서 시간마다 화살 따라 생성되면서!
	m_bDead = false;
	_matrix _TargetWorld = *(_matrix*)pArg;

	m_pTransformCom->Set_WorldMatrix(_TargetWorld);
	m_pTransformCom->Set_Scale(XMVectorSet(0.01f, 0.005f, 0.005f, 0.f));

	//행렬을 받아와서 세팅해준다

	m_pTransformCom->Turn_Angle(m_pTransformCom->Get_State(CTransform::STATE_UP), XMConvertToRadians(270.f));
	return S_OK;
}

const _bool& CArrowTwist::Update(_float fTimeDelta)
{
		
	if (m_bDead)
	{
		m_fCurTime = 0.f;
		m_iPass = 8;
		m_fEffectTime = 0.f;
		m_fAccSpeed = 1.f;
		return false;
	}

	m_fCurTime += fTimeDelta;
	m_fAccSpeed *= 1.01f;
	m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE_RIGHT), fTimeDelta * (m_fCurTime + 10.f) /** m_fAccSpeed*/);
	m_iPass = 9;
	m_fEffectTime = (m_fCurTime - m_fMaxTime) * 1.05f;
	_float3 _vScale = m_pTransformCom->Get_Scale();
	if (m_fCurTime > m_fMaxTime)
	{
		
		_vScale.y *= 1.02f; // y,z 사이즈만!
		_vScale.z *= 1.02f;
		
		
	}
	_vScale.x += 0.005f;
	m_pTransformCom->Set_Scale(XMLoadFloat3(&_vScale));
	Compute_CamZ(m_pTransformCom->Get_State(CTransform::STATE_POSITION));
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_ALPHABLEND, this);

	return true;
}

HRESULT CArrowTwist::Render()
{
	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);

	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4))))
		return E_FAIL;

	if (m_iPass == 9)
	{
		if (FAILED(m_pShaderCom->Set_RawValue("g_fTime", &(m_fEffectTime), sizeof(_float))))
			return E_FAIL;
		if (m_fEffectTime > 1.0f)
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

void CArrowTwist::Move_Effect(_vector _vPos)
{
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vPos);
}

HRESULT CArrowTwist::Ready_Components()
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
	if (FAILED(__super::Add_Component(LEVEL_STAGE_LOBBY, TEXT("Prototype_Component_Model_Weapon_Twist2"), TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

CArrowTwist * CArrowTwist::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CArrowTwist*		pInstance = new CArrowTwist(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CArrowTwist"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CArrowTwist::Clone(void * pArg)
{
	CArrowTwist*		pInstance = new CArrowTwist(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CMonster"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CArrowTwist::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
}
