#pragma once

#include "PointLight.h"

BEGIN(Engine)
/* 빛 정볼르 보관한다. */

class CStaticPointLight : public CPointLight
{
private:
	CStaticPointLight(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CStaticPointLight() = default;


public:
	HRESULT Initialize(const POINTLIGHTDESC& LightDesc);
	HRESULT Render(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer, class CFrustum* _pFrustum);

public:
	static CStaticPointLight* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const POINTLIGHTDESC& LightDesc);
	virtual void Free() override;
};

END