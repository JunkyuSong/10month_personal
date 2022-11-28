#pragma once

#include "Client_Defines.h"
#include "Weapon.h"

BEGIN(Engine)
class CTexture;
END

BEGIN(Client)

class CBow final : public CWeapon
{
public:
	enum BOWSTATE { BOW_FULL, BOW_START, BOW_END };
private:
	CBow(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBow(const CBow& rhs);
	virtual ~CBow() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(_float fTimeDelta);
	virtual void Tick(_float fTimeDelta, CGameObject* _pUser);
	virtual void LateTick(_float fTimeDelta);
	virtual HRESULT Render();
	virtual void End();
	void Set_Stop(const _bool& _bStop) { m_bStop = _bStop; }

	void Shoot();

private:
	_uint		m_iPass = 1;
	BOWSTATE	m_eState = BOW_START;
	_bool		m_bStop = false;
	_bool		m_bRender = false;

	CTexture*	m_pDissolve = nullptr;

	_float		m_fTime = 0.f;

private:
	HRESULT Ready_Components();

public:
	static CBow* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END