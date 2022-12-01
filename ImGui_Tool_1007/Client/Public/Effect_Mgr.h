#pragma once
#include "Base.h"
#include "Client_Defines.h"


#include "Claw.h"

#include "Straight_Particle.h"

#include "Arrow.h"
#include "Wind.h"

BEGIN(Client)

class CEffect_Mgr :
	public CBase
{
	DECLARE_SINGLETON(CEffect_Mgr)
public:
	enum EFFECT_TYPE {
		EFFECT_CLAW,
		EFFECT_FIRE,
		EFFECT_PARTICLE,
		EFFECT_CROSSTRAIL,
		EFFECT_DISAPPEAR,
		EFFECT_ARROW,
		EFFECT_WIND,
		EFFECT_TWIST,
		EFFECT_TARGET,
		EFFECT_BATCLAW,
		EFFECT_END
	};

private:
	CEffect_Mgr() = default;
	virtual ~CEffect_Mgr() = default;

public:
	void Tick(_float fTimeDelta);
	
	CEffect* Add_Effect(EFFECT_TYPE _eType, void* pArg);
	HRESULT Add_ParticleEffect(CEffect_Particle::PARTICLETYPE _eParticleType, void* pArg);

	void Clear_Level(LEVEL _eLv);

private:
	list<CEffect*>				m_pEffects[EFFECT_END];
	list<CEffect*>				m_pDeadEffects[EFFECT_END];

public:
	// CBase을(를) 통해 상속됨
	virtual void Free() override;
};

END