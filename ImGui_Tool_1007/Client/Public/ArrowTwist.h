#pragma once

#include "Effect.h"

BEGIN(Engine)
class CNonAnimModel;
END

BEGIN(Client)
class CArrowTwist final : public CEffect
{
private:
	CArrowTwist(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CArrowTwist(const CArrowTwist& rhs);
	virtual ~CArrowTwist();
	
public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual const _bool& Update(_float fTimeDelta);
	virtual HRESULT Render();

	void	Move_Effect(_vector _vPos);

private:
	CNonAnimModel*	m_pModelCom = nullptr;

	_float			m_fMaxTime = 0.1f;
	_float			m_fCurTime = 0.f;
	_float			m_fAccSpeed = 1.f;
	_float			m_fEffectTime = 0.f;

	_float3			m_fAngle;

	_uint			m_iPass = 8;

private:
	HRESULT Ready_Components();

public:
	static CArrowTwist* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;

};

END