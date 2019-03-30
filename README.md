# Dx9Wrapper
DirectX9.0を用いた2D描画ライブラリです。

### 概要
テクスチャや文字、簡単な図形の描画ができる2D描画ライブラリです。

### 環境
- Visual Studio 2017 (vs2015以前は未検証)
- Windows SDK 10.0.x.x くらい
- UNICODE文字セットのみ (マルチバイト文字セットにも後で対応します)

### リファレンス
[Wikiページ](https://github.com/Yamamoto0773/Dx9Wrapper/wiki)に載っけてます

### 使い方 (VS 2017の場合)
1. Visual Studioで`空のプロジェクト`でプロジェクトを作成します
2. プロジェクトのプロパティから、`全般`→`文字セット`が`UNICODE文字セットを使用する`になっているか確認
3. プロジェクトのプロパティから、`C/C++`→`ランタイムライブラリ`が次のようになっているか確認
	- `マルチスレッドデバッグ(/MTd)`(Debug環境の場合)
	- `マルチスレッド(/MT)`(Release環境の場合)
4. ダウンロードした`Dx9Wrapper`ディレクトリの中の`include`ディレクトリ内にある`effect.rc`をプロジェクトに追加
5. 最後に次のコードをコピペして実行！※ パス内の`../../Dx9Wrapper_v0.01`は、ダウンロードしたディレクトリを指定して下さい

```cpp
#include "../../Dx9Wrapper_v0.01/include/Dx9Wrapper.hpp"


#ifdef _DEBUG
#ifdef _WIN64
#pragma comment (lib, "../../Dx9Wrapper_v0.01/lib/x64/debug/Dx9Wrapper.lib")   // Debug x64
#else
#pragma comment (lib, "../../Dx9Wrapper_v0.01/lib/debug/Dx9Wrapper.lib")       // Debug x86
#endif
#else
#ifdef _WIN64
#pragma comment (lib, "../../Dx9Wrapper_v0.01/lib/x64/release/Dx9Wrapper.lib") // Release x64
#else
#pragma comment (lib, "../../Dx9Wrapper_v0.01/lib/release/Dx9Wrapper.lib")     // Release x86
#endif
#endif	// _DEBUG


int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
	using namespace dx9;
	auto &graphic = Graphics::GetInstance();

	Window window(hInstance, { 640, 640 }, L"test window", false);
	graphic.Create(window.getDetail(), { 640, 640 });
	AsciiFont font(L"Arial", 72, FontWeight::BOLD);

	graphic.SetClearColor(ColorRGB(255, 255, 255));

	while (window.update()) {
		graphic.StartDrawing(true);
		font.SetFontColor(ColorRGB(0, 0, 0));
		font.SetAlignment(TextAlign::CENTERXY);
		font.DrawInRect({ 0, 0, 640, 640 }, L"Hello World!");
		graphic.EndDrawing();
	}

	return 0;
}
```


### ライセンス (about License)
(This software is released under the BSD 2-clause "Simplified" License, see LICENSE.md)

このアプリはBSD 2-clause "Simplified" Licenseのもとで公開されています。
BSD 2-clause "Simplified" LicenseについてはLICENSE.mdを参照して下さい。
ざっくり説明すると以下のようになっています。

<dl>
	<dt>必須</dt>
	<dd>著作権の表示</dd>
	<dt>許可</dt>
	<dd>商用利用</dd>
	<dd>修正</dd>
	<dd>配布</dd>
	<dd>個人利用</dd>
	<dt>禁止</dt>
	<dd>責任免除</dd>
</dl>

  
  
-------------

#### コミットメッセージの書式
1行目:コミットの種類

2行目:改行

3行目:コミットの要約


#### コミットの種類
add     新規追加

fix     バグ修正

update  バグではない変更

disable コメントアウトなど、機能の無効化

remove  ファイルやコードの削除

clean   ファイルやコードの整理

    
#### バージョンナンバーについて
タグに用いられているバージョンナンバーは，[Semantic Versioning 2.0.0](https://semver.org/)に従ってつけています．

