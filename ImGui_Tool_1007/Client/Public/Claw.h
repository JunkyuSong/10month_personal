#pragma once

#include "Effect.h"

BEGIN(Engine)
class CNonAnimModel;
END

BEGIN(Client)
class CClaw final : public CEffect
{
public:
	enum CLAWTYPE { CLAWTYPE_RIGHT, CLAWTYPE_LEFT, CLAWTYPE_CENTER, CLAWTYPE_END };

	struct CLAW_DESC
	{
		_float4x4	TargetMatrix;
		CLAWTYPE	eClaw = CLAWTYPE_END;
	};
private:
	CClaw(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CClaw(const CClaw& rhs);
	virtual ~CClaw();
	
public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual const _bool& Update(_float fTimeDelta);
	virtual HRESULT Render();

private:
	CNonAnimModel*	m_pModelCom = nullptr;
	CLAW_DESC		m_tClaw;

	_float			m_fMaxTime = 0.1f;
	_float			m_fCurTime = 0.f;
	_float			m_fEffectTime = 0.f;

	_float3			m_fAngle;

	_uint			m_iPass = 0;


	_float			m_fCenterMaxTime = 0.1f;
	_bool			m_bCreate = false;


private:
	HRESULT Ready_Components();

public:
	static CClaw* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};

END