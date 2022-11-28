#pragma once

#include "GameObject.h"
#include "Client_Defines.h"

BEGIN(Engine)
class CShader;
class CRenderer;
class CTransform;
END

BEGIN(Client)
class CEffect abstract: public CGameObject
{
protected:
	CEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEffect(const CEffect& rhs);
	virtual ~CEffect() = default;

public:
	virtual const _bool& Update(_float _fTimeDelta) = 0;
	virtual void Clear() {}

protected:
	CShader*				m_pShaderCom = nullptr;
	CRenderer*				m_pRendererCom = nullptr;
	CTransform*				m_pTransformCom = nullptr;

	_bool					m_bDead = false;

public:
	virtual CGameObject * Clone(void * pArg) = 0;
	virtual void Free();
};
END