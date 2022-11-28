#pragma once
#include "Base.h"
#include "Client_Defines.h"
#include "Effect_Particle.h"

BEGIN(Client)
class CDeadParticle_Mgr :
	public CBase
{
	DECLARE_SINGLETON(CDeadParticle_Mgr)
public:
	enum PARTICLETYPE { TYPE_Straight, TYPE_CURVE, TYPE_END };
private:
	CDeadParticle_Mgr();
	virtual ~CDeadParticle_Mgr() = default;

public:
	CEffect_Particle*				Get_Straight_Particle();
	void							Dead_Straight_Particle(class CStraight_Particle* _pParticle);

private:
	list<class CStraight_Particle*> m_Dead_Straight_Particles;
	//list<class CInstance_Particle*> m_Straight_Particles;
	//list<class CCurve_Particle*> m_Straight_Particles;


public:
	// CBase을(를) 통해 상속됨
	virtual void Free() override;
};

END

/*

CEffect_Particle::OPTION _tOption;
//_tOption.Center = _float3(45.f, 2.f, 45.f);
_vector	_vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
_float4x4	_vWorld = m_pTransformCom->Get_WorldFloat4x4();;
_tOption.Center = _float3(_vPos.m128_f32[0], _vPos.m128_f32[1] + 1.f, _vPos.m128_f32[2]);
_tOption.eType = CEffect_Particle::PARTICLETYPE::TYPE_STRIGHT;
_tOption.fAccSpeed = 0.99f;
_tOption.fSpeed = { 1.5f, 3.3f };
_tOption.fGravity = 0.f;
_tOption.fLifeTime = 0.f;
_tOption.fRange = _float3(5.f, 5.f, 1.f);
_tOption.iNumParticles = 30;
_tOption.Size = _float2(0.2f, 0.2f);
_tOption.Spread = CEffect_Particle::SPREAD::SPREAD_EDGE;
_tOption.szMaskTag = TEXT("Prototype_Component_Texture_Mask_Blood");
_tOption.szTextureTag = TEXT("Prototype_Component_Texture_Diffuse_Blood");
//_tOption.vColor = CLIENT_RGB(119.f, 245.f, 200.f);
_tOption.vColor = CLIENT_RGB(82.f, 9.f, 4.f);
//_tOption.bPlayerDir = true;
_tOption.fSpead_Angle = _float3(0.f, 20.f, 20.f);
_tOption.vStart_Dir = _float3(1.f, 0.f, 0.f);
_tOption.eDiffuseType = CEffect_Particle::DIFFUSE_COLOR;
_tOption.eDirType = CEffect_Particle::DIR_TYPE::DIR_ANGLE;
_tOption.eStartType = CEffect_Particle::START_CENTER;
_tOption.fMaxDistance = { 0.4f, 1.2f };
_tOption.bPlayerDir = true;
_tOption.matPlayerAxix = _vWorld;

if (FAILED(CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_PARTICLE, &_tOption)))
{
MSG_BOX(TEXT("effect"));
return;
}

*/