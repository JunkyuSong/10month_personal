#include "..\Public\Camera.h"
#include "PipeLine.h"
#include "Timer_Manager.h"

CCamera::CCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
{
	m_eTypeObj = CGameObject::TYPE_CAMERA;
}

CCamera::CCamera(const CCamera & rhs, CTransform::TRANSFORMDESC* pArg)
	: CGameObject(rhs)
	, m_pTransformCom((CTransform*)rhs.m_pTransformCom->Clone(pArg))
{

}

HRESULT CCamera::Initialize_Prototype()
{
	m_pTransformCom = CTransform::Create(m_pDevice, m_pContext);
	if (nullptr == m_pTransformCom)
		return E_FAIL;

	return S_OK;
}

HRESULT CCamera::Initialize(void * pArg)
{
	memcpy(&m_CameraDesc, pArg, sizeof(CAMERADESC));

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMLoadFloat4(&m_CameraDesc.vEye));
	m_pTransformCom->LookAt(XMLoadFloat4(&m_CameraDesc.vAt));

	return S_OK;
}

void CCamera::Tick(_float fTimeDelta)
{
	fTimeDelta /= CTimer_Manager::Get_Instance()->Get_TimeSpeed(TEXT("Timer_Main"));
	
	if (m_bZoomIn)
	{
		Cam_ZoomIn(fTimeDelta);
	}
	else if (m_bZoomOut)
	{
		Cam_ZoomOut(fTimeDelta);
	}
	else
	{
		if (fabs(m_CameraDesc.fFovy - XMConvertToRadians(60.f)) < XMConvertToRadians(3.f))
		{
			m_CameraDesc.fFovy = XMConvertToRadians(60.f);
		}
		else
		{
			if (m_CameraDesc.fFovy > XMConvertToRadians(60.f))
				m_CameraDesc.fFovy -= XMConvertToRadians(m_fZoomSpeed * fTimeDelta);
			else if (m_CameraDesc.fFovy < XMConvertToRadians(60.f))
				m_CameraDesc.fFovy += XMConvertToRadians(m_fZoomSpeed * fTimeDelta);
		}
	}

 	Cam_Shake(fTimeDelta);

	CPipeLine*			pPipeLine = GET_INSTANCE(CPipeLine);

	pPipeLine->Set_Transform(CPipeLine::D3DTS_VIEW, m_pTransformCom->Get_WorldMatrixInverse());
	pPipeLine->Set_Transform(CPipeLine::D3DTS_PROJ, XMMatrixPerspectiveFovLH(m_CameraDesc.fFovy, m_CameraDesc.fAspect, m_CameraDesc.fNear, m_CameraDesc.fFar));

	RELEASE_INSTANCE(CPipeLine);
}

void CCamera::LateTick(_float fTimeDelta)
{
}

HRESULT CCamera::Render()
{
	return S_OK;
}


void CCamera::ZoomOut(_float fFoV, _float fSpeed)
{
	m_bZoomOut = true;
	m_bZoomIn = false;
	m_fZoomFOV = XMConvertToRadians( fFoV);
	m_fZoomSpeed = fSpeed;
}

void CCamera::ZoomOff(_float fSpeed)
{
	m_bZoomOut = false;
	m_bZoomIn = false;
	m_fZoomSpeed = fSpeed;
}

void CCamera::Set_FOV(_float fFOV)
{
	m_CameraDesc.fFovy = XMConvertToRadians(fFOV);
}

void CCamera::Shake_On(_float fTime, _float _fShakePower)
{
	m_bShake = true;
	m_fShakeTime = fTime;
	//m_fCurShakeTime = 0.f;
	m_fShakePower = m_fShakeOriginPower =_fShakePower;
	/*m_fShakePowerTime = 0.f;
	m_fShakeSpeedTime = 0.f;
	m_iDir = 0;*/
}

void CCamera::Shake_Off()
{
	m_bShake = false;
	m_fShakeTime = 0.f;
	m_fCurShakeTime = 0.f;
	m_fShakePower = m_fShakeOriginPower= 0.f;
	m_fShakePowerTime = 0.f;
	m_fShakeSpeedTime = 0.f;
	m_iDir = 0;
}

