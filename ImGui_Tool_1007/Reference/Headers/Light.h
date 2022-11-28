#pragma once

#include "Base.h"

BEGIN(Engine)
/* 빛 정볼르 보관한다. */

class CLight : public CBase
{
protected:
	CLight(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLight() = default;

public:


public:
	HRESULT Initialize();

	HRESULT Render(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer, class CFrustum* _pFrustum);

private:
	ID3D11Device*			m_pDevice = nullptr;
	ID3D11DeviceContext*	m_pContext = nullptr;

public:
	virtual void Free() override;
};

END