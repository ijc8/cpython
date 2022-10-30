/* Minimal main program -- everything is loaded from the library */

#include "Python.h"

// Alternator exports:
#define N 1024

static float output[N];
static PyObject *pProcess, *pProcessArgs;

void *setup(float sample_rate) {
    PyObject *pName, *pModule, *pSetup, *pArgs, *pValue, *pTmp;

    // Set up interpreter.
    wchar_t *path = Py_DecodeLocale(":/Lib", NULL);
    Py_SetPath(path);
    Py_Initialize();

    // Load module.
    pName = PyUnicode_FromString("main");
    pModule = PyImport_Import(pName);
    if (pModule == NULL) {
        PyErr_Print();
        return NULL;
    }
    Py_DECREF(pName);
    pSetup = PyObject_GetAttrString(pModule, "setup");
    if (pSetup == NULL) {
        PyErr_Print();
        return NULL;
    }
    pProcess = PyObject_GetAttrString(pModule, "process");
    if (pProcess == NULL) {
        PyErr_Print();
        return NULL;
    }
    Py_DECREF(pModule);

    // Call `setup()`.
    pValue = PyObject_CallFunction(pSetup, "f", sample_rate);
    if (pValue == NULL) {
        PyErr_Print();
        return NULL;
    }
    Py_DECREF(pValue);
    Py_DECREF(pSetup);

    // Set up `memoryview` object and argument tuple for calls to `process()`.
    pTmp = PyMemoryView_FromMemory((char *)output, sizeof(float) * N, PyBUF_WRITE);
    pValue = PyObject_CallMethod(pTmp, "cast", "s", "f");
    Py_DECREF(pTmp);
    pProcessArgs = PyTuple_Pack(1, pValue); // pValue reference is stolen.

    PyMem_RawFree(path);

    return output;
}

int process() {
    PyObject *pNumFrames = PyObject_CallObject(pProcess, pProcessArgs);
    int result = _PyLong_AsInt(pNumFrames);
    Py_DECREF(pNumFrames);
    return result;
}

// Normal Python:
#ifdef MS_WINDOWS
int
wmain(int argc, wchar_t **argv)
{
    return Py_Main(argc, argv);
}
#else
int
main(int argc, char **argv)
{
    return Py_BytesMain(argc, argv);
}
#endif
