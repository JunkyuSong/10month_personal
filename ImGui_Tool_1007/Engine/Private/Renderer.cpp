#include "..\Public\Renderer.h"
#include "GameObject.h"
#include "Target_Manager.h"
#include "Shader.h"
#include "VIBuffer_Rect.h"
#include "Light_Manager.h"
#include "PipeLine.h"
#include "Transform.h"
#include "GameInstance.h"
#include "RenderTarget.h"
#include "HDR_Mgr.h"

#include "Level_Manager.h"

CRenderer::CRenderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent(pDevice, pContext)
	, m_pTarget_Manager(CTarget_Manager::Get_Instance())
	, m_pLight_Manager(CLight_Manager::Get_Instance())
	, m_pHDRMgr(CHDR_Mgr::Get_Instance())
{
	Safe_AddRef(m_pLight_Manager);
	Safe_AddRef(m_pTarget_Manager);
}

HRESULT CRenderer::Initialize_Prototype()
{
	if (nullptr == m_pTarget_Manager)
		return E_FAIL;

	_uint		iNumViewport = 1;
	D3D11_VIEWPORT		ViewportDesc;

	m_pContext->RSGetViewports(&iNumViewport, &ViewportDesc);

	m_pHDRMgr->Initialize(ViewportDesc.Width, ViewportDesc.Height, m_pDevice, m_pContext);
	
	/* For.Target_BackBuffer */
	if (FAILED(m_pTarget_Manager->Add_RenderTarget(m_pDevice, m_pContext, TEXT("Target_BackBuffer"), ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R16G16B16A16_TYPELESS, &_float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* For.Target_BackBufferRTV */
	if (FAILED(m_pTarget_Manager->Add_RenderTarget(m_pDevice, m_pContext, TEXT("Target_BackBufferRTV"), ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R16G16B16A16_TYPELESS, &_float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* For.Target_BlendingBackBuffer */
	if (FAILED(m_pTarget_Manager->Add_RenderTarget(m_pDevice, m_pContext, TEXT("Target_BlendingBackBuffer"), ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_B8G8R8A8_UNORM, &_float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* For.Target_Diffuse */
	if (FAILED(m_pTarget_Manager->Add_RenderTarget(m_pDevice, m_pContext, TEXT("Target_Diffuse"), ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_B8G8R8A8_UNORM, &_float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* For.Target_Distortion */
	if (FAILED(m_pTarget_Manager->Add_RenderTarget(m_pDevice, m_pContext, TEXT("Target_Distortion"), ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_B8G8R8A8_UNORM, &_float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* For.Target_Normal */
	if (FAILED(m_pTarget_Manager->Add_RenderTarget(m_pDevice, m_pContext, TEXT("Target_Normal"), ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R16G16B16A16_UNORM, &_float4(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* For.Target_Depth */
	if (FAILED(m_pTarget_Manager->Add_RenderTarget(m_pDevice, m_pContext, TEXT("Target_Depth"), ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, &_float4(0.0f, 0.f, 0.f, 1.f))))
		return E_FAIL;

	/* For.Target_Shader */
	if (FAILED(m_pTarget_Manager->Add_RenderTarget(m_pDevice, m_pContext, TEXT("Target_Shade"), ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R16G16B16A16_UNORM, &_float4(0.0f, 0.f, 0.f, 1.f))))
		return E_FAIL;

	/* For.Target_Specular */
	if (FAILED(m_pTarget_Manager->Add_RenderTarget(m_pDevice, m_pContext, TEXT("Target_Specular"), ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R16G16B16A16_UNORM, &_float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* For.Target_Fog */
	if (FAILED(m_pTarget_Manager->Add_RenderTarget(m_pDevice, m_pContext, TEXT("Target_Fog"), ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R16G16B16A16_UNORM, &_float4(0.0f, 0.f, 0.f, 1.f))))
		return E_FAIL;

	/* For.Target_Alpha */
	if (FAILED(m_pTarget_Manager->Add_RenderTarget(m_pDevice, m_pContext, TEXT("Target_Alpha"), ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R8G8B8A8_UNORM, &_float4(0.0f, 0.f, 0.f, 1.f))))
		return E_FAIL;

	/* For.Target_AlphaDepth */
	if (FAILED(m_pTarget_Manager->Add_RenderTarget(m_pDevice, m_pContext, TEXT("Target_AlphaDepth"), ViewportDesc.Width, ViewportDesc.Height, DXGI_FORMAT_R32G32B32A32_FLOAT, &_float4(0.0f, 0.f, 0.f, 1.f))))
		return E_FAIL;

	_uint		iShadowMapCX = 8000;
	_uint		iShadowMapCY = 4500;
	if (FAILED(Make_Depth(iShadowMapCX, iShadowMapCY)))
		return E_FAIL;

	/* For.Target_TotalDepth */
	if (FAILED(m_pTarget_Manager->Add_RenderTarget(m_pDevice, m_pContext, TEXT("Target_ShadowDepth"), iShadowMapCX, iShadowMapCY, DXGI_FORMAT_R32G32B32A32_FLOAT, &_float4(1.f, 1.f, 1.f, 1.f))))
		return E_FAIL;

	/* For.MRT_Back */
	if (FAILED(m_pTarget_Manager->Add_MRT(TEXT("MRT_Back"), TEXT("Target_BackBuffer"))))
		return E_FAIL;

	/* For.MRT_Shadow */
	if (FAILED(m_pTarget_Manager->Add_MRT(TEXT("MRT_Shadow"), TEXT("Target_ShadowDepth"))))
		return E_FAIL;

	/* For.MRT_Deferred */
	if (FAILED(m_pTarget_Manager->Add_MRT(TEXT("MRT_Deferred"), TEXT("Target_Diffuse"))))
		return E_FAIL;
	if (FAILED(m_pTarget_Manager->Add_MRT(TEXT("MRT_Deferred"), TEXT("Target_Normal"))))
		return E_FAIL;
	if (FAILED(m_pTarget_Manager->Add_MRT(TEXT("MRT_Deferred"), TEXT("Target_Depth"))))
		return E_FAIL;
	if (FAILED(m_pTarget_Manager->Add_MRT(TEXT("MRT_Deferred"), TEXT("Target_Distortion"))))
		return E_FAIL;

	/* For.MRT_LightAcc */
	if (FAILED(m_pTarget_Manager->Add_MRT(TEXT("MRT_LightAcc"), TEXT("Target_Shade"))))
		return E_FAIL;
	if (FAILED(m_pTarget_Manager->Add_MRT(TEXT("MRT_LightAcc"), TEXT("Target_Specular"))))
		return E_FAIL;


	/* For.MRT_forHDR */
	if (FAILED(m_pTarget_Manager->Add_MRT(TEXT("MRT_forHDR"), TEXT("Target_BackBuffer"))))
		return E_FAIL;

	/* For.MRT_AfterHDR */
	if (FAILED(m_pTarget_Manager->Add_MRT(TEXT("MRT_AfterHDR"), TEXT("Target_BackBufferRTV"))))
		return E_FAIL;
	if (FAILED(m_pTarget_Manager->Add_MRT(TEXT("MRT_AfterHDR"), TEXT("Target_Depth"))))
		return E_FAIL;
	if (FAILED(m_pTarget_Manager->Add_MRT(TEXT("MRT_AfterHDR"), TEXT("Target_Distortion"))))
		return E_FAIL;

#ifdef _DEBUG
	
	if (FAILED(m_pTarget_Manager->Initialize_Debug(TEXT("Target_Diffuse"), 70.f, 70.f, 140.f, 140.f)))
		return E_FAIL;
	if (FAILED(m_pTarget_Manager->Initialize_Debug(TEXT("Target_Normal"), 70.f, 210.f, 140.f, 140.f)))
		return E_FAIL;
	if (FAILED(m_pTarget_Manager->Initialize_Debug(TEXT("Target_Depth"), 70.f, 350.f, 140.f, 140.f)))
		return E_FAIL;
	if (FAILED(m_pTarget_Manager->Initialize_Debug(TEXT("Target_Shade"), 210.f, 70.f, 140.f, 140.f)))
		return E_FAIL;
	if (FAILED(m_pTarget_Manager->Initialize_Debug(TEXT("Target_Specular"), 210.f, 210.f, 140.f, 140.f)))
		return E_FAIL;

	if (FAILED(m_pTarget_Manager->Initialize_Debug(TEXT("Target_ShadowDepth"), 210.f, 350.f, 140.f, 140.f)))
		return E_FAIL;

	if (FAILED(m_pTarget_Manager->Initialize_Debug(TEXT("Target_Distortion"), 70.f, 490.f, 140.f, 140.f)))
		return E_FAIL;

	m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_Deferred.hlsl"), VTXTEX_DECLARATION::Elements, VTXTEX_DECLARATION::iNumElements);
	if (nullptr == m_pShader)
		return E_FAIL;

	m_pVIBuffer = CVIBuffer_Rect::Create(m_pDevice, m_pContext);
	if (nullptr == m_pVIBuffer)
		return E_FAIL;

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixTranspose(XMMatrixOrthographicLH(ViewportDesc.Width, ViewportDesc.Height, 0.f, 1.f)));

#endif // _DEBUG
	m_pTextureCom = CTexture::Create(m_pDevice, m_pContext, TEXT("../Bin/Resources/Textures/TFX_Noise_01.dds"));
	if (nullptr == m_pTextureCom)
		return E_FAIL;
	//static_cast<CTexture*>(pGameInstance->Clone_Component(0, TEXT("Prototype_Component_Texture_Distortion")));

	return S_OK;
}

HRESULT CRenderer::Initialize(void * pArg)
{
	return S_OK;
}

HRESULT CRenderer::Add_RenderGroup(RENDERGROUP eRenderGroup, CGameObject * pGameObject)
{
	if (nullptr == pGameObject)
		return E_FAIL;

	m_RenderObjects[eRenderGroup].push_back(pGameObject);


	Safe_AddRef(pGameObject);

	return S_OK;
}

HRESULT CRenderer::Draw()
{
	if (nullptr == m_pTarget_Manager)
		return E_FAIL;

	m_pTarget_Manager->Clear_RTVs();

	m_pTarget_Manager->BackBuffer_Start(m_pContext, TEXT("Target_BackBuffer"),&m_pBackBuffer);

	if (FAILED(Render_Priority()))
		return E_FAIL;

	if (FAILED(Render_Shadow()))
		return E_FAIL;

	if (FAILED(Render_NonAlphaBlend()))
		return E_FAIL;

	if (FAILED(Render_Lights()))
		return E_FAIL;

	if (FAILED(Render_Blend()))
		return E_FAIL;

	

	if (FAILED(Render_PostProcessing()))
		return E_FAIL;



	if (FAILED(Render_NonLight()))
		return E_FAIL;
	
	if (FAILED(Render_AlphaBlend()))
		return E_FAIL;

	if (FAILED(Render_BackBuffer()))
		return E_FAIL;

	if (FAILED(Render_Fog()))
		return E_FAIL;

	if (FAILED(Render_Distortion()))
		return E_FAIL;






	if (FAILED(Render_UI()))
		return E_FAIL;

	

#ifdef _DEBUG


	if (FAILED(Render_Debug()))
		return E_FAIL;


#endif

	
	return S_OK;
}

HRESULT CRenderer::Add_DebugGroup(CComponent* pDebugCom)
{
	m_DebugObject.push_back(pDebugCom);

	Safe_AddRef(pDebugCom);

	return S_OK;
}

HRESULT CRenderer::Render_Priority()
{
	for (auto& pRenderObject : m_RenderObjects[RENDER_PRIORITY])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();

		Safe_Release(pRenderObject);
	}
	m_RenderObjects[RENDER_PRIORITY].clear();

	return S_OK;
}

HRESULT CRenderer::Render_Shadow()
{
	if (nullptr == m_pTarget_Manager)
		return E_FAIL;
	m_pContext->ClearDepthStencilView(m_pShadowDepthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	if (FAILED(m_pTarget_Manager->Begin_MRT(m_pContext, TEXT("MRT_Shadow"), m_pShadowDepthStencil, 8000.f, 4500.f)))
		return E_FAIL;	

	for (auto& pRenderObject : m_RenderObjects[RENDER_SHADOW])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render_Shadow();

		Safe_Release(pRenderObject);
	}
	m_RenderObjects[RENDER_SHADOW].clear();

	if (FAILED(m_pTarget_Manager->End_MRT(m_pContext)))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_NonAlphaBlend()
{
	if (nullptr == m_pTarget_Manager)
		return E_FAIL;

	if (FAILED(m_pTarget_Manager->Begin_MRT(m_pContext, TEXT("MRT_Deferred"))))
		return E_FAIL;

	for (auto& pRenderObject : m_RenderObjects[RENDER_NONALPHABLEND])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();

		Safe_Release(pRenderObject);
	}
	m_RenderObjects[RENDER_NONALPHABLEND].clear();

	if (FAILED(m_pTarget_Manager->End_MRT(m_pContext)))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_Lights()
{
	if (nullptr == m_pTarget_Manager)
		return E_FAIL;

	/* Target_Shader타겟에 빛 연산한 결과를 그린다. */
	if (FAILED(m_pTarget_Manager->Begin_MRT(m_pContext, TEXT("MRT_LightAcc"))))
		return E_FAIL;

	_float4x4			WorldMatrix;

	_uint				iNumViewport = 1;
	D3D11_VIEWPORT		ViewportDesc;

	m_pContext->RSGetViewports(&iNumViewport, &ViewportDesc);

	XMStoreFloat4x4(&WorldMatrix,
		XMMatrixTranspose(XMMatrixScaling(ViewportDesc.Width, ViewportDesc.Height, 0.f) * XMMatrixTranslation(0.0f, 0.0f, 0.f)));

	if (FAILED(m_pShader->Set_RawValue("g_WorldMatrix", &WorldMatrix, sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShader->Set_RawValue("g_ViewMatrix", &m_ViewMatrix, sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShader->Set_RawValue("g_ProjMatrix", &m_ProjMatrix, sizeof(_float4x4))))
		return E_FAIL;
	
	AUTOINSTANCE(CPipeLine, pPipeLine);

	_float4x4			ViewMatrixInv;
	_float4x4			ProjMatrixInv;

	XMStoreFloat4x4(&ViewMatrixInv, XMMatrixTranspose( XMMatrixInverse(nullptr, pPipeLine->Get_TransformMatrix(CPipeLine::D3DTS_VIEW))));
	XMStoreFloat4x4(&ProjMatrixInv, XMMatrixTranspose(XMMatrixInverse(nullptr, pPipeLine->Get_TransformMatrix(CPipeLine::D3DTS_PROJ))));

	if (FAILED(m_pShader->Set_RawValue("g_ViewMatrixInv", &ViewMatrixInv, sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShader->Set_RawValue("g_ProjMatrixInv", &ProjMatrixInv, sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShader->Set_RawValue("g_vCamPosition", &pPipeLine->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;

	if (FAILED(m_pTarget_Manager->Bind_SRV(TEXT("Target_Normal"), m_pShader, "g_NormalTexture")))
		return E_FAIL;
	if (FAILED(m_pTarget_Manager->Bind_SRV(TEXT("Target_Depth"), m_pShader, "g_DepthTexture")))
		return E_FAIL;

	if (FAILED(m_pTarget_Manager->Bind_SRV(TEXT("Target_ShadowDepth"), m_pShader, "g_ShadowDepthTexture")))
		return E_FAIL;

	m_pLight_Manager->Render(m_pShader, m_pVIBuffer);

	if (FAILED(m_pTarget_Manager->End_MRT(m_pContext)))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_Blend()
{
	_float4x4			WorldMatrix;

	_uint				iNumViewport = 1;
	D3D11_VIEWPORT		ViewportDesc;

	m_pContext->RSGetViewports(&iNumViewport, &ViewportDesc);

	XMStoreFloat4x4(&WorldMatrix,
		XMMatrixTranspose(XMMatrixScaling(ViewportDesc.Width, ViewportDesc.Height, 0.f) * XMMatrixTranslation(0.0f, 0.0f, 0.f)));

	if (FAILED(m_pShader->Set_RawValue("g_WorldMatrix", &WorldMatrix, sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShader->Set_RawValue("g_ViewMatrix", &m_ViewMatrix, sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShader->Set_RawValue("g_ProjMatrix", &m_ProjMatrix, sizeof(_float4x4))))
		return E_FAIL;
	AUTOINSTANCE(CGameInstance, _pInstance);
	AUTOINSTANCE(CLevel_Manager, _pLv);
	//DIRLIGHTDESC* _DirLightDesc = _pInstance->Get_DirLightDesc(_pLv->Get_CurLv(), 0);
	_float4x4* _LightView = nullptr;
	_float4x4* _LightProj = nullptr;

	_bool		_bView = false;
	
	_LightView = _pInstance->Get_LightMatrix(_pLv->Get_CurLv(), CLight_Manager::LIGHT_FIRST, CLight_Manager::LIGHT_VIEW);
	_LightProj = _pInstance->Get_LightMatrix(_pLv->Get_CurLv(), CLight_Manager::LIGHT_FIRST, CLight_Manager::LIGHT_PROJ);
	if (_LightView != nullptr)
	{
		if (FAILED(m_pShader->Set_RawValue("g_LightView", _LightView, sizeof(_float4x4))))
			return E_FAIL;
	}
	else
	{
		_LightView = new _float4x4;
		XMStoreFloat4x4(_LightView, XMMatrixIdentity());
		_LightProj = new _float4x4;
		XMStoreFloat4x4(_LightProj, XMMatrixIdentity());
		_bView = true;
		if (FAILED(m_pShader->Set_RawValue("g_LightView", _LightView, sizeof(_float4x4))))
			return E_FAIL;
	}
	
	AUTOINSTANCE(CPipeLine, _pPipeLine);

	_float4x4			ViewMatrixInv;
	_float4x4			ProjMatrixInv;
	XMStoreFloat4x4(&ViewMatrixInv, XMMatrixTranspose(XMMatrixInverse(nullptr, _pPipeLine->Get_TransformMatrix(CPipeLine::D3DTS_VIEW))));
	XMStoreFloat4x4(&ProjMatrixInv, XMMatrixTranspose(XMMatrixInverse(nullptr, _pPipeLine->Get_TransformMatrix(CPipeLine::D3DTS_PROJ))));

	if (FAILED(m_pShader->Set_RawValue("g_ViewMatrixInv", &ViewMatrixInv, sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShader->Set_RawValue("g_ProjMatrixInv", &ProjMatrixInv, sizeof(_float4x4))))
		return E_FAIL;



	if (FAILED(m_pShader->Set_RawValue("g_LProjMatrix", _LightProj, sizeof(_float4x4))))
		return E_FAIL;

	if (FAILED(m_pTarget_Manager->Bind_SRV(TEXT("Target_Diffuse"), m_pShader, "g_DiffuseTexture")))
		return E_FAIL;

	if (FAILED(m_pTarget_Manager->Bind_SRV(TEXT("Target_Shade"), m_pShader, "g_ShadeTexture")))
		return E_FAIL;

	if (FAILED(m_pTarget_Manager->Bind_SRV(TEXT("Target_Specular"), m_pShader, "g_SpecularTexture")))
		return E_FAIL;

	if (FAILED(m_pTarget_Manager->Bind_SRV(TEXT("Target_ShadowDepth"), m_pShader, "g_ShadowDepthTexture")))
		return E_FAIL;

	if (FAILED(m_pTarget_Manager->Bind_SRV(TEXT("Target_Depth"), m_pShader, "g_DepthTexture")))
		return E_FAIL;

	m_pShader->Begin(3);

	m_pVIBuffer->Render();

	if (_bView)
	{
		Safe_Delete(_LightView);
		Safe_Delete(_LightProj);
	}

	return S_OK;
}

HRESULT CRenderer::Render_NonLight()
{
	if (nullptr == m_pTarget_Manager)
		return E_FAIL;

	/*if (FAILED(m_pTarget_Manager->AddBinding_RTV(m_pContext, TEXT("Target_Depth"), 1)))
		return E_FAIL;*/
	
	for (auto& pRenderObject : m_RenderObjects[RENDER_NONLIGHT])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();

		Safe_Release(pRenderObject);
	}
	m_RenderObjects[RENDER_NONLIGHT].clear();

	//if (FAILED(m_pTarget_Manager->End_MRT(m_pContext)))
	//	return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_AlphaBlend()
{
	if (nullptr == m_pTarget_Manager)
		return E_FAIL;
	//_uint iDepthStencil = 0;
	if (FAILED(m_pTarget_Manager->Begin_MRT(m_pContext, TEXT("MRT_AfterHDR"))))
		return E_FAIL;
	_uint iDepth = 0;
	/*if (FAILED(m_pTarget_Manager->AddBinding_RTV(m_pContext, TEXT("Target_Depth"), 1)))
		return E_FAIL;
	++iDepth;*/
	/*if (FAILED(m_pTarget_Manager->AddBinding_RTV(m_pContext, TEXT("Target_Distortion"), 2)))
		return E_FAIL;
	++iDepth;*/

	m_RenderObjects[RENDER_ALPHABLEND].sort([](CGameObject* pSour, CGameObject* pDest)
	{
		return pSour->Get_CamDistance() > pDest->Get_CamDistance();
	});

	for (auto& pRenderObject : m_RenderObjects[RENDER_ALPHABLEND])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();

		Safe_Release(pRenderObject);
	}
	m_RenderObjects[RENDER_ALPHABLEND].clear();

	if (FAILED(m_pTarget_Manager->End_MRT(m_pContext, iDepth)))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_Distortion()
{
	_uint iDepthStencil = 0;
	

	if (FAILED(m_pTarget_Manager->Bind_SRV(TEXT("Target_BackBufferRTV"), m_pShader, "g_DiffuseTexture")))
		return E_FAIL;
	if (FAILED(m_pTarget_Manager->Bind_SRV(TEXT("Target_Distortion"), m_pShader, "g_DistortionTexture")))
		return E_FAIL;

	//if (FAILED(m_pTarget_Manager->AddBinding_RTV(m_pContext, TEXT("Target_BackBuffer"), 1)))
	//	return E_FAIL;
	//++iDepthStencil;
	
	
	if (FAILED(m_pTextureCom->Set_SRV(m_pShader, "g_DistortionTexture_Bump")))
		return E_FAIL;
	m_fTick += CGameInstance::Get_Instance()->Get_TimeDelta(TEXT("Timer_Main")) / CGameInstance::Get_Instance()->Get_TimeSpeed(TEXT("Timer_Main"));
	if (FAILED(m_pShader->Set_RawValue("g_fTick", &m_fTick, sizeof(_float))))
		return E_FAIL;

	_float4x4			WorldMatrix;

	_uint				iNumViewport = 1;
	D3D11_VIEWPORT		ViewportDesc;

	m_pContext->RSGetViewports(&iNumViewport, &ViewportDesc);

	XMStoreFloat4x4(&WorldMatrix,
		XMMatrixTranspose(XMMatrixScaling(ViewportDesc.Width, ViewportDesc.Height, 0.f) * XMMatrixTranslation(0.0f, 0.0f, 0.f)));

	if (FAILED(m_pShader->Set_RawValue("g_WorldMatrix", &WorldMatrix, sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShader->Set_RawValue("g_ViewMatrix", &m_ViewMatrix, sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShader->Set_RawValue("g_ProjMatrix", &m_ProjMatrix, sizeof(_float4x4))))
		return E_FAIL;

	m_pShader->Begin(5);

	m_pVIBuffer->Render();

	/*if (FAILED(m_pTarget_Manager->End_MRT(m_pContext, iDepthStencil)))
		return E_FAIL;*/

	return S_OK;
}

HRESULT CRenderer::Render_PostProcessing()
{

	//겟 해주고 -> 비긴
	if (FAILED(m_pTarget_Manager->Begin_MRT(m_pContext, TEXT("MRT_forHDR"))))
		return E_FAIL;

	_float4x4			WorldMatrix;

	_uint				iNumViewport = 1;
	D3D11_VIEWPORT		ViewportDesc;

	m_pContext->RSGetViewports(&iNumViewport, &ViewportDesc);

	XMStoreFloat4x4(&WorldMatrix,
		XMMatrixTranspose(XMMatrixScaling(ViewportDesc.Width, ViewportDesc.Height, 0.f) * XMMatrixTranslation(0.0f, 0.0f, 0.f)));

	_float4x4 _Matrix[3];
	_Matrix[0] = WorldMatrix;
	_Matrix[1] = m_ViewMatrix;
	_Matrix[2] = m_ProjMatrix;

	m_pHDRMgr->PostProcessing(m_pTarget_Manager->Find_RenderTarget(TEXT("Target_BackBuffer"))->Get_SRV(), m_pTarget_Manager->Find_RenderTarget(TEXT("Target_BackBufferRTV"))->Get_RTV()
		,m_pVIBuffer
		, _Matrix);

	//위에서 겟 한걸로 셋 해주고.->엔드 해도 되지 않을까
	if (FAILED(m_pTarget_Manager->End_MRT(m_pContext)))
		return E_FAIL;
/*
	_uint iDepthStencil = 0;


	if (FAILED(m_pTarget_Manager->Bind_SRV(TEXT("Target_BackBufferRTV"), m_pShader, "g_DiffuseTexture")))
		return E_FAIL;


	m_pContext->RSGetViewports(&iNumViewport, &ViewportDesc);

	XMStoreFloat4x4(&WorldMatrix,
		XMMatrixTranspose(XMMatrixScaling(ViewportDesc.Width, ViewportDesc.Height, 0.f) * XMMatrixTranslation(0.0f, 0.0f, 0.f)));

	if (FAILED(m_pShader->Set_RawValue("g_WorldMatrix", &WorldMatrix, sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShader->Set_RawValue("g_ViewMatrix", &m_ViewMatrix, sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShader->Set_RawValue("g_ProjMatrix", &m_ProjMatrix, sizeof(_float4x4))))
		return E_FAIL;

	if (FAILED(m_pShader->Begin(4)))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Render()))
		return E_FAIL;
*/

	return S_OK;
}

HRESULT CRenderer::Render_Fog()
{
	if (FAILED(m_pTarget_Manager->Bind_SRV(TEXT("Target_BackBufferRTV"), m_pShader, "g_DiffuseTexture")))
		return E_FAIL;
	if (FAILED(m_pTarget_Manager->Bind_SRV(TEXT("Target_Depth"), m_pShader, "g_DepthTexture")))
		return E_FAIL;

	_float4x4			WorldMatrix;

	_uint				iNumViewport = 1;
	D3D11_VIEWPORT		ViewportDesc;

	m_pContext->RSGetViewports(&iNumViewport, &ViewportDesc);

	XMStoreFloat4x4(&WorldMatrix,
		XMMatrixTranspose(XMMatrixScaling(ViewportDesc.Width, ViewportDesc.Height, 0.f) * XMMatrixTranslation(0.0f, 0.0f, 0.f)));

	if (FAILED(m_pShader->Set_RawValue("g_WorldMatrix", &WorldMatrix, sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShader->Set_RawValue("g_ViewMatrix", &m_ViewMatrix, sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShader->Set_RawValue("g_ProjMatrix", &m_ProjMatrix, sizeof(_float4x4))))
		return E_FAIL;

	CPipeLine*			pPipeLine = GET_INSTANCE(CPipeLine);

	_float4x4			ViewMatrixInv;
	_float4x4			ProjMatrixInv;

	XMStoreFloat4x4(&ViewMatrixInv, XMMatrixTranspose(XMMatrixInverse(nullptr, pPipeLine->Get_TransformMatrix(CPipeLine::D3DTS_VIEW))));
	XMStoreFloat4x4(&ProjMatrixInv, XMMatrixTranspose(XMMatrixInverse(nullptr, pPipeLine->Get_TransformMatrix(CPipeLine::D3DTS_PROJ))));

	if (FAILED(m_pShader->Set_RawValue("g_ViewMatrixInv", &ViewMatrixInv, sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShader->Set_RawValue("g_ProjMatrixInv", &ProjMatrixInv, sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShader->Set_RawValue("g_vCamPosition", &pPipeLine->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;

	RELEASE_INSTANCE(CPipeLine);

	m_pShader->Begin(6);

	m_pVIBuffer->Render();

	return S_OK;
}

HRESULT CRenderer::Render_UI()
{
	for (auto& pRenderObject : m_RenderObjects[RENDER_UI])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();

		Safe_Release(pRenderObject);
	}
	m_RenderObjects[RENDER_UI].clear();

	return S_OK;
}

HRESULT CRenderer::Render_BackBuffer()
{

	m_pTarget_Manager->BackBuffer_End(m_pContext, &m_pBackBuffer);

	//디퓨즈로 넣고 그걸 그대로 그리면 됨
	if (FAILED(m_pTarget_Manager->Bind_SRV(TEXT("Target_BackBufferRTV"), m_pShader, "g_DiffuseTexture")))
		return E_FAIL;

	_float4x4			WorldMatrix;

	_uint				iNumViewport = 1;
	D3D11_VIEWPORT		ViewportDesc;

	m_pContext->RSGetViewports(&iNumViewport, &ViewportDesc);

	XMStoreFloat4x4(&WorldMatrix,
		XMMatrixTranspose(XMMatrixScaling(ViewportDesc.Width, ViewportDesc.Height, 0.f) * XMMatrixTranslation(0.0f, 0.0f, 0.f)));

	if (FAILED(m_pShader->Set_RawValue("g_WorldMatrix", &WorldMatrix, sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShader->Set_RawValue("g_ViewMatrix", &m_ViewMatrix, sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShader->Set_RawValue("g_ProjMatrix", &m_ProjMatrix, sizeof(_float4x4))))
		return E_FAIL;

	m_pShader->Begin(4);

	m_pVIBuffer->Render();

	return S_OK;
}

#ifdef _DEBUG
HRESULT CRenderer::Render_Debug()
{
	m_pShader->Set_RawValue("g_ViewMatrix", &m_ViewMatrix, sizeof(_float4x4));
	m_pShader->Set_RawValue("g_ProjMatrix", &m_ProjMatrix, sizeof(_float4x4));

	m_pTarget_Manager->Render_Debug(TEXT("MRT_Deferred"), m_pVIBuffer, m_pShader);
	m_pTarget_Manager->Render_Debug(TEXT("MRT_LightAcc"), m_pVIBuffer, m_pShader);
	m_pTarget_Manager->Render_Debug(TEXT("MRT_Shadow"), m_pVIBuffer, m_pShader);

	for (auto& pDebugCom : m_DebugObject)
	{
		pDebugCom->Render();
		Safe_Release(pDebugCom);
	}

	m_DebugObject.clear();

	return S_OK;
}
#endif // _DEBUG

HRESULT CRenderer::Make_Depth(_uint iShadowMapCX, _uint iShadowMapCY)
{
	if (nullptr == m_pDevice)
		return E_FAIL;

	ID3D11Texture2D*		pDepthStencilTexture = nullptr;

	D3D11_TEXTURE2D_DESC	TextureDesc;
	ZeroMemory(&TextureDesc, sizeof(D3D11_TEXTURE2D_DESC));

	TextureDesc.Width = iShadowMapCX;
	TextureDesc.Height = iShadowMapCY;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = 0;

	if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &pDepthStencilTexture)))
		return E_FAIL;

	if (FAILED(m_pDevice->CreateDepthStencilView(pDepthStencilTexture, nullptr, &m_pShadowDepthStencil)))
		return E_FAIL;


	Safe_Release(pDepthStencilTexture);

	return S_OK;
}

CRenderer * CRenderer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CRenderer*			pInstance = new CRenderer(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CRenderer"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CRenderer::Clone(void * pArg)
{
	AddRef();

	return this;
}

void CRenderer::Free()
{
	__super::Free();
	Safe_Release(m_pShadowDepthStencil);

	for (auto& List : m_RenderObjects)
	{
		for (auto& pGameObject : List)
		{
			Safe_Release(pGameObject);
		}
		List.clear();
	}
	Safe_Release(m_pBackBuffer);
#ifdef _DEBUG
	Safe_Release(m_pShader);
	Safe_Release(m_pVIBuffer);
#endif // _DEBUG

	Safe_Release(m_pLight_Manager);
	Safe_Release(m_pTarget_Manager);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pHDRMgr);

	CHDR_Mgr::Destroy_Instance();
}
