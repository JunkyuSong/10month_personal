#include "stdafx.h"
#include "..\Public\DisappearWeapon.h"
#include "GameInstance.h"
#include "Effect_Mgr.h"


CDisappearWeapon::CDisappearWeapon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffect(pDevice, pContext)
{
}

CDisappearWeapon::CDisappearWeapon(const CDisappearWeapon & rhs)
	: CEffect(rhs)
{
}

CDisappearWeapon * CDisappearWeapon::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CDisappearWeapon*			pInstance = new CDisappearWeapon(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CDisappearWeapon"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CDisappearWeapon::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
}

CGameObject * CDisappearWeapon::Clone(void * pArg)
{
	CDisappearWeapon*			pInstance = new CDisappearWeapon(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CDisappearWeapon"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

const _bool & CDisappearWeapon::Update(_float _fTimeDelta)
{
	if (m_bDead)
	{
		Safe_Release(m_pModelCom);
		m_fTick = 0.f;
		return false;
	}

	
	AUTOINSTANCE(CGameInstance, _pGame);
	m_fTick += _fTimeDelta / _pGame->Get_TimeSpeed(TEXT("Timer_Main")) * 0.5f;
	if (m_tDW.bBow)
	{


		_vector Center = (XMLoadFloat3(&m_tDW.HighAndLow.vHigh) + XMLoadFloat3(&m_tDW.HighAndLow.vLow)) * 0.5f;

		XMStoreFloat3(&m_tOption.Center,
			XMVectorSetW(XMVectorLerp(Center, XMLoadFloat3(&m_tDW.HighAndLow.vLow), m_fTick * m_tDW.fSpeed), 1.f));

		m_tOption.Size = _float2(_pGame->Rand_Float(0.03f, 0.13f), _pGame->Rand_Float(0.03f, 0.13f));
		if (nullptr == CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_PARTICLE, &m_tOption))
		{
			MSG_BOX(TEXT("fail effect"));
			return false;
		}

		XMStoreFloat3(&m_tOption.Center,
			XMVectorSetW(XMVectorLerp(Center, XMLoadFloat3(&m_tDW.HighAndLow.vHigh), m_fTick * m_tDW.fSpeed), 1.f));

		m_tOption.Size = _float2(_pGame->Rand_Float(0.03f, 0.13f), _pGame->Rand_Float(0.03f, 0.13f));
		if (nullptr == CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_PARTICLE, &m_tOption))
		{
			MSG_BOX(TEXT("fail effect"));
			return false;
		}
	}
	else
	{
	

		XMStoreFloat3(&m_tOption.Center,
			XMVectorSetW(XMVectorLerp(XMLoadFloat3(&m_tDW.HighAndLow.vHigh), XMLoadFloat3(&m_tDW.HighAndLow.vLow), m_fTick * m_tDW.fSpeed), 1.f));

		m_tOption.Size = _float2(_pGame->Rand_Float(0.03f, 0.13f), _pGame->Rand_Float(0.03f, 0.13f));
		if (nullptr == CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_PARTICLE, &m_tOption))
		{
			MSG_BOX(TEXT("fail effect"));
			return false;
		}
	}
	

	if (m_fTick > 1.f)
		m_bDead = true;

	Compute_CamZ(m_pTransformCom->Get_State(CTransform::STATE_POSITION));

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_ALPHABLEND, this);

	return true;
}


HRESULT CDisappearWeapon::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CDisappearWeapon::Initialize(void * pArg)
{
	if (!m_bDead)
	{
		if (FAILED(Ready_Components()))
			return E_FAIL;
	}
	m_bDead = false;
	m_tDW = *(DISAPPEARWEAPON*)pArg;
	m_pTransformCom->Set_WorldFloat4x4(m_tDW.World);

	
	if (m_tDW.bBow)
	{
		m_pModelCom = static_cast<CNonAnimModel*>( CGameInstance::Get_Instance()->Clone_Component(LEVEL_STAGE_LOBBY, TEXT("Prototype_Component_Model_Weapon_StaticBow")));
		m_pTransformCom->Set_Scale(XMVectorSet(0.01f, 0.01f, 0.01f, 0.f));
		//m_pTransformCom->Turn_Angle(m_pTransformCom->Get_State(CTransform::STATE_RIGHT), XMConvertToRadians(-90.0f));
		m_pTransformCom->Turn_Angle(m_pTransformCom->Get_State(CTransform::STATE_UP), XMConvertToRadians(180.0f));
	}
	else
	{
		m_pModelCom = m_tDW.pModel;
		Safe_AddRef(m_tDW.pModel);
	}



	m_tOption.eType = CEffect_Particle::PARTICLETYPE::TYPE_STRIGHT;
	m_tOption.fAccSpeed = 0.99f;
	m_tOption.fSpeed = { 0.5f, 1.3f };
	m_tOption.fGravity = 0.f;
	m_tOption.fLifeTime = 0.5f;
	m_tOption.fRange = _float3(5.f, 5.f, 1.f);
	m_tOption.iNumParticles = 3;
	m_tOption.Size = _float2(0.1f, 0.1f);
	m_tOption.Spread = CEffect_Particle::SPREAD::SPREAD_EDGE;
	m_tOption.szMaskTag = TEXT("Prototype_Component_Texture_Mask_Blood");
	m_tOption.szTextureTag = TEXT("Prototype_Component_Texture_Diffuse_Blood");
	m_tOption.vColor = CLIENT_RGB(119.f, 245.f, 200.f);
	m_tOption.fSpead_Angle = _float3(0.f, 50.f, 50.f);
	m_tOption.vStart_Dir = _float3(0.f, 1.f, 0.f);
	m_tOption.eDiffuseType = CEffect_Particle::DIFFUSE_COLOR;
	m_tOption.eDirType = CEffect_Particle::DIR_TYPE::DIR_ANGLE;
	m_tOption.eStartType = CEffect_Particle::START_CENTER;
	m_tOption.fMaxDistance = { 0.4f, 1.2f };
	m_tOption.eDissapear = CEffect_Particle::DISSAPEAR_ALPHA;
	

	return S_OK;
}

HRESULT CDisappearWeapon::Render()
{
	if (nullptr == m_pModelCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);

	if (FAILED(m_pShaderCom->Set_RawValue("g_fColor", &CLIENT_RGB(119.f, 245.f, 200.f), sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_fTime", &m_fTick, sizeof(_float))))
		return E_FAIL;

	_float4x4		WorldMatrix;
	
	//XMStoreFloat4x4(&WorldMatrix,);
	
	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4))))
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

		if (FAILED(m_pShaderCom->Begin(m_tDW.iPass)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}


	return S_OK;

}

HRESULT CDisappearWeapon::Ready_Components()
{
	/* For.Com_Transform */
	CTransform::TRANSFORMDESC	_Desc;
	_Desc.fRotationPerSec = XMConvertToRadians(90.f);
	//_Desc.fSpeedPerSec = 1.5f;
	_Desc.fSpeedPerSec = 2.5f;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform"), (CComponent**)&m_pTransformCom, &_Desc)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_Effect"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	return S_OK;
}
