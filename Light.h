#pragma once
#include <wrl.h>
#include <DirectXMath.h>
#include <d3dx12.h>

/// <summary>
/// ライト
/// </summary>
class Light
{
private: // エイリアス
	template <class T>using ComPtr = Microsoft::WRL::ComPtr<T>;
	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMVECTOR = DirectX::XMVECTOR;
	using XMMATRIX = DirectX::XMMATRIX;

public: // サブクラス
	// 定数バッファ用データ構造体
	struct ConstBufferData {
		XMVECTOR lightv; // ライトへの方向を表すベクトル
		XMFLOAT3 lightcolor; // ライトの色
	};

	// 定数バッファ用データ構造体(マテリアル)
	struct ConstBufferDataMaterial {
		XMFLOAT4 color; // 色
	};

private: // 静的メンバ変数
	// デバイス
	static ID3D12Device* device;

public: // 静的メンバ関数
	/// <summary>
	/// 静的初期化
	/// </summary>
	/// <param name="device">デバイス</param>
	static void StaticInitialize(ID3D12Device* device);

	/// <summary>
	/// インスタンス生成
	/// </summary>
	/// <returns>インスタンス</returns>
	static Light* Create();

private: // メンバ変数
	// 定数バッファ
	ComPtr<ID3D12Resource> constBuff;
	// ライト光線方向(単位ベクトル)
	XMVECTOR lightdir = { 1,0,0,0 };
	// ライトの色
	XMFLOAT3 lightcolor = { 1,1,1 };
	// ダーティフラグ
	bool dirty = false;

public: // メンバ関数
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 定数バッファ転送
	/// </summary>
	void TransferConstBuffer();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw(ID3D12GraphicsCommandList* cmdList, UINT rootParameterIndex);

public: // アクセッサ
	/// <summary>
	/// ライト方向のセット
	/// </summary>
	/// <param name="lightdir">ライト方向</param>
	void SetLightDir(const XMVECTOR& lightdir);

	/// <summary>
	/// ライト色をセット
	/// </summary>
	/// <param name="lightcolor">ライト色</param>
	void SetLightColor(const XMFLOAT3& lightcolor);
};
