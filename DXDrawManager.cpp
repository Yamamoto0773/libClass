﻿#include "DXDrawManager.hpp"

#define _USE_MATH_DEFINES
#include <math.h>  
#include <array>

#define WRITELOG(x, ...) { if (log != nullptr) { log->tlnwrite(x, __VA_ARGS__); } }


namespace dx9 {


	size_t DXDrawManager::topLayerPos = 0;


	DXDrawManager::DXDrawManager() {
	}

	DXDrawManager::~DXDrawManager() {
	}

	bool DXDrawManager::Create(HWND hwnd, Size size, MultiSampleLv level, bool isRightHand) {
		if (!isResCreated) {
			bool isFull = false;
			
			WINDOWINFO info;
			info.cbSize = sizeof(info);
			GetWindowInfo(hwnd, &info);
			
			if ((info.dwStyle&WS_POPUP) == WS_POPUP) {
				isFull = true;
			}
		
			if (!Create(hwnd, isFull, size.w, size.h, level, isRightHand)) {
				Clear();
				Delete();
				return false;
			}
			isResCreated = true;
		}

		return true;
	}




	bool DXDrawManager::CreateFromFile(Texture & tex, const std::wstring & fileName) {
		return texMng.CreateFromFile(tex, fileName);
	}

	bool DXDrawManager::CreateFromFile(Texture & tex, const std::wstring & fileName, size_t x, size_t y, size_t w, size_t h) {
		return texMng.CreateFromFile(tex, fileName, x, y, w, h);
	}

	bool DXDrawManager::CreateEmptyTex(Texture & tex, size_t w, size_t h) {
		return texMng.CreateEmptyTex(tex, w, h);
	}

	int DXDrawManager::CleanTexPool() {
		return texMng.CleanTexPool();
	}
	
	bool DXDrawManager::DrawTexture(Texture & tex, float x, float y, DrawTexCoord coord, float alpha, float xscale, float yscale, int rotDeg) {
		return texMng.DrawTexture(tex, x, y, coord, alpha, xscale, yscale, rotDeg);
	}



	bool DXDrawManager::DrawBegin(bool isClear) {
		if (!d3ddev9) {
			return false;
		}
		if (isLost) {
			if (!DeviceReset()) {
				return false;
			}
		}


		if (!isDrawStarted) {
			if (isClear) {
				d3ddev9->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, backGroundColor, 1.0f, 0);
			}

			if (FAILED(d3ddev9->BeginScene())) {
				//WRITELOG("failed to BeginScene()");
				return false;
			}
			isDrawStarted = true;
			topLayerPos = 0;
		}

