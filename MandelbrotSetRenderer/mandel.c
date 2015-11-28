// �}���f���u���W���̕`��
// �E�}�E�X�̍��h���b�O�ŕ`��͈͑I��
// �E�}�E�X�̉E�N���b�N�łЂƂO�̕\���ɂ��ǂ�

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

// �֐��v���g�^�C�v�錾
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
ATOM InitApp(HINSTANCE);
BOOL InitInstance(HINSTANCE, int);
void dprintf(const char *pFormat, ...);
void saveBMP(HWND, LPCTSTR);

// �O���[�o���ϐ�
TCHAR szClassName[] = TEXT("mandelbrot01");	// �E�B���h�E�N���X
double  Cr1 = -2.3;                           // �萔���� �n�_
double  Cr2 = 0.7;                           // �萔���� �I�_
double  Ci1 = -1.5;                           // �萔���� �n�_
double  Ci2 = 1.5;                           // �萔���� �I�_

double cct[X_MAX][Y_MAX][2];				// ���f���ʍ��W�e�[�u��([][][0]:r/[][][1]:img)

double UndoBuf[MAX_UNDO][4];				// undo�p�o�b�t�@
int UndoIdx = 0;							// undo�o�b�t�@�̃C���f�b�N�X

double  E = 4.0;                            // ���U�Ƃ���l
int     imax = 512;                         // �ő�v�Z��

HINSTANCE hInst;
POINTS RectStart, RectEnd, RectOldEnd;		// �͈͑I����`�̍��W

// Windows�v���O�����̃G���g���[�|�C���g
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

	// ���b�Z�[�W���擾
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
		if (bRet == -1) {
			break;
		}
		else {
			TranslateMessage(&msg);	// ���b�Z�[�W��ϊ�
			DispatchMessage(&msg);	// ���b�Z�[�W�𑗏o
		}
	}
	return (int)msg.wParam;
}

// �E�B���h�E�N���X�̓o�^
ATOM InitApp(HINSTANCE hInst) {
	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);							// �\���̂̃T�C�Y
	wc.style = CS_HREDRAW | CS_VREDRAW;						// �N���X�̃X�^�C��
	wc.lpfnWndProc = WndProc;								// �v���V�[�W����
	wc.cbClsExtra = 0;										// �⏕������
	wc.cbWndExtra = 0;										// �⏕������
	wc.hInstance = hInst;									// �C���X�^���X
	wc.hIcon = (HICON)LoadImage(NULL,						// �A�C�R��
		MAKEINTRESOURCE(IDI_APPLICATION),
		IMAGE_ICON,
		0,
		0,
		LR_DEFAULTSIZE | LR_SHARED);
	wc.hCursor = (HCURSOR)LoadImage(NULL,					// �J�[�\��
		MAKEINTRESOURCE(IDC_ARROW),
		IMAGE_CURSOR,
		0,
		0,
		LR_DEFAULTSIZE | LR_SHARED);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);	// �w�i�u���V
	wc.lpszMenuName = NULL;									// ���j���[��
	wc.lpszClassName = szClassName;							// �N���X��
	wc.hIconSm = (HICON)LoadImage(NULL,						// �������A�C�R��
		MAKEINTRESOURCE(IDI_APPLICATION),
		IMAGE_ICON,
		0,
		0,
		LR_DEFAULTSIZE | LR_SHARED);

	return RegisterClassEx(&wc);
}

