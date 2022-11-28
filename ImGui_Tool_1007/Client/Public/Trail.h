#pragma once
#include "VIBuffer.h"

#include "Client_Defines.h"

BEGIN(Engine)
class CTransform;
class CTexture;
class CShader;
END

BEGIN(Client)
class CTrail :
	public CVIBuffer
{
public:
	enum TRAILOPTION {
		TRAILOPTION_DITORTION,
		TRAILOPTION_DEFAULT,
		TRAILOPTION_TEXTURE,
		TRAILOPTION_END
	};
public:
	struct TRAILINFO {
		TRAILPOS	_HighAndLow;
		_float4		_Color;
		TRAILOPTION _eOption = TRAILOPTION_DEFAULT;
		_tchar*		_szTexture = TEXT("../Bin/Resources/Textures/Trail/Trail_Default.png");
	};

	/*struct REALDATA {
		_float3		vPosition;
		_float2		vTexture;
	};*/
private:
	CTrail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTrail(const CTrail& rhs);
	virtual ~CTrail() = default;

public:
	virtual HRESULT			Initialize_Prototype() override;
	virtual HRESULT			Initialize(void* pArg) override;
	HRESULT Render();

	void Tick(const _float& _fTimeDelta, _matrix _matWeapon);

	void			TrailOn(_matrix _matWeapon);
	
	void			TrailOff() { m_bTrailOn = false; } 

	const _bool&	Get_On() { return m_bTrailOn; }

	TRAILPOS		Get_HighAndLow(_matrix _matWorld);
	TRAILPOS		Get_HighAndLow() { return m_CurWorldHighAndLow; };

	void			Set_Option(TRAILOPTION _ePass) { m_ePass = _ePass; }

	void			Set_Color(_float4 _Color) { m_Color = _Color; }

	void			Spline(_matrix _matWeapon);

public:
	static	CTrail*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent*		Clone(void* pArg);
	virtual void			Free() override;

private:
	//_float3					m_vPosition[22];
	//_float2					m_vTexture[22];

	VTXTRAIL				m_RealData[22];// temp[22];

	_float4					m_Color;

	TRAILPOS				m_HighAndLow;
	TRAILPOS				m_CurWorldHighAndLow;

	TRAILOPTION				m_ePass = TRAILOPTION_END;

	CShader*				m_pShaderCom;

	_float					m_fCurTime;
	_float					m_fMaxTIme;
	_bool					m_bTrailOn;
	_bool					m_bRealOn = true;

	_int					m_iVtxCount;

	_uint					m_iEndIndex;
	_uint					m_iCatmullRomCount = 4;
	_uint					m_iCatmullRomIndex[4] = { NULL };

	CTexture*				m_pTextureCom = nullptr;
	CTexture*				m_pNoiseTextureCom = nullptr;
	CTexture*				m_pAlphaTextureCom = nullptr;
};
END