		return true;
	}

	bool DXDrawManager::DrawEnd() {
		if (!isDrawStarted) {
			return false;
		}
		if (!d3ddev9) {
			return false;
		}

		// 描画終了
		d3ddev9->EndScene();
		isDrawStarted = false;

		// フリップ
		if (d3ddev9->Present(NULL, NULL, NULL, NULL) == D3DERR_DEVICELOST) {
			WRITELOG("Device Lost");
			isLost = true;
			return false;
		}


		return true;
	}

	


	void DXDrawManager::SetBackGroundColor(size_t r, size_t g, size_t b) {
		// 最大値を255までにする
		r &= 0xff;
		g &= 0xff;
		b &= 0xff;
		SetBackGroundColor((unsigned long)(r|g|b));
	}


	void DXDrawManager::SetBackGroundColor(unsigned long rgb) {
		rgb &= 0xffffff;
		backGroundColor = rgb;
	}


	void DXDrawManager::SetBlendMode(BLENDMODE mode) {
		switch (mode) {
			case BLENDMODE::NORMAL:
				blendMode = mode;
				break;
			case BLENDMODE::ADD:
				blendMode = mode;
				break;
			default:
				break;
		}

	}

	void DXDrawManager::SetTexFilter(TextureFilter mode) {
		texFilter = mode;
	}

	bool DXDrawManager::CreateMaskBegin() {
		return maskMng.regionBegin(d3ddev9);
	}

	bool DXDrawManager::CreateMaskEnd() {
		return maskMng.regionEnd(d3ddev9);
	}

	bool DXDrawManager::SetMask() {
		return maskMng.drawBegin(d3ddev9);
	}

	bool DXDrawManager::SetRectMask(RectF maskArea) {
		if (!d3ddev9) return false;

		maskMng.regionBegin(d3ddev9);
		bool r = DrawRect(maskArea, 0xff000000);
		maskMng.regionEnd(d3ddev9);

		if (!r) return false;

		maskMng.drawBegin(d3ddev9);

		return true;
	}

	bool DXDrawManager::SetCircleMask(RectF maskArea) {
		if (!d3ddev9) return false;

		maskMng.regionBegin(d3ddev9);
		bool r = DrawCircle(maskArea, 0xff000000);
		maskMng.regionEnd(d3ddev9);

		if (!r) return false;

		maskMng.drawBegin(d3ddev9);

		return true;
	}

	bool DXDrawManager::RemoveMask() {
		return maskMng.drawEnd(d3ddev9);
	}

	void DXDrawManager::SetMaskType(MaskType type) {
		stencil::MaskColor maskCol;

		switch (type) {
			case dx9::MaskType::DrawableMask:
				maskCol = stencil::MaskColor::Trans;
				break;
			case dx9::MaskType::NotDrawableMask:
				maskCol = stencil::MaskColor::Fill;
				break;
		}

		maskMng.setMaskingColor(d3ddev9, maskCol);
	}

	MaskType DXDrawManager::GetMaskType() {
		stencil::MaskColor color = maskMng.getRefMaskColor();

		MaskType type;
		switch (color) {
			case stencil::MaskColor::Trans:
				type = MaskType::DrawableMask;
				break;
			case stencil::MaskColor::Fill:
				type = MaskType::NotDrawableMask;
				break;
			case stencil::MaskColor::None:
				break;
		}
		
		return type;
	}


	bool DXDrawManager::DrawLine(float begin_x, float begin_y, float end_x, float end_y, DWORD color, float lineWidth) {
		PointF begin = {begin_x, begin_y};
		PointF end = {end_x, end_y};
		return DrawLine(begin, end, color, lineWidth);
	}

	bool DXDrawManager::DrawLine(PointF &begin, PointF &end, DWORD color, float lineWidth) {
		if (lineWidth < 0.0f) {
			return false;
		}
		if (!isDrawable()) {
			return false;
		}

		figure::Line line;

		line.SetPos(begin, end);
		line.SetColor(color);
		line.SetLineWidth(lineWidth);

		return line.Draw(d3ddev9, effect, vertex_rect, &projMat, blendMode, topLayerPos);
	}






	bool DXDrawManager::DrawRectFrame(PointF &topLeft, PointF &bottomRight, DWORD color, float lineWidth) {
		return DrawRectFrame(topLeft.x, topLeft.y, bottomRight.x-topLeft.x, bottomRight.y-topLeft.y, color, lineWidth);
	}

	bool DXDrawManager::DrawRectFrame(RectF &rect, DWORD color, float lineWidth) {
		return DrawRectFrame(rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, color, lineWidth);
	}

	bool DXDrawManager::DrawRectFrame(float x, float y, float w, float h, DWORD color, float lineWidth) {
		if (lineWidth < 0.0f) {
			return false;
		}
		if (w < 0.0f || h < 0.0f) {
			return false;
		}
		if (!isDrawable()) {
			return false;
		}

		figure::RectFrame rectframe;

		rectframe.SetPos(x, y, w, h);
		rectframe.SetColor(color);
		rectframe.SetLineWidth(lineWidth);

		return rectframe.Draw(d3ddev9, effect, vertex_rect, &projMat, blendMode, topLayerPos);
	}





	bool DXDrawManager::DrawRect(RectF & rect, DWORD color) {
		return DrawRect(rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, color);
	}

	bool DXDrawManager::DrawRect(PointF & topLeft, PointF & bottomRight, DWORD color) {
		return DrawRect(topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y, color);
	}

	bool DXDrawManager::DrawRect(float x, float y, float w, float h, DWORD color) {
		if (w < 0.0f || h < 0.0f) {
			return false;
		}
		if (!isDrawable()) {
			return false;
		}

		figure::Rect rectframe;

		rectframe.SetPos(x, y, w, h);
		rectframe.SetColor(color);

		return rectframe.Draw(d3ddev9, effect, vertex_rect, &projMat, blendMode, topLayerPos);

	}




	bool DXDrawManager::DrawCircleFrame(RectF & rectArea, DWORD color, float lineWidth) {
		return DrawCircleFrame(
			(rectArea.left+rectArea.right)/2.0f,
			(rectArea.top+rectArea.bottom)/2.0f,
			rectArea.right-rectArea.left,
			rectArea.bottom-rectArea.top,
			color,
			lineWidth
			);
	}

	bool DXDrawManager::DrawCircleFrame(float x, float y, float w, float h, DWORD color, float lineWidth) {
		if (w < 0.0f || h < 0.0f) {
			return false;
		}
		if (lineWidth < 0.0f) {
			return false;
		}
		if (!isDrawable()) {
			return false;
		}

		stencil::Mode mode = maskMng.getCurrectMode();

		// draw circle frame
		figure::CircleFrame circleFrame;
		circleFrame.SetPos({x, y}, w, h);
		circleFrame.SetColor(color);
		circleFrame.setLineWidth(lineWidth);
		bool r = circleFrame.Draw(d3ddev9, effect, vertex_circle, &projMat, blendMode, topLayerPos);


		switch (mode) {
			case stencil::Mode::Masking:
				maskMng.regionBegin(d3ddev9, false);
				break;
			case stencil::Mode::Draw:
				maskMng.drawBegin(d3ddev9);
				break;
			case stencil::Mode::Idle:
				break;
		}


		return r;
	}







	bool DXDrawManager::DrawCircle(RectF &rectArea, DWORD color) {
		return DrawCircle(
			(rectArea.left+rectArea.right)/2.0f,
			(rectArea.top+rectArea.bottom)/2.0f,
			rectArea.right-rectArea.left,
			rectArea.bottom-rectArea.top,
			color
			);
	}

	bool DXDrawManager::DrawCircle(float x, float y, float w, float h, DWORD color) {
		if (w < 0.0f || h < 0.0f) {
			return false;
		}
		if (!isDrawable()) {
			return false;
		}

		figure::Circle circle;
		circle.SetPos({x, y}, w, h);
		circle.SetColor(color);

		return circle.Draw(d3ddev9, effect, vertex_circle, &projMat, blendMode, topLayerPos);
	}




	void DXDrawManager::SetLogWriteDest(LogManager* dest) {
		if (dest == nullptr) {
			return;
		}

		log = dest;
	}




	bool DXDrawManager::Create(HWND hwnd, bool isfull, size_t w, size_t h, MultiSampleLv level, bool isRightHand) {
		Delete();
		Clear();

		HRESULT ret;

		if (resource::CIRCLE_VERTEXCNT < 3) return false;


		// Direct3D9オブジェクトの取得
		d3d9.Attach(Direct3DCreate9(D3D_SDK_VERSION));
		if (d3d9 == nullptr) {
			WRITELOG("Failed to Create D3D9 Object");
			return false;
		}


		// ハードウェアの能力の取得
		ret = d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dcaps9);
		if (FAILED(ret)) {
			WRITELOG("Failed to Get D3DCAPS9");
			return false;
		}


		// Direct3D9デバイス取得のための引数を設定
		d3dpresent.Windowed						= !isfull;
		d3dpresent.BackBufferWidth				= (UINT)w;
		d3dpresent.BackBufferHeight				= (UINT)h;
		if (isfull) {
			d3dpresent.BackBufferFormat			= D3DFMT_X8R8G8B8;
		}
		else {
			D3DDISPLAYMODE dispMode;
			ret = d3d9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &dispMode);
			if (FAILED(ret)) {
				WRITELOG("Failed to Get D3DDISPLAYMODE");
				return false;
			}
			d3dpresent.BackBufferFormat			= dispMode.Format;
		}
		d3dpresent.BackBufferCount				= 1;
		d3dpresent.MultiSampleType				= static_cast<D3DMULTISAMPLE_TYPE>(level);
		d3dpresent.MultiSampleQuality			= 0;
		d3dpresent.SwapEffect					= D3DSWAPEFFECT_DISCARD;
		d3dpresent.hDeviceWindow				= NULL;	// あとで指定
		d3dpresent.EnableAutoDepthStencil		= true;
		d3dpresent.AutoDepthStencilFormat		= D3DFMT_D24S8;
		d3dpresent.FullScreen_RefreshRateInHz	= D3DPRESENT_RATE_DEFAULT;
		d3dpresent.PresentationInterval			= D3DPRESENT_INTERVAL_DEFAULT;



		/*
		Direct3D9デバイスの取得開始
		速度の早い順番でテスト

		1.ハードウェア処理&HW頂点処理
		2.ハードウェア処理&SW頂点処理
		3.ソフトウェア処理&HW頂点処理
		4.ソフトウェア処理&SW頂点処理
		*/
		for (int i=0; i<4; i++) {
			D3DDEVTYPE devtype;
			DWORD behaviorFlags;

			switch (i) {
				case 0:
					devtype = D3DDEVTYPE_HAL;
					behaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
					break;
				case 1:
					devtype = D3DDEVTYPE_HAL;
					behaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
					break;
				case 2:
					devtype = D3DDEVTYPE_REF;
					behaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
					break;
				case 3:
					devtype = D3DDEVTYPE_REF;
					behaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
					break;
			}

			// マルチスレッド
			behaviorFlags |= D3DCREATE_MULTITHREADED;



			// フルシーンマルチサンプリングが適用可能かテスト
			if (static_cast<int>(d3dpresent.MultiSampleType) >= 2) {

				DWORD backBufferLevel, ZBufferLevel;

				while (true) {
					if (SUCCEEDED(
						d3d9->CheckDeviceMultiSampleType(
							D3DADAPTER_DEFAULT,
							devtype,
							d3dpresent.BackBufferFormat,
							d3dpresent.Windowed,
							d3dpresent.MultiSampleType,
							&backBufferLevel
							))
						&&
						SUCCEEDED(
							d3d9->CheckDeviceMultiSampleType(
								D3DADAPTER_DEFAULT,
								devtype,
								d3dpresent.AutoDepthStencilFormat,
								d3dpresent.Windowed,
								d3dpresent.MultiSampleType,
								&ZBufferLevel
								))) {
						// バックバッファと深度ステンシルバッファがともに対応している場合

						if (backBufferLevel < ZBufferLevel) {
							d3dpresent.MultiSampleQuality = backBufferLevel-1;
						}
						else {
							d3dpresent.MultiSampleQuality = ZBufferLevel-1;
						}

						break;
					}
					else {
						// 対応していない

						// 1つ下のレベルで再試行
						int index = static_cast<int>(d3dpresent.MultiSampleType) - 1;

						if (index < 2) {
							// フルシーンマルチサンプリングが利用できない
							break;
						}

						d3dpresent.MultiSampleType = static_cast<D3DMULTISAMPLE_TYPE>(index);

					}
				}

			}


			IDirect3DDevice9 *ptr;
			ret = d3d9->CreateDevice(D3DADAPTER_DEFAULT, devtype, hwnd, behaviorFlags, &d3dpresent, &ptr);
			if (SUCCEEDED(ret)) {
				d3ddev9.Attach(ptr);

				switch (i) {
					case 0: WRITELOG("Create D3DDevice9 ... OK\n  DeviceInfo>DeviceType:GPU VertexProcessing:Hardware\n  DeviceInfo>MultiSampleType:%dsamples MultiSampleQuality:%lu", static_cast<D3DMULTISAMPLE_TYPE>(d3dpresent.MultiSampleType), d3dpresent.MultiSampleQuality); break;
					case 1: WRITELOG("Create D3DDevice9 ... OK\n  DeviceInfo>DeviceType:GPU VertexProcessing:Software\n  DeviceInfo>MultiSampleType:%dsamples MultiSampleQuality:%lu", static_cast<D3DMULTISAMPLE_TYPE>(d3dpresent.MultiSampleType), d3dpresent.MultiSampleQuality); break;
					case 2: WRITELOG("Create D3DDevice9 ... OK\n  DeviceInfo>DeviceType:CPU VertexProcessing:Hardware\n  DeviceInfo>MultiSampleType:%dsamples MultiSampleQuality:%lu", static_cast<D3DMULTISAMPLE_TYPE>(d3dpresent.MultiSampleType), d3dpresent.MultiSampleQuality); break;
					case 3: WRITELOG("Create D3DDevice9 ... OK\n  DeviceInfo>DeviceType:CPU VertexProcessing:Software\n  DeviceInfo>MultiSampleType:%dsamples MultiSampleQuality:%lu", static_cast<D3DMULTISAMPLE_TYPE>(d3dpresent.MultiSampleType), d3dpresent.MultiSampleQuality); break;
				}

				break;	// 作成終了
			}

			if (i==3) {	// 最終ループなら
				WRITELOG("Failed to Create D3DDevice9");
				return false;
			}
		}



		if (!maskMng.create(d3ddev9))
			return false;


		// 頂点バッファ作成
		this->isRightHand = isRightHand;

		std::array<float, 20> vtx_rect;
		std::array<std::array<float, 5>, resource::CIRCLE_VERTEXCNT+2> vtx_circle;
		if (isRightHand) {
			vtx_rect = {
				1.0f, 1.0f, 0.0f,   1.0f, 1.0f,  // 3
				0.0f, 1.0f, 0.0f,   0.0f, 1.0f,  // 2
				1.0f, 0.0f, 0.0f,   1.0f, 0.0f,  // 1
				0.0f, 0.0f, 0.0f,   0.0f, 0.0f,  // 0
			};
			vtx_circle[0] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
			for (int i=0; i<resource::CIRCLE_VERTEXCNT+1; i++) {
				float rad = 2.0f*M_PI/resource::CIRCLE_VERTEXCNT * i;
				vtx_circle[i+1] = {cos(-rad)/2.0f, sin(-rad)/2.0f, 0.0f, cos(-rad)/2.0f+0.5f, sin(-rad)/2.0f+0.5f}; // create vertex data for circle which is 1 in diameter
			}
		}
		else {
			vtx_rect = {
				0.0f, 0.0f, 0.0f,   0.0f, 0.0f,  // 0
				1.0f, 0.0f, 0.0f,   1.0f, 0.0f,  // 1
				0.0f, 1.0f, 0.0f,   0.0f, 1.0f,  // 2
				1.0f, 1.0f, 0.0f,   1.0f, 1.0f,  // 3
			};
			vtx_circle[0] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
			for (int i=0; i<resource::CIRCLE_VERTEXCNT+1; i++) {
				float rad = 2.0f*M_PI/resource::CIRCLE_VERTEXCNT * i;
				vtx_circle[i+1] = {cos(rad)/2.0f, sin(rad)/2.0f, 0.0f, cos(rad)/2.0f+0.5f, sin(rad)/2.0f+0.5f};  // create vertex data for circle which is 1 in diameter
			}
		}


		{
			IDirect3DVertexBuffer9 *ptr_rect;
			IDirect3DVertexBuffer9 *ptr_circle;
			if (FAILED(d3ddev9->CreateVertexBuffer(sizeof(vtx_rect), 0, 0, D3DPOOL_MANAGED, &ptr_rect, 0)) ||
				FAILED(d3ddev9->CreateVertexBuffer(sizeof(vtx_circle), 0, 0, D3DPOOL_MANAGED, &ptr_circle, 0))) {
				WRITELOG("failed to create \"Vertex Buffer9\"");
				return false;
			}
			float *p = nullptr;
			ptr_rect->Lock(0, 0, (void**)&p, 0);
			memcpy(p, vtx_rect.data(), sizeof(vtx_rect));
			ptr_rect->Unlock();
			vertex_rect.Attach(ptr_rect);

			ptr_circle->Lock(0, 0, (void**)&p, 0);
			memcpy(p, vtx_circle.data(), sizeof(vtx_circle));
			ptr_circle->Unlock();
			vertex_circle.Attach(ptr_circle);

		}


		// シェーダ作成
		{
			ID3DXBuffer *error = nullptr;
			ID3DXEffect *ptr = nullptr;
			if (FAILED(D3DXCreateEffectFromFile(d3ddev9, L"sprite2.fx", 0, 0, 0, 0, &ptr, &error))) {
				OutputDebugStringA((const char*)error->GetBufferPointer());
				return false;
			}

			effect.Attach(ptr);
		}

		// 頂点宣言作成
		{
			IDirect3DVertexDeclaration9 *ptr;
			D3DVERTEXELEMENT9 elems[] = {
				{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
				{0, sizeof(float) * 3, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
				D3DDECL_END()
			};

			if (FAILED(d3ddev9->CreateVertexDeclaration(elems, &ptr))) {
				WRITELOG("failed to create \"Bertex Declaration9\"");
				return false;
			}
			verDecl.Attach(ptr);
		}

		// 頂点宣言を登録
		d3ddev9->SetVertexDeclaration(verDecl);


		// 2D描画用射影変換行列を作成
		D3DXMatrixIdentity(&projMat);
		projMat._41 = -1.0f;
		projMat._42 =  1.0f;
		projMat._11 =  2.0f / d3dpresent.BackBufferWidth;
		projMat._22 = -2.0f / d3dpresent.BackBufferHeight;


		// デフォルトステートのセット
		d3ddev9->SetRenderState(D3DRS_LIGHTING, FALSE);							// ライティング無効
		if (isRightHand)
			d3ddev9->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);				// 右回りを消去(右手系)
		else
			d3ddev9->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);				// 左回りを消去(左手系)

		d3ddev9->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		d3ddev9->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		d3ddev9->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);		// テクスチャがはみ出た時に表示しないにする
		d3ddev9->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

		d3ddev9->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		d3ddev9->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		d3ddev9->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		d3ddev9->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		d3ddev9->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		d3ddev9->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

		return true;
	}

	void DXDrawManager::Delete() {
		verDecl.Release();
		effect.Release();
		vertex_rect.Release();
		vertex_circle.Release();
		d3ddev9.Release();
		d3d9.Release();
		
		isResCreated = false;
	}

	void DXDrawManager::Clear() {
		isLost = false;
		isDrawStarted = false;
		isRightHand = false;
		backGroundColor = 0xffffff;
		blendMode = BLENDMODE::NORMAL;

		ZeroMemory(&d3dcaps9, sizeof(d3dcaps9));
		ZeroMemory(&d3dpresent, sizeof(d3dpresent));

	}


	bool DXDrawManager::DeviceReset() {
		if (!isResCreated) {
			return false;
		}

		if (isLost) {
			if (d3ddev9->TestCooperativeLevel() != D3DERR_DEVICENOTRESET) {
				// デバイスを復帰できる状態でない
				return false;
			}

			if (FAILED(d3ddev9->Reset(&d3dpresent))) {
				return false;
			}



			isLost = false;



			effect.Release();
			verDecl.Release();


			// シェーダ作成
			{
				ID3DXBuffer *error = nullptr;
				ID3DXEffect *ptr = nullptr;
				if (FAILED(D3DXCreateEffectFromFile(d3ddev9, L"sprite2.fx", 0, 0, 0, 0, &ptr, &error))) {
					OutputDebugStringA((const char*)error->GetBufferPointer());
					return false;
				}

				effect.Attach(ptr);
			}

			// 頂点宣言作成
			{
				IDirect3DVertexDeclaration9 *ptr;
				D3DVERTEXELEMENT9 elems[] = {
					{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
					{0, sizeof(float) * 3, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
					D3DDECL_END()
				};

				if (FAILED(d3ddev9->CreateVertexDeclaration(elems, &ptr))) {
					WRITELOG("failed to create \"Bertex Declaration9\"");
					return false;
				}
				verDecl.Attach(ptr);
			}

			
			// 頂点宣言を登録
			d3ddev9->SetVertexDeclaration(verDecl);



			// デフォルトステートのセット
			d3ddev9->SetRenderState(D3DRS_LIGHTING, FALSE);							// ライティング無効
			if (isRightHand)
				d3ddev9->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);				// 右回りを消去(右手系)
			else
				d3ddev9->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);				// 左回りを消去(左手系)

			d3ddev9->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			d3ddev9->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			d3ddev9->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);		// テクスチャがはみ出た時に表示しないにする
			d3ddev9->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

			d3ddev9->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			d3ddev9->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			d3ddev9->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			d3ddev9->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			d3ddev9->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			d3ddev9->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

		
			WRITELOG("Device Reset ... OK");
		}
		
		return true;
	}


}

