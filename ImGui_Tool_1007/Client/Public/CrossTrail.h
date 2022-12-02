#pragma once
#include "Effect.h"
#include "Client_Defines.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CTransform;
class CRenderer;
class CVIBuffer_Point;
END

BEGIN(Client)

class CCrossTrail :
	public CEffect
{
public:
	struct CROSS_DESC
	{
		_float4 vRGBA;
		_float	fWidth = 0.005f;
		_bool	bLook = false;
	};
public:
	CCrossTrail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCrossTrail(const CCrossTrail& rhs);
	virtual ~CCrossTrail() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(_float fTimeDelta);
	virtual void LateTick(_float fTimeDelta);
	virtual HRESULT Render();

	virtual const _bool & Update(_float _fTimeDelta) override;

	void	Add_Point(_float3 _vPos);


private:
	_float					m_fMaxTime = 0.8f;
	_float					m_fCurTime = 0.f;

	_float4					m_vColor;
	_float					m_fWidth;

	vector<_float3>			m_vPos;
	vector<_float3>			m_vRight;
	vector<_float3>			m_vUp;
	vector<_float3>			m_vLook;

	CShader*				m_pShaderCom = nullptr;
	CTransform*				m_pTransCom = nullptr; // 쓸모없는데?
	CRenderer*				m_pRendererCom = nullptr;
	CVIBuffer_Point*		m_pVIBufferCom = nullptr;

	_bool					m_bLook = false;

private:
	HRESULT Ready_Components();

public:
	static CCrossTrail* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject * Clone(void * pArg) override;
	virtual void Free() override;




};

END