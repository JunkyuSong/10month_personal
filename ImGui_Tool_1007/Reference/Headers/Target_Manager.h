#pragma once

#include "Base.h"

BEGIN(Engine)

class CTarget_Manager final : public CBase
{
	DECLARE_SINGLETON(CTarget_Manager)
public:
	CTarget_Manager();
	virtual ~CTarget_Manager() = default;

public:	
	HRESULT Add_RenderTarget(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pTargetTag, _uint iSizeX, _uint iSizeY, DXGI_FORMAT eFormat, const _float4* pClearColor);
	HRESULT Add_MRT(const _tchar* pMRTTag, const _tchar* pTargetTag);

public:
	HRESULT Bind_SRV(const _tchar* pTargetTag, class CShader* pShader, const char* pConstantName);

	/* 기존에 있던 렌더타겟을 빼고, 지정한 렌더타겟들(mrt)을 장치에 순서대로 바인딩한다. */
	HRESULT Begin_MRT(ID3D11DeviceContext* pContext, const _tchar* pMRTTag);
	HRESULT Begin_MRT(ID3D11DeviceContext* pContext, const _tchar* pMRTTag, ID3D11DepthStencilView* _pDepthStencil, _float fWidth, _float fHeight);

	/* 원래 상태로 복구한다.(BackBuffer를 장치에 셋한다.) */
	HRESULT End_MRT(ID3D11DeviceContext* pContext, _uint iDepthStencil = 1);

	HRESULT	AddBinding_RTV(ID3D11DeviceContext* pContext, const _tchar* pRTVTag, _uint _iIndex);

	HRESULT	Clear_RTVs();

	HRESULT	BackBuffer_Start(ID3D11DeviceContext * pContext, const _tchar * pBackBufferTag, ID3D11RenderTargetView** _pBackBuffer);

	HRESULT	BackBuffer_End(ID3D11DeviceContext * pContext, ID3D11RenderTargetView** _pBackBuffer);

#ifdef _DEBUG
public:	
	HRESULT Initialize_Debug(const _tchar* pTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY);
	HRESULT Render_Debug(const _tchar* pMRTTag, class CVIBuffer* pVIBuffer, class CShader* pShader);
#endif // _DEBUG

private: // 이번 프레임에 사용한 렌더타겟들 : 프레임의 시작에서 비운다
	list<class CRenderTarget*>							m_UsingTargets;


private: /* 생성한 렌더타겟들을 전체 다 모아놓는다. */
	map<const _tchar*, class CRenderTarget*>			m_RenderTargets;
	typedef map<const _tchar*, class CRenderTarget*>	RENDERTARGETS;

private: /* 동시에 바인딩되어야할 렌더타겟들을 LIST로 묶어놓는다. (Diffuse + Normal + Depth, Shader + Specular) */
	map<const _tchar*, list<class CRenderTarget*>>			m_MRTs;
	typedef map<const _tchar*, list<class CRenderTarget*>>	MRTS;

private:
	list<ID3D11RenderTargetView*>					m_pClearRenderTargets;
	ID3D11RenderTargetView*							m_pOldRenderTargets[8] = { nullptr };
	ID3D11DepthStencilView*							m_pOldDepthStencil = nullptr;

public:
	class CRenderTarget* Find_RenderTarget(const _tchar* pTargetTag);
	list<class CRenderTarget*>* Find_MRT(const _tchar* pMRTTag);

public:
	virtual void Free() override;
};

END