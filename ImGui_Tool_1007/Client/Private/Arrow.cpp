#include "stdafx.h"
#include "Arrow.h"
#include "GameInstance.h"

#include "Status.h"
#include "Effect_Mgr.h"
#include "ArrowTwist.h"

CArrow::CArrow(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CEffect(pDevice, pContext)
{
}

CArrow::CArrow(const CArrow & rhs)
	: CEffect(rhs)
{
}

HRESULT CArrow::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CArrow::Initialize(void * pArg)
{
	if (!m_bDead)
	{
		if (FAILED(Ready_Components()))
			return E_FAIL;

	}

	if (!pArg)
	{
		return E_FAIL;
	}

	m_tInfo = *(ARROW*)pArg;

	m_tInfo.StartMatrix;

	_matrix _Start;

	_Start = XMLoadFloat4x4(&m_tInfo.StartMatrix);
	_Start.r[2] = XMVector3Normalize( _Start.r[1] * -1.f);

	_Start.r[0] = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), _Start.r[2]);

	_Start.r[1] = XMVectorSet(0.f, 1.f, 0.f, 0.f);

	XMStoreFloat4x4(&m_tInfo.StartMatrix, _Start);
	m_pTransCom->Set_WorldFloat4x4(m_tInfo.StartMatrix);

	XMStoreFloat4(&m_vStartPos, _Start.r[3]);

	m_vPos.clear();

	_float3 _fvPos;
	//화살
	XMStoreFloat3(&_fvPos, m_pTransCom->Get_State(CTransform::STATE_POSITION) - m_pTransCom->Get_State(CTransform::STATE_LOOK));
	m_vPos.push_back(_fvPos);
	XMStoreFloat3(&_fvPos, m_pTransCom->Get_State(CTransform::STATE_POSITION) + m_pTransCom->Get_State(CTransform::STATE_LOOK));
	m_vPos.push_back(_fvPos);
	//꼬리
	//XMStoreFloat3(&_fvPos, m_pTransCom->Get_State(CTransform::STATE_POSITION) - m_pTransCom->Get_State(CTransform::STATE_LOOK));
	//m_vPos.push_back(_fvPos);
	//m_vPos.push_back(_fvPos);

	m_pArrowTwist.push_back(static_cast<CArrowTwist*>(CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_TWIST, &m_pTransCom->Get_WorldMatrix())));
	m_pTransCom->Turn_Angle(m_pTransCom->Get_State(CTransform::STATE_LOOK), XMConvertToRadians(90.f));
	m_pArrowTwist.push_back(static_cast<CArrowTwist*>(CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_TWIST, &m_pTransCom->Get_WorldMatrix())));
	m_pTransCom->Turn_Angle(m_pTransCom->Get_State(CTransform::STATE_LOOK), XMConvertToRadians(90.f));
	m_pArrowTwist.push_back(static_cast<CArrowTwist*>(CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_TWIST, &m_pTransCom->Get_WorldMatrix())));
	m_pTransCom->Turn_Angle(m_pTransCom->Get_State(CTransform::STATE_LOOK), XMConvertToRadians(90.f));
	m_pArrowTwist.push_back(static_cast<CArrowTwist*>(CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_TWIST, &m_pTransCom->Get_WorldMatrix())));
	
	/*if (!m_pArrowTwist)
		return E_FAIL;*/
	for (auto& iter : m_pArrowTwist)
	{
		Safe_AddRef(iter);
	}
	m_bDead = false;
	m_fCurTime = 0.f;
	m_fTick = 0.f;


	//포스 두개 만들고...


	return S_OK;
}

void CArrow::Tick(_float fTimeDelta)
{
}

void CArrow::LateTick(_float fTimeDelta)
{

}

