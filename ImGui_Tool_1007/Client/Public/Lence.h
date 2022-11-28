#pragma once

#include "Client_Defines.h"
#include "Weapon.h"


BEGIN(Client)

class CLence final : public CWeapon
{
private:
	enum COLLIDERTYPE {
		COLLILDERTYPE_OBB, COLLILDERTYPE_END
	};
private:
	CLence(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLence(const CLence& rhs);
	virtual ~CLence() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(_float fTimeDelta);
	virtual void Tick(_float fTimeDelta, CGameObject* _pUser);
	virtual void LateTick(_float fTimeDelta);
	virtual HRESULT Render();

	virtual void Clear(void* pArg = nullptr) override;
	virtual void End() override;

private:
	_uint m_iPass = 1;

	_bool	m_bHitTime = false;

	_float	m_fDelayTime = 0.05f;
	_float	m_fCurDelayTime = 0.0f;
	_float	m_fStopTime = 0.05f;
	_float	m_fCurStopTime = 0.0f;

private:
	HRESULT Ready_Components();
	void Hit(CGameObject* _pTarget);

public:
	static CLence* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END