void CCamera::ZoomIn(_float fFoV, _float fSpeed, _float fTime)
{
	m_bZoomOut = false;
	m_bZoomIn = true;
	m_fZoomFOV = XMConvertToRadians(fFoV);
	m_fZoomSpeed = fSpeed;
	m_fZoomTime = fTime;
	m_fZoomCurTime = 0.f;
}

void CCamera::Cam_ZoomOut(_float fTimeDelta)
{
	//∞¢µµ∞° ¥√∏È ¡‹æ∆øÙ
	if (fabs(m_CameraDesc.fFovy - m_fZoomFOV) < XMConvertToRadians(3.f))
	{
		m_CameraDesc.fFovy = m_fZoomFOV;
	}
	else
	{
		m_CameraDesc.fFovy += XMConvertToRadians( m_fZoomSpeed * fTimeDelta);
	}

}

void CCamera::Cam_Shake(_float fTimeDelta)
{
	if (m_bShake == false)
		return;
	m_fCurShakeTime += fTimeDelta;
	if (m_fShakeTime < m_fCurShakeTime)
	{
		if (m_fShakePower < 0.f)
		{
			Shake_Off();
			return;
		}
		else
		{
			m_fShakePower -= 0.01f*m_fShakeOriginPower;
		}
	}

	m_fShakeSpeedTime += fTimeDelta;
	if (m_fShakeSpeed < m_fShakeSpeedTime)
	{
		m_iDir += 1;
		m_fShakePowerTime = 0.f;
		m_fShakeSpeedTime = 0.f;
	}

	m_fShakePowerTime += m_fShakePower * fTimeDelta * 0.5f;
	
	_vector vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	//_vector vLookAt = vPos + XMVector3Normalize( m_pTransformCom->Get_State(CTransform::STATE_LOOK))*m_CameraDesc.fNear;
	switch (m_iDir %5)
	{
	case 0:
		vPos.m128_f32[0] += m_fShakePowerTime;
		vPos.m128_f32[1] += m_fShakePowerTime;
		vPos.m128_f32[2] += m_fShakePowerTime;

		break;
	case 1:
		vPos.m128_f32[0] += m_fShakePowerTime;
		vPos.m128_f32[1] -= m_fShakePowerTime;
		vPos.m128_f32[2] -= m_fShakePowerTime;

		break;
	case 2:
		vPos.m128_f32[0] -= m_fShakePowerTime;
		vPos.m128_f32[1] += m_fShakePowerTime;
		vPos.m128_f32[2] += m_fShakePowerTime;

		break;
	case 3:
		vPos.m128_f32[0] += m_fShakePowerTime;
		vPos.m128_f32[1] -= m_fShakePowerTime;
		vPos.m128_f32[2] += m_fShakePowerTime;

		break;
	case 4:
		vPos.m128_f32[0] += m_fShakePowerTime;
		vPos.m128_f32[1] += m_fShakePowerTime;
		vPos.m128_f32[2] -= m_fShakePowerTime;

		break;
	case 5:
		vPos.m128_f32[0] -= m_fShakePowerTime;
		vPos.m128_f32[1] -= m_fShakePowerTime;
		vPos.m128_f32[2] -= m_fShakePowerTime;

		break;
	}
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
	//m_pTransformCom->LookAt(XMLoadFloat4(&m_CameraDesc.vAt));
}

void CCamera::Cam_ZoomIn(_float fTimeDelta)
{
	//∞¢µµ∞° ¡Ÿ∏È ¡‹¿Œ
	if (fabs(m_CameraDesc.fFovy - m_fZoomFOV) < XMConvertToRadians(3.f))
	{
		m_CameraDesc.fFovy = m_fZoomFOV;
		if (m_fZoomTime != 0.f)
		{
			m_fZoomCurTime += fTimeDelta;
			if (m_fZoomCurTime > m_fZoomTime)
			{
				m_fZoomCurTime = 0.f;
				ZoomOff(m_fZoomSpeed);
			}
		}
	}
	else
	{
		m_CameraDesc.fFovy -= XMConvertToRadians(m_fZoomSpeed * fTimeDelta);
	}
}

void CCamera::Free()
{
	__super::Free();

	Safe_Release(m_pTransformCom);
}
