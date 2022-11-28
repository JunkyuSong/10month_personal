#pragma once

#include "Base.h"

BEGIN(Engine)
class ENGINE_DLL CHDR_Mgr : public CBase
{

	DECLARE_SINGLETON(CHDR_Mgr)

private:
	CHDR_Mgr() = default;

	~CHDR_Mgr() = default;

public:
	HRESULT Initialize(const _uint& width, const _uint& height, ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

	// Entry point for post processing
	void PostProcessing(class CRenderTarget* _pRenderTarget, class CVIBuffer* pVIBuffer, _float4x4* _Matrix);
	void PostProcessing(ID3D11ShaderResourceView* _pSRV, ID3D11RenderTargetView* _pRTV, class CVIBuffer* pVIBuffer, _float4x4* _Matrix);

	inline void SetParameters(const _float& fMiddleGrey, const _float& fWhite) { m_fMiddleGrey = fMiddleGrey; m_fWhite = fWhite; }

	_float* Get_White() { return &m_fWhite; }
	_float* Get_Grey() { return &m_fMiddleGrey; }

private:
	HRESULT CreateComputeShader(PWCHAR strPath, D3D10_SHADER_MACRO* pMacros, char* strEntryPoint, char* strProfile, DWORD dwShaderFlags, ID3D11ComputeShader** _ppComputeShader);

private:
	// Down scale the full size HDR image
	void DownScale(ID3D11ShaderResourceView* pHDRSRV);

	// Final pass that composites all the post processing calculations
	void FinalPass(ID3D11ShaderResourceView* pHDRSRV, class CVIBuffer* pVIBuffer);

	// 1D intermediate storage for the down scale operation
	ID3D11Buffer* m_pDownScale1DBuffer = nullptr;
	ID3D11UnorderedAccessView* m_pDownScale1DUAV = nullptr;
	ID3D11ShaderResourceView* m_pDownScale1DSRV = nullptr;

private:// Average luminance
	ID3D11Buffer* m_pAvgLumBuffer = nullptr;
	ID3D11UnorderedAccessView* m_pAvgLumUAV = nullptr;
	ID3D11ShaderResourceView* m_pAvgLumSRV = nullptr;

	_uint m_iWidth = 0;
	_uint m_iHeight = 0;
	_uint m_iDownScaleGroups = 0;
	_float m_fMiddleGrey = 1.f;
	_float m_fWhite = 1.2f;

	typedef struct
	{
		_uint iWidth;
		_uint iHeight;
		_uint iTotalPixels;
		_uint iGroupSize;
	} TDOWNSCALE_DESC;

	ID3D11Buffer* m_pDownScale = nullptr;

	typedef struct
	{
		_float fMiddleGrey;
		_float fLumWhiteSqr;
		_uint pad[2];
	} TFinalPass_DESC;

	ID3D11Buffer* m_pFinalPass = nullptr;

private:// Shaders
	ID3D11ComputeShader* m_pDownScaleFirstPassCS = nullptr;
	ID3D11ComputeShader* m_pDownScaleSecondPassCS = nullptr;
	class CShader*      m_pFullScreenShaderCom = nullptr;

	ID3D11Device*            m_pDevice = nullptr;
	ID3D11DeviceContext*      m_pContext = nullptr;

public:
	// CBase을(를) 통해 상속됨
	virtual void Free() override;
};

END