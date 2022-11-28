#pragma once
#include "Model.h"

BEGIN(Engine)
class ENGINE_DLL CInstancingModel final :
	public CModel
{
public:
	CInstancingModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CInstancingModel(const CInstancingModel& rhs);
	virtual ~CInstancingModel() = default;

public:
	HRESULT Initialize_Prototype(const char* pModelFilePath, const char* pModelFileName, _uint iNumInstance, vector<_float4x4>* matWorld);

	HRESULT Initialize(void* pArg);

	HRESULT Render(_uint _iMeshIndex);
	_uint   Get_MaterialIndex(_uint _iMeshIndex) const;

	TMODEL Get_ForSave() {return m_tModel;}

	_bool Picking(class CTransform* pTransform, _vector& pOut);

	void Culling(_float fSize);

private:
	HRESULT Ready_MeshContainers(_uint iNumInstance, vector<_float4x4>* matWorld);

	HRESULT Load_Dat(const char * pModelFilePath, const char * pModelFileName, _uint iNumInstance, vector<_float4x4>* matWorld);


public:
	static CInstancingModel * Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const char * pModelFilePath, const char * pModelFileName, _uint iNumInstance, vector<_float4x4>* matWorld);
	CComponent * CInstancingModel::Clone(void * pArg);
	virtual void Free() override;

private:
	typedef vector<class CInstancingMesh*>	MESHES;
	vector<class CInstancingMesh*>			m_Meshes;
	TMODEL									m_tModel;

};

END