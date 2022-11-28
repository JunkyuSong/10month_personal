#pragma once

#include "Client_Defines.h"
#include "Client_Camere.h"

BEGIN(Engine)
class CNavigation;
class CAnimModel;
class CHierarchyNode;
END

BEGIN(Client)

class CCamera_CutScene final : public CClient_Camere
{
private:
	CCamera_CutScene(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCamera_CutScene(const CCamera_CutScene& rhs, CTransform::TRANSFORMDESC* pArg);
	virtual ~CCamera_CutScene() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void LateTick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void Get_Target(CTransform* _pTarget) { m_pTarget = _pTarget; }
	HRESULT	LookAt(char* _szBone);


public:
	static CCamera_CutScene* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

private:
	CTransform*			m_pTarget = nullptr;
	CAnimModel*			m_pModel = nullptr;
	CHierarchyNode*		m_pCameraBone = nullptr;
	CHierarchyNode*		m_pLookAtBone = nullptr;

};

END