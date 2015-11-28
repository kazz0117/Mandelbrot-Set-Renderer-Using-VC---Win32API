// マンデルブロ集合の描画
// ・マウスの左ドラッグで描画範囲選択
// ・マウスの右クリックでひとつ前の表示にもどる

#include <windows.h>
#include <stdio.h>
#include <math.h>
//#include "debug.h"
#include <math.h>
#include <time.h>
#pragma comment(lib, "winmm.lib")

#define X_MAX (800)
#define Y_MAX (800)
#define MAX_UNDO (64)

// 関数プロトタイプ宣言
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
ATOM InitApp(HINSTANCE);
BOOL InitInstance(HINSTANCE, int);
void dprintf(const char *pFormat, ...);
void saveBMP(HWND, LPCTSTR);

// グローバル変数
TCHAR szClassName[] = TEXT("mandelbrot01");	// ウィンドウクラス
double  Cr1 = -2.3;                           // 定数実部 始点
double  Cr2 = 0.7;                           // 定数実部 終点
double  Ci1 = -1.5;                           // 定数虚部 始点
double  Ci2 = 1.5;                           // 定数虚部 終点

double cct[X_MAX][Y_MAX][2];				// 複素平面座標テーブル([][][0]:r/[][][1]:img)

double UndoBuf[MAX_UNDO][4];				// undo用バッファ
int UndoIdx = 0;							// undoバッファのインデックス

double  E = 4.0;                            // 発散とする値
int     imax = 512;                         // 最大計算回数

HINSTANCE hInst;
POINTS RectStart, RectEnd, RectOldEnd;		// 範囲選択矩形の座標

// Windowsプログラムのエントリーポイント
int WINAPI WinMain(HINSTANCE hCurInst, HINSTANCE hPrevInst, LPSTR lpsCmdLine, int nCmdShow) {
	MSG msg;
	BOOL bRet;

	hInst = hCurInst;

	if (!InitApp(hCurInst)) {
		return FALSE;
	}
	if (!InitInstance(hCurInst, nCmdShow)) {
		return FALSE;
	}

	// メッセージを取得
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
		if (bRet == -1) {
			break;
		}
		else {
			TranslateMessage(&msg);	// メッセージを変換
			DispatchMessage(&msg);	// メッセージを送出
		}
	}
	return (int)msg.wParam;
}

// ウィンドウクラスの登録
ATOM InitApp(HINSTANCE hInst) {
	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);							// 構造体のサイズ
	wc.style = CS_HREDRAW | CS_VREDRAW;						// クラスのスタイル
	wc.lpfnWndProc = WndProc;								// プロシージャ名
	wc.cbClsExtra = 0;										// 補助メモリ
	wc.cbWndExtra = 0;										// 補助メモリ
	wc.hInstance = hInst;									// インスタンス
	wc.hIcon = (HICON)LoadImage(NULL,						// アイコン
		MAKEINTRESOURCE(IDI_APPLICATION),
		IMAGE_ICON,
		0,
		0,
		LR_DEFAULTSIZE | LR_SHARED);
	wc.hCursor = (HCURSOR)LoadImage(NULL,					// カーソル
		MAKEINTRESOURCE(IDC_ARROW),
		IMAGE_CURSOR,
		0,
		0,
		LR_DEFAULTSIZE | LR_SHARED);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);	// 背景ブラシ
	wc.lpszMenuName = NULL;									// メニュー名
	wc.lpszClassName = szClassName;							// クラス名
	wc.hIconSm = (HICON)LoadImage(NULL,						// 小さいアイコン
		MAKEINTRESOURCE(IDI_APPLICATION),
		IMAGE_ICON,
		0,
		0,
		LR_DEFAULTSIZE | LR_SHARED);

	return RegisterClassEx(&wc);
}

