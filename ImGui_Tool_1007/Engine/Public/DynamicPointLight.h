#pragma once

#include "PointLight.h"

BEGIN(Engine)
/* 빛 정볼르 보관한다. */

class CDynamicPointLight : public CPointLight
{
private:
	CDynamicPointLight(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CDynamicPointLight() = default;


public:
	HRESULT			Initialize(const POINTLIGHTDESC& LightDesc, _float fSpeed, _float fTime);
	virtual _bool	Tick(_float TimeDelta);
	HRESULT			Render(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer, class CFrustum* _pFrustum);

	void			Reset_Time() { m_fCurTime = 0.f; }

private:
	_float			m_fSpeed = 0.f;
	_float			m_fTime = 0.f;
	_float			m_fCurTime = 0.f;

public:
	static CDynamicPointLight* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const POINTLIGHTDESC& LightDesc, _float fSpeed, _float fTime);
	virtual void Free() override;
};

END