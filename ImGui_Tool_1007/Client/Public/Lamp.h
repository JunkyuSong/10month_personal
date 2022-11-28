#pragma once

#include "Client_Defines.h"
#include "Obj_NonAnim.h"


BEGIN(Engine)
class CNonAnimModel;
END

BEGIN(Client)

class CLamp final : public CObj_NonAnim
{
private:
	CLamp(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLamp(const CLamp& rhs);
	virtual ~CLamp() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick( _float fTimeDelta);
	virtual void LateTick( _float fTimeDelta);
	virtual HRESULT Render();

public:


private:
	HRESULT Ready_Components();
	HRESULT SetUp_ShaderResources();

private:
	_uint m_iLightIndex = -1;

public:
	static CLamp* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END