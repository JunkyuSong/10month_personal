#pragma once

#include "Client_Defines.h"
#include "Client_Camere.h"

BEGIN(Engine)
class CNavigation;
class CAnimModel;
class CHierarchyNode;
END

BEGIN(Client)

class CCamera_CutScene_Enter final : public CClient_Camere
{
private:
	CCamera_CutScene_Enter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCamera_CutScene_Enter(const CCamera_CutScene_Enter& rhs, CTransform::TRANSFORMDESC* pArg);
	virtual ~CCamera_CutScene_Enter() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void Set_Target(CGameObject* _pTarget);
	void Set_Trans(_vector _pTarget) 
	{		
		XMStoreFloat4(&m_vLookAt, _pTarget);
	}
	void Set_CutSceneNum(_uint _iCutScene)
	{
		m_iCutScene = _iCutScene;
	}
	_uint Get_CutSceneNum()
	{
		return m_iCutScene;
	}

private:
	void End_Lv_Stage_02_1(_float fTimeDelta);
	void Enter_Lv_Stage_01(_float fTimeDelta);
	void Enter_Lv_Stage_02(_float fTimeDelta);

public:
	static CCamera_CutScene_Enter* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

private:
	CTransform*			m_pTarget = nullptr;
	_float4				m_vLookAt;
	CAnimModel*			m_pModel = nullptr;
	CHierarchyNode*		m_pCameraBone = nullptr;

	//ÄÆ¾À º¤ÅÍ
	_uint				m_iCutScene = 0;
	_float				m_fAngleY = 0.f;
	_float				m_fAngleX = 0.f;

	_float				m_CamSpeed = 0.f;
};

END