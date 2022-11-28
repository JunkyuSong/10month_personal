#include "stdafx.h"
#include "Stage_01.h"
#include "GameInstance.h"
#include "ImGuiMgr.h"

CStage_01::CStage_01(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject(pDevice, pContext)
{
}

CStage_01::CStage_01(const CStage_01 & rhs)
	: CGameObject(rhs)
{
}

HRESULT CStage_01::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CStage_01::Initialize(void * pArg)
{
	if (FAILED(Ready_Components()))
		return E_FAIL;

	__super::Initialize(pArg);
	m_pTransformCom->Set_Scale(XMVectorSet(0.01f, 0.01f, 0.01f, 0.f));
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(45.f, 0.1f, 45.f, 1.f));

	return S_OK;
}

void CStage_01::Tick( _float fTimeDelta)
{
	ImGuiTick();

}

void CStage_01::LateTick( _float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, this);
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
}

HRESULT CStage_01::Render()
{
	if (nullptr == m_pModelCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;



	SetUp_ShaderResources();

	_uint		iNumMeshes = m_pModelCom->Get_NumMesh();//�޽� ������ �˰� �޽� ������ŭ ������ �� ����. ���⼭!

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(m_pModelCom->SetUp_OnShader(m_pShaderCom, m_pModelCom->Get_MaterialIndex(i), aiTextureType_DIFFUSE, "g_DiffuseTexture")))
			return E_FAIL;

		if (FAILED(m_pModelCom->SetUp_OnShader(m_pShaderCom, m_pModelCom->Get_MaterialIndex(i), aiTextureType_NORMALS, "g_NormalTexture")))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(5)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}


	return S_OK;
}


HRESULT CStage_01::Render_Shadow()
{
	if (nullptr == m_pModelCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	AUTOINSTANCE(CGameInstance, pGameInstance);

	SetUp_ShaderResources();

	_uint		iNumMeshes = m_pModelCom->Get_NumMesh();//�޽� ������ �˰� �޽� ������ŭ ������ �� ����. ���⼭!

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(m_pModelCom->SetUp_OnShader(m_pShaderCom, m_pModelCom->Get_MaterialIndex(i), aiTextureType_DIFFUSE, "g_DiffuseTexture")))
			return E_FAIL;

		if (FAILED(m_pModelCom->SetUp_OnShader(m_pShaderCom, m_pModelCom->Get_MaterialIndex(i), aiTextureType_NORMALS, "g_NormalTexture")))
			return E_FAIL;

		DIRLIGHTDESC* _DirLightDesc = pGameInstance->Get_DirLightDesc(g_eCurLevel, 0);

		if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", (_DirLightDesc->LightDirInverseMatrix), sizeof(_float4x4))))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(11)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}


	return S_OK;
}


_bool CStage_01::Picking(_float3 & _vPos)
{
	_bool			_bPick;
	_vector			vPickPos;
	if (_bPick = m_pModelCom->Picking(m_pTransformCom, vPickPos))
	{
		XMStoreFloat3(&_vPos, vPickPos);
	}
	return _bPick;
}

void CStage_01::ImGuiTick()
{

}

HRESULT CStage_01::Ready_Components()
{
	/* For.Com_Transform */
	CTransform::TRANSFORMDESC	_Desc;
	_Desc.fRotationPerSec = XMConvertToRadians(90.f);
	//_Desc.fSpeedPerSec = 1.5f;
	_Desc.fSpeedPerSec =2.5f;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform"), (CComponent**)&m_pTransformCom, &_Desc)))
		return E_FAIL;

	/* For.Com_Renderer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_Model"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Stage_01"), TEXT("Com_Model"), (CComponent**)&m_pModelCom);

	return S_OK;
}

HRESULT CStage_01::SetUp_ShaderResources()
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


	RELEASE_INSTANCE(CGameInstance);

	return S_OK;
}

CStage_01 * CStage_01::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CStage_01*		pInstance = new CStage_01(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CStage_01"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CStage_01::Clone(void * pArg)
{
	CStage_01*		pInstance = new CStage_01(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CStage_01"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CStage_01::Free()
{
	__super::Free();

	
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRendererCom);
	Safe_Release(m_pTransformCom);

}