const _bool & CArrow::Update(_float _fTimeDelta)
{
	if (m_bDead)
	{
		m_vPos.clear();
		
		return false;
	}
		

	AUTOINSTANCE(CGameInstance, _pGameInstance);

	m_fCurTime += _fTimeDelta/* / _pGameInstance->Get_TimeSpeed(TEXT("Timer_Main"))*/;
	m_fTick += _fTimeDelta / _pGameInstance->Get_TimeSpeed(TEXT("Timer_Main"));
	_vector _vPos = m_pTransCom->Get_State(CTransform::STATE_POSITION);
	_float	_fDis = fabs(XMVector3Length(_vPos - XMLoadFloat4(&m_vStartPos)).m128_f32[0]);
	if (_fDis < 130.f)
	{		
		m_pTransCom->Go_Straight(m_fSpeed * _fTimeDelta /*/ _pGameInstance->Get_TimeSpeed(TEXT("Timer_Main"))*/);
		XMStoreFloat3(&m_vPos[0], XMLoadFloat3(&m_vPos[0]) + m_fSpeed *
			( m_pTransCom->Get_State(CTransform::STATE_LOOK)) * _fTimeDelta /*/ _pGameInstance->Get_TimeSpeed(TEXT("Timer_Main"))*/ );
		
		_vector _vCurPos = m_pTransCom->Get_State(CTransform::STATE_POSITION);
		//if (_fDis > 7.f)
		//{
		//	m_pArrowTwist.push_back(static_cast<CArrowTwist*>(CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_TWIST, &m_pTransCom->Get_WorldMatrix())));
		//	Safe_AddRef(m_pArrowTwist.back());
		//	//m_pTransCom->Turn_Angle(m_pTransCom->Get_State(CTransform::STATE_LOOK), XMConvertToRadians(90.f));
		//}

		for (auto& iter : m_pArrowTwist)
		{
			iter->Move_Effect(m_pTransCom->Get_State(CTransform::STATE_POSITION));
		}
	}
	
	if (m_fCurTime > m_fMaxTime)
	{
		XMStoreFloat3(&m_vPos[1], XMLoadFloat3(&m_vPos[1]) +  10.f * _fTimeDelta * m_pTransCom->Get_State(CTransform::STATE_LOOK));
		//m_fCurTime = 0.f;
	}

	if (m_fTick > 4.f)
	{
		m_bDead = true;
		for (auto& iter : m_pArrowTwist)
		{
			Safe_Release(iter);
		}
		m_pArrowTwist.clear();
	}

	Compute_CamZ(m_pTransCom->Get_State(CTransform::STATE_POSITION));
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_ALPHABLEND, this) ;

	return true;
}

HRESULT CArrow::Render()
{
	if (nullptr == m_pVIBufferCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	/*if (m_vPos.size() < 2)
		return S_OK;*/

	AUTOINSTANCE(CGameInstance, pGameInstance);

	{
	//up은 카메라를 보는 방향 넣고, 그리고 룩은 고정이니까 그걸로 넣어서 right구하고, 그걸로 제대로 된 up 구한다.
		_vector _vUp = XMLoadFloat4(&pGameInstance->Get_CamPosition()) - m_pTransCom->Get_State(CTransform::STATE_POSITION);

		_vector _vRight = XMVector3Cross(_vUp, m_pTransCom->Get_State(CTransform::STATE_LOOK));
		_vUp = XMVector3Cross(m_pTransCom->Get_State(CTransform::STATE_LOOK), _vRight);

		m_pTransCom->Set_State(CTransform::STATE_RIGHT, _vRight);
		m_pTransCom->Set_State(CTransform::STATE_UP, _vUp);
		m_pTransCom->Set_Scale(XMVectorSet(1.f, 1.f, 1.f, 0.f));
	}

	_float		_fWidth = 0.3f;

	m_pShaderCom->Set_RawValue("g_Width", &_fWidth, sizeof(_float));
	m_pShaderCom->Set_RawValue("g_fTime", &m_fTick, sizeof(_float));
	if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4))))
		return E_FAIL;
	m_pShaderCom->Set_RawValue("g_Color", &(CLIENT_RGB(119.f, 245.f, 100.f)), sizeof(_float4));
	//m_pTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture");

	
	
	{
		_float3 _vRight, _vUp, _vLook;

		XMStoreFloat3(&_vRight, m_pTransCom->Get_State(CTransform::STATE_RIGHT));
		XMStoreFloat3(&_vUp, m_pTransCom->Get_State(CTransform::STATE_UP));

		m_pShaderCom->Set_RawValue("g_Right", &(_vRight), sizeof(_float3));
		m_pShaderCom->Set_RawValue("g_Up", &(_vUp), sizeof(_float3));
		_fWidth = 1.5f;

		m_pArrowTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture");
		_float3 vPos[2];
		vPos[0] = m_vPos[1];
		vPos[1] = m_vPos[0];

		m_pShaderCom->Set_RawValue("g_vPos_1", &(vPos[0]), sizeof(_float3));
		m_pShaderCom->Set_RawValue("g_vPos_2", &(vPos[1]), sizeof(_float3));

		if (FAILED(m_pShaderCom->Begin(6)))
			return E_FAIL;

		if (FAILED(m_pVIBufferCom->Render()))
			return E_FAIL;

		/*vPos[0] = m_vPos[3];
		vPos[1] = m_vPos[2];
		_fWidth = 3.f;
		m_pShaderCom->Set_RawValue("g_vPos_1", &(vPos[0]), sizeof(_float3));
		m_pShaderCom->Set_RawValue("g_vPos_2", &(vPos[1]), sizeof(_float3));
		m_pTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture");
		if (FAILED(m_pShaderCom->Begin(6)))
			return E_FAIL;

		if (FAILED(m_pVIBufferCom->Render()))
			return E_FAIL;*/
	}

	
	


	
	/*
	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &XMMatrixTranspose(XMLoadFloat4x4(&m_tInfo.StartMatrix)), sizeof(_float4x4))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Set_RawValue("g_vCamPosition", &pGameInstance->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;

	_float2 vSize;
	vSize.x = 5.f + m_fTick * 10.f;
	vSize.y = 0.3f + m_fTick * 5.f;
	m_pShaderCom->Set_RawValue("g_Size", &vSize, sizeof(_float2));

	m_pFlareTextureCom->Set_SRV(m_pShaderCom, "g_DiffuseTexture");
	if (FAILED(m_pShaderCom->Begin(7)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;*/

	return S_OK;
}

