#pragma once
#include "GameObject.h"
#include "Client_Defines.h"
#include "Trail.h"

BEGIN(Engine)
class CRenderer;
END

BEGIN(Client)
class CTrail_Obj :
	public CGameObject
{
private:
	CTrail_Obj(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTrail_Obj(const CTrail_Obj& rhs);
	virtual ~CTrail_Obj() = default;

public:
	virtual HRESULT			Initialize_Prototype() override;
	virtual HRESULT			Initialize(void* pArg) override;
	HRESULT Render();

	void Tick(const _float& _fTimeDelta, _matrix _matWeapon);

	void			TrailOn(_matrix _matWeapon);

	void			TrailOff();

	const _bool&	Get_On();

	TRAILPOS		Get_HighAndLow();
	TRAILPOS		Get_HighAndLow(_matrix _matWorld);


	void			Set_Color(_float4 _Color);

private:
	HRESULT Ready_Components();

private:
	CTrail*			m_pVIBuffer = nullptr;
	CRenderer*		m_pRenderer = nullptr;



public:
	static	CTrail_Obj*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;

	// CGameObject을(를) 통해 상속됨
	virtual CGameObject * Clone(void * pArg) override;
};

END