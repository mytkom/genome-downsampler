#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "gds.h"

static PyObject* py_downsample(PyObject* self, PyObject* args, PyObject* kwargs) {
    static char* kwlist[] = {
        "input_path",
        "output_path",
        "max_coverage",
        "solver_name",
        "hts_thread_count",
        "min_mapq",
        "min_seq_length",
        "amp_overflow",
        "min_alignment",
        "bed_path",
        "tsv_path",
        "preprocessing_out_path",
        "verbose",
        NULL,
    };

    const char* input_path = NULL;
    const char* output_path = NULL;
    unsigned int max_coverage = 0;
    const char* solver_name = NULL;
    unsigned int hts_thread_count = 0;
    unsigned int min_mapq = 0;
    unsigned int min_seq_length = 0;
    unsigned int amp_overflow = 0;
    double min_alignment = 0.0;
    const char* bed_path = NULL;
    const char* tsv_path = NULL;
    const char* preprocessing_out_path = NULL;
    int verbose = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ssI|zIIIIdzzzp", kwlist, &input_path,
                                     &output_path, &max_coverage, &solver_name,
                                     &hts_thread_count, &min_mapq, &min_seq_length,
                                     &amp_overflow, &min_alignment, &bed_path, &tsv_path,
                                     &preprocessing_out_path, &verbose)) {
        return NULL;
    }

    GdsConfig config;
    gds_config_init(&config);
    config.max_coverage = max_coverage;
    config.solver_name = solver_name;
    config.hts_thread_count = hts_thread_count;
    config.min_mapq = min_mapq;
    config.min_seq_length = min_seq_length;
    config.amp_overflow = amp_overflow;
    config.min_alignment = (float)min_alignment;
    config.bed_path = bed_path;
    config.tsv_path = tsv_path;
    config.preprocessing_out_path = preprocessing_out_path;
    config.verbose = verbose;

    int rc = 0;
    Py_BEGIN_ALLOW_THREADS
    rc = gds_downsample(&config, input_path, output_path);
    Py_END_ALLOW_THREADS

    if (rc != 0) {
        PyErr_SetString(PyExc_RuntimeError, gds_last_error());
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyMethodDef gds_methods[] = {
    {"downsample", (PyCFunction)py_downsample, METH_VARARGS | METH_KEYWORDS,
     PyDoc_STR(
         "downsample(input_path, output_path, max_coverage, *, solver_name='quasi-mcp', ...)\n"
         "--\n\n"
         "Run downsampling on a BAM file using libgds_c.\n"
         "Optional keyword arguments match GdsConfig fields; unset values use native defaults.")},
    {NULL, NULL, 0, NULL},
};

static struct PyModuleDef gds_module = {
    PyModuleDef_HEAD_INIT,
    "_gds",
    "Low-level genome-downsampler extension module.",
    -1,
    gds_methods,
};

PyMODINIT_FUNC PyInit__gds(void) { return PyModule_Create(&gds_module); }
