#pragma once
#include "Effect.h"
#include "Client_Defines.h"

BEGIN(Engine)
class CNonAnimModel;
END

BEGIN(Client)
class CReadyBow :
	public CEffect
{
private:
	CReadyBow(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CReadyBow(const CEffect& rhs);
	virtual ~CReadyBow();
	
public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual const _bool& Update(_float fTimeDelta);
	virtual void LateTick(_float fTimeDelta);
	virtual HRESULT Render();

private:
	CNonAnimModel*	m_pModelCom = nullptr;

private:
	HRESULT Ready_Components();

public:
	static CReadyBow* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CEffect* Clone(void* pArg);
	virtual void Free() override;

};

END