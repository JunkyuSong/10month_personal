#pragma once
#include "Effect_Particle.h"

BEGIN(Client)

class CStraight_Particle final :
	public CEffect_Particle
{
private:
	CStraight_Particle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CStraight_Particle(const CEffect_Particle& rhs);
	virtual ~CStraight_Particle() = default;

public:
	HRESULT Initialize_Prototype(_tchar * szTextureTag);
	virtual HRESULT Initialize(void* pArg);
	virtual HRESULT Render();
	virtual const _bool & Update(_float _fTimeDelta) override;

public:
	void			Init_Again(OPTION _tOption, CTexture* _pTextureCom, CTexture* _pMaskCom, CVIBuffer_Point* _pBufferCom) override; //데드관리자에서 해주는거 -> 버퍼랑 텍스쳐 포인터 있어야함.
	virtual void	Recycle() override;

protected:
	virtual void	Re_Option();


private:
	_float3		m_vDirect;
	_float		m_fCurGravity = 0.f;
	_float		m_fDistane = 0.f;

	_float		m_fTime = 0.f;


private:
	HRESULT			Ready_Components();

public:
	static CEffect_Particle* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _tchar* szTextureTag = nullptr);
	virtual CGameObject * Clone(void * pArg) override;
	
	virtual void Free();
};

END