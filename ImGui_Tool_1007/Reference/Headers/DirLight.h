#pragma once

#include "Base.h"

BEGIN(Engine)
/* 빛 정볼르 보관한다. */

class CDirLight final : public CBase
{
private:
	CDirLight(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CDirLight() = default;

public:
	DIRLIGHTDESC* Get_LightDesc() {
		return &m_LightDesc;
	}


public:
	HRESULT Initialize(const DIRLIGHTDESC& LightDesc);
	HRESULT Render(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer, class CFrustum* _pFrustum);

private:
	ID3D11Device*			m_pDevice = nullptr;
	ID3D11DeviceContext*	m_pContext = nullptr;

private:
	DIRLIGHTDESC				m_LightDesc;
	

public:
	static CDirLight* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const DIRLIGHTDESC& LightDesc);
	virtual void Free() override;
};

END