// ウィンドウの生成
BOOL InitInstance(HINSTANCE hInst, int nCmdShow) {
	HWND hWnd;

	hWnd = CreateWindow(szClassName,				// クラス名
		TEXT("MANDELBROT SET"),		// ウィンドウ名
		WS_OVERLAPPEDWINDOW,		// ウィンドウスタイル
		CW_USEDEFAULT,				// x位置
		CW_USEDEFAULT,				// y位置
		X_MAX,						// ウィンドウ幅
		Y_MAX,						// ウィンドウ高さ
		NULL,						// 親ウィンドウのハンドル(親を作るときはNULL)
		NULL,						// メニューハンドル(クラスメニューを使うときはNULL)
		hInst,						// インスタンスハンドル
		NULL);						// ウィンドウ作成データ(???)
	if (!hWnd) {
		return FALSE;
	}
	ShowWindow(hWnd, nCmdShow);	// ウィンドウの表示状態を設定
	UpdateWindow(hWnd);			// ウィンドウを更新

	return TRUE;
}

void PushForUndo(double crs, double cre, double cis, double cie) {
	int i = 0;

	if (UndoIdx < MAX_UNDO) {
		UndoBuf[UndoIdx][0] = crs;
		UndoBuf[UndoIdx][1] = cre;
		UndoBuf[UndoIdx][2] = cis;
		UndoBuf[UndoIdx][3] = cie;
		UndoIdx++;
	}
	else {
		for (i = 1; i < MAX_UNDO; i++) {
			UndoBuf[i - 1][0] = UndoBuf[i][0];
			UndoBuf[i - 1][1] = UndoBuf[i][1];
			UndoBuf[i - 1][2] = UndoBuf[i][2];
			UndoBuf[i - 1][3] = UndoBuf[i][3];
		}
		UndoIdx--;
		UndoBuf[UndoIdx][0] = crs;
		UndoBuf[UndoIdx][1] = cre;
		UndoBuf[UndoIdx][2] = cis;
		UndoBuf[UndoIdx][3] = cie;
	}
}

BOOL PopForUndo(double* crs, double* cre, double* cis, double* cie) {
	if (UndoIdx > 0) {
		*crs = UndoBuf[UndoIdx - 1][0];
		*cre = UndoBuf[UndoIdx - 1][1];
		*cis = UndoBuf[UndoIdx - 1][2];
		*cie = UndoBuf[UndoIdx - 1][3];
		UndoIdx--;
	}
	else {
		return FALSE;
	}

	return TRUE;
}

void DrawMandelbrotSet(LPWORD lpDIB, int width, int height) {
	double  Cr, Ci, dCr, dCi;
	double  zr, zi, zrNext, ziNext;
	int     i, x, y;
	BYTE    r, g, b;
	int     idx;

	BYTE* p = (BYTE*)lpDIB;

	ZeroMemory(lpDIB, width * height * 3);

	dCr = (Cr2 - Cr1) / width;	// 実部(横)刻み幅
	dCi = (Ci2 - Ci1) / height;	// 虚部(縦)刻み幅

	for (Ci = Ci1, y = 0; y < height; Ci += dCi, y++) {			// 定数虚部(縦)
		for (Cr = Cr1, x = 0; x < width; Cr += dCr, x++) {	// 定数実部(横)
			zr = 0.0; zi = 0.0;

			// 複素平面座標テーブルを作成
			cct[x][y][0] = Cr;
			cct[x][y][1] = Ci;

			for (i = 0; i < imax; i++) {
				zrNext = zr * zr - zi * zi + Cr;	// 実部漸化式計算
				ziNext = 2 * zr * zi + Ci;		// 虚部漸化式計算

				// 発散判定
				if (zr * zr + zi * zi > E) {
					break;
				}

				zr = zrNext; zi = ziNext;
			}

			if (i >= imax) {
				// 集合の中(収束)

			}
			else {
				// 集合の外(発散)
				r = ((i - 64) % 64) * 4;
				g = ((i - 32) % 32) * 8;
				b = ((i) % 16) * 16;

				idx = x * 3 + y * width * 3;
				p[idx + 0] = b;
				p[idx + 1] = g;
				p[idx + 2] = r;
			}
		}
	}
}

