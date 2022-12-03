#include "stdafx.h"
#include "CrossTrail.h"
#include "GameInstance.h"

#include "Status.h"

CCrossTrail::CCrossTrail(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CEffect(pDevice, pContext)
{
}

CCrossTrail::CCrossTrail(const CCrossTrail & rhs)
	: CEffect(rhs)
{
}

HRESULT CCrossTrail::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCrossTrail::Initialize(void * pArg)
{
	if (!m_bDead)
	{
		if (FAILED(Ready_Components()))
			return E_FAIL;

	}

	if (pArg)
	{
		CROSS_DESC _tDesc = *(CROSS_DESC*)pArg;
		m_vColor = _tDesc.vRGBA;
		m_fWidth = _tDesc.fWidth;
		m_bLook = _tDesc.bLook;
		m_fMaxTime = _tDesc.fMaxTime;
		m_bAlpha = _tDesc.bAlpha;
	}
	else
	{
		m_vColor = CLIENT_RGB(255.f, 255.f, 255.f);
		m_fWidth = 0.005f;
	}

	m_bDead = false;
	m_fCurTime = 0.f;
	return S_OK;
}

void CCrossTrail::Tick(_float fTimeDelta)
{
}

void CCrossTrail::LateTick(_float fTimeDelta)
{
	
}

const _bool & CCrossTrail::Update(_float _fTimeDelta)
{
	if (m_bDead)
	{
		return false;
	}
		

	AUTOINSTANCE(CGameInstance, _pGameInstance);

	m_fCurTime += _fTimeDelta / _pGameInstance->Get_TimeSpeed(TEXT("Timer_Main"));
	if (m_fCurTime > m_fMaxTime)
	{
		m_vPos.erase(m_vPos.begin());
		m_vRight.erase(m_vRight.begin());
		m_vUp.erase(m_vUp.begin());
		m_vUV.erase(m_vUV.begin());
	}

	if (m_vPos.size() < 1)
	{
		m_bDead = true;
	}

	if (m_bAlpha)
	{
		for (_uint i = 0; i < m_vUV.size(); ++i)
		{
			_float _iVtxCount = 0.f;
			if (m_vUV.size() < 1)
			{
				_iVtxCount = 1.f;
			}
			else
			{
				_iVtxCount = (_float)m_vUV.size() - 1.f;
			}
			m_vUV[i] = _float(i) / ((_float)_iVtxCount);
		}
	}

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_ALPHABLEND, this);

	return true;
}

