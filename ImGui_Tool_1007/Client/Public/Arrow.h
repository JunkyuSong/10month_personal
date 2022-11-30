#pragma once
#include "Effect.h"
#include "Client_Defines.h"

#include "Effect_Particle.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CTransform;
class CRenderer;
class CCollider;
class CVIBuffer_Point;
END

BEGIN(Client)

class CArrow :
	public CEffect
{
public:
	struct ARROW
	{
		_float4x4 StartMatrix;
	};

public:
	CArrow(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CArrow(const CArrow& rhs);
	virtual ~CArrow() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(_float fTimeDelta);
	virtual void LateTick(_float fTimeDelta);
	virtual HRESULT Render();

	virtual const _bool & Update(_float _fTimeDelta) override;

private:
	ARROW					m_tInfo;


	_float					m_fMaxTime = 0.1f;
	_float					m_fCurTime = 0.f;

	_float					m_fTick = 0.f;

	_float4					m_vColor;

	_float4					m_vStartPos;

	vector<_float3>			m_vPos;
	vector<_float2>			m_vUV;

	_float					m_fSpeed = 30.f;

	CShader*				m_pShaderCom = nullptr;
	CRenderer*				m_pRendererCom = nullptr;
	CVIBuffer_Point*		m_pVIBufferCom = nullptr;
	CTexture*				m_pTextureCom = nullptr;
	CTexture*				m_pArrowTextureCom = nullptr;
	CTexture*				m_pFlareTextureCom = nullptr;
	CCollider*				m_pColliderCom = nullptr;

	vector<class CArrowTwist*>		m_pArrowTwist;

	CEffect_Particle::OPTION		m_tOption;

	class CStatus*					m_pStatusCom = nullptr;

private:
	HRESULT Ready_Components();

public:
	static CArrow* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject * Clone(void * pArg) override;
	virtual void Free() override;




};

END