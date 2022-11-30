#pragma once

#include "Effect.h"

BEGIN(Engine)
class CNonAnimModel;
END

BEGIN(Client)
class CArrowTarget final : public CEffect
{

private:
	CArrowTarget(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CArrowTarget(const CArrowTarget& rhs);
	virtual ~CArrowTarget();
	
public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual const _bool& Update(_float fTimeDelta);
	virtual HRESULT Render();

	void	Setting(_vector _vPos, _bool _bLast);

private:
	CNonAnimModel*	m_pModelCom = nullptr;

	_float			m_fMaxTime = 0.1f;
	_float			m_fCurTime = 0.f;
	_float			m_fEffectTime = 0.f;

	_float			m_fSpeed = 0.f;
	_float			m_fAccSpeed = 1.f;

	_bool			m_bLast = false;

	_uint			m_iPass = 10;

private:
	HRESULT Ready_Components();

public:
	static CArrowTarget* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};

END