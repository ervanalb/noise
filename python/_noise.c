#include <Python.h>
#include <noise.h>

static PyObject * noise_obj_create(PyObject * self, PyObject * args) {
    return self;
}

static PyMethodDef module_methods[] = {
    { "obj_create", noise_obj_create, METH_VARARGS, NULL },
    { NULL, NULL, 0, NULL }
};

//static const char * module_docstring = "Noise - like MaxMSP but not.";

PyMODINIT_FUNC init_noise(void) {
    PyObject * m = Py_InitModule("_noise", module_methods);
    if (m == NULL) return;
}