// �E�B���h�E�̐���
BOOL InitInstance(HINSTANCE hInst, int nCmdShow) {
	HWND hWnd;

	hWnd = CreateWindow(szClassName,				// �N���X��
		TEXT("MANDELBROT SET"),		// �E�B���h�E��
		WS_OVERLAPPEDWINDOW,		// �E�B���h�E�X�^�C��
		CW_USEDEFAULT,				// x�ʒu
		CW_USEDEFAULT,				// y�ʒu
		X_MAX,						// �E�B���h�E��
		Y_MAX,						// �E�B���h�E����
		NULL,						// �e�E�B���h�E�̃n���h��(�e�����Ƃ���NULL)
		NULL,						// ���j���[�n���h��(�N���X���j���[���g���Ƃ���NULL)
		hInst,						// �C���X�^���X�n���h��
		NULL);						// �E�B���h�E�쐬�f�[�^(???)
	if (!hWnd) {
		return FALSE;
	}
	ShowWindow(hWnd, nCmdShow);	// �E�B���h�E�̕\����Ԃ�ݒ�
	UpdateWindow(hWnd);			// �E�B���h�E���X�V

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

	dCr = (Cr2 - Cr1) / width;	// ����(��)���ݕ�
	dCi = (Ci2 - Ci1) / height;	// ����(�c)���ݕ�

	for (Ci = Ci1, y = 0; y < height; Ci += dCi, y++) {			// �萔����(�c)
		for (Cr = Cr1, x = 0; x < width; Cr += dCr, x++) {	// �萔����(��)
			zr = 0.0; zi = 0.0;

			// ���f���ʍ��W�e�[�u�����쐬
			cct[x][y][0] = Cr;
			cct[x][y][1] = Ci;

			for (i = 0; i < imax; i++) {
				zrNext = zr * zr - zi * zi + Cr;	// �����Q�����v�Z
				ziNext = 2 * zr * zi + Ci;		// �����Q�����v�Z

				// ���U����
				if (zr * zr + zi * zi > E) {
					break;
				}

				zr = zrNext; zi = ziNext;
			}

			if (i >= imax) {
				// �W���̒�(����)

			}
			else {
				// �W���̊O(���U)
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

// �͈͑I����`�̕`��
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

// �E�B���h�E�v���V�[�W��
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	static LPWORD lpDIB = NULL;
	static BITMAPINFO biDIB;
	static HDC hdc;
	static PAINTSTRUCT ps;
	static HANDLE hHeap;

	static BOOL flgDrawRect = FALSE;	// �͈͑I����`�t���O
	static BOOL flgCapture = FALSE;		// �}�E�X�L���v�`���[���t���O
	// �h���b�O���Ƀ}�E�X���E�B���h�E�̊O�ɏo���ꍇ�̑Ή�
	int iMouseWheel = 0;	// �}�E�X�z�C�[�����񂳂ꂽ�Ƃ��̒l
	int SelectedXLength;	// �I��͈͉���
	int SelectedYLength;	// �I��͈͏c��
	int SelectedLength;		// �I��͈͕�(�����`�Ƃ݂Ȃ�����)
	double tmpC;			// �n�_�ƏI�_�̌����p
	short tmpP;				// �n�_�ƏI�_�̌����p

	TCHAR szFileName[128];
	MMTIME mm;

	switch (msg) {
	case WM_CREATE:
		lpDIB = (LPWORD)HeapAlloc(hHeap = GetProcessHeap(), HEAP_ZERO_MEMORY, X_MAX * Y_MAX * 4);

		DrawMandelbrotSet(lpDIB, X_MAX, Y_MAX);

		// undo�p�Ɏn�_�E�I�_���Ƃ��Ă���
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

		if (iMouseWheel > 0) {	// ��ɉ�]���ꂽ(�g�呀��)
			Cr1 *= 0.9;
			Cr2 *= 0.9;
			Ci1 *= 0.9;
			Ci2 *= 0.9;
		}
		else {				// ���ɉ�]���ꂽ(�k������)
			Cr1 *= 1.1;
			Cr2 *= 1.1;
			Ci1 *= 1.1;
			Ci2 *= 1.1;
		}

		DrawMandelbrotSet(lpDIB, X_MAX, Y_MAX);

		InvalidateRect(hWnd, NULL, TRUE);

		break;
#endif

	case WM_LBUTTONDOWN:	// �}�E�X�̍��{�^���������ꂽ
		if (flgCapture == FALSE) {
			SetCapture(hWnd);
			flgCapture = TRUE;
		}

		flgDrawRect = TRUE;

		// �}�E�X�J�[�\���̕ύX
		SetCursor(LoadCursor(NULL, IDC_CROSS));

		RectOldEnd = RectEnd = RectStart = MAKEPOINTS(lp);	// �����ꂽ�ꏊ���擾
		DrawRect(hWnd, RectStart, RectEnd);
		break;
	case WM_MOUSEMOVE:
		if (flgDrawRect == TRUE) {
			RectEnd = MAKEPOINTS(lp);	// ���݂̃}�E�X�ʒu���擾

			// �}�E�X���E�B���h�E�̊O�ɏo���ꍇ�Ƀ}�E�X�ʒu��␳����
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

			DrawRect(hWnd, RectStart, RectOldEnd);	// �Â���`������
			DrawRect(hWnd, RectStart, RectEnd);
			RectOldEnd = RectEnd;
		}
		else {
			return DefWindowProc(hWnd, msg, wp, lp);
		}
		break;
	case WM_LBUTTONUP:	// �}�E�X�̍��{�^���������ꂽ
		if (flgCapture == TRUE) {
			ReleaseCapture();	// �L���v�`���I��
			flgCapture = FALSE;
		}

		if (flgDrawRect == TRUE) {
			// �}�E�X�J�[�\����߂�
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			DrawRect(hWnd, RectStart, RectOldEnd);
			flgDrawRect = FALSE;

			// �I��͈͂�����������Ƃ��͉������Ȃ�
			if (abs(RectStart.x - RectEnd.x) < 3 || abs(RectStart.y - RectEnd.y) < 3) {
				break;
			}

			// �I��͈͂�x���W�̑召�֌W�̏C��
			if (RectStart.x > RectEnd.x) {
				tmpP = RectStart.x;
				RectStart.x = RectEnd.x;
				RectEnd.x = tmpP;
			}

			// �I��͈͂�y���W�̑召�֌W�̏C��
			if (RectStart.y > RectEnd.y) {
				tmpP = RectStart.y;
				RectStart.y = RectEnd.y;
				RectEnd.y = tmpP;
			}

			SelectedXLength = abs(RectEnd.x - RectStart.x);
			SelectedYLength = abs(RectEnd.y - RectStart.y);
			// �I��͈͂̏c�Ɖ��ŒZ�������̗p
			if (SelectedXLength <= SelectedYLength) {
				SelectedLength = SelectedXLength;
			}
			else {
				SelectedLength = SelectedYLength;
			}


			// undo�p�Ɏn�_�E�I�_���Ƃ��Ă���
			PushForUndo(Cr1, Cr2, Ci1, Ci2);

			// �`��͈͂�ݒ�
			Cr1 = cct[RectStart.x][Y_MAX - RectStart.y][0];	// �V�����n�_(����)
			Cr2 = cct[RectStart.x + SelectedLength][Y_MAX - (RectStart.y + SelectedLength)][0];	// �V�����I�_(����)
			// �n�_���I�_�ɂ���
			if (Cr1 > Cr2) {
				tmpC = Cr1;
				Cr1 = Cr2;
				Cr2 = tmpC;
			}

			Ci1 = cct[RectStart.x][Y_MAX - RectStart.y][1];	// �V�����n�_(����)
			Ci2 = cct[RectStart.x + SelectedLength][Y_MAX - (RectStart.y + SelectedLength)][1];	// �V�����I�_(����)
			// �n�_���I�_�ɂ���
			if (Ci1 > Ci2) {
				tmpC = Ci1;
				Ci1 = Ci2;
				Ci2 = tmpC;
			}

			// �����Ɏ��Ԃ�������̂Ń}�E�X�J�[�\�������邭��ɕύX
			SetCursor(LoadCursor(NULL, IDC_WAIT));

			DrawMandelbrotSet(lpDIB, X_MAX, Y_MAX);

			// �}�E�X�J�[�\������ɖ߂�
			SetCursor(LoadCursor(NULL, IDC_ARROW));

			InvalidateRect(hWnd, NULL, TRUE);

		}
		else {
			return DefWindowProc(hWnd, msg, wp, lp);
		}
		break;

	case WM_RBUTTONDOWN:	// �E�{�^���������ꂽ(undo�@�\)
		PopForUndo(&Cr1, &Cr2, &Ci1, &Ci2);

		// �����Ɏ��Ԃ�������̂Ń}�E�X�J�[�\�������邭��ɕύX
		SetCursor(LoadCursor(NULL, IDC_WAIT));
		DrawMandelbrotSet(lpDIB, X_MAX, Y_MAX);
		// �}�E�X�J�[�\����߂�
		SetCursor(LoadCursor(NULL, IDC_ARROW));

		InvalidateRect(hWnd, NULL, TRUE);

		break;

	case WM_DESTROY:

		HeapFree(hHeap, HEAP_NO_SERIALIZE, lpDIB);

		PostQuitMessage(0);

		break;
	case WM_KEYUP:	// �X�y�[�X�L�[�����ŉ摜��ۑ�
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

// �N���C�A���g�̈���r�b�g�}�b�v�ŕۑ�����
void saveBMP(HWND hWnd, LPCTSTR lpszFn) {
	DWORD dwSize, dwFSize, dwWidth, dwHeight, dwLength;
	HANDLE hFile;
	LPBITMAPFILEHEADER lpHead;
	LPBITMAPINFOHEADER lpInfo;
	LPBYTE lpBuf, lpPixel;
	RECT rect;
	HDC hdc, hdcMem;
	HBITMAP hBMP, hOld;

	GetClientRect(hWnd, &rect);	//�N���C�A���g�̈�擾

	dwWidth = rect.right;
	dwHeight = rect.bottom;

	//�o�b�t�@�̂P���C���̒������v�Z
	if ((dwWidth * 3) % 4 == 0) {
		dwLength = dwWidth * 3;
	}
	else {
		dwLength = dwWidth * 3 + (4 - (dwWidth * 3) % 4);
	}

	// �������ݗp�o�b�t�@�̃T�C�Y�v�Z
	dwFSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwLength * dwHeight;

	// �o�b�t�@�m�ۂƃ|�C���^�ݒ�
	lpBuf = (LPBYTE)GlobalAlloc(GPTR, dwFSize);
	lpHead = (LPBITMAPFILEHEADER)lpBuf;
	lpInfo = (LPBITMAPINFOHEADER)(lpBuf + sizeof(BITMAPFILEHEADER));
	lpPixel = lpBuf + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	// 24�r�b�gBMP�t�@�C���̃w�b�_�쐬
	lpHead->bfType = 'B' + ('M' << 8);
	lpHead->bfSize = dwFSize;
	lpHead->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	lpInfo->biSize = sizeof(BITMAPINFOHEADER);
	lpInfo->biWidth = dwWidth;
	lpInfo->biHeight = dwHeight;
	lpInfo->biPlanes = 1;
	lpInfo->biBitCount = 24;

	// �E�C���h�E�̃f�o�C�X�R���e�L�X�g�擾
	hdc = GetDC(hWnd);
	// �E�C���h�E�̃f�o�C�X�R���e�L�X�g�݊���BITMAP�쐬
	hBMP = CreateCompatibleBitmap(hdc, dwWidth, dwHeight);

	// BITMAP�ɃE�C���h�E�̃N���C�A���g�̈���R�s�[
	hdcMem = CreateCompatibleDC(hdc);
	hOld = SelectObject(hdcMem, hBMP);
	BitBlt(hdcMem, 0, 0, dwWidth, dwHeight, hdc, 0, 0, SRCCOPY);
	SelectObject(hdcMem, hOld);
	GetDIBits(hdc, hBMP, 0, dwHeight, lpPixel, (LPBITMAPINFO)lpInfo, DIB_RGB_COLORS);

	ReleaseDC(hWnd, hdc);
	DeleteObject(hBMP);
	DeleteObject(hdcMem);

	// �o�b�t�@���t�@�C���ɏ����o��
	hFile = CreateFile(lpszFn, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	WriteFile(hFile, lpBuf, dwFSize, &dwSize, NULL);
	CloseHandle(hFile);

	GlobalFree(lpBuf);
}
