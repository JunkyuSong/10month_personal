#pragma once

#include "Client_Defines.h"
#include "Obj_Plus.h"


BEGIN(Engine)
class CInstancingModel;
END

BEGIN(Client)

class CInstancingObj : public CObj_Plus
{
protected:
	CInstancingObj(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CInstancingObj(const CInstancingObj& rhs);
	virtual ~CInstancingObj() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick( _float fTimeDelta);
	virtual void LateTick( _float fTimeDelta);
	virtual HRESULT Render();
	virtual HRESULT Render_Shadow();

public:
	virtual HRESULT		Set_Info(OBJ_DESC _tInfo) override;
	HRESULT				Set_Instancing(_tchar* _szModelKey, vector<_float4x4>* _vecWorld);

protected:
	CInstancingModel*					m_pModelCom = nullptr;

private:
	HRESULT Ready_Components();
	HRESULT SetUp_ShaderResources();

public:
	static CInstancingObj* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END