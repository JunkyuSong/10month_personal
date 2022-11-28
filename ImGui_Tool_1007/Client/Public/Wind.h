#pragma once

#include "Effect.h"

BEGIN(Engine)
class CNonAnimModel;
END

BEGIN(Client)
class CWind final : public CEffect
{
public:

	struct WIND_DESC
	{
		_float4	TargetPos;
		_float	fAngle;
	};
private:
	CWind(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CWind(const CWind& rhs);
	virtual ~CWind();
	
public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual const _bool& Update(_float fTimeDelta);
	virtual HRESULT Render();

	void	Setting(_vector _vPos, _bool _bLast);

private:
	CNonAnimModel*	m_pModelCom = nullptr;
	WIND_DESC		m_tWind;

	_float			m_fMaxTime = 0.1f;
	_float			m_fCurTime = 0.f;
	_float			m_fEffectTime = 0.f;

	_float			m_fSpeed = 1.f;

	_bool			m_bLast = false;

	_uint			m_iPass = 7;

private:
	HRESULT Ready_Components();

public:
	static CWind* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};

END