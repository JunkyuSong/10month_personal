#pragma once

#include "Client_Defines.h"
#include "Weapon.h"


BEGIN(Client)

class CScythe final : public CWeapon
{
private:
	enum COLLIDERTYPE {
		COLLILDERTYPE_OBB, COLLILDERTYPE_END
	};
private:
	CScythe(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CScythe(const CScythe& rhs);
	virtual ~CScythe() = default;

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
	_float4 m_vPreFirePos;
	_float3 m_vPreFireDir;

	_float	m_fFireTime = 0.f;
	_bool	m_bFire = false;

	_bool	m_bHitTime = false;
	_float	m_fDelayTime = 0.05f;
	_float	m_fCurDelayTime = 0.0f;
	_float	m_fStopTime = 0.05f;
	_float	m_fCurStopTime = 0.0f;

private:
	HRESULT Ready_Components();
	void	Fire();
	void	Hit();

public:
	static CScythe* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END