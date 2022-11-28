#pragma once

#include "Client_Defines.h"
#include "Obj_NonAnim.h"


BEGIN(Engine)
class CNonAnimModel;
END

BEGIN(Client)

class CStreetLight final : public CObj_NonAnim
{
private:
	CStreetLight(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CStreetLight(const CStreetLight& rhs);
	virtual ~CStreetLight() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick( _float fTimeDelta);
	virtual void LateTick( _float fTimeDelta);
	virtual HRESULT Render();

private:
	HRESULT Ready_Components();
	HRESULT SetUp_ShaderResources();

private:
	CGameObject* m_pLamp = nullptr;

public:
	static CStreetLight* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END