//#include "..\Public\VIBuffer_Point.h"
#include "..\Public\VIBuffer_Point_Instancing.h"

CVIBuffer_Point_Instancing::CVIBuffer_Point_Instancing(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CVIBuffer(pDevice, pContext)
{
}

CVIBuffer_Point_Instancing::CVIBuffer_Point_Instancing(const CVIBuffer_Point_Instancing & rhs)
	: CVIBuffer(rhs)
	, m_pVBInstance(rhs.m_pVBInstance)
	, m_iNumInstance(rhs.m_iNumInstance)
	, m_iInstanceStride(rhs.m_iInstanceStride)
{

}

HRESULT CVIBuffer_Point_Instancing::Initialize_Prototype(_uint iNumInstance, vector<_float4x4>* _WorldMatrix, vector<_float3> _vecDirect)
{
	if (FAILED(Ready_VertexBuffer()))
		return E_FAIL;

	if (FAILED(Ready_IndexBuffer(iNumInstance)))
		return E_FAIL;

	if (FAILED(Ready_InstancingBuffer(iNumInstance, _WorldMatrix, _vecDirect)))
		return E_FAIL;

	return S_OK;
}

HRESULT CVIBuffer_Point_Instancing::Initialize(void * pArg)
{
	//pArg는 갯수로 들고오자?
	return S_OK;
}

CVIBuffer_Point_Instancing * CVIBuffer_Point_Instancing::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, _uint iNumInstance, vector<_float4x4>* _WorldMatrix, vector<_float3> _vecDirect)
{
	CVIBuffer_Point_Instancing*			pInstance = new CVIBuffer_Point_Instancing(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(iNumInstance, _WorldMatrix, _vecDirect)))
	{
		MSG_BOX(TEXT("Failed To Created : CVIBuffer_Point_Instancing"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CVIBuffer_Point_Instancing::Clone(void * pArg)
{
	CVIBuffer_Point_Instancing*			pInstance = new CVIBuffer_Point_Instancing(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CVIBuffer_Point_Instancing"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVIBuffer_Point_Instancing::Free()
{
	__super::Free();
	Safe_Release(m_pVBInstance);
}

HRESULT CVIBuffer_Point_Instancing::Render()
{
	if (nullptr == m_pContext)
		return E_FAIL;
	//여기는 솔팅(카메라 바라보는 방향으로 -> 근데 파티클이 여러개 합쳐지면 문제임)
	ID3D11Buffer*		pVertexBuffers[] = {
		m_pVB,
		m_pVBInstance,
	};

	_uint			iStrides[] = {
		m_iStride,
		m_iInstanceStride
	};

	_uint			iOffsets[] = {
		0,
		0,
	};

	m_pContext->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffers, iStrides, iOffsets);

	m_pContext->IASetIndexBuffer(m_pIB, m_eIndexFormat, 0);

	m_pContext->IASetPrimitiveTopology(m_eTopology);
	//m_iNumInstance 컬링했을때 바뀌는 숫자임
	m_pContext->DrawIndexedInstanced(m_iNumPrimitives * m_iNumIndicesofPrimitive, m_iNumInstance, 0, 0, 0);

	return S_OK;
}

HRESULT CVIBuffer_Point_Instancing::Ready_VertexBuffer()
{
	m_iNumVertexBuffers = 2; // 왜 두개지?-> 인스턴싱 버퍼까지 2개
	m_iNumVertices = 1;
	m_iStride = sizeof(VTXPOINT);

	ZeroMemory(&m_BufferDesc, sizeof(D3D11_BUFFER_DESC));
	m_BufferDesc.ByteWidth = m_iNumVertices * m_iStride;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = m_iStride;

	VTXPOINT*		pVertices = new VTXPOINT;
	ZeroMemory(pVertices, sizeof(VTXPOINT));

	pVertices->vPosition = _float3(0.f, 0.f, 0.f);


	ZeroMemory(&m_SubResourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	m_SubResourceData.pSysMem = pVertices;

	if (FAILED(__super::Create_VertexBuffer()))
		return E_FAIL;

	Safe_Delete_Array(pVertices);

	return S_OK;
}

HRESULT CVIBuffer_Point_Instancing::Ready_IndexBuffer(_uint iNumInstance)
{
	if (iNumInstance > 200)
	{
		return E_FAIL;
	}
	m_iNumInstance = iNumInstance;

	m_iNumPrimitives = 1;
	m_iIndexSizeofPrimitive = sizeof(FACEINDICES16);
	m_iNumIndicesofPrimitive = 1;
	m_eIndexFormat = DXGI_FORMAT_R16_UINT;
	m_eTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;

	ZeroMemory(&m_BufferDesc, sizeof(D3D11_BUFFER_DESC));
	m_BufferDesc.ByteWidth = m_iNumPrimitives * m_iIndexSizeofPrimitive * iNumInstance;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = 0;

	_ushort*		pIndices = new _ushort[m_iNumPrimitives * iNumInstance];
	ZeroMemory(pIndices, sizeof(_ushort) * m_iNumPrimitives);

	ZeroMemory(&m_SubResourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	m_SubResourceData.pSysMem = pIndices;

	if (FAILED(__super::Create_IndexBuffer()))
		return E_FAIL;

	Safe_Delete_Array(pIndices);

	return S_OK;
}

HRESULT CVIBuffer_Point_Instancing::Ready_InstancingBuffer(_uint iNumInstance, vector<_float4x4>* _WorldMatrix, vector<_float3> _vecDirect)
{
	if (iNumInstance > 200)
	{
		return E_FAIL;
	}
	m_iNumInstance = iNumInstance;
	m_iInstanceStride = sizeof(VTXINSTANCE);

	ZeroMemory(&m_BufferDesc, sizeof(D3D11_BUFFER_DESC));
	m_BufferDesc.ByteWidth = m_iInstanceStride * iNumInstance;
	m_BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = m_iInstanceStride;

	VTXINSTANCE*		pInstanceVtx = new VTXINSTANCE[200];
	ZeroMemory(pInstanceVtx, sizeof(VTXINSTANCE) * 200);
	for (_uint i = 0; i < iNumInstance; ++i)
	{
		memcpy(&(pInstanceVtx[i]), &((*_WorldMatrix)[i]), sizeof(_float4x4));
		m_vecWorldMatrix.push_back((*_WorldMatrix)[i]);
	}

	ZeroMemory(&m_SubResourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	m_SubResourceData.pSysMem = pInstanceVtx;

	if (FAILED(m_pDevice->CreateBuffer(&m_BufferDesc, &m_SubResourceData, &m_pVBInstance)))
		return E_FAIL;

	Safe_Delete_Array(pInstanceVtx);


	return S_OK;
}
