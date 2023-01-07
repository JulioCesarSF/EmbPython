#pragma once

#include <iostream>
#include <string>

#include "main.h"

#define PY_SSIZE_T_CLEAN
#include "Python.h"

void set_text_to_display(std::string new_text); //prototype hack

//function to export to python
static PyObject* j_set_text(PyObject* self, PyObject* args)
{
	char* new_text;
	if (!PyArg_ParseTuple(args, "s", &new_text))
	{
		return nullptr;
	}

	set_text_to_display(new_text);

	return PyBool_FromLong(1);
}

//exported functions
static PyMethodDef exported_functions[] =
{
	{"j_set_text", j_set_text, METH_VARARGS, "Set a new text to be displayed"},
	{NULL, NULL, 0, NULL}
};

//exported module to be used in python script
static struct PyModuleDef exported_module =
{
	PyModuleDef_HEAD_INIT,
	"myapi", //module name
	"My module description",
	-1,
	exported_functions
};

PyMODINIT_FUNC PyInit_myapi()
{
	//auto created_module = PyModule_Create(&exported_module);
	return PyModule_Create(&exported_module);
}


class python_context
{
private:
	wchar_t* program_name;
	PyConfig config;

	bool is_initialized;
public:

	python_context() : program_name(nullptr), is_initialized(false)
	{
		PyStatus status;

		program_name = Py_DecodeLocale("EmbPython", NULL);
		PyConfig_InitPythonConfig(&config);

		//configure python paths
		status = PyConfig_SetString(&config, &config.program_name, program_name);
		status = PyConfig_SetString(&config, &config.base_executable, L"C:\\Program Files\\Python311\\python.exe");
		status = PyConfig_SetString(&config, &config.base_prefix, L"C:\\Program Files\\Python311");
		status = PyConfig_SetString(&config, &config.base_exec_prefix, L"C:\\Program Files\\Python311");
		status = PyConfig_SetString(&config, &config.platlibdir, L"lib");
		status = PyConfig_SetString(&config, &config.executable, L"C:\\Program Files\\Python311\\python.exe");
		status = PyConfig_SetString(&config, &config.exec_prefix, L"C:\\Program Files\\Python311");

		//add current directory as search path for modules
		config.module_search_paths_set = 1;
		status = PyWideStringList_Append(&config.module_search_paths, L"C:\\Users\\schin\\Documents\\GitHub\\EmbPython\\EmbPython");
		status = PyWideStringList_Append(&config.module_search_paths, L"C:\\Program Files\\Python311");
		status = PyWideStringList_Append(&config.module_search_paths, L"C:\\Program Files\\Python311\\Lib");

		status = Py_InitializeFromConfig(&config);
		if (status._type == status._PyStatus_TYPE_OK)
		{			
			//create module
			auto pObjec = PyImport_AddModule("myapi");
			//initialize
			auto init_module = PyImport_AppendInittab("myapi", &PyInit_myapi);
			//add functions
			auto add_functions = PyModule_AddFunctions(pObjec, exported_functions);
			Py_InitializeEx(0);
			is_initialized = Py_IsInitialized() != 0;
		}
		else
		{
			OutputDebugString(status.err_msg);
			MessageBox(NULL, status.err_msg, "Failed to initialize Python", MB_ICONERROR);
			is_initialized = false;
		}
	}

	~python_context()
	{
		Py_FinalizeEx();
		PyConfig_Clear(&config);
	}

	bool get_is_initialized()
	{
		return is_initialized;
	}

	bool execute_function_module_by_name(std::string module_name, std::string function_name)
	{
		PyObject* s_mod = PyUnicode_FromString(module_name.c_str());
		PyObject* p_module = PyImport_Import(s_mod);

		if (p_module != nullptr)
		{
			PyObject* p_function = PyObject_GetAttrString(p_module, function_name.c_str());

			if (p_function != nullptr && PyCallable_Check(p_function))
			{
				PyObject* myResult = PyObject_CallObject(p_function, NULL);
				int result = _PyLong_AsInt(myResult);
				OutputDebugString(std::to_string(result).c_str());
				return true;
			}
		}
		return false;
	}

	int execute_python_script_by_file(std::string file_path)
	{
		FILE* cp;
		errno_t error = fopen_s(&cp, file_path.c_str(), "r");
		if (cp == nullptr) return -1;
		int executed = PyRun_SimpleFile(cp, file_path.c_str());
		fclose(cp);
		return executed;
	}

	const char* execute_python_text_script(std::string text_script)
	{
		FILE* p_file = nullptr;
		freopen_s(&p_file, "output_execution.txt", "w+", stdout);

		if (p_file == nullptr)
			return nullptr;

		// this is ugly
		std::string stdOutErr =
			"import sys\n\
class CatchOutErr:\n\
    def __init__(self):\n\
        self.value = ''\n\
    def write(self, txt):\n\
        self.value += txt\n\
catchOutErr = CatchOutErr()\n\
sys.stdout = catchOutErr\n\
sys.stderr = catchOutErr\n\
"; //this is python code to redirect stdouts/stderr
		PyObject* pModule = PyImport_AddModule("__main__"); //create main module
		int result = PyRun_SimpleString(stdOutErr.c_str());
		result = PyRun_SimpleString(text_script.c_str());
		PyObject* catcher = PyObject_GetAttrString(pModule, "catchOutErr");
		PyErr_Print();
		PyObject* output = PyObject_GetAttrString(catcher, "value");
		const char* text = PyUnicode_AsUTF8(output);
		std::cout << text;
		fclose(stdout);

		return text;
	}
};