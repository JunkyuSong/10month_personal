#include "stdafx.h"
#include "..\Public\Trail_Obj.h"
#include "Renderer.h"

CTrail_Obj::CTrail_Obj(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
{
}

CTrail_Obj::CTrail_Obj(const CTrail_Obj & rhs)
	: CGameObject(rhs)
{
}

CTrail_Obj * CTrail_Obj::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CTrail_Obj*			pInstance = new CTrail_Obj(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CTrail_Obj"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTrail_Obj::Free()
{
	__super::Free();

	Safe_Release(m_pVIBuffer);
	Safe_Release(m_pRenderer);
}

CGameObject * CTrail_Obj::Clone(void * pArg)
{
	CTrail_Obj*			pInstance = new CTrail_Obj(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CTrail_Obj"));
		Safe_Release(pInstance);
	}

	return pInstance;
}


HRESULT CTrail_Obj::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTrail_Obj::Initialize(void * pArg)
{
	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (pArg == nullptr)
	{
		MSG_BOX(TEXT("Plz High and Low"));
		return E_FAIL;
	}

	CTrail::TRAILINFO _tInfo = *static_cast<CTrail::TRAILINFO*>(pArg);
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Trail"), TEXT("Com_Trail"), (CComponent**)&m_pVIBuffer, &_tInfo)))
	{
		MSG_BOX(TEXT("fail to trail"));
		return E_FAIL;
	}
	

	return S_OK;
}

HRESULT CTrail_Obj::Render()
{
	m_pVIBuffer->Render();
	return S_OK;
}

void CTrail_Obj::Tick(const _float & _fTimeDelta, _matrix _matWeapon)
{
	m_pVIBuffer->Tick(_fTimeDelta, _matWeapon);
	if (m_pVIBuffer->Get_On())
		m_pRenderer->Add_RenderGroup(CRenderer::RENDER_ALPHABLEND, this);
}

void CTrail_Obj::TrailOn(_matrix _matWeapon)
{
	m_pVIBuffer->TrailOn(_matWeapon);
}

void CTrail_Obj::TrailOff()
{
	m_pVIBuffer->TrailOff();
}

const _bool & CTrail_Obj::Get_On()
{
	return m_pVIBuffer->Get_On();
}

TRAILPOS CTrail_Obj::Get_HighAndLow()
{
	return m_pVIBuffer->Get_HighAndLow();
}

TRAILPOS CTrail_Obj::Get_HighAndLow(_matrix _matWorld)
{
	return m_pVIBuffer->Get_HighAndLow(_matWorld);
}

void CTrail_Obj::Set_Color(_float4 _Color)
{
	m_pVIBuffer->Set_Color(_Color);
}

HRESULT CTrail_Obj::Ready_Components()
{
	/* For.Com_Renderer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), TEXT("Com_Renderer"), (CComponent**)&m_pRenderer)))
		return E_FAIL;



	return S_OK;
}
