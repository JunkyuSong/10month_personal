#include "InstancingModel.h"

#include "InstancingMesh.h"

#include "Texture.h"

#include "PipeLine.h"

#include "Component_Manager.h"
#include "Frustum.h"


CInstancingModel::CInstancingModel(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CModel(pDevice, pContext)
{
}

CInstancingModel::CInstancingModel(const CInstancingModel & rhs)
	: CModel(rhs)
	, m_Meshes(rhs.m_Meshes)
	, m_tModel(rhs.m_tModel)
{
	for (auto& pMeshContainer : m_Meshes)
		Safe_AddRef(pMeshContainer);
}

HRESULT CInstancingModel::Initialize_Prototype(const char * pModelFilePath, const char * pModelFileName, _uint iNumInstance, vector<_float4x4>* matWorld)
{
	m_isCloned = false;

	char szExt[MAX_PATH] = "";

	_splitpath_s(pModelFileName, nullptr, 0, nullptr, 0, nullptr, 0, szExt, MAX_PATH);

	if (!strcmp(szExt, ".dat"))
	{
		m_datLoad = true;
		return Load_Dat(pModelFilePath, pModelFileName, iNumInstance, matWorld);
	}

	ZeroMemory(&m_tModel, sizeof(TMODEL));
	m_tModel.bAnim = false;
	strcpy_s(m_tModel.Name, pModelFileName);
	strcpy_s(m_tModel.Path, pModelFilePath);
	char		szFullPath[MAX_PATH] = "";

	strcpy_s(szFullPath, pModelFilePath);
	strcat_s(szFullPath, pModelFileName);

	_uint		iFlag = 0;


	iFlag |= aiProcess_PreTransformVertices | aiProcess_ConvertToLeftHanded | aiProcess_CalcTangentSpace;
	//	iFlag |= aiProcess_ConvertToLeftHanded | aiProcess_CalcTangentSpace;

	m_pAIScene = m_Importer.ReadFile(szFullPath, iFlag);

	if (nullptr == m_pAIScene)
		return E_FAIL;

	/* 모델을 구성하는 메시들을 만든다. */
	if (FAILED(Ready_MeshContainers(iNumInstance, matWorld)))
		return E_FAIL;

	// 텍스쳐들을 불러온다
	if (FAILED(Ready_Materials(pModelFilePath, &(m_tModel.AllMaterials))))
		return E_FAIL;


	return S_OK;
}

HRESULT CInstancingModel::Initialize(void * pArg)
{
	m_isCloned = true;
	return S_OK;
}

HRESULT CInstancingModel::Render(_uint _iMeshIndex)
{
	m_Meshes[_iMeshIndex]->Render();
	return S_OK;
}

_bool CInstancingModel::Picking(CTransform * pTransform, _vector & pOut)
{
	_vector	_vFinalPos{ 0.f,0.f,0.f,1.f };
	_float	_vFinalDis = 10000.f;

	_float	_vCurDis = 0.f;

	_bool	_bPicking = false;

	for (auto& _Mesh : m_Meshes)
	{
		if (_Mesh->Picking(pTransform, pOut))
		{
			
			_vCurDis = XMVectorGetX(XMVector3Length(pOut - XMLoadFloat4(&(CPipeLine::Get_Instance()->Get_CamPosition()))));
			if (_vCurDis < _vFinalDis)
			{
				_vFinalDis = _vCurDis;
				_vFinalPos = pOut;
			}
			_bPicking = true;

		}
			
	}
	pOut = _vFinalPos;
	return _bPicking;
}

void CInstancingModel::Culling(_float fSize)
{
	for (auto& iter : m_Meshes)
	{
		iter->Culling(fSize);
	}
}


HRESULT CInstancingModel::Ready_MeshContainers(_uint iNumInstance, vector<_float4x4>* matWorld)
{
	m_iNumMeshes = m_pAIScene->mNumMeshes;
	m_tModel.NumMeshes = m_iNumMeshes;
	
	m_tModel.tMeshes = new TCONTAINER[m_tModel.NumMeshes];

 
	{
		for (_uint i = 0; i < m_iNumMeshes; ++i)
		{
			CInstancingMesh*		pMeshContainer = CInstancingMesh::Create(m_pDevice, m_pContext, m_pAIScene->mMeshes[i], &(m_tModel.tMeshes[i]), iNumInstance, matWorld);
			if (nullptr == pMeshContainer)
				return E_FAIL;

			m_Meshes.push_back(pMeshContainer);
		}
	}

 	

	return S_OK;
}

HRESULT CInstancingModel::Load_Dat(const char * pModelFilePath, const char * pModelFileName, _uint iNumInstance, vector<_float4x4>* matWorld)
{
	char szFullPath[MAX_PATH] = ""; //여기에 넣을 예정



	strcpy_s(szFullPath, pModelFilePath);
	//파일 경로 넣고
	strcat_s(szFullPath, pModelFileName);
	//파일 이름 넣고


	_tchar			szWideFullPath[MAX_PATH] = TEXT("");

	MultiByteToWideChar(CP_ACP, 0, szFullPath, strlen(szFullPath), szWideFullPath, MAX_PATH);

	HANDLE		hFile = CreateFile(szWideFullPath,
		GENERIC_READ,
		NULL,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);



	DWORD		dwByte = 0;

	ReadFile(hFile, &(m_tModel.bAnim), sizeof(_bool), &dwByte, nullptr);
	ReadFile(hFile, &(m_tModel.Path), sizeof(char) * 260, &dwByte, nullptr);
	ReadFile(hFile, &(m_tModel.Name), sizeof(char) * 260, &dwByte, nullptr);
	ReadFile(hFile, &(m_tModel.NumMeshes), sizeof(int), &dwByte, nullptr);
	m_tModel.tMeshes = new TCONTAINER[m_tModel.NumMeshes];
	for (int i = 0; i < m_tModel.NumMeshes; ++i)
	{
		ReadFile(hFile, &(m_tModel.tMeshes[i].szName), sizeof(char) * 260, &dwByte, nullptr);
		ReadFile(hFile, &(m_tModel.tMeshes[i].iIndex), sizeof(int), &dwByte, nullptr);

		ReadFile(hFile, &(m_tModel.tMeshes[i].NumVertices), sizeof(int), &dwByte, nullptr);
		m_tModel.tMeshes[i].pVertices = new VTXMODEL[m_tModel.tMeshes[i].NumVertices];
		for (int j = 0; j < m_tModel.tMeshes[i].NumVertices; ++j)
		{
			ReadFile(hFile, &(m_tModel.tMeshes[i].pVertices[j]), sizeof(VTXMODEL), &dwByte, nullptr);
		}

		ReadFile(hFile, &(m_tModel.tMeshes[i].NumFaces), sizeof(int), &dwByte, nullptr);
		m_tModel.tMeshes[i].pIndices = new FACEINDICES32[m_tModel.tMeshes[i].NumFaces];
		for (int j = 0; j < m_tModel.tMeshes[i].NumFaces; ++j)
		{
			ReadFile(hFile, &(m_tModel.tMeshes[i].pIndices[j]), sizeof(FACEINDICES32), &dwByte, nullptr);
		}
	}
	ReadFile(hFile, &(m_tModel.AllMaterials.NumMaterials), sizeof(int), &dwByte, nullptr);
	m_tModel.AllMaterials.tMaterial = new TMATERIAL[m_tModel.AllMaterials.NumMaterials];
	for (int i = 0; i < m_tModel.AllMaterials.NumMaterials; ++i)
	{
		ReadFile(hFile, &(m_tModel.AllMaterials.tMaterial[i]), sizeof(TMATERIAL), &dwByte, nullptr);
	}

	CloseHandle(hFile);

#pragma endregion

	//메쉬
	m_iNumMeshes = m_tModel.NumMeshes;
	for (int i = 0; i < m_tModel.NumMeshes; ++i)
	{
		//ID3D11Device * pDevice, ID3D11DeviceContext * pContext, _fmatrix PivotMatrix, TANIMCONTAINER _tIn)
		CInstancingMesh*		pMeshContainer = CInstancingMesh::Create(m_pDevice, m_pContext, m_tModel.tMeshes[i], iNumInstance, matWorld);
		if (nullptr == pMeshContainer)
			return E_FAIL;

		m_Meshes.push_back(pMeshContainer);
	}
	m_iNumMaterials = m_tModel.NumMeshes;
	//머테리얼
	if (FAILED(Ready_Materials(m_tModel.AllMaterials)))
		return E_FAIL;
	return S_OK;
}

_uint CInstancingModel::Get_MaterialIndex(_uint _iMeshIndex) const
{

	return m_Meshes[_iMeshIndex]->Get_MaterialIndex();

}

CInstancingModel * CInstancingModel::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const char * pModelFilePath, const char * pModelFileName, _uint iNumInstance, vector<_float4x4>* matWorld)
{
	CInstancingModel*			pInstance = new CInstancingModel(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pModelFilePath, pModelFileName, iNumInstance, matWorld)))
	{
		MSG_BOX(TEXT("Failed To Created : CTexture"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CInstancingModel::Clone(void * pArg)
{
	CModel*			pInstance = new CInstancingModel(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CModel"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CInstancingModel::Free()
{
	__super::Free();
	for (auto& pMeshContainer : m_Meshes)
		Safe_Release(pMeshContainer);

	m_Meshes.clear();

	if (m_isCloned == false)
	{
		for (int i = 0; i < m_tModel.NumMeshes; ++i)
		{
			Safe_Delete_Array(m_tModel.tMeshes[i].pVertices);
			Safe_Delete_Array(m_tModel.tMeshes[i].pIndices);

		}
		Safe_Delete_Array(m_tModel.tMeshes);
		
		Safe_Delete_Array(m_tModel.AllMaterials.tMaterial);
	}
}