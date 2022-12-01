#pragma once

#include "Client_Defines.h"
#include "Weapon.h"
#include "Effect_Particle.h"

BEGIN(Client)

class CSaber final : public CWeapon
{
private:
	enum COLLIDERTYPE {
		COLLILDERTYPE_OBB, COLLILDERTYPE_END
	};
private:
	CSaber(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSaber(const CSaber& rhs);
	virtual ~CSaber() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(_float fTimeDelta) override;
	virtual void Tick(_float fTimeDelta, CGameObject* _pUser);
	virtual void LateTick(_float fTimeDelta);
	virtual HRESULT Render();
	virtual HRESULT Render_Shadow() override;


	void		Light_On();
	void		Hit();
	void		Blood();

private:
	CEffect_Particle::OPTION m_tOption;
	_uint	m_iLight = 0;

	_bool	m_bHitTime = false;

	_float	m_fDelayTime = 0.05f;
	_float	m_fCurDelayTime = 0.0f;
	_float	m_fStopTime = 0.05f;
	_float	m_fCurStopTime = 0.0f;


private:
	HRESULT Ready_Components();

public:
	static CSaber* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END