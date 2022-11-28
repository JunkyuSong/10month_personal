#pragma once

#include "Client_Defines.h"
#include "Obj_Plus.h"


BEGIN(Engine)
class CNonAnimModel;
END

BEGIN(Client)

class CObj_NonAnim : public CObj_Plus
{
protected:
	CObj_NonAnim(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CObj_NonAnim(const CObj_NonAnim& rhs);
	virtual ~CObj_NonAnim() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick( _float fTimeDelta);
	virtual void LateTick( _float fTimeDelta);
	virtual HRESULT Render();
	virtual HRESULT Render_Shadow();

public:
	virtual HRESULT		Set_Info(OBJ_DESC _tInfo) override;

	void		ImGuiTick();

	_bool	Picking(_float3& _vPos);

protected:
	CNonAnimModel*					m_pModelCom = nullptr;

private:
	HRESULT Ready_Components();
	HRESULT SetUp_ShaderResources();

public:
	static CObj_NonAnim* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	static CObj_NonAnim* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, vector<OBJ_DESC>* _tInfo);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END