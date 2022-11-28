#include "stdafx.h"
#include "Wind.h"
#include "GameInstance.h"

#include "Effect_Mgr.h"

CWind::CWind(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CEffect(pDevice, pContext)
{
}

CWind::CWind(const CWind & rhs)
	: CEffect(rhs)
{
}

CWind::~CWind()
{
}

HRESULT CWind::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CWind::Initialize(void * pArg)
{

	if (pArg == nullptr)
	{
		return E_FAIL;
	}

	m_tWind = *(WIND_DESC*)pArg;

	if (m_bDead == false)
	{
		if (FAILED(Ready_Components()))
			return E_FAIL;
	}

	m_bDead = false;
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMLoadFloat4(&m_tWind.TargetPos));
	AUTOINSTANCE(CGameInstance, _pGame);
	_float _fRand = _pGame->Rand_Float(0.03f, 0.08f);
	_float _fRandY = _pGame->Rand_Float(0.01f, 0.03f);

	m_pTransformCom->Set_Scale(XMVectorSet(_fRand, _fRandY, _fRand, 0.f));
	m_pTransformCom->Rotation(XMVectorSet(1.f, 0.f, 0.f, 0.f), XMConvertToRadians(180.f));
	m_pTransformCom->Turn_Angle(XMVectorSet(0.f,1.f,0.f,0.f), XMConvertToRadians(m_tWind.fAngle));
	m_iPass = 7;
	return S_OK;
}

const _bool& CWind::Update(_float fTimeDelta)
{
	if (m_bDead)
	{
		m_fCurTime = 0.f;
		m_fEffectTime = 0.f;
		m_fSpeed = 1.f;
		m_iPass = 7;
		return false;
	}
	m_fSpeed *= 1.03f;
	m_pTransformCom->Turn_Angle(XMVectorSet(0.f, 1.f, 0.f, 0.f), -m_fSpeed * 3.f * fTimeDelta);



	if (m_bLast) // �̰� �m ������
	{
		m_fCurTime += fTimeDelta;
		m_fEffectTime = (m_fCurTime) * 1.1f;
		_float3 _vScale = m_pTransformCom->Get_Scale();
		_vScale.x *= 1.05f;
		_vScale.y *= 1.01f;
		_vScale.z *= 1.05f;
		m_pTransformCom->Set_Scale(XMLoadFloat3(&_vScale));
		
		m_iPass = 6;
	}
	else
	{
		AUTOINSTANCE(CGameInstance, _pGame);
		_float _fRand = _pGame->Rand_Float(-0.003f, 0.003f);
		_float _fRandY = _pGame->Rand_Float(-0.001f, 0.001f);
		_float3 _vScale = m_pTransformCom->Get_Scale();
		_vScale.x += _fRand;
		//_vScale.y += _fRandY;
		_vScale.z += _fRand;
		m_pTransformCom->Set_Scale(XMLoadFloat3(&_vScale));
	}
	
	Compute_CamZ(m_pTransformCom->Get_State(CTransform::STATE_POSITION));
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_ALPHABLEND, this);

	return true;
}

HRESULT CWind::Render()
{
	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);
	if (FAILED(m_pShaderCom->Set_RawValue("g_fColor", &CLIENT_RGB(255.f, 255.f, 255.f), sizeof(_float4))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4))))
		return E_FAIL;

	if (m_iPass == 6)
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

void CWind::Setting(_vector _vPos, _bool _bLast)
{
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vPos);
	m_bLast = _bLast;
}

HRESULT CWind::Ready_Components()
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
	if (FAILED(__super::Add_Component(LEVEL_STAGE_LOBBY, TEXT("Prototype_Component_Model_Effect_Wind"), TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

CWind * CWind::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CWind*		pInstance = new CWind(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CWind"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CWind::Clone(void * pArg)
{
	CWind*		pInstance = new CWind(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CMonster"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CWind::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
}