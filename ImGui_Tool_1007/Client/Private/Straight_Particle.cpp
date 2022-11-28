#include "stdafx.h"
#include "..\Public\Straight_Particle.h"
#include "GameInstance.h"

CStraight_Particle::CStraight_Particle(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CEffect_Particle(pDevice, pContext)
{
}

CStraight_Particle::CStraight_Particle(const CEffect_Particle & rhs)
	: CEffect_Particle(rhs)
{
}


HRESULT CStraight_Particle::Initialize_Prototype(_tchar * szTextureTag)
{
	return S_OK;
}

HRESULT CStraight_Particle::Initialize(void * pArg)
{

	return S_OK;
}


const _bool & CStraight_Particle::Update(_float _fTimeDelta)
{	
	if (m_bRecycle)
	{
		Recycle();
	}

	if (m_bDead)
	{
		if (m_tOption.eDissapear == DISSAPEAR_ALPHA)
		{
			m_fTime += _fTimeDelta;
			if (m_fTime > 1.f)
			{
				m_fTime = 0.f;
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	_vector _vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	_float	_vDis = fabs(XMVector3Length((XMVectorSetW(XMLoadFloat3(&m_vOriginPos), 1.f) - _vPos)).m128_f32[0]);
	if (m_fDistane < _vDis)
	{
		m_bDead = true;

		//return false;
	}



	if (MoveFrame(_fTimeDelta))
	{
		if (m_pTextureCom->Get_Type() == CTexture::TYPE_MULTI)
		{
			m_bDead = true;
			//return false;
		}
	}

	


	m_fCurSpeed *= m_tOption.fAccSpeed;
	_vPos += XMLoadFloat3(&m_vDirect) * _fTimeDelta * m_fCurSpeed;

	

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vPos);

	Compute_CamZ(m_pTransformCom->Get_State(CTransform::STATE_POSITION));

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_ALPHABLEND, this);

	return true;
}

HRESULT CStraight_Particle::Render()
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

	if (FAILED(m_pShaderCom->Set_RawValue("g_Size", &(m_tOption.Size), sizeof(_float2))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Set_RawValue("g_fTime", &m_fTime, sizeof(_float))))
		return E_FAIL;

	
	if (m_pMaskTextureCom)
	{
		if (FAILED(m_pMaskTextureCom->Set_SRV(m_pShaderCom, "g_NoiseTexture")))
			return E_FAIL;
	}
	if (m_tOption.eDiffuseType == DIFFUSE_TEXTURE)
	{
		if (FAILED(m_pTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture")))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(2)))
			return E_FAIL;
	}
	else
	{
		if (FAILED(m_pShaderCom->Set_RawValue("g_Color", &(m_tOption.vColor), sizeof(_float4))))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(3)))
			return E_FAIL;
	}
	

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

void CStraight_Particle::Init_Again(OPTION _tOption, CTexture* _pTextureCom, CTexture* _pMaskCom, CVIBuffer_Point* _pBufferCom)
{
	AUTOINSTANCE(CGameInstance, _pInstance);

	m_tOption = _tOption;

	if (!m_bDead)
	{
		//이때만 새로 생성된 경우임!
		//컴포넌트 생성 해줌
		if (FAILED(Ready_Components()))
			return;
	}
	m_bDead = false;

	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pMaskTextureCom);

	m_pVIBufferCom = _pBufferCom;
	m_pTextureCom = _pTextureCom;
	m_pMaskTextureCom = _pMaskCom;
	Safe_AddRef(m_pVIBufferCom);
	Safe_AddRef(m_pTextureCom);
	Safe_AddRef(m_pMaskTextureCom);

	Re_Option();

}

void CStraight_Particle::Recycle()
{
	Re_Option();
	m_bRecycle = false;
	
}

void CStraight_Particle::Re_Option()
{
	AUTOINSTANCE(CGameInstance, _pInstance);
	_vector _vPos;
	switch (m_tOption.eStartType)
	{
	case STARTTYPE::START_RANDOM:		
		_vPos.m128_f32[0] = _pInstance->Rand_Float(m_tOption.fMinMaxX.x, m_tOption.fMinMaxX.y);
		_vPos.m128_f32[1] = _pInstance->Rand_Float(m_tOption.fMinMaxY.x, m_tOption.fMinMaxY.y);
		_vPos.m128_f32[2] = _pInstance->Rand_Float(m_tOption.fMinMaxZ.x, m_tOption.fMinMaxZ.y);
		_vPos.m128_f32[3] = 1.f;
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vPos);

		m_fDistane = fabs((XMVectorSetW(XMLoadFloat3(&m_tOption.Center), 1.f) - _vPos).m128_f32[0]);
		XMStoreFloat3(&m_vOriginPos, _vPos);
		break;
	case STARTTYPE::START_CENTER:
		_vPos = XMLoadFloat3(&m_tOption.Center);
		_vPos.m128_f32[3] = 1.f;
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vPos);

		m_fDistane = _pInstance->Rand_Float(m_tOption.fMaxDistance.x, m_tOption.fMaxDistance.y);
		XMStoreFloat3(&m_vOriginPos, _vPos);
		break;
	}	

	

	switch (m_tOption.eDirType)
	{
	case DIR_NORMAL:
	{
		switch (m_tOption.Spread)
		{
		case SPREAD_CENTER:
			XMStoreFloat3(&m_vDirect, XMLoadFloat3(&m_tOption.Center) - _vPos);
			break;
		case SPREAD_EDGE:
			XMStoreFloat3(&m_vDirect, (XMLoadFloat3(&m_tOption.Center) - _vPos) * -1.f);
			break;
		}
	}
		break;
	case DIR_ANGLE:
	{
		if (m_tOption.bPlayerDir)
		{
			_matrix _matTarget;


			m_tOption.matPlayerAxix._41 = 0.f;
			m_tOption.matPlayerAxix._42 = 0.f;
			m_tOption.matPlayerAxix._43 = 0.f;
			m_tOption.matPlayerAxix._44 = 1.f;

			_matTarget = XMLoadFloat4x4(&m_tOption.matPlayerAxix);

			_matTarget.r[0] = XMVector3Normalize(_matTarget.r[0]);
			_matTarget.r[1] = XMVector3Normalize(_matTarget.r[1]);
			_matTarget.r[2] = XMVector3Normalize(_matTarget.r[2]);

			XMStoreFloat3(&m_vDirect, XMVector3TransformNormal(XMLoadFloat3(&m_tOption.vStart_Dir), _matTarget));
			//랜덤으로 방향을 지정-> -들어온 각도 ~ 들어온 각도
			_float fAngle_X = _pInstance->Rand_Float(-1.f * m_tOption.fSpead_Angle.x, m_tOption.fSpead_Angle.x);
			_float fAngle_Y = _pInstance->Rand_Float(-1.f * m_tOption.fSpead_Angle.y, m_tOption.fSpead_Angle.y);
			_float fAngle_Z = _pInstance->Rand_Float(-1.f * m_tOption.fSpead_Angle.z, m_tOption.fSpead_Angle.z);


			_matrix		RotationMatrix = XMMatrixRotationAxis(XMVectorSet(0.f, 0.f, 1.f, 0.f), XMConvertToRadians(fAngle_Z))
										* XMMatrixRotationAxis(XMVectorSet(1.f, 0.f, 0.f, 0.f), XMConvertToRadians(fAngle_X))
										* XMMatrixRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(fAngle_Y));


			XMStoreFloat3(&m_vDirect, XMVector3TransformNormal(XMLoadFloat3(&m_vDirect), RotationMatrix));
		}
		else
		{
			_float fAngle_X = _pInstance->Rand_Float(-1.f * m_tOption.fSpead_Angle.x, m_tOption.fSpead_Angle.x);
			_float fAngle_Y = _pInstance->Rand_Float(-1.f * m_tOption.fSpead_Angle.y, m_tOption.fSpead_Angle.y);
			_float fAngle_Z = _pInstance->Rand_Float(-1.f * m_tOption.fSpead_Angle.z, m_tOption.fSpead_Angle.z);


			_matrix		RotationMatrix = XMMatrixRotationAxis(XMVectorSet(0.f, 0.f, 1.f, 0.f), XMConvertToRadians(fAngle_Z))
				* XMMatrixRotationAxis(XMVectorSet(1.f, 0.f, 0.f, 0.f), XMConvertToRadians(fAngle_X))
				* XMMatrixRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(fAngle_Y));

			XMStoreFloat3(&m_vDirect, XMVector3TransformNormal(XMLoadFloat3(&m_tOption.vStart_Dir), RotationMatrix));
		}
	}
		break;
	
	}
	
	m_fCurSpeed = _pInstance->Rand_Float(m_tOption.fSpeed.x, m_tOption.fSpeed.y);
}

HRESULT CStraight_Particle::Ready_Components()
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

	return S_OK;
}

CEffect_Particle * CStraight_Particle::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, _tchar * szTextureTag)
{
	CStraight_Particle*		pInstance = new CStraight_Particle(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(szTextureTag)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CStraight_Particle"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CStraight_Particle::Clone(void * pArg)
{
	CStraight_Particle*		pInstance = new CStraight_Particle(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CStraight_Particle"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CStraight_Particle::Free()
{
	__super::Free();
}