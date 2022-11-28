#pragma once
#include "Effect_Particle.h"

BEGIN(Engine)
class CTexture;
class CVIBuffer_Point;
END

BEGIN(Client)

class CFire final :
	public CEffect
{
public:
	struct FIRE_DESC
	{
		_float3	vDirect;
		_float4 vColor;
		_float4 vPos;
		_float2	vSize;
		_float	fLifeTime;
		_uint	iChance = 1;
	};
private:
	CFire(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFire(const CEffect_Particle& rhs);
	virtual ~CFire() = default;

public:
	HRESULT Initialize_Prototype(_tchar * szTextureTag);
	virtual HRESULT Initialize(void* pArg);
	virtual HRESULT Render();
	virtual const _bool & Update(_float _fTimeDelta) override;

private:
	CTexture*						m_pTextureCom = nullptr;
	_uint							m_iSpriteNum = 0;
	_uint							m_iCurSpriteY = 0;
	_uint							m_iCurSpriteX = 0;

	_float							m_fSpriteTime = 0.f;

	_float							m_fCurSpeed = 0.f;
	_float							m_fAccSpeed = 0.f;
	_float3							m_vDirect;

	_float							m_fLifeTime = 0.f;
	_float							m_fTime = 0.f;

	_float2							m_vSize;
	CVIBuffer_Point*				m_pVIBufferCom = nullptr;
	_float4							m_vColor;
	_uint							m_iCurChance = 0;

	_int							m_iLight = 0;

	LEVEL							m_eCurLightLevel = LEVEL::LEVEL_END;

private:
	HRESULT			Ready_Components();
	_bool			MoveFrame(_float _fDeltaTime);

public:
	static CEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _tchar* szTextureTag = nullptr);
	virtual CGameObject * Clone(void * pArg) override;
	
	virtual void Free();
};

END