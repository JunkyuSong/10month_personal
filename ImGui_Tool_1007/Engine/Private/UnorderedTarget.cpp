#include "..\Public\UnorderedTarget.h"
#include "VIBuffer_Rect.h"
#include "Shader.h"

CUnorderedTarget::CUnorderedTarget(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: m_pDevice(pDevice)
	, m_pContext(pContext)
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CUnorderedTarget::Initialize(_uint iSizeX, _uint iSizeY, DXGI_FORMAT eFormat, const _float4 * pClearColor)
{
	m_vClearColor = *pClearColor;

	D3D11_TEXTURE2D_DESC	TextureDesc;
	ZeroMemory(&TextureDesc, sizeof(D3D11_TEXTURE2D_DESC));

	TextureDesc.Width = iSizeX;
	TextureDesc.Height = iSizeY;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = eFormat;

	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	/*if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &m_pTexture2D)))
		return E_FAIL;


	D3D11_BUFFER_DESC		tDesc = {};

	tDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	tDesc.StructureByteStride = sizeof(float);
	tDesc.ByteWidth = iWidth * tDesc.StructureByteStride;
	tDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	if (FAILED(m_pDevice->CreateBuffer(&tDesc, NULL, &m_pBuffer)))
		return false;*/

	D3D11_UNORDERED_ACCESS_VIEW_DESC	tUAVDesc = {};

	tUAVDesc.Format = DXGI_FORMAT_UNKNOWN;
	tUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	tUAVDesc.Buffer.NumElements = iWidth;
	if (FAILED(m_pDevice->CreateUnorderedAccessView(m_pBuffer, &tUAVDesc, &m_pURV)))
		return false;


	return S_OK;
}

HRESULT CUnorderedTarget::Clear()
{
	if (nullptr == m_pContext)
		return E_FAIL;

	m_pContext->ClearUnorderedAccessViewFloat(m_pURV, (_float*)&m_vClearColor);

	return S_OK;
}

HRESULT CUnorderedTarget::Bind_SRV(CShader * pShader, const char * pConstantName)
{
	//return pShader->Set_ShaderResourceView(pConstantName, m_pURV);
}

#ifdef _DEBUG

HRESULT CUnorderedTarget::Initialize_Debug(_float fX, _float fY, _float fSizeX, _float fSizeY)
{
	_uint		iNumViewport = 1;
	D3D11_VIEWPORT		ViewportDesc;
	m_pContext->RSGetViewports(&iNumViewport, &ViewportDesc);

	XMStoreFloat4x4(&m_WorldMatrix,
		XMMatrixTranspose(XMMatrixScaling(fSizeX, fSizeY, 0.f) * XMMatrixTranslation(fX - ViewportDesc.Width * 0.5f, -fY + ViewportDesc.Height * 0.5f, 0.f)));

	return S_OK;
}


HRESULT CUnorderedTarget::Render_Debug(CShader* pShader, CVIBuffer * pVIBuffer)
{
	/*pShader->Set_RawValue("g_WorldMatrix", &m_WorldMatrix, sizeof(_float4x4));

	pShader->Set_un("g_DiffuseTexture", m_pURV);

	pShader->Begin(0);

	return pVIBuffer->Render();*/
}
#endif // _DEBUG

CUnorderedTarget * CUnorderedTarget::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, _uint iSizeX, _uint iSizeY, DXGI_FORMAT eFormat, const _float4 * pClearColor)
{
	CUnorderedTarget*		pInstance = new CUnorderedTarget(pDevice, pContext);

	if (FAILED(pInstance->Initialize(iSizeX, iSizeY, eFormat, pClearColor)))
	{
		MSG_BOX(TEXT("Failed To Created : CUnorderedTarget"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUnorderedTarget::Free()
{
	Safe_Release(m_pTexture2D);
	Safe_Release(m_pURV);
	Safe_Release(m_pRTV);

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
