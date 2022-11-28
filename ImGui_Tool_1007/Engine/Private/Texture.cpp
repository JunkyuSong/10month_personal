#include "..\Public\Texture.h"
#include "Shader.h"

CTexture::CTexture(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CComponent(pDevice, pContext)
{
	ZeroMemory(m_szName, sizeof(_tchar)*MAX_PATH);
}

CTexture::CTexture(const CTexture & rhs)
	: CComponent(rhs)
	, m_iNumTextures(rhs.m_iNumTextures)
	, m_SRVs(rhs.m_SRVs)
	, m_iHighNum(rhs.m_iHighNum)
	, m_iWidthNum(rhs.m_iWidthNum)
	, m_fSpriteSpeed(rhs.m_fSpriteSpeed)
	, m_eType(rhs.m_eType)
{
	for (auto& pSRV : m_SRVs)
		Safe_AddRef(pSRV);
	lstrcpy(m_szName, rhs.m_szName);
}

HRESULT CTexture::Initialize_Prototype(const _tchar * pTextureFilePath, _uint iNumTextures, _float _fSpeed)
{
	m_iNumTextures = iNumTextures;
	m_iWidthNum = 1;
	m_iHighNum = 1;
	m_fSpriteSpeed = _fSpeed;
	m_eType = TYPE_SINGLE;

	for (_uint i = 0; i < iNumTextures; ++i)
	{
		_tchar			szFullPath[MAX_PATH] = TEXT("");

		wsprintf(szFullPath, pTextureFilePath, i);

		_tchar			szDrive[MAX_PATH] = TEXT("");
		_tchar			szName[MAX_PATH] = TEXT("");
		_tchar			szExt[MAX_PATH] = TEXT("");

		_wsplitpath_s(szFullPath, szDrive, MAX_PATH, nullptr, 0, szName, MAX_PATH, szExt, MAX_PATH);
		
		HRESULT hr = 0;

		ID3D11ShaderResourceView*			pSRV = nullptr;		

		if (false == lstrcmp(TEXT(".dds"), szExt))
			hr = DirectX::CreateDDSTextureFromFile(m_pDevice, szFullPath, nullptr, &pSRV);

		else if (false == lstrcmp(TEXT(".tga"), szExt))
			hr = E_FAIL;
		
		else
			hr = DirectX::CreateWICTextureFromFile(m_pDevice, szFullPath, nullptr, &pSRV);

		if (FAILED(hr))
			return E_FAIL;
		lstrcpy(m_szName, szName);
		m_SRVs.push_back(pSRV);
	}
	
	return S_OK;
}

HRESULT CTexture::Initialize_Prototype(const _tchar * pTextureFilePath, _uint iWidthNum, _uint iHighNum, _float _fSpeed)
{
	m_iNumTextures = iWidthNum * iHighNum;
	m_iWidthNum = iWidthNum;
	m_iHighNum = iHighNum;
	m_fSpriteSpeed = _fSpeed;
	m_eType = TYPE_MULTI;

	for (_uint i = 0; i < 1; ++i)
	{
		_tchar			szFullPath[MAX_PATH] = TEXT("");

		wsprintf(szFullPath, pTextureFilePath, i);

		_tchar			szDrive[MAX_PATH] = TEXT("");
		_tchar			szName[MAX_PATH] = TEXT("");
		_tchar			szExt[MAX_PATH] = TEXT("");

		_wsplitpath_s(szFullPath, szDrive, MAX_PATH, nullptr, 0, szName, MAX_PATH, szExt, MAX_PATH);

		HRESULT hr = 0;

		ID3D11ShaderResourceView*			pSRV = nullptr;

		if (false == lstrcmp(TEXT(".dds"), szExt))
			hr = DirectX::CreateDDSTextureFromFile(m_pDevice, szFullPath, nullptr, &pSRV);

		else if (false == lstrcmp(TEXT(".tga"), szExt))
			hr = E_FAIL;

		else
			hr = DirectX::CreateWICTextureFromFile(m_pDevice, szFullPath, nullptr, &pSRV);

		if (FAILED(hr))
			return E_FAIL;
		lstrcpy(m_szName, szName);
		m_SRVs.push_back(pSRV);
	}

	return S_OK;
}

HRESULT CTexture::Initialize(void * pArg)
{
	return S_OK;
}

HRESULT CTexture::Set_SRV(CShader * pShader, const char * pConstantName, _uint iTextureIndex)
{
	if (iTextureIndex >= m_iNumTextures)
		return E_FAIL;

	return pShader->Set_ShaderResourceView(pConstantName, m_SRVs[iTextureIndex]);	

}

CTexture * CTexture::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const _tchar * pTextureFilePath, _uint iNumTextures, _float _fSpeed)
{
	CTexture*			pInstance = new CTexture(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pTextureFilePath, iNumTextures, _fSpeed)))
	{
		MSG_BOX(TEXT("Failed To Created : CTexture"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CTexture * CTexture::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const _tchar * pTextureFilePath, _uint iWidthNum, _uint iHighNum, _float _fSpeed)
{
	CTexture*			pInstance = new CTexture(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pTextureFilePath, iWidthNum, iHighNum, _fSpeed)))
	{
		MSG_BOX(TEXT("Failed To Created : CTexture"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CTexture::Clone(void * pArg)
{
	CTexture*			pInstance = new CTexture(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CTexture"));
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CTexture::Free()
{
	__super::Free();

	for (auto& pSRV : m_SRVs)
		Safe_Release(pSRV);

	m_SRVs.clear();
}
