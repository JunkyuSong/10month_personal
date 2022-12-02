#pragma once

#include "Client_Defines.h"
#include "Level_Client.h"

BEGIN(Engine)
class CTexture;
class CShader;
class CVIBuffer_Rect;
class CTransform;
END


BEGIN(Client)

class CLevel_Loading final : public CLevel_Client
{
private:
	CLevel_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Loading() = default;

public:
	virtual HRESULT Initialize(const _uint& _eNextLevel);

	virtual void Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT	Ready_Component();

private:
	LEVEL					m_eNextLevel = LEVEL_END;
	class CLoader*			m_pLoader = nullptr;
	_bool					m_OneReady = false;
	CTexture*				m_pTextureCom = nullptr;
	CShader*				m_pShaderCom = nullptr;
	CVIBuffer_Rect*			m_pVIBufferCom = nullptr;
	CTransform*				m_pTransformCom = nullptr;
	_float4x4				m_ViewMatrix, m_ProjMatrix;

public:
	static CLevel_Loading* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevel);
	virtual void Free() override;
};

END