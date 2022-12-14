#include "Mesh.h"
#include <cassert>
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")
using namespace DirectX;

/// <summary>
/// 静的メンバ変数の実体
/// </summary>
ID3D12Device* Mesh::device = nullptr;

void Mesh::StaticInitialize(ID3D12Device* device) {
	Mesh::device = device;

	// マテリアルの静的初期化
	Material::StaticInitialize(device);
}

void Mesh::SetName(const std::string& name) { this->name = name; }

void Mesh::SetMaterial(Material* material) { this->material = material; }

void Mesh::AddVertex(const VertexPosNormalUv& vertex) { vertices.emplace_back(vertex); }

void Mesh::AddIndex(unsigned short index) { indices.emplace_back(index); }

void Mesh::AddSmoothData(unsigned short indexPosition, unsigned short indexVertex)
{
	smoothData[indexPosition].emplace_back(indexVertex);
}

void Mesh::CreateBuffers() {
	HRESULT result;

	UINT sizeVB = static_cast<UINT>(sizeof(VertexPosNormalUv) * vertices.size());

	// ヒーププロパティ
	CD3DX12_HEAP_PROPERTIES heapPropsVertexBuffer = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	// リソース設定
	CD3DX12_RESOURCE_DESC resourceDescVertexBuffer = CD3DX12_RESOURCE_DESC::Buffer(sizeVB);

	// 頂点バッファ生成
	result = device->CreateCommittedResource(
		&heapPropsVertexBuffer, D3D12_HEAP_FLAG_NONE, &resourceDescVertexBuffer,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertBuff));
	assert(SUCCEEDED(result));

	// 頂点バッファのマッピング
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	assert(SUCCEEDED(result));

	// 頂点バッファへのデータ転送
	std::copy(vertices.begin(), vertices.end(), vertMap);

	// 頂点バッファビューの作成
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeVB;
	vbView.StrideInBytes = sizeof(vertices[0]);

	UINT sizeIB = static_cast<UINT>(sizeof(unsigned short) * indices.size());

	// ヒーププロパティ
	CD3DX12_HEAP_PROPERTIES heapPropsIndexBuffer = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	// リソース設定
	CD3DX12_RESOURCE_DESC resourceDescIndexBuffer = CD3DX12_RESOURCE_DESC::Buffer(sizeIB);

	// インデックスバッファ生成
	result = device->CreateCommittedResource(
		&heapPropsIndexBuffer, D3D12_HEAP_FLAG_NONE, &resourceDescIndexBuffer,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&indexBuff));
	assert(SUCCEEDED(result));

	// インデックスバッファのマッピング
	result = indexBuff->Map(0, nullptr, (void**)&indexMap);
	assert(SUCCEEDED(result));

	// インデックスバッファへのデータ転送
	std::copy(indices.begin(), indices.end(), indexMap);

	// インデックスバッファビューの作成
	ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeIB;
}

void Mesh::CalculateSmoothedVertexNormals()
{
	auto itr = smoothData.begin();
	for (; itr != smoothData.end(); ++itr) {
		// 各面用の共通頂点コレクション
		std::vector<unsigned short>& v = itr->second;
		// 全頂点の法線を平均する
		XMVECTOR normal = {};
		for (unsigned short index : v) {
			normal += XMVectorSet(vertices[index].normal.x, vertices[index].normal.y, vertices[index].normal.z, 0);
		}
		normal += XMVector3Normalize(normal / (float)v.size());
		// 共通法線を使用する全ての頂点データに書き込む
		for (unsigned short index : v) {
			vertices[index].normal = { normal.m128_f32[0],normal.m128_f32[1],normal.m128_f32[2] };
		}
	}
}

void Mesh::Draw(ID3D12GraphicsCommandList* cmdList) {
	// 頂点バッファをセット
	cmdList->IASetVertexBuffers(0, 1, &vbView);
	// インデックスバッファをセット
	cmdList->IASetIndexBuffer(&ibView);

	// シェーダリソースビューをセット
	cmdList->SetGraphicsRootDescriptorTable(2, material->GetGpuHandle());

	// マテリアルの定数バッファをセット
	ID3D12Resource* constBuff = material->GetConstantBuffer();
	cmdList->SetGraphicsRootConstantBufferView(1, constBuff->GetGPUVirtualAddress());

	// 描画コマンド
	cmdList->DrawIndexedInstanced((UINT)indices.size(), 1, 0, 0, 0);
}
