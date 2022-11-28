#include "..\Public\DirLight.h"
#include "Shader.h"
#include "VIBuffer_Rect.h"
#include "Frustum.h"

#include "GameInstance.h"
#include "PipeLine.h"

#include "Level_Manager.h"

CDirLight::CDirLight(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
}

HRESULT CDirLight::Initialize(const DIRLIGHTDESC & LightDesc)
{
	m_LightDesc = LightDesc;

	/*_matrix _LightDirInverseMatrix = XMMatrixIdentity();

	_LightDirInverseMatrix.r[2] = XMVector4Normalize( XMLoadFloat4(&m_LightDesc.vDirection));

	_LightDirInverseMatrix.r[0] = XMVector4Normalize(XMVector3Cross(XMVectorSet(0.f,1.f,0.f,0.f), _LightDirInverseMatrix.r[2]));
	_LightDirInverseMatrix.r[1] = XMVector4Normalize(XMVector3Cross(_LightDirInverseMatrix.r[2], _LightDirInverseMatrix.r[0]));

	_LightDirInverseMatrix.r[3] = XMLoadFloat4(&LightDesc.vDirection) * -5.f;
	_LightDirInverseMatrix.r[3].m128_f32[3] = 1.f;

	_LightDirInverseMatrix = XMMatrixInverse(nullptr, _LightDirInverseMatrix);

	_LightDirInverseMatrix = XMMatrixTranspose(_LightDirInverseMatrix);

	XMStoreFloat4x4(m_LightDesc.LightDirInverseMatrix, _LightDirInverseMatrix);*/


	{
		_matrix _LightDirInverseMatrix = XMMatrixIdentity();

		_LightDirInverseMatrix.r[2] = XMVector4Normalize(XMLoadFloat4(&m_LightDesc.vDirection));

		_LightDirInverseMatrix.r[0] = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), _LightDirInverseMatrix.r[2]);
		_LightDirInverseMatrix.r[1] = XMVector3Cross(_LightDirInverseMatrix.r[2], _LightDirInverseMatrix.r[0]);

		AUTOINSTANCE(CGameInstance, _pGameInstance);
		_vector _vPos = XMLoadFloat4(&m_LightDesc.vPosition);

		/*static_cast<CTransform*>(_pGameInstance->Get_Player()->Get_ComponentPtr(TEXT("Com_Transform")))->Get_State(CTransform::STATE_POSITION);*/
		_LightDirInverseMatrix.r[3] = _vPos;//XMVectorSetW( XMLoadFloat4(&m_LightDesc.vDirection) * -5.f + _vPos, 1.f);

		_LightDirInverseMatrix = XMMatrixInverse(nullptr, _LightDirInverseMatrix);

		_LightDirInverseMatrix = XMMatrixTranspose(_LightDirInverseMatrix);

		XMStoreFloat4x4(m_LightDesc.LightViewMatrix, _LightDirInverseMatrix);
	}
	
	return S_OK;
}

HRESULT CDirLight::Render(CShader * pShader, CVIBuffer_Rect * pVIBuffer, CFrustum* _pFrustum)
{
	


	
	_uint		iPassIndex = 1;

	//if (CLevel_Manager::Get_Instance()->Get_CurLv() == 5)
	//{
	//	_matrix _LightDirInverseMatrix = XMMatrixIdentity();
	//	AUTOINSTANCE(CGameInstance, _pGameInstance);
	//	_vector _vPos = XMLoadFloat4(&_pGameInstance->Get_CamPosition());
	//	XMStoreFloat4(&m_LightDesc.vDirection, XMVector3Normalize(XMVectorSet(45.f, 0.1f, 45.f, 1.f) - _vPos)
	//	);

	//	_LightDirInverseMatrix.r[2] = XMVector4Normalize(XMLoadFloat4(&m_LightDesc.vDirection));

	//	_LightDirInverseMatrix.r[0] = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), _LightDirInverseMatrix.r[2]);
	//	_LightDirInverseMatrix.r[1] = XMVector3Cross(_LightDirInverseMatrix.r[2], _LightDirInverseMatrix.r[0]);

	//	

	//	/*static_cast<CTransform*>(_pGameInstance->Get_Player()->Get_ComponentPtr(TEXT("Com_Transform")))->Get_State(CTransform::STATE_POSITION);*/

	//	_LightDirInverseMatrix.r[3] = _vPos;//XMVectorSetW( XMLoadFloat4(&m_LightDesc.vDirection) * -5.f + _vPos, 1.f);

	//	_LightDirInverseMatrix = XMMatrixInverse(nullptr, _LightDirInverseMatrix);

	//	_LightDirInverseMatrix = XMMatrixTranspose(_LightDirInverseMatrix);

	//	XMStoreFloat4x4(m_LightDesc.LightDirInverseMatrix, _LightDirInverseMatrix);
	//}


	if (FAILED(pShader->Set_RawValue("g_vLightDir", &m_LightDesc.vDirection, sizeof(_float4))))
		return E_FAIL;

	if (FAILED(pShader->Set_RawValue("g_LightView", m_LightDesc.LightViewMatrix, sizeof(_float4x4))))
		return E_FAIL;

	if (FAILED(pShader->Set_RawValue("g_vLightDiffuse", &m_LightDesc.vDiffuse, sizeof(_float4))))
		return E_FAIL;

	if (FAILED(pShader->Set_RawValue("g_vLightAmbient", &m_LightDesc.vAmbient, sizeof(_float4))))
		return E_FAIL;

	if (FAILED(pShader->Set_RawValue("g_vLightSpecular", &m_LightDesc.vSpecular, sizeof(_float4))))
		return E_FAIL;

	pShader->Begin(iPassIndex);

	pVIBuffer->Render();

	return S_OK;
}

CDirLight * CDirLight::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const DIRLIGHTDESC & LightDesc)
{
	CDirLight*			pInstance = new CDirLight(pDevice, pContext);

	if (FAILED(pInstance->Initialize(LightDesc)))
	{
		MSG_BOX(TEXT("Failed To Created : CPointLight"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CDirLight::Free()
{
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

	Safe_Delete(m_LightDesc.LightViewMatrix);
	Safe_Delete(m_LightDesc.LightProjMatrix);
}
