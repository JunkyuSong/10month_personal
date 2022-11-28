#include "stdafx.h"
#include "..\Public\Bow.h"
#include "GameInstance.h"


#include "Trail_Obj.h"

#include "Effect_Mgr.h"
#include "DisappearWeapon.h"

CBow::CBow(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CWeapon(pDevice, pContext)
{
}

CBow::CBow(const CBow & rhs)
	: CWeapon(rhs)
{
}

HRESULT CBow::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBow::Initialize(void * pArg)
{
	if (FAILED(Ready_Components()))
		return E_FAIL;

	//m_pTransformCom->Set_Scale(XMVectorSet(0.01f, 0.01f, 0.01f, 1.f));
	m_pTransformCom->Rotation(XMVectorSet(1.f, 0.f, 0.f, 0.f), XMConvertToRadians(-90.0f));
	m_pTransformCom->Turn_Angle(m_pTransformCom->Get_State(CTransform::STATE_UP), XMConvertToRadians(180.0f));






	return S_OK;
}

void CBow::Tick(_float fTimeDelta)
{
	m_pTrailCom->Tick(fTimeDelta, m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix());

	if (m_bColliderOn)
		m_pColliderCom->Update(m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix());
}

void CBow::Tick(_float fTimeDelta, CGameObject * _pUser)
{
	if (m_bStop)
		return;
	m_fTime += fTimeDelta * 0.2f;
	_float4 _vAnim;
	XMStoreFloat4(&_vAnim, XMVectorSet(0.f, 0.f, 0.f, 1.f));
	_float _fPlayTime = 0.f;
	_bool	_bTemp = false;
	if (m_pAnimModelCom->Play_Animation(fTimeDelta, &_vAnim, &_fPlayTime, _bTemp))
	{
		m_bStop = true;
		m_fTime = 0.f;
	}

	if (_fPlayTime > 3.f)
	{
		m_bRender = true;
	}
	else if (_fPlayTime > 0.f)
	{
		m_bRender = false;
	}
	Compute_CamZ(m_pTransformCom->Get_State(CTransform::STATE_POSITION));
}

void CBow::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;


}

HRESULT CBow::Render()
{
	if (nullptr == m_pAnimModelCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	if (!m_bRender)
		return S_OK;

	AUTOINSTANCE(CGameInstance, pGameInstance);

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

	if (FAILED(m_pShaderCom->Set_RawValue("g_fTime", &m_fTime, sizeof(_float))))
		return E_FAIL;

	_uint		iNumMeshes;//메쉬 갯수를 알고 메쉬 갯수만큼 렌더를 할 것임. 여기서!


	iNumMeshes = m_pAnimModelCom->Get_NumMesh();//메쉬 갯수를 알고 메쉬 갯수만큼 렌더를 할 것임. 여기서!

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		_uint _iPass = 6;
		if (!m_bRender)
		{
			_iPass = 7;
			if (FAILED(m_pDissolve->Set_SRV(m_pShaderCom, "g_DissolveTexture")))
				return E_FAIL;
		}
		else
		{
			if (FAILED(m_pAnimModelCom->SetUp_OnShader(m_pShaderCom, m_pAnimModelCom->Get_MaterialIndex(i), aiTextureType_DIFFUSE, "g_DiffuseTexture")))
				return E_FAIL;
		}
			

		if (FAILED(m_pAnimModelCom->Render(m_pShaderCom, _iPass, i)))
			return E_FAIL;
	}

	if (nullptr != m_pColliderCom && m_bColliderOn)
		m_pColliderCom->Render();

	return S_OK;
}

void CBow::Shoot()
{
	CArrow::ARROW _tArrow;
	XMStoreFloat4x4(&_tArrow.StartMatrix, m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix());
	CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_ARROW, &_tArrow);
}

void CBow::End()
{
	CDisappearWeapon::DISAPPEARWEAPON _tInfo;
	_tInfo.pModel = m_pModelCom;
	XMStoreFloat4x4(&_tInfo.World, m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix());
	_tInfo.HighAndLow.vHigh = _float3(0.7f, 0.f, 0.f);
	_tInfo.HighAndLow.vLow = _float3(-0.7f, 0.f, 0.f);
	_matrix World = m_pTransformCom->Get_WorldMatrix() * m_pParentTransformCom->Get_WorldMatrix();
	XMStoreFloat3(&_tInfo.HighAndLow.vHigh, XMVector3TransformCoord(XMLoadFloat3(&_tInfo.HighAndLow.vHigh), World));
	XMStoreFloat3(&_tInfo.HighAndLow.vLow, XMVector3TransformCoord(XMLoadFloat3(&_tInfo.HighAndLow.vLow), World));
	_tInfo.fSpeed = 0.8f;
	_tInfo.bBow = true;
	CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_DISAPPEAR, &_tInfo);
}

HRESULT CBow::Ready_Components()
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
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_AnimModel"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Dissolve */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_Dissolve_Weapon"), TEXT("Com_Dissolve"), (CComponent**)&m_pDissolve)))
		return E_FAIL;

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Weapon_Bow"), TEXT("Com_Model"), (CComponent**)&m_pAnimModelCom)))
		return E_FAIL;



	/* For.Com_OBB */
	CCollider::COLLIDERDESC		ColliderDesc;
	ZeroMemory(&ColliderDesc, sizeof(CCollider::COLLIDERDESC));

	ColliderDesc.vSize = _float3(50.0f, 10.f, 5.f);
	ColliderDesc.vCenter = _float3(25.f, ColliderDesc.vSize.y * 0.5f - 5.f, 0.f);
	ColliderDesc.vRotation = _float3(0.f, XMConvertToRadians(0.f), 0.f);
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Collider_OBB"), TEXT("Com_OBB"), (CComponent**)&m_pColliderCom, &ColliderDesc)))
		return E_FAIL;

	return S_OK;
}

CBow * CBow::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CBow*		pInstance = new CBow(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CBow"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CBow::Clone(void * pArg)
{
	CBow*		pInstance = new CBow(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CMonster"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBow::Free()
{
	__super::Free();
	Safe_Release(m_pDissolve);
}
