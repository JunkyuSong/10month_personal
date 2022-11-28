#include "stdafx.h"
#include "InstancingObj.h"
#include "GameInstance.h"
#include "ImGuiMgr.h"

CInstancingObj::CInstancingObj(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CObj_Plus(pDevice, pContext)
{
}

CInstancingObj::CInstancingObj(const CInstancingObj & rhs)
	: CObj_Plus(rhs)
{
}

HRESULT CInstancingObj::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CInstancingObj::Initialize(void * pArg)
{
	if (FAILED(Ready_Components()))
		return E_FAIL;



	return S_OK;
}

void CInstancingObj::Tick( _float fTimeDelta)
{
	ImGuiTick();

}

void CInstancingObj::LateTick( _float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	AUTOINSTANCE(CGameInstance, _pInstance);

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, this);
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);

}

HRESULT CInstancingObj::Render()
{
	if (nullptr == m_pModelCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	m_pModelCom->Culling(5.f);

	SetUp_ShaderResources();

	_uint		iNumMeshes = m_pModelCom->Get_NumMesh();//메쉬 갯수를 알고 메쉬 갯수만큼 렌더를 할 것임. 여기서!

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(m_pModelCom->SetUp_OnShader(m_pShaderCom, m_pModelCom->Get_MaterialIndex(i), aiTextureType_DIFFUSE, "g_DiffuseTexture")))
			return E_FAIL;
		if (FAILED(m_pModelCom->SetUp_OnShader(m_pShaderCom, m_pModelCom->Get_MaterialIndex(i), aiTextureType_NORMALS, "g_NormalTexture")))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(m_ePass)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}


	return S_OK;
}

HRESULT CInstancingObj::Render_Shadow()
{
	if (nullptr == m_pModelCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	AUTOINSTANCE(CGameInstance, pGameInstance);

	SetUp_ShaderResources();

	_uint		iNumMeshes = m_pModelCom->Get_NumMesh();//메쉬 갯수를 알고 메쉬 갯수만큼 렌더를 할 것임. 여기서!

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(m_pModelCom->SetUp_OnShader(m_pShaderCom, m_pModelCom->Get_MaterialIndex(i), aiTextureType_DIFFUSE, "g_DiffuseTexture")))
			return E_FAIL;

		if (FAILED(m_pModelCom->SetUp_OnShader(m_pShaderCom, m_pModelCom->Get_MaterialIndex(i), aiTextureType_NORMALS, "g_NormalTexture")))
			return E_FAIL;

		DIRLIGHTDESC* _DirLightDesc = pGameInstance->Get_DirLightDesc(g_eCurLevel, 0);

		if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", (_DirLightDesc->LightDirInverseMatrix), sizeof(_float4x4))))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(1)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}


	return S_OK;
}

HRESULT CInstancingObj::Set_Info(OBJ_DESC _tInfo)
{
	
	return S_OK;
}

HRESULT CInstancingObj::Set_Instancing(_tchar * _szModelKey, vector<_float4x4>* _vecWorld)
{
	AUTOINSTANCE(CGameInstance, _pGameInstance);
	char* _Path = nullptr;
	char* _Name = nullptr;
	
	CModel* _pModel = static_cast<CModel*>(_pGameInstance->Clone_Component(g_eCurLevel, _szModelKey));
	_pModel->Get_Path(&_Path, &_Name);

	m_pModelCom = CInstancingModel::Create(m_pDevice, m_pContext, _Path, _Name, (*_vecWorld).size(), _vecWorld);
	Safe_Release(_pModel);
	return S_OK;
}

HRESULT CInstancingObj::Ready_Components()
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
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_InstancingModel"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;



	return S_OK;
}

HRESULT CInstancingObj::SetUp_ShaderResources()
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

CInstancingObj * CInstancingObj::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CInstancingObj*		pInstance = new CInstancingObj(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CInstancingObj"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CInstancingObj::Clone(void * pArg)
{
	CInstancingObj*		pInstance = new CInstancingObj(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CInstancingObj"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CInstancingObj::Free()
{
	__super::Free();

	if (m_tInfo.szModelTag != nullptr)
	{
		//Safe_Delete_Array(m_tInfo.szModelTag);
	}
	Safe_Release(m_pModelCom);
}