// 範囲選択矩形の描画
void DrawRect(HWND hWnd, POINTS start, POINTS end) {
	HDC hdc;

	hdc = GetDC(hWnd);
	SetROP2(hdc, R2_NOT);

	MoveToEx(hdc, start.x, start.y, NULL);
	LineTo(hdc, end.x, start.y);
	LineTo(hdc, end.x, end.y);
	LineTo(hdc, start.x, end.y);
	LineTo(hdc, start.x, start.y);

	ReleaseDC(hWnd, hdc);

	return;
}

// ウィンドウプロシージャ
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	static LPWORD lpDIB = NULL;
	static BITMAPINFO biDIB;
	static HDC hdc;
	static PAINTSTRUCT ps;
	static HANDLE hHeap;

	static BOOL flgDrawRect = FALSE;	// 範囲選択矩形フラグ
	static BOOL flgCapture = FALSE;		// マウスキャプチャー中フラグ
	// ドラッグ中にマウスがウィンドウの外に出た場合の対応
	int iMouseWheel = 0;	// マウスホイールが回されたときの値
	int SelectedXLength;	// 選択範囲横幅
	int SelectedYLength;	// 選択範囲縦幅
	int SelectedLength;		// 選択範囲幅(正方形とみなすため)
	double tmpC;			// 始点と終点の交換用
	short tmpP;				// 始点と終点の交換用

	TCHAR szFileName[128];
	MMTIME mm;

	switch (msg) {
	case WM_CREATE:
		lpDIB = (LPWORD)HeapAlloc(hHeap = GetProcessHeap(), HEAP_ZERO_MEMORY, X_MAX * Y_MAX * 4);

		DrawMandelbrotSet(lpDIB, X_MAX, Y_MAX);

		// undo用に始点・終点をとっておく
		PushForUndo(Cr1, Cr2, Ci1, Ci2);

		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		ZeroMemory(&biDIB, sizeof(biDIB));
		biDIB.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		biDIB.bmiHeader.biWidth = X_MAX;
		biDIB.bmiHeader.biHeight = Y_MAX;
		biDIB.bmiHeader.biPlanes = 1;
		biDIB.bmiHeader.biBitCount = 24;
		biDIB.bmiHeader.biCompression = BI_RGB;

		StretchDIBits(hdc, 0, 0, X_MAX, Y_MAX, 0, 0, X_MAX, Y_MAX, lpDIB, &biDIB, DIB_RGB_COLORS, SRCCOPY);

		EndPaint(hWnd, &ps);


		break;
#if 0
	case WM_MOUSEWHEEL:
		iMouseWheel = GET_WHEEL_DELTA_WPARAM(wp);

		if (iMouseWheel > 0) {	// 上に回転された(拡大操作)
			Cr1 *= 0.9;
			Cr2 *= 0.9;
			Ci1 *= 0.9;
			Ci2 *= 0.9;
		}
		else {				// 下に回転された(縮小操作)
			Cr1 *= 1.1;
			Cr2 *= 1.1;
			Ci1 *= 1.1;
			Ci2 *= 1.1;
		}

		DrawMandelbrotSet(lpDIB, X_MAX, Y_MAX);

		InvalidateRect(hWnd, NULL, TRUE);

		break;
#endif

	case WM_LBUTTONDOWN:	// マウスの左ボタンが押された
		if (flgCapture == FALSE) {
			SetCapture(hWnd);
			flgCapture = TRUE;
		}

		flgDrawRect = TRUE;

		// マウスカーソルの変更
		SetCursor(LoadCursor(NULL, IDC_CROSS));

		RectOldEnd = RectEnd = RectStart = MAKEPOINTS(lp);	// 押された場所を取得
		DrawRect(hWnd, RectStart, RectEnd);
		break;
	case WM_MOUSEMOVE:
		if (flgDrawRect == TRUE) {
			RectEnd = MAKEPOINTS(lp);	// 現在のマウス位置を取得

			// マウスがウィンドウの外に出た場合にマウス位置を補正する
			if (RectEnd.x >= X_MAX - 17) {
				RectEnd.x = X_MAX - 17;
			}
			if (RectEnd.x <= 1) {
				RectEnd.x = 1;
			}
			if (RectEnd.y >= Y_MAX - 39) {
				RectEnd.y = Y_MAX - 39;
			}
			if (RectEnd.y <= 1) {
				RectEnd.y = 1;
			}

			DrawRect(hWnd, RectStart, RectOldEnd);	// 古い矩形を消す
			DrawRect(hWnd, RectStart, RectEnd);
			RectOldEnd = RectEnd;
		}
		else {
			return DefWindowProc(hWnd, msg, wp, lp);
		}
		break;
	case WM_LBUTTONUP:	// マウスの左ボタンが離された
		if (flgCapture == TRUE) {
			ReleaseCapture();	// キャプチャ終了
			flgCapture = FALSE;
		}

		if (flgDrawRect == TRUE) {
			// マウスカーソルを戻す
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			DrawRect(hWnd, RectStart, RectOldEnd);
			flgDrawRect = FALSE;

			// 選択範囲が小さすぎるときは何もしない
			if (abs(RectStart.x - RectEnd.x) < 3 || abs(RectStart.y - RectEnd.y) < 3) {
				break;
			}

			// 選択範囲のx座標の大小関係の修正
			if (RectStart.x > RectEnd.x) {
				tmpP = RectStart.x;
				RectStart.x = RectEnd.x;
				RectEnd.x = tmpP;
			}

			// 選択範囲のy座標の大小関係の修正
			if (RectStart.y > RectEnd.y) {
				tmpP = RectStart.y;
				RectStart.y = RectEnd.y;
				RectEnd.y = tmpP;
			}

			SelectedXLength = abs(RectEnd.x - RectStart.x);
			SelectedYLength = abs(RectEnd.y - RectStart.y);
			// 選択範囲の縦と横で短い方を採用
			if (SelectedXLength <= SelectedYLength) {
				SelectedLength = SelectedXLength;
			}
			else {
				SelectedLength = SelectedYLength;
			}


			// undo用に始点・終点をとっておく
			PushForUndo(Cr1, Cr2, Ci1, Ci2);

			// 描画範囲を設定
			Cr1 = cct[RectStart.x][Y_MAX - RectStart.y][0];	// 新しい始点(実部)
			Cr2 = cct[RectStart.x + SelectedLength][Y_MAX - (RectStart.y + SelectedLength)][0];	// 新しい終点(実部)
			// 始点≦終点にする
			if (Cr1 > Cr2) {
				tmpC = Cr1;
				Cr1 = Cr2;
				Cr2 = tmpC;
			}

			Ci1 = cct[RectStart.x][Y_MAX - RectStart.y][1];	// 新しい始点(虚部)
			Ci2 = cct[RectStart.x + SelectedLength][Y_MAX - (RectStart.y + SelectedLength)][1];	// 新しい終点(虚部)
			// 始点≦終点にする
			if (Ci1 > Ci2) {
				tmpC = Ci1;
				Ci1 = Ci2;
				Ci2 = tmpC;
			}

			// 処理に時間がかかるのでマウスカーソルをくるくるに変更
			SetCursor(LoadCursor(NULL, IDC_WAIT));

			DrawMandelbrotSet(lpDIB, X_MAX, Y_MAX);

			// マウスカーソルを矢印に戻す
			SetCursor(LoadCursor(NULL, IDC_ARROW));

			InvalidateRect(hWnd, NULL, TRUE);

		}
		else {
			return DefWindowProc(hWnd, msg, wp, lp);
		}
		break;

	case WM_RBUTTONDOWN:	// 右ボタンが押された(undo機能)
		PopForUndo(&Cr1, &Cr2, &Ci1, &Ci2);

		// 処理に時間がかかるのでマウスカーソルをくるくるに変更
		SetCursor(LoadCursor(NULL, IDC_WAIT));
		DrawMandelbrotSet(lpDIB, X_MAX, Y_MAX);
		// マウスカーソルを戻す
		SetCursor(LoadCursor(NULL, IDC_ARROW));

		InvalidateRect(hWnd, NULL, TRUE);

		break;

	case WM_DESTROY:

		HeapFree(hHeap, HEAP_NO_SERIALIZE, lpDIB);

		PostQuitMessage(0);

		break;
	case WM_KEYUP:	// スペースキー押下で画像を保存
		if ((BYTE)wp == VK_SPACE) {
			mm.wType = TIME_MS;
			timeGetSystemTime(&mm, sizeof(MMTIME));
			wsprintf(szFileName, TEXT("img%d.bmp"), mm.u.ms);
			saveBMP(hWnd, szFileName);
		}
		break;

	default:
		return DefWindowProc(hWnd, msg, wp, lp);
	}

	return 0;
}

