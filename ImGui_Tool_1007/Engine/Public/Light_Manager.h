#pragma once

#include "Base.h"

BEGIN(Engine)

/* 빛들을 보관한다. */

class CLight_Manager final : public CBase
{
	DECLARE_SINGLETON(CLight_Manager)

public:
	enum LIGHTTYPE { DIRLIGHT, DYNAMICPOINTLIHGT, STATICPOINTLIHGT, LIHGTEND };
	enum LIGHTNUM { LIGHT_FIRST, LIGHT_SECOND, LIHGTNUM_END };
	enum LIGHTMATRIX { LIGHT_VIEW, LIGHT_PROJ, LIHGTMATRIX_END };

private:
	CLight_Manager();
	virtual ~CLight_Manager() = default;

public:
	HRESULT	Initialize(_uint iLv);
	void	Tick(_float fTimeDelta);

public:
	DIRLIGHTDESC* Get_DirLightDesc(_uint iLv, _uint iIndex);
	POINTLIGHTDESC* Get_PointLightDesc(_uint iLv, LIGHTTYPE eLightType,_uint iIndex);
	

public:
	_uint Add_Light(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,  _uint iLv, const DIRLIGHTDESC& LightDesc);
	_uint Add_Light(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iLv, LIGHTTYPE eLightType, const POINTLIGHTDESC& LightDesc, _float fSpeed = 0.f, _float fTime = 0.f);
	HRESULT Render(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);

	void	CheckDisLight(_float4x4* LightViewMatrix, _float4x4* LightProjMatrix, _vector vLightPos);

	_float4x4*	Get_LightMatrix(_uint iLv, LIGHTNUM _eLightNum, LIGHTMATRIX _eMatrix) { return m_Light[_eLightNum][_eMatrix]; }

public:
	HRESULT	Light_On(_uint iLv, LIGHTTYPE eLightType, _uint _iIndex);
	HRESULT	Light_Off(_uint iLv, LIGHTTYPE eLightType, _uint _iIndex);
	HRESULT Clear(_uint iLv);

private:
	_uint							m_iNumLevels = 0;
	class CFrustum*					m_pFrustum = nullptr;

	vector<class CDynamicPointLight*>*			m_DynamicPointLights = nullptr;
	vector<class CDynamicPointLight*>*			m_DeadDynamicPointLights = nullptr;

	vector<class CStaticPointLight*>*			m_StaticPointLights = nullptr;
	vector<class CStaticPointLight*>*			m_DeadStaticPointLights = nullptr;

	vector<class CDirLight*>*					m_DirLights = nullptr;
	vector<class CDirLight*>*					m_DeadDirLights = nullptr;

private:
	_float4x4*									m_Light[LIHGTNUM_END][LIHGTMATRIX_END];
	_float 										m_LightDis[LIHGTNUM_END];
	_float4										m_vPlayerPos;

public:
	virtual void Free() override;
};

END