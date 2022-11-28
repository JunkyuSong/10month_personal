#include "stdafx.h"
#include "..\Public\Fire.h"
#include "GameInstance.h"

CFire::CFire(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CEffect(pDevice, pContext)
{
}

CFire::CFire(const CEffect_Particle & rhs)
	: CEffect(rhs)
{
}


HRESULT CFire::Initialize_Prototype(_tchar * szTextureTag)
{
	return S_OK;
}

HRESULT CFire::Initialize(void * pArg)
{
	if (pArg == nullptr)
		return E_FAIL;
	AUTOINSTANCE(CGameInstance, _pInstance);
	if (!m_bDead)
	{
		if (FAILED(Ready_Components()))
			return E_FAIL;
	}

	



	m_bDead = false;
	m_fCurSpeed = 3.f;
	FIRE_DESC	_tFire = *(FIRE_DESC*)pArg;
	m_vColor = _tFire.vColor;
	m_vDirect = _tFire.vDirect;
	m_vSize = _tFire.vSize;
	m_iCurChance = _tFire.iChance;
	m_fLifeTime = _tFire.fLifeTime;
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMLoadFloat4(&_tFire.vPos));

	AUTOINSTANCE(CGameInstance, _pGaemInstance);
	m_fAccSpeed = _pGaemInstance->Rand_Float(0.99f, 1.07f);

	return S_OK;
}


const _bool & CFire::Update(_float _fTimeDelta)
{	
	AUTOINSTANCE(CGameInstance, _pGame);
	if (m_bDead)
	{
		m_iSpriteNum = 0;
		m_fSpriteTime = 0;
		m_fTime = 0.f;
		return false;
	}
	_vector _vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	



	if (MoveFrame(_fTimeDelta))
	{
		--m_iCurChance;
		if (m_iCurChance == 0)
			m_bDead = true;
	}

	m_fTime += _fTimeDelta / _pGame->Get_TimeSpeed(TEXT("Timer_Main"));
	if (m_fTime > m_fLifeTime)
	{
		m_bDead = true;
	}


	/*m_fCurSpeed *= m_fAccSpeed;
	_vPos += XMLoadFloat3(&m_vDirect) * _fTimeDelta * m_fCurSpeed;*/

	

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vPos);

	Compute_CamZ(m_pTransformCom->Get_State(CTransform::STATE_POSITION));

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_ALPHABLEND, this);

	return true;
}

HRESULT CFire::Render()
{
	if (nullptr == m_pVIBufferCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	AUTOINSTANCE(CGameInstance, pGameInstance);


	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Set_RawValue("g_vCamPosition", &pGameInstance->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;


	_uint _iMaxY, _iMaxX;

	m_pTextureCom->Get_HeightWidth(_iMaxY, _iMaxX);

	m_iCurSpriteY = m_iSpriteNum / _iMaxY;
	m_iCurSpriteX = m_iSpriteNum % _iMaxY;

	if (FAILED(m_pShaderCom->Set_RawValue("g_iMaxY", &_iMaxY, sizeof(_uint))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Set_RawValue("g_iMaxX", &_iMaxX, sizeof(_uint))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Set_RawValue("g_iCurY", &m_iCurSpriteY, sizeof(_uint))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Set_RawValue("g_iCurX", &m_iCurSpriteX, sizeof(_uint))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Set_RawValue("g_Size", &m_vSize, sizeof(_float2))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Set_RawValue("g_Color", &m_vColor, sizeof(_float4))))
		return E_FAIL;

	
	
	if (FAILED(m_pTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(5)))
		return E_FAIL;
	

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}
_bool CFire::MoveFrame(_float _fDeltaTime)
{
	m_fSpriteTime += _fDeltaTime;

	if (m_fSpriteTime > m_pTextureCom->Get_Speed())
	{
		m_fSpriteTime -= m_pTextureCom->Get_Speed();
		++m_iSpriteNum;
	}

	if (m_iSpriteNum >= m_pTextureCom->Get_MaxTexture())
	{
		m_iSpriteNum = 0;
		return true;
	}

	return false;
}
HRESULT CFire::Ready_Components()
{
	/* For.Com_Transform */
	CTransform::TRANSFORMDESC	_Desc;
	_Desc.fRotationPerSec = XMConvertToRadians(90.f);
	_Desc.fSpeedPerSec = 1.f;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform"), (CComponent**)&m_pTransformCom, &_Desc)))
		return E_FAIL;

	/* For.Com_Renderer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_Point"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_Fire"), TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Point"), TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

CEffect * CFire::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, _tchar * szTextureTag)
{
	CFire*		pInstance = new CFire(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(szTextureTag)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CFire"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CFire::Clone(void * pArg)
{
	CFire*		pInstance = new CFire(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CFire"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFire::Free()
{
	__super::Free();
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);
	
}