#pragma once

#include "Effect.h"

BEGIN(Engine)
class CNonAnimModel;
END

BEGIN(Client)
class CBat_Claw final : public CEffect
{
public:
	enum CLAWTYPE { CLAWTYPE_RIGHT, CLAWTYPE_LEFT, CLAWTYPE_CENTER, CLAWTYPE_END };

	struct CLAW_DESC
	{
		_float4x4	TargetMatrix;

		CLAWTYPE	eClaw = CLAWTYPE_END;

		_float3		vDir;
		_float3		vDis = _float3(0.f, 0.f, 0.f);
		_float3		vAngle;
	};
private:
	CBat_Claw(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBat_Claw(const CBat_Claw& rhs);
	virtual ~CBat_Claw();
	
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

	_uint			m_iPass = 11;


	_float			m_fCenterMaxTime = 0.1f;
	_bool			m_bCreate = false;


private:
	HRESULT Ready_Components();

public:
	static CBat_Claw* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};

END