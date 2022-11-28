#include "stdafx.h"
#include "..\Public\SkillSlot.h"
#include "GameInstance.h"

CSkillSlot::CSkillSlot(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject(pDevice, pContext)
{

}

CSkillSlot::CSkillSlot(const CSkillSlot & rhs)
	: CGameObject(rhs)

{

}

HRESULT CSkillSlot::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSkillSlot::Initialize(void * pArg)
{
	if (FAILED(Ready_Components()))
		return E_FAIL;

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixTranspose(XMMatrixOrthographicLH((_float)g_iWinSizeX, (_float)g_iWinSizeY, 0.f, 1.f)));

	for (_uint i = 0; i < SKILL_CLAW; ++i)
	{
		m_fSkillCoolDown[i] = 20.f;
	}

	return S_OK;
}

void CSkillSlot::Tick(_float fTimeDelta)
{
	AUTOINSTANCE(CGameInstance, _pGameInstance);
	_float _fTimeDelta = fTimeDelta / _pGameInstance->Get_TimeSpeed(TEXT("Timer_Main"));
	for (_uint i = 0; i < SKILL_CLAW; ++i)
	{
		m_fSkillCoolDown[i] += _fTimeDelta;
	}
}

void CSkillSlot::LateTick(_float fTimeDelta)
{
	if (nullptr == m_pRendererCom)
		return;

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CSkillSlot::Render()
{
	if (nullptr == m_pVIBufferCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;

	m_pShaderCom->Set_RawValue("g_ViewMatrix", &m_ViewMatrix, sizeof(_float4x4));
	m_pShaderCom->Set_RawValue("g_ProjMatrix", &m_ProjMatrix, sizeof(_float4x4));


	if (FAILED(Render_Slot()))
		return E_FAIL;





	return S_OK;
}

void CSkillSlot::Reset()
{
	for (_uint i = 0; i < SKILL_6 ; ++i)
	{
		m_fSkillCoolDown[i] += 20.f;
	}
}

_bool CSkillSlot::Check_CoolDown(SKILL _eSkillNum)
{
	if (m_fSkillCoolDown[_eSkillNum] > m_fMaxCoolDown)
	{
		m_fSkillCoolDown[_eSkillNum] = 0.f;
		return true;
	}
		
	return false;
}

HRESULT CSkillSlot::Ready_Components()
{
	/* For.Com_Transform */
	if (FAILED(Ready_Transform()))
		return E_FAIL;

	/* For.Com_Transform */
	if (FAILED(Ready_Texture()))
		return E_FAIL;



	/* For.Com_Renderer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxTex"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"), TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CSkillSlot::Ready_Transform()
{

	// 스킬 슬롯 6개
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform_Slot1"), (CComponent**)&m_pTransformCom_SkillSlot[SKILL_1])))
		return E_FAIL;
	m_pTransformCom_SkillSlot[SKILL_1]->Set_State(CTransform::STATE_POSITION, XMVectorSet(278.24f, -265.f, 0.f, 1.f));
	m_pTransformCom_SkillSlot[SKILL_1]->Set_Scale(XMVectorSet(70.f, 70.f, 1.f, 0.f));

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform_Slot2"), (CComponent**)&m_pTransformCom_SkillSlot[SKILL_2])))
		return E_FAIL;
	m_pTransformCom_SkillSlot[SKILL_2]->Set_State(CTransform::STATE_POSITION, XMVectorSet(313.24f, -230.f, 0.f, 1.f));
	m_pTransformCom_SkillSlot[SKILL_2]->Set_Scale(XMVectorSet(70.f, 70.f, 1.f, 0.f));

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform_Slot3"), (CComponent**)&m_pTransformCom_SkillSlot[SKILL_3])))
		return E_FAIL;
	m_pTransformCom_SkillSlot[SKILL_3]->Set_State(CTransform::STATE_POSITION, XMVectorSet(348.24f, -265.f, 0.f, 1.f));
	m_pTransformCom_SkillSlot[SKILL_3]->Set_Scale(XMVectorSet(70.f, 70.f, 1.f, 0.f));

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform_Slot4"), (CComponent**)&m_pTransformCom_SkillSlot[SKILL_4])))
		return E_FAIL;
	m_pTransformCom_SkillSlot[SKILL_4]->Set_State(CTransform::STATE_POSITION, XMVectorSet(383.24f, -230.f, 0.f, 1.f));
	m_pTransformCom_SkillSlot[SKILL_4]->Set_Scale(XMVectorSet(70.f, 70.f, 1.f, 0.f));

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform_Slot5"), (CComponent**)&m_pTransformCom_SkillSlot[SKILL_5])))
		return E_FAIL;
	m_pTransformCom_SkillSlot[SKILL_5]->Set_State(CTransform::STATE_POSITION, XMVectorSet(418.24f, -265.f, 0.f, 1.f));
	m_pTransformCom_SkillSlot[SKILL_5]->Set_Scale(XMVectorSet(70.f, 70.f, 1.f, 0.f));

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform_Slot6"), (CComponent**)&m_pTransformCom_SkillSlot[SKILL_6])))
		return E_FAIL;
	m_pTransformCom_SkillSlot[SKILL_6]->Set_State(CTransform::STATE_POSITION, XMVectorSet(452.24f, -230.f, 0.f, 1.f));
	m_pTransformCom_SkillSlot[SKILL_6]->Set_Scale(XMVectorSet(70.f, 70.f, 1.f, 0.f));

	// 뺏어온 스킬 + 클로 
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform_Claw"), (CComponent**)&m_pTransformCom_SkillSlot[SKILL_CLAW])))
		return E_FAIL;
	m_pTransformCom_SkillSlot[SKILL_CLAW]->Set_State(CTransform::STATE_POSITION, XMVectorSet(535.74f, -267.5f, 0.f, 1.f));
	m_pTransformCom_SkillSlot[SKILL_CLAW]->Set_Scale(XMVectorSet(70.f, 70.f, 1.f, 0.f));
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform_ClawImage"), (CComponent**)&m_pTransformCom_Claw)))
		return E_FAIL;
	m_pTransformCom_Claw->Set_State(CTransform::STATE_POSITION, XMVectorSet(535.74f, -265.f, 0.f, 1.f));
	m_pTransformCom_Claw->Set_Scale(XMVectorSet(150.f, 150.f, 1.f, 0.f));

	return S_OK;
}

HRESULT CSkillSlot::Ready_Texture()
{
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_UI_MainUI_Slot_Green"), TEXT("Com_Textrue_Green"), (CComponent**)&m_pTextureCom_SkillSlot[SKILLSLOT_GREEN])))
		return E_FAIL;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_UI_MainUI_MiniSlot"), TEXT("Com_Textrue_Mini"), (CComponent**)&m_pTextureCom_SkillSlot[SKILLSLOT_MINI])))
		return E_FAIL;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_UI_MainUI_Slot_Gray"), TEXT("Com_Textrue_Gray"), (CComponent**)&m_pTextureCom_SkillSlot[SKILLSLOT_GRAY])))
		return E_FAIL;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_UI_MainUI_Slot_Claw"), TEXT("Com_Textrue_Claw"), (CComponent**)&m_pTextureCom_SkillSlot[SKILLSLOT_CLAW])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_UI_Icon_Skill_Axe"), TEXT("Com_Textrue_Skill1"), (CComponent**)&m_pTextureCom_Skill[SKILL_2])))
		return E_FAIL;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_UI_Icon_Skill_Twin"), TEXT("Com_Textrue_Skill2"), (CComponent**)&m_pTextureCom_Skill[SKILL_1])))
		return E_FAIL;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_UI_Icon_Skill_Bow"), TEXT("Com_Textrue_Skill3"), (CComponent**)&m_pTextureCom_Skill[SKILL_3])))
		return E_FAIL;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_UI_Icon_Skill_Scythe"), TEXT("Com_Textrue_Skill4"), (CComponent**)&m_pTextureCom_Skill[SKILL_4])))
		return E_FAIL;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_UI_Icon_Skill_Halberd"), TEXT("Com_Textrue_Skill5"), (CComponent**)&m_pTextureCom_Skill[SKILL_5])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_Alpha"), TEXT("Com_Alpha"), (CComponent**)&m_pAlphaTexture)))
		return E_FAIL;


	

	return S_OK;
}

HRESULT CSkillSlot::Render_Slot()
{


	m_pShaderCom->Set_RawValue("g_WorldMatrix", &(m_pTransformCom_Claw->Get_WorldFloat4x4_TP()), sizeof(_float4x4));
	if (FAILED(m_pTextureCom_SkillSlot[SKILLSLOT_CLAW]->Set_SRV(m_pShaderCom, "g_DiffuseTexture")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Begin(0)))
		return E_FAIL;
	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	for (_uint i = 0; i < SKILL_CLAW - 1; ++i)
	{

		_float fPer = m_fSkillCoolDown[i] / m_fMaxCoolDown * 360.f;
		m_pTransformCom_SkillSlot[i]->Set_Scale(XMVectorSet(70.f, 70.f, 1.f, 0.f));
		m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom_SkillSlot[i]->Get_WorldFloat4x4_TP(), sizeof(_float4x4));
		m_pShaderCom->Set_RawValue("g_fPercent", &fPer, sizeof(_float));

		if (FAILED(m_pTextureCom_SkillSlot[SKILLSLOT_GREEN]->Set_SRV(m_pShaderCom, "g_DiffuseTexture")))
			return E_FAIL;
		if (FAILED(m_pTextureCom_SkillSlot[SKILLSLOT_MINI]->Set_SRV(m_pShaderCom, "g_DiffuseTexture_2")))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(5)))
			return E_FAIL;
		if (FAILED(m_pVIBufferCom->Render()))
			return E_FAIL;

		m_pTransformCom_SkillSlot[i]->Set_Scale(XMVectorSet(120.f, 120.f, 1.f, 0.f));
		m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom_SkillSlot[i]->Get_WorldFloat4x4_TP(), sizeof(_float4x4));

		if (FAILED(m_pTextureCom_Skill[i]->Set_SRV(m_pShaderCom, "g_DiffuseTexture")))
			return E_FAIL;
		if (FAILED(m_pAlphaTexture->Set_SRV(m_pShaderCom, "g_DiffuseTexture_2")))
			return E_FAIL;
		

		if (FAILED(m_pShaderCom->Begin(5)))
			return E_FAIL;
		if (FAILED(m_pVIBufferCom->Render()))
			return E_FAIL;
	}

	m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom_SkillSlot[SKILL_CLAW]->Get_WorldFloat4x4_TP(), sizeof(_float4x4));

	if (FAILED(m_pTextureCom_SkillSlot[SKILLSLOT_MINI]->Set_SRV(m_pShaderCom, "g_DiffuseTexture")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(0)))
		return E_FAIL;
	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;


	return S_OK;
}

CSkillSlot * CSkillSlot::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CSkillSlot*		pInstance = new CSkillSlot(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CSkillSlot"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CSkillSlot::Clone(void * pArg)
{
	CSkillSlot*		pInstance = new CSkillSlot(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CSkillSlot"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSkillSlot::Free()
{
	__super::Free();

	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRendererCom);
	
	for (_uint i = 0; i < SKILLSLOT_END; ++i)
	{
		
		Safe_Release(m_pTextureCom_SkillSlot[i]);
	}

	for (_uint i = 0; i < SKILL_END; ++i)
	{
		Safe_Release(m_pTransformCom_Skill[i]);
		Safe_Release(m_pTransformCom_SkillSlot[i]);
		Safe_Release(m_pTextureCom_Skill[i]);
	}
	Safe_Release(m_pTransformCom_Claw);
	Safe_Release(m_pAlphaTexture);
}
