#include "stdafx.h"
#include "..\Public\Lamp.h"
#include "Obj_NonAnim.h"
#include "Renderer.h"
#include "Shader.h"
#include "NonAnimModel.h"
#include "Transform.h"
#include "GameInstance.h"

CLamp::CLamp(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CObj_NonAnim(pDevice, pContext)
{
}

CLamp::CLamp(const CLamp & rhs)
	: CObj_NonAnim(rhs)
{
}

HRESULT CLamp::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLamp::Initialize(void * pArg)
{
	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (pArg == nullptr)
		return E_FAIL;

	AUTOINSTANCE(CGameInstance, pGameInstance);

	_matrix matStreetLight= *(_matrix*)pArg;
	matStreetLight.r[0] *= 100.f;
	matStreetLight.r[1] *= 100.f;
	matStreetLight.r[2] *= 100.f;
	_vector _vPos = XMVector3TransformCoord(XMVectorSet(-0.03f, 3.02f, 1.05f, 1.f), matStreetLight);

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vPos);
	m_pTransformCom->Turn_Angle(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(62.f));
	m_pTransformCom->Set_Scale(XMVectorSet(0.01f, 0.01f, 0.01f, 0.f));

	_vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	_vPos.m128_f32[1] += 0.15f;
	POINTLIGHTDESC			LightDesc;
	ZeroMemory(&LightDesc, sizeof(POINTLIGHTDESC));

	XMStoreFloat4(&LightDesc.vPosition, _vPos);
	LightDesc.fRange = 15.f;
	LightDesc.vDiffuse = CLIENT_RGB(255.f, 127.f, 0.f);
	LightDesc.vAmbient = _float4(0.3f, 0.3f, 0.3f, 1.f);
	LightDesc.vSpecular = _float4(0.1f, 0.1f, 0.1f, 1.f);
	LightDesc.LightViewMatrix = new _float4x4;

	_vector _vLook, _vRight, _vUp;

	_vLook = XMVectorSetW(XMVector3Normalize(XMVectorSet(45.f, -25.f, 45.f, 1.f) - _vPos), 0.f);
	_vRight = XMVectorSetW(XMVector3Normalize(XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), _vLook)), 0.f);
	_vUp = XMVectorSetW(XMVector3Normalize(XMVector3Cross(_vLook, _vRight)), 0.f);

	_matrix _World;
	_World.r[0] = _vRight;
	_World.r[1] = _vUp;
	_World.r[2] = _vLook;
	_World.r[3] = _vPos;
	_World = XMMatrixInverse(nullptr, _World);
	_World = XMMatrixTranspose(_World);

	XMStoreFloat4x4(LightDesc.LightViewMatrix, _World);

	LightDesc.LightProjMatrix = new _float4x4;
	XMStoreFloat4x4(LightDesc.LightProjMatrix, XMMatrixTranspose(XMMatrixPerspectiveFovLH(XMConvertToRadians(160.f), 1280.f / 720.f, 0.2f, 300.f)));
	
	m_iLightIndex = pGameInstance->Add_Light(m_pDevice, m_pContext, LEVEL_STAGE_LAST, CLight_Manager::STATICPOINTLIHGT, LightDesc);

	if (m_iLightIndex == -1)
		return E_FAIL;


	pGameInstance->Light_On(LEVEL_STAGE_LAST, CLight_Manager::STATICPOINTLIHGT, m_iLightIndex);

	return S_OK;
}

void CLamp::Tick( _float fTimeDelta)
{


}

void CLamp::LateTick( _float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, this);
}

HRESULT CLamp::Render()
{
	if (nullptr == m_pModelCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;



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

HRESULT CLamp::Ready_Components()
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

	if (FAILED(__super::Add_Component(LEVEL_STAGE_LAST, TEXT("Prototype_Component_Model_Lamp"), TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLamp::SetUp_ShaderResources()
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

CLamp * CLamp::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CLamp*		pInstance = new CLamp(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CLamp"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CLamp::Clone(void * pArg)
{
	CLamp*		pInstance = new CLamp(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CLamp"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLamp::Free()
{
	__super::Free();
}