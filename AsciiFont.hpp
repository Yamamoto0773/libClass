﻿#pragma once

#include "FontTexture.hpp"
#include "FontBase.hpp"
#include <array>

// ASCII文字描画クラス


namespace dx9 {

	class AsciiFont : public font::FontBase {

		static const int CHARACTER_MAXCNT = 1024;	// 文字列の最大文字数


	private:
		HFONT					hFont;			// 設定中のフォントのハンドル
		AntialiasLevel			antialiasLv;

		std::array<wchar_t, CHARACTER_MAXCNT+1> workBuf;

		TEXTMETRICW				tm;	

		std::vector< std::unique_ptr<texture::FontTexture> > texRes;

	public:
		AsciiFont();
		AsciiFont(
			const wchar_t*		fontName,
			size_t			fontSize,
			FontWeight		fontWeight = FontWeight::NORMAL,
			bool			isItalic = false,
			bool			isUnderLine = false,
			bool			isStrikeOut = false,
			AntialiasLevel	level = AntialiasLevel::_15STEPS
		);

		~AsciiFont();

		bool Create(
			const wchar_t*		fontName,
			size_t			fontSize,
			FontWeight		fontWeight = FontWeight::NORMAL,
			bool			isItalic = false,
			bool			isUnderLine = false,
			bool			isStrikeOut = false,
			AntialiasLevel	level = AntialiasLevel::_15STEPS
		);


		//////////////////////////////////////////////
		// 文字描画

		// シンプル描画
		bool Draw(
			float x,
			float y,		
			const wchar_t* s, ...
			);

		// 指定領域内へ文字描画
		bool DrawInRect(
			const RectF &rect,	
			const wchar_t* s, ...
			);

		// 文字送り
		// note:最後の文字まで描画する時は，drawChCntに負数を指定
		bool Draw(
			float x,
			float y,			
			size_t startCharCnt,
			int drawCharCnt,
			const wchar_t* s, ...
			);

		// 文字送りx指定領域内描画
		// note:最後の文字まで描画する時は，drawChCntに負数を指定
		bool DrawInRect(
			const RectF &rect,	
			size_t startCharCnt,
			int drawCharCnt,
			const wchar_t* s, ...
			);

	
		// カスタム描画
		bool DrawInRect(
			const RectF &rect,	
			size_t startCharCnt,
			int drawCharCnt,
			size_t fontSize,
			const wchar_t* s, ...
			);


		

	private:


		// 描画を管理する関数
		bool DrawFont(
			const RectF &rect,
			bool isAlign,
			size_t startCharCnt,
			int drawCharCnt,
			size_t fontSize,
			const wchar_t* s,
			va_list vlist
			);


		// フォントテクスチャを開放する
		bool DeleteAll();


		// 文字列strのoffset番目の文字から，長さlimitに1行で入る文字数を取得
		// 1行の長さは，lengthに書き込まれる
		// limit < 0.0fの場合，長さの指定を無視する
		int GetStrLength(const wchar_t* str, size_t offset, int letterSpace, size_t fontSize, float limit, int &length);



	};






};
