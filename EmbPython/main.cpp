#include "main.h"

/// <summary>
/// Entry point
/// </summary>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
	if (!register_class(hInstance))
		return -1;

	auto p_context = std::make_shared<python_context>();

	if (!p_context->get_is_initialized()) return -99;

	h_main_window =
		CreateWindowEx(NULL,
			CLASS_NAME, CLASS_NAME, WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, //position
			width, height, //size
			NULL, NULL,
			hInstance, p_context.get());

	if (h_main_window == nullptr)
		return -2;

	ShowWindow(h_main_window, nCmdShow);
	UpdateWindow(h_main_window);

	return loop_message(hInstance);
}

/// <summary>
/// Handle messages
/// </summary>
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
	{
		python_context* p_context = (python_context*)((LPCREATESTRUCT)lParam)->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)p_context);
		create_child_windows(hwnd);
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, RGB(0, 0, 255));
		RECT windowRect = { 0 };
		GetClientRect(hwnd, &windowRect);
		windowRect.top = height / 4;
		windowRect.left = (width / 2) + (width * 0.70);
		windowRect.bottom = 200;
		windowRect.right = 200;
		DrawText(hdc, text_to_string.c_str(), text_to_string.size(), &windowRect, DT_CENTER);
		EndPaint(hwnd, &ps);
	}
	break;
	case WM_COMMAND:
	{
		switch (wParam)
		{
		case ID_PYTHON_EDITOR_BUTTON:
		{
			HWND hwnd_edit = GetDlgItem(hwnd, ID_PYTHON_EDITOR_EDIT);
			auto script_size = GetWindowTextLength(hwnd_edit);

			if (script_size > 0)
			{
				char* text = new char[script_size + 1];
				GetWindowText(hwnd_edit, text, script_size + 1);

				auto lp_user_data = GetWindowLongPtr(hwnd, GWLP_USERDATA);
				python_context* p_context = (python_context*)(lp_user_data);
				auto result = p_context->execute_python_text_script(text);

				if (result != nullptr)
				{
					HWND hwnd_result = GetDlgItem(hwnd, ID_PYTHON_EDITOR_RESULT);
					SetWindowText(hwnd_result, result);
				}

				delete[] text;
			}

		}
		break;
		}
	}
	break;
	case WM_QUIT:
		destroy_child_windows();
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}