// クライアント領域をビットマップで保存する
void saveBMP(HWND hWnd, LPCTSTR lpszFn) {
	DWORD dwSize, dwFSize, dwWidth, dwHeight, dwLength;
	HANDLE hFile;
	LPBITMAPFILEHEADER lpHead;
	LPBITMAPINFOHEADER lpInfo;
	LPBYTE lpBuf, lpPixel;
	RECT rect;
	HDC hdc, hdcMem;
	HBITMAP hBMP, hOld;

	GetClientRect(hWnd, &rect);	//クライアント領域取得

	dwWidth = rect.right;
	dwHeight = rect.bottom;

	//バッファの１ラインの長さを計算
	if ((dwWidth * 3) % 4 == 0) {
		dwLength = dwWidth * 3;
	}
	else {
		dwLength = dwWidth * 3 + (4 - (dwWidth * 3) % 4);
	}

	// 書き込み用バッファのサイズ計算
	dwFSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwLength * dwHeight;

	// バッファ確保とポインタ設定
	lpBuf = (LPBYTE)GlobalAlloc(GPTR, dwFSize);
	lpHead = (LPBITMAPFILEHEADER)lpBuf;
	lpInfo = (LPBITMAPINFOHEADER)(lpBuf + sizeof(BITMAPFILEHEADER));
	lpPixel = lpBuf + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	// 24ビットBMPファイルのヘッダ作成
	lpHead->bfType = 'B' + ('M' << 8);
	lpHead->bfSize = dwFSize;
	lpHead->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	lpInfo->biSize = sizeof(BITMAPINFOHEADER);
	lpInfo->biWidth = dwWidth;
	lpInfo->biHeight = dwHeight;
	lpInfo->biPlanes = 1;
	lpInfo->biBitCount = 24;

	// ウインドウのデバイスコンテキスト取得
	hdc = GetDC(hWnd);
	// ウインドウのデバイスコンテキスト互換のBITMAP作成
	hBMP = CreateCompatibleBitmap(hdc, dwWidth, dwHeight);

	// BITMAPにウインドウのクライアント領域をコピー
	hdcMem = CreateCompatibleDC(hdc);
	hOld = SelectObject(hdcMem, hBMP);
	BitBlt(hdcMem, 0, 0, dwWidth, dwHeight, hdc, 0, 0, SRCCOPY);
	SelectObject(hdcMem, hOld);
	GetDIBits(hdc, hBMP, 0, dwHeight, lpPixel, (LPBITMAPINFO)lpInfo, DIB_RGB_COLORS);

	ReleaseDC(hWnd, hdc);
	DeleteObject(hBMP);
	DeleteObject(hdcMem);

	// バッファをファイルに書き出す
	hFile = CreateFile(lpszFn, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	WriteFile(hFile, lpBuf, dwFSize, &dwSize, NULL);
	CloseHandle(hFile);

	GlobalFree(lpBuf);
}
