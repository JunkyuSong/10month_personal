#pragma once
#include "Effect.h"
#include "Client_Defines.h"
#include "Effect_Particle.h"

BEGIN(Engine)
class CRenderer;
class CNonAnimModel;
END

BEGIN(Client)
class CDisappearWeapon :
	public CEffect
{
public:
	struct DISAPPEARWEAPON
	{
		_float4x4	World;
		TRAILPOS	HighAndLow;
		CNonAnimModel*	pModel = nullptr;
		_float		fSpeed = 1.f;
		_uint		iPass = 3;
		_bool		bBow = false;
	};
private:
	CDisappearWeapon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDisappearWeapon(const CDisappearWeapon& rhs);
	virtual ~CDisappearWeapon() = default;

public:
	virtual HRESULT			Initialize_Prototype() override;
	virtual HRESULT			Initialize(void* pArg) override;
	HRESULT Render();


private:
	HRESULT Ready_Components();

private:
	CNonAnimModel*	m_pModelCom = nullptr;

	_float			m_fTick = 0.f;

	DISAPPEARWEAPON	m_tDW;
	_float3			m_vDir;

	CEffect_Particle::OPTION m_tOption;

public:
	static	CDisappearWeapon*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;

	// CGameObject을(를) 통해 상속됨
	virtual CGameObject * Clone(void * pArg) override;

	// CEffect을(를) 통해 상속됨
	virtual const _bool & Update(_float _fTimeDelta) override;
};

END