HRESULT CCrossTrail::Render()
{
	if (nullptr == m_pVIBufferCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	if (m_vPos.size() < 2)
		return S_OK;
	
	AUTOINSTANCE(CGameInstance, pGameInstance);

	_uint _iPass = 4;
		
	//m_pShaderCom->Set_RawValue("g_WorldMatrix", &XMMatrixTranspose(XMLoadFloat4x4(&m_matLook)), sizeof(_float4x4));
	
	m_pShaderCom->Set_RawValue("g_Width", &m_fWidth, sizeof(_float));

	if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4))))
		return E_FAIL;
	if (m_bLook)
		m_pTransCom->Set_WorldMatrix(XMMatrixIdentity());
	_float vUv[2];
	for (_uint i = 0; i < m_vPos.size() - 2; ++i)
	{
		_float3 vPos[2];
		
		vPos[0] = m_vPos[i];
		vPos[1] = m_vPos[i+1];
		vUv[0] = m_vUV[i];
		vUv[1] = m_vUV[i + 1];
		m_pShaderCom->Set_RawValue("g_Uv1", &(vUv[0]), sizeof(_float));
		m_pShaderCom->Set_RawValue("g_Uv2", &(vUv[1]), sizeof(_float));
		_float3 _vRight, _vUp, _vLook;
		m_pTransCom->Set_State(CTransform::STATE_POSITION, XMLoadFloat3(&vPos[0]));
		if (m_bLook)
		{
			_float3 _vRights[2], _vUps[2];


			_vRights[0] = m_vRight[i];
			_vRights[1] = m_vRight[i+1];
			_vUps[0] = m_vUp[i];
			_vUps[1] = m_vUp[i + 1];

			m_pShaderCom->Set_RawValue("g_Right1", &(_vRights[0]), sizeof(_float3));
			m_pShaderCom->Set_RawValue("g_Up1", &(_vUps[0]), sizeof(_float3));
			m_pShaderCom->Set_RawValue("g_Right2", &(_vRights[1]), sizeof(_float3));
			m_pShaderCom->Set_RawValue("g_Up2", &(_vUps[1]), sizeof(_float3));
			_iPass = 8;
		}			
		else
		{
			if (i != 0)
			{			
				m_pTransCom->LookAt_ForLandObject(XMLoadFloat3(&(vPos[1])));
				XMStoreFloat3(&_vRight, m_pTransCom->Get_State(CTransform::STATE_RIGHT));
				XMStoreFloat3(&_vUp, m_pTransCom->Get_State(CTransform::STATE_UP));
			}
			else
			{
				XMStoreFloat3(&_vRight, XMVectorSet(1.f, 0.f, 0.f, 0.f));
				XMStoreFloat3(&_vUp, XMVectorSet(0.f, 1.f, 0.f, 0.f));
			}
			m_pShaderCom->Set_RawValue("g_Right", &(_vRight), sizeof(_float3));
			m_pShaderCom->Set_RawValue("g_Up", &(_vUp), sizeof(_float3));
		}
	


		//m_pShaderCom->Set_RawValue("g_Look", &(m_vLook[i]), sizeof(_float3));

		m_pShaderCom->Set_RawValue("g_vPos_1", &(vPos[0]), sizeof(_float3));
		m_pShaderCom->Set_RawValue("g_vPos_2", &(vPos[1]), sizeof(_float3));
		m_pShaderCom->Set_RawValue("g_Color", &(m_vColor), sizeof(_float4));

		if (FAILED(m_pShaderCom->Begin(_iPass)))
			return E_FAIL;

		if (FAILED(m_pVIBufferCom->Render()))
			return E_FAIL;
	}

	_float3 vPos[2];
	vPos[0] = m_vPos[m_vPos.size() - 2];
	vPos[1] = m_vPos[m_vPos.size() - 1];

	vUv[0] = m_vUV[m_vPos.size() - 2];
	vUv[1] = m_vUV[m_vPos.size() - 1];

	m_pShaderCom->Set_RawValue("g_Uv1", &(vUv[0]), sizeof(_float));
	m_pShaderCom->Set_RawValue("g_Uv2", &(vUv[1]), sizeof(_float));
	_float3 _vRight, _vUp, _vLook;

	XMStoreFloat3(&_vRight, XMVectorSet(1.f, 0.f, 0.f, 0.f));
	XMStoreFloat3(&_vUp, XMVectorSet(0.f, 1.f, 0.f, 0.f));


	m_pShaderCom->Set_RawValue("g_Right", &(_vRight), sizeof(_float3));
	m_pShaderCom->Set_RawValue("g_Up", &(_vUp), sizeof(_float3));

	m_pShaderCom->Set_RawValue("g_vPos_1", &(vPos[0]), sizeof(_float3));
	m_pShaderCom->Set_RawValue("g_vPos_2", &(vPos[1]), sizeof(_float3));
	m_pShaderCom->Set_RawValue("g_Color", &(m_vColor), sizeof(_float4));

	if (FAILED(m_pShaderCom->Begin(_iPass)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

CCrossTrail * CCrossTrail::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CCrossTrail*		pInstance = new CCrossTrail(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CUI_Plus"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CCrossTrail::Clone(void * pArg)
{
	CGameObject*		pInstance = new CCrossTrail(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CUI_Plus"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCrossTrail::Free()
{
	__super::Free();

	Safe_Release(m_pRendererCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pTransCom);
}



void CCrossTrail::Add_Point(_float3 _vPos)
{
	_float3 _vRight, _vUp, _vLook;
	if (m_vPos.size() == 0)
	{
		m_pTransCom->Set_State(CTransform::STATE_POSITION, XMLoadFloat3(&_vPos));
		XMStoreFloat3(&_vRight, XMVectorSet(1.f,0.f,0.f,0.f));
		XMStoreFloat3(&_vUp, XMVectorSet(0.f, 1.f, 0.f, 0.f));
	}
	else
	{
		m_pTransCom->Set_WorldMatrix(XMMatrixIdentity());
		m_pTransCom->Set_State(CTransform::STATE_POSITION, XMLoadFloat3(&_vPos));
		m_pTransCom->LookAt(XMLoadFloat3(&(m_vPos.back())));
		XMStoreFloat3(&_vRight, m_pTransCom->Get_State(CTransform::STATE_RIGHT));
		XMStoreFloat3(&_vUp, m_pTransCom->Get_State(CTransform::STATE_UP));
	}
	m_vPos.push_back(_vPos);
	m_vRight.push_back(_vRight);
	m_vUp.push_back(_vUp);
	m_vUV.push_back(1.f);
}

HRESULT CCrossTrail::Ready_Components()
{
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform"), (CComponent**)&m_pTransCom)))
		return E_FAIL;

	m_pTransCom->Set_Scale(XMVectorSet( 1.f, 1.f, 1.f, 0.f));
	/* For.Com_Renderer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_Point"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Point"), TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}