CArrow * CArrow::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CArrow*		pInstance = new CArrow(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CUI_Plus"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CArrow::Clone(void * pArg)
{
	CGameObject*		pInstance = new CArrow(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CUI_Plus"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CArrow::Free()
{
	__super::Free();

	Safe_Release(m_pRendererCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pTransCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pFlareTextureCom);
	Safe_Release(m_pArrowTextureCom);
	for (auto& iter : m_pArrowTwist)
	{
		Safe_Release(iter);
	}
	m_pArrowTwist.clear();
}



void CArrow::Add_Point(_float3 _vPos)
{
	_float3 _vRight, _vUp, _vLook;
	if (m_vPos.size() == 0)
	{
		m_pTransCom->Set_State(CTransform::STATE_POSITION, XMLoadFloat3(&_vPos));
		//XMStoreFloat3(&_vRight, XMVectorSet(1.f,0.f,0.f,0.f));
		//XMStoreFloat3(&_vUp, XMVectorSet(0.f, 1.f, 0.f, 0.f));
	}
	else
	{
		m_pTransCom->Set_State(CTransform::STATE_POSITION, XMLoadFloat3(&_vPos));
		//m_pTransCom->LookAt_ForLandObject(XMLoadFloat3(&(m_vPos.back())));
		//XMStoreFloat3(&_vRight, m_pTransCom->Get_State(CTransform::STATE_RIGHT));
		//XMStoreFloat3(&_vUp, m_pTransCom->Get_State(CTransform::STATE_UP));
		//m_pTransCom->Set_Scale(XMVectorSet( 1.f, 1.f, 1.f, 0.f));
		////XMStoreFloat3(&_vRight, XMVectorSet(1.f, 0.f, 0.f, 0.f));
		////XMStoreFloat3(&_vUp, XMVectorSet(0.f, 1.f, 0.f, 0.f));
		////XMStoreFloat3(&_vLook, XMVectorSet(0.f, 0.f, 1.f, 0.f));
	}
	m_vPos.push_back(_vPos);
	//m_vRight.push_back(_vRight);
	//m_vUp.push_back(_vUp);
	//m_vLook.push_back(_vLook);
}

HRESULT CArrow::Ready_Components()
{
	/* For.Com_Transform */
	CTransform::TRANSFORMDESC	_Desc;
	_Desc.fRotationPerSec = XMConvertToRadians(90.f);
	//_Desc.fSpeedPerSec = 1.5f;
	_Desc.fSpeedPerSec = 1.f;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform"), (CComponent**)&m_pTransCom, &_Desc)))
		return E_FAIL;

	m_pTransCom->Set_Scale(XMVectorSet(1.f, 1.f, 1.f, 0.f));
	/* For.Com_Renderer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_Point"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Point"), TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_Mask_Electronic"), TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	/* For.Com_FlareTexture */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_Mask_Flare"), TEXT("Com_FlareTexture"), (CComponent**)&m_pFlareTextureCom)))
		return E_FAIL;

	/* For.Com_ArrowTexture */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_Mask_Arrow"), TEXT("Com_ArrowTexture"), (CComponent**)&m_pArrowTextureCom)))
		return E_FAIL;

	return S_OK;
}
