#include "stdafx.h"
#include "..\Public\Effect.h"
#include "GameInstance.h"

CEffect::CEffect(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject(pDevice, pContext)
{
}

CEffect::CEffect(const CEffect & rhs)
	: CGameObject(rhs)
	, m_pRendererCom(rhs.m_pRendererCom)
	, m_pShaderCom(rhs.m_pShaderCom)
	, m_pTransformCom(rhs.m_pTransformCom)
{
}

void CEffect::Free()
{
	__super::Free();
	Safe_Release(m_pRendererCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTransformCom);

}