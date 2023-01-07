#pragma once

#include <Windows.h>
#include <string>
#include <memory>

#include "python_context.h"

const int width = 800;
const int height = 600;
const char CLASS_NAME[] = "EmbPython";

static std::string text_to_string("Hello Python");
static HWND h_main_window = nullptr;

//child windows
const int ID_PYTHON_EDITOR_EDIT = 1;
static HWND python_editor_edit = nullptr;

const int ID_PYTHON_EDITOR_BUTTON = 2;
static HWND python_editor_button = nullptr;

const int ID_PYTHON_EDITOR_RESULT = 3;
static HWND python_editor_result = nullptr;

const int ID_PYTHON_EDITOR_RESULT_TITLE = 4;
static HWND python_editor_result_title = nullptr;

static HFONT text_font = nullptr;

// messages callback
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/// <summary>
/// Register a WNDCLASSEX to Windows
/// </summary>
static bool register_class(HINSTANCE hInstance)
{
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

	auto set = RegisterClassEx(&wc) != 0;

#ifdef _DEBUG
	if (!set)
	{
		auto last_error = GetLastError();
		OutputDebugString(std::to_string(last_error).c_str());
	}
#endif // DEBUG

	return set;
}

/// <summary>
/// Bump messages from queue
/// </summary>
static int loop_message(HINSTANCE hInstance)
{
	MSG msg = { 0 };

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnregisterClass(CLASS_NAME, hInstance);

	return static_cast<int>(msg.wParam);
}

static void create_child_windows(HWND hwnd)
{
	text_font = CreateFont(26, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
	
	//https://learn.microsoft.com/en-us/windows/win32/controls/use-a-multiline-edit-control
	python_editor_edit = CreateWindowEx(
		0, "EDIT",   // predefined class 
		NULL,         // no window title 
		WS_CHILD | WS_VISIBLE | WS_VSCROLL |
		ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
		0, 0, width / 2, height / 2,   // set size in WM_SIZE message 
		hwnd,         // parent window 
		reinterpret_cast<HMENU>(ID_PYTHON_EDITOR_EDIT),   // edit control ID 
		(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
		NULL);        // pointer not needed 
	SendMessage(python_editor_edit, WM_SETFONT, WPARAM(text_font), TRUE);

	python_editor_result_title = CreateWindow(
		"STATIC", NULL,
		WS_VISIBLE | WS_CHILD,
		0, (height / 2),
		75, 55,
		hwnd,
		reinterpret_cast<HMENU>(ID_PYTHON_EDITOR_RESULT_TITLE),
		(HINSTANCE)GetWindowLong(hwnd, GWLP_HINSTANCE),
		NULL);

	SetWindowText(python_editor_result_title, "Execution result");

	python_editor_result = CreateWindowEx(
		ES_READONLY, "EDIT",
		NULL,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL |
		ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
		0, (height / 2) + 40,
		width / 2, height / 2 - 120,
		hwnd,
		reinterpret_cast<HMENU>(ID_PYTHON_EDITOR_RESULT),
		(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

	SendDlgItemMessage(python_editor_result, ID_PYTHON_EDITOR_RESULT, EM_SETREADONLY, TRUE, 0);
	SendMessage(python_editor_result, WM_SETFONT, WPARAM(text_font), TRUE);

	//https://learn.microsoft.com/pt-br/windows/win32/controls/create-a-button
	python_editor_button = CreateWindow(
		"BUTTON",
		"EXECUTE",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		0, height - 75,
		width / 2, 35,
		hwnd,
		reinterpret_cast<HMENU>(ID_PYTHON_EDITOR_BUTTON),
		(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
		NULL);
	SendMessage(python_editor_button, WM_SETFONT, WPARAM(text_font), TRUE);
}

static void destroy_child_windows()
{
	DestroyWindow(python_editor_edit);
	DestroyWindow(python_editor_button);
	DestroyWindow(python_editor_result_title);
	DestroyWindow(python_editor_result);
	DeleteObject(text_font);
}

static void set_text_to_display(std::string new_text)
{
	text_to_string = new_text;
	InvalidateRect(h_main_window, NULL, TRUE);	
}