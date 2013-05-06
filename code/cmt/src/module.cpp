#define PY_ARRAY_UNIQUE_SYMBOL CMT_ARRAY_API

#include <Python.h>
#include <arrayobject.h>
#include <structmember.h>
#include <stdlib.h>
#include <sys/time.h>
#include "conditionaldistributioninterface.h"
#include "distribution.h"
#include "distributioninterface.h"
#include "glminterface.h"
#include "mcbminterface.h"
#include "mcgsminterface.h"
#include "preconditionerinterface.h"
#include "toolsinterface.h"
#include "trainableinterface.h"
#include "Eigen/Core"

static const char* cmt_doc =
	"This module provides C++ implementations of conditional models such "
	"as the MCGSM, as well as tools for solving applications such as "
	"filling-in, denoising, or classification.";

static PyGetSetDef Distribution_getset[] = {
	{"dim", (getter)Distribution_dim, 0, "Dimensionality of the distribution."},
	{0}
};

static PyMethodDef Distribution_methods[] = {
	{"sample", (PyCFunction)Distribution_sample, METH_VARARGS|METH_KEYWORDS, Distribution_sample_doc},
	{"loglikelihood",
		(PyCFunction)Distribution_loglikelihood,
		METH_VARARGS|METH_KEYWORDS,
		Distribution_loglikelihood_doc},
	{"evaluate",
		(PyCFunction)Distribution_evaluate,
		METH_VARARGS|METH_KEYWORDS,
		Distribution_evaluate_doc},
	{0}
};

PyTypeObject Distribution_type = {
	PyObject_HEAD_INIT(0)
	0,                                /*ob_size*/
	"cmt.ConditionalDistribution",    /*tp_name*/
	sizeof(DistributionObject),       /*tp_basicsize*/
	0,                                /*tp_itemsize*/
	(destructor)Distribution_dealloc, /*tp_dealloc*/
	0,                                /*tp_print*/
	0,                                /*tp_getattr*/
	0,                                /*tp_setattr*/
	0,                                /*tp_compare*/
	0,                                /*tp_repr*/
	0,                                /*tp_as_number*/
	0,                                /*tp_as_sequence*/
	0,                                /*tp_as_mapping*/
	0,                                /*tp_hash */
	0,                                /*tp_call*/
	0,                                /*tp_str*/
	0,                                /*tp_getattro*/
	0,                                /*tp_setattro*/
	0,                                /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,               /*tp_flags*/
	Distribution_doc,                 /*tp_doc*/
	0,                                /*tp_traverse*/
	0,                                /*tp_clear*/
	0,                                /*tp_richcompare*/
	0,                                /*tp_weaklistoffset*/
	0,                                /*tp_iter*/
	0,                                /*tp_iternext*/
	Distribution_methods,             /*tp_methods*/
	0,                                /*tp_members*/
	Distribution_getset,              /*tp_getset*/
	0,                                /*tp_base*/
	0,                                /*tp_dict*/
	0,                                /*tp_descr_get*/
	0,                                /*tp_descr_set*/
	0,                                /*tp_dictoffset*/
	(initproc)Distribution_init,      /*tp_init*/
	0,                                /*tp_alloc*/
	Distribution_new,                 /*tp_new*/
};

static PyGetSetDef CD_getset[] = {
	{"dim_in", (getter)CD_dim_in, 0, "Dimensionality of inputs."},
	{"dim_out", (getter)CD_dim_out, 0, "Dimensionality of outputs."},
	{0}
};

static PyMethodDef CD_methods[] = {
	{"sample", (PyCFunction)CD_sample, METH_VARARGS|METH_KEYWORDS, CD_sample_doc},
	{"loglikelihood",
		(PyCFunction)CD_loglikelihood,
		METH_VARARGS|METH_KEYWORDS,
		CD_loglikelihood_doc},
	{"evaluate",
		(PyCFunction)CD_evaluate,
		METH_VARARGS|METH_KEYWORDS,
		CD_evaluate_doc},
	{0}
};

PyTypeObject CD_type = {
	PyObject_HEAD_INIT(0)
	0,                             /*ob_size*/
	"cmt.ConditionalDistribution", /*tp_name*/
	sizeof(CDObject),              /*tp_basicsize*/
	0,                             /*tp_itemsize*/
	(destructor)CD_dealloc,        /*tp_dealloc*/
	0,                             /*tp_print*/
	0,                             /*tp_getattr*/
	0,                             /*tp_setattr*/
	0,                             /*tp_compare*/
	0,                             /*tp_repr*/
	0,                             /*tp_as_number*/
	0,                             /*tp_as_sequence*/
	0,                             /*tp_as_mapping*/
	0,                             /*tp_hash */
	0,                             /*tp_call*/
	0,                             /*tp_str*/
	0,                             /*tp_getattro*/
	0,                             /*tp_setattro*/
	0,                             /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,            /*tp_flags*/
	CD_doc,                        /*tp_doc*/
	0,                             /*tp_traverse*/
	0,                             /*tp_clear*/
	0,                             /*tp_richcompare*/
	0,                             /*tp_weaklistoffset*/
	0,                             /*tp_iter*/
	0,                             /*tp_iternext*/
	CD_methods,                    /*tp_methods*/
	0,                             /*tp_members*/
	CD_getset,                     /*tp_getset*/
	0,                             /*tp_base*/
	0,                             /*tp_dict*/
	0,                             /*tp_descr_get*/
	0,                             /*tp_descr_set*/
	0,                             /*tp_dictoffset*/
	(initproc)CD_init,             /*tp_init*/
	0,                             /*tp_alloc*/
	CD_new,                        /*tp_new*/
};

static PyGetSetDef MCGSM_getset[] = {
	{"num_components", (getter)MCGSM_num_components, 0, "Numer of predictors."},
	{"num_scales", (getter)MCGSM_num_scales, 0, "Number of scale variables per component."},
	{"num_features",
		(getter)MCGSM_num_features, 0,
		"Number of features available to approximate input covariances."},
	{"priors",
		(getter)MCGSM_priors,
		(setter)MCGSM_set_priors,
		"Log-weights of mixture components and scales, $\\eta_{cs}$."},
	{"scales",
		(getter)MCGSM_scales,
		(setter)MCGSM_set_scales,
		"Log-precision variables, $\\alpha_{cs}$."},
	{"weights",
		(getter)MCGSM_weights,
		(setter)MCGSM_set_weights,
		"Weights relating features and mixture components, $\\beta_{ci}$."},
	{"features", 
		(getter)MCGSM_features,
		(setter)MCGSM_set_features,
		"Features used for capturing structure in inputs, $\\mathbf{b}_i$."},
	{"cholesky_factors", 
		(getter)MCGSM_cholesky_factors, 
		(setter)MCGSM_set_cholesky_factors, 
		"A list of Cholesky factors of residual precision matrices, $\\mathbf{L}_c$."},
	{"predictors",
		(getter)MCGSM_predictors,
		(setter)MCGSM_set_predictors,
		"A list of linear predictors, $\\mathbf{A}_c$."},
	{0}
};

static PyMethodDef MCGSM_methods[] = {
	{"initialize",
		(PyCFunction)MCGSM_initialize,
		METH_VARARGS|METH_KEYWORDS,
		MCGSM_initialize_doc},
	{"train", (PyCFunction)MCGSM_train, METH_VARARGS|METH_KEYWORDS, MCGSM_train_doc},
	{"posterior",
		(PyCFunction)MCGSM_posterior,
		METH_VARARGS|METH_KEYWORDS,
		MCGSM_posterior_doc},
	{"sample_posterior",
		(PyCFunction)MCGSM_sample_posterior,
		METH_VARARGS|METH_KEYWORDS,
		MCGSM_sample_posterior_doc},
	{"_check_gradient",
		(PyCFunction)MCGSM_check_gradient,
		METH_VARARGS|METH_KEYWORDS, 0},
	{"_check_performance",
		(PyCFunction)MCGSM_check_performance,
		METH_VARARGS|METH_KEYWORDS, 0},
	{"_parameters",
		(PyCFunction)MCGSM_parameters,
		METH_VARARGS|METH_KEYWORDS,
		MCGSM_parameters_doc},
	{"_set_parameters",
		(PyCFunction)MCGSM_set_parameters,
		METH_VARARGS|METH_KEYWORDS,
		MCGSM_set_parameters_doc},
	{"_parameter_gradient",
		(PyCFunction)MCGSM_parameter_gradient,
		METH_VARARGS|METH_KEYWORDS, 0},
	{"_compute_data_gradient",
		(PyCFunction)MCGSM_compute_data_gradient,
		METH_VARARGS|METH_KEYWORDS, 0},
	{"__reduce__", (PyCFunction)MCGSM_reduce, METH_NOARGS, MCGSM_reduce_doc},
	{"__setstate__", (PyCFunction)MCGSM_setstate, METH_VARARGS, MCGSM_setstate_doc},
	{0}
};

PyTypeObject MCGSM_type = {
	PyObject_HEAD_INIT(0)
	0,                      /*ob_size*/
	"cmt.MCGSM",            /*tp_name*/
	sizeof(MCGSMObject),    /*tp_basicsize*/
	0,                      /*tp_itemsize*/
	(destructor)CD_dealloc, /*tp_dealloc*/
	0,                      /*tp_print*/
	0,                      /*tp_getattr*/
	0,                      /*tp_setattr*/
	0,                      /*tp_compare*/
	0,                      /*tp_repr*/
	0,                      /*tp_as_number*/
	0,                      /*tp_as_sequence*/
	0,                      /*tp_as_mapping*/
	0,                      /*tp_hash */
	0,                      /*tp_call*/
	0,                      /*tp_str*/
	0,                      /*tp_getattro*/
	0,                      /*tp_setattro*/
	0,                      /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,     /*tp_flags*/
	MCGSM_doc,              /*tp_doc*/
	0,                      /*tp_traverse*/
	0,                      /*tp_clear*/
	0,                      /*tp_richcompare*/
	0,                      /*tp_weaklistoffset*/
	0,                      /*tp_iter*/
	0,                      /*tp_iternext*/
	MCGSM_methods,          /*tp_methods*/
	0,                      /*tp_members*/
	MCGSM_getset,           /*tp_getset*/
	&CD_type,               /*tp_base*/
	0,                      /*tp_dict*/
	0,                      /*tp_descr_get*/
	0,                      /*tp_descr_set*/
	0,                      /*tp_dictoffset*/
	(initproc)MCGSM_init,   /*tp_init*/
	0,                      /*tp_alloc*/
	CD_new,                 /*tp_new*/
};

static PyGetSetDef MCBM_getset[] = {
	{"num_components", (getter)MCBM_num_components, 0, "Numer of predictors."},
	{"num_features",
		(getter)MCBM_num_features, 0,
		"Number of features available to approximate input covariances."},
	{"priors",
		(getter)MCBM_priors,
		(setter)MCBM_set_priors,
		"Log-weights of mixture components and scales, $\\eta_{cs}$."},
	{"weights",
		(getter)MCBM_weights,
		(setter)MCBM_set_weights,
		"Weights relating features and mixture components, $\\beta_{ci}$."},
	{"features", 
		(getter)MCBM_features,
		(setter)MCBM_set_features,
		"Features used for capturing structure in inputs, $\\mathbf{b}_i$."},
	{"predictors",
		(getter)MCBM_predictors,
		(setter)MCBM_set_predictors,
		"Parameters relating inputs and outputs, $\\mathbf{A}_c$."},
	{"input_bias",
		(getter)MCBM_input_bias,
		(setter)MCBM_set_input_bias,
		"Input bias vectors, $\\mathbf{w}_c$."},
	{"output_bias",
		(getter)MCBM_output_bias,
		(setter)MCBM_set_output_bias,
		"Output biases, $v_c$."},
	{0}
};

static PyMethodDef MCBM_methods[] = {
	{"train", (PyCFunction)MCBM_train, METH_VARARGS|METH_KEYWORDS, MCBM_train_doc},
	{"_parameters",
		(PyCFunction)MCBM_parameters,
		METH_VARARGS|METH_KEYWORDS,
		MCBM_parameters_doc},
	{"_set_parameters",
		(PyCFunction)MCBM_set_parameters,
		METH_VARARGS|METH_KEYWORDS,
		MCBM_set_parameters_doc},
	{"_parameter_gradient",
		(PyCFunction)MCBM_parameter_gradient,
		METH_VARARGS|METH_KEYWORDS, 0},
	{"_check_gradient",
		(PyCFunction)MCBM_check_gradient,
		METH_VARARGS|METH_KEYWORDS, 0},
	{"__reduce__", (PyCFunction)MCBM_reduce, METH_NOARGS, MCBM_reduce_doc},
	{"__setstate__", (PyCFunction)MCBM_setstate, METH_VARARGS, MCBM_setstate_doc},
	{0}
};

PyTypeObject MCBM_type = {
	PyObject_HEAD_INIT(0)
	0,                      /*ob_size*/
	"cmt.MCBM",             /*tp_name*/
	sizeof(MCBMObject),     /*tp_basicsize*/
	0,                      /*tp_itemsize*/
	(destructor)CD_dealloc, /*tp_dealloc*/
	0,                      /*tp_print*/
	0,                      /*tp_getattr*/
	0,                      /*tp_setattr*/
	0,                      /*tp_compare*/
	0,                      /*tp_repr*/
	0,                      /*tp_as_number*/
	0,                      /*tp_as_sequence*/
	0,                      /*tp_as_mapping*/
	0,                      /*tp_hash */
	0,                      /*tp_call*/
	0,                      /*tp_str*/
	0,                      /*tp_getattro*/
	0,                      /*tp_setattro*/
	0,                      /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,     /*tp_flags*/
	MCBM_doc,               /*tp_doc*/
	0,                      /*tp_traverse*/
	0,                      /*tp_clear*/
	0,                      /*tp_richcompare*/
	0,                      /*tp_weaklistoffset*/
	0,                      /*tp_iter*/
	0,                      /*tp_iternext*/
	MCBM_methods,           /*tp_methods*/
	0,                      /*tp_members*/
	MCBM_getset,            /*tp_getset*/
	&CD_type,               /*tp_base*/
	0,                      /*tp_dict*/
	0,                      /*tp_descr_get*/
	0,                      /*tp_descr_set*/
	0,                      /*tp_dictoffset*/
	(initproc)MCBM_init,    /*tp_init*/
	0,                      /*tp_alloc*/
	CD_new,                 /*tp_new*/
};

static PyMappingMethods PatchMCBM_as_mapping = {
	0,                                      /*mp_length*/
	(binaryfunc)PatchMCBM_subscript,        /*mp_subscript*/
	(objobjargproc)PatchMCBM_ass_subscript, /*mp_ass_subscript*/
};

static PyGetSetDef PatchMCBM_getset[] = {
	{"rows", (getter)PatchMCBM_rows, 0, "Number of rows of the modeled patches."},
	{"cols", (getter)PatchMCBM_cols, 0, "Number of columns of the modeled patches."},
	{"preconditioners", 
		(getter)PatchMCBM_preconditioners,
		(setter)PatchMCBM_set_preconditioners,
		"A dictionary containing all preconditioners."},
	{0}
};

static PyMethodDef PatchMCBM_methods[] = {
	{"initialize", (PyCFunction)PatchMCBM_initialize, METH_KEYWORDS, PatchMCBM_initialize_doc},
	{"train", (PyCFunction)PatchMCBM_train, METH_KEYWORDS, PatchMCBM_train_doc},
	{"loglikelihood", (PyCFunction)PatchMCBM_loglikelihood, METH_KEYWORDS, 0},
	{"preconditioner", (PyCFunction)PatchMCBM_preconditioner, METH_VARARGS, 0},
	{"input_mask", (PyCFunction)PatchMCBM_input_mask, METH_VARARGS, 0},
	{"output_mask", (PyCFunction)PatchMCBM_output_mask, METH_VARARGS, 0},
	{"__reduce__", (PyCFunction)PatchMCBM_reduce, METH_NOARGS, PatchMCBM_reduce_doc},
	{"__setstate__", (PyCFunction)PatchMCBM_setstate, METH_VARARGS, PatchMCBM_setstate_doc},
	{0}
};

PyTypeObject PatchMCBM_type = {
	PyObject_HEAD_INIT(0)
	0,                                /*ob_size*/
	"cmt.PatchMCBM",                  /*tp_name*/
	sizeof(PatchMCBMObject),          /*tp_basicsize*/
	0,                                /*tp_itemsize*/
	(destructor)Distribution_dealloc, /*tp_dealloc*/
	0,                                /*tp_print*/
	0,                                /*tp_getattr*/
	0,                                /*tp_setattr*/
	0,                                /*tp_compare*/
	0,                                /*tp_repr*/
	0,                                /*tp_as_number*/
	0,                                /*tp_as_sequence*/
	&PatchMCBM_as_mapping,            /*tp_as_mapping*/
	0,                                /*tp_hash */
	0,                                /*tp_call*/
	0,                                /*tp_str*/
	0,                                /*tp_getattro*/
	0,                                /*tp_setattro*/
	0,                                /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,               /*tp_flags*/
	PatchMCBM_doc,                    /*tp_doc*/
	0,                                /*tp_traverse*/
	0,                                /*tp_clear*/
	0,                                /*tp_richcompare*/
	0,                                /*tp_weaklistoffset*/
	0,                                /*tp_iter*/
	0,                                /*tp_iternext*/
	PatchMCBM_methods,                /*tp_methods*/
	0,                                /*tp_members*/
	PatchMCBM_getset,                 /*tp_getset*/
	&Distribution_type,               /*tp_base*/
	0,                                /*tp_dict*/
	0,                                /*tp_descr_get*/
	0,                                /*tp_descr_set*/
	0,                                /*tp_dictoffset*/
	(initproc)PatchMCBM_init,         /*tp_init*/
	0,                                /*tp_alloc*/
	Distribution_new,                 /*tp_new*/
};

static PyGetSetDef GLM_getset[] = {
	{"weights",
		(getter)GLM_weights,
		(setter)GLM_set_weights,
		"Linear filter, $w$."},
	{"nonlinearity",
		(getter)GLM_nonlinearity,
		(setter)GLM_set_nonlinearity,
		"Nonlinearity applied to output of linear filter, $g$."},
	{"distribution",
		(getter)GLM_distribution,
		(setter)GLM_set_distribution,
		"Distribution whose average value is determined by output of nonlinearity."},
	{0}
};

static PyMethodDef GLM_methods[] = {
	{"train", (PyCFunction)GLM_train, METH_VARARGS|METH_KEYWORDS, 0},
	{"_parameter_gradient",
		(PyCFunction)GLM_parameter_gradient,
		METH_VARARGS|METH_KEYWORDS, 0},
	{"_check_gradient",
		(PyCFunction)GLM_check_gradient,
		METH_VARARGS|METH_KEYWORDS, 0},
	{"__reduce__", (PyCFunction)GLM_reduce, METH_NOARGS, GLM_reduce_doc},
	{"__setstate__", (PyCFunction)GLM_setstate, METH_VARARGS, GLM_setstate_doc},
	{0}
};

PyTypeObject GLM_type = {
	PyObject_HEAD_INIT(0)
	0,                      /*ob_size*/
	"cmt.GLM",              /*tp_name*/
	sizeof(GLMObject),      /*tp_basicsize*/
	0,                      /*tp_itemsize*/
	(destructor)CD_dealloc, /*tp_dealloc*/
	0,                      /*tp_print*/
	0,                      /*tp_getattr*/
	0,                      /*tp_setattr*/
	0,                      /*tp_compare*/
	0,                      /*tp_repr*/
	0,                      /*tp_as_number*/
	0,                      /*tp_as_sequence*/
	0,                      /*tp_as_mapping*/
	0,                      /*tp_hash */
	0,                      /*tp_call*/
	0,                      /*tp_str*/
	0,                      /*tp_getattro*/
	0,                      /*tp_setattro*/
	0,                      /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,     /*tp_flags*/
	GLM_doc,                /*tp_doc*/
	0,                      /*tp_traverse*/
	0,                      /*tp_clear*/
	0,                      /*tp_richcompare*/
	0,                      /*tp_weaklistoffset*/
	0,                      /*tp_iter*/
	0,                      /*tp_iternext*/
	GLM_methods,            /*tp_methods*/
	0,                      /*tp_members*/
	GLM_getset,             /*tp_getset*/
	&CD_type,               /*tp_base*/
	0,                      /*tp_dict*/
	0,                      /*tp_descr_get*/
	0,                      /*tp_descr_set*/
	0,                      /*tp_dictoffset*/
	(initproc)GLM_init,     /*tp_init*/
	0,                      /*tp_alloc*/
	CD_new,                 /*tp_new*/
};

PyTypeObject Nonlinearity_type = {
	PyObject_HEAD_INIT(0)
	0,                                /*ob_size*/
	"cmt.Nonlinearity",               /*tp_name*/
	sizeof(NonlinearityObject),       /*tp_basicsize*/
	0,                                /*tp_itemsize*/
	(destructor)Nonlinearity_dealloc, /*tp_dealloc*/
	0,                                /*tp_print*/
	0,                                /*tp_getattr*/
	0,                                /*tp_setattr*/
	0,                                /*tp_compare*/
	0,                                /*tp_repr*/
	0,                                /*tp_as_number*/
	0,                                /*tp_as_sequence*/
	0,                                /*tp_as_mapping*/
	0,                                /*tp_hash */
	(ternaryfunc)Nonlinearity_call,   /*tp_call*/
	0,                                /*tp_str*/
	0,                                /*tp_getattro*/
	0,                                /*tp_setattro*/
	0,                                /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,               /*tp_flags*/
	Nonlinearity_doc,                 /*tp_doc*/
	0,                                /*tp_traverse*/
	0,                                /*tp_clear*/
	0,                                /*tp_richcompare*/
	0,                                /*tp_weaklistoffset*/
	0,                                /*tp_iter*/
	0,                                /*tp_iternext*/
	0,                                /*tp_methods*/
	0,                                /*tp_members*/
	0,                                /*tp_getset*/
	0,                                /*tp_base*/
	0,                                /*tp_dict*/
	0,                                /*tp_descr_get*/
	0,                                /*tp_descr_set*/
	0,                                /*tp_dictoffset*/
	(initproc)Nonlinearity_init,      /*tp_init*/
	0,                                /*tp_alloc*/
	Nonlinearity_new,                 /*tp_new*/
};

static PyMethodDef LogisticFunction_methods[] = {
	{"__reduce__", (PyCFunction)LogisticFunction_reduce, METH_NOARGS, LogisticFunction_reduce_doc},
	{0}
};

PyTypeObject LogisticFunction_type = {
	PyObject_HEAD_INIT(0)
	0,                                /*ob_size*/
	"cmt.LogisticFunction",           /*tp_name*/
	sizeof(LogisticFunctionObject),   /*tp_basicsize*/
	0,                                /*tp_itemsize*/
	(destructor)Nonlinearity_dealloc, /*tp_dealloc*/
	0,                                /*tp_print*/
	0,                                /*tp_getattr*/
	0,                                /*tp_setattr*/
	0,                                /*tp_compare*/
	0,                                /*tp_repr*/
	0,                                /*tp_as_number*/
	0,                                /*tp_as_sequence*/
	0,                                /*tp_as_mapping*/
	0,                                /*tp_hash */
	0,                                /*tp_call*/
	0,                                /*tp_str*/
	0,                                /*tp_getattro*/
	0,                                /*tp_setattro*/
	0,                                /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,               /*tp_flags*/
	LogisticFunction_doc,             /*tp_doc*/
	0,                                /*tp_traverse*/
	0,                                /*tp_clear*/
	0,                                /*tp_richcompare*/
	0,                                /*tp_weaklistoffset*/
	0,                                /*tp_iter*/
	0,                                /*tp_iternext*/
	LogisticFunction_methods,         /*tp_methods*/
	0,                                /*tp_members*/
	0,                                /*tp_getset*/
	&Nonlinearity_type,               /*tp_base*/
	0,                                /*tp_dict*/
	0,                                /*tp_descr_get*/
	0,                                /*tp_descr_set*/
	0,                                /*tp_dictoffset*/
	(initproc)LogisticFunction_init,  /*tp_init*/
	0,                                /*tp_alloc*/
	Nonlinearity_new,                 /*tp_new*/
};

PyTypeObject UnivariateDistribution_type = {
	PyObject_HEAD_INIT(0)
	0,                                     /*ob_size*/
	"cmt.UnivariateDistribution",          /*tp_name*/
	sizeof(UnivariateDistributionObject),  /*tp_basicsize*/
	0,                                     /*tp_itemsize*/
	(destructor)Distribution_dealloc,      /*tp_dealloc*/
	0,                                     /*tp_print*/
	0,                                     /*tp_getattr*/
	0,                                     /*tp_setattr*/
	0,                                     /*tp_compare*/
	0,                                     /*tp_repr*/
	0,                                     /*tp_as_number*/
	0,                                     /*tp_as_sequence*/
	0,                                     /*tp_as_mapping*/
	0,                                     /*tp_hash */
	0,                                     /*tp_call*/
	0,                                     /*tp_str*/
	0,                                     /*tp_getattro*/
	0,                                     /*tp_setattro*/
	0,                                     /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,                    /*tp_flags*/
	UnivariateDistribution_doc,            /*tp_doc*/
	0,                                     /*tp_traverse*/
	0,                                     /*tp_clear*/
	0,                                     /*tp_richcompare*/
	0,                                     /*tp_weaklistoffset*/
	0,                                     /*tp_iter*/
	0,                                     /*tp_iternext*/
	0,                                     /*tp_methods*/
	0,                                     /*tp_members*/
	0,                                     /*tp_getset*/
	&Distribution_type,                    /*tp_base*/
	0,                                     /*tp_dict*/
	0,                                     /*tp_descr_get*/
	0,                                     /*tp_descr_set*/
	0,                                     /*tp_dictoffset*/
	(initproc)UnivariateDistribution_init, /*tp_init*/
	0,                                     /*tp_alloc*/
	Distribution_new,                      /*tp_new*/
};

static PyMethodDef Bernoulli_methods[] = {
	{"__reduce__", (PyCFunction)Bernoulli_reduce, METH_NOARGS, Bernoulli_reduce_doc},
	{0}
};

PyTypeObject Bernoulli_type = {
	PyObject_HEAD_INIT(0)
	0,                                /*ob_size*/
	"cmt.Bernoulli",                  /*tp_name*/
	sizeof(BernoulliObject),          /*tp_basicsize*/
	0,                                /*tp_itemsize*/
	(destructor)Distribution_dealloc, /*tp_dealloc*/
	0,                                /*tp_print*/
	0,                                /*tp_getattr*/
	0,                                /*tp_setattr*/
	0,                                /*tp_compare*/
	0,                                /*tp_repr*/
	0,                                /*tp_as_number*/
	0,                                /*tp_as_sequence*/
	0,                                /*tp_as_mapping*/
	0,                                /*tp_hash */
	0,                                /*tp_call*/
	0,                                /*tp_str*/
	0,                                /*tp_getattro*/
	0,                                /*tp_setattro*/
	0,                                /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,               /*tp_flags*/
	Bernoulli_doc,                    /*tp_doc*/
	0,                                /*tp_traverse*/
	0,                                /*tp_clear*/
	0,                                /*tp_richcompare*/
	0,                                /*tp_weaklistoffset*/
	0,                                /*tp_iter*/
	0,                                /*tp_iternext*/
	Bernoulli_methods,                /*tp_methods*/
	0,                                /*tp_members*/
	0,                                /*tp_getset*/
	&UnivariateDistribution_type,     /*tp_base*/
	0,                                /*tp_dict*/
	0,                                /*tp_descr_get*/
	0,                                /*tp_descr_set*/
	0,                                /*tp_dictoffset*/
	(initproc)Bernoulli_init,         /*tp_init*/
	0,                                /*tp_alloc*/
	Distribution_new,                 /*tp_new*/
};

static PyGetSetDef Preconditioner_getset[] = {
	{"dim_in", (getter)Preconditioner_dim_in, 0, 0},
	{"dim_out", (getter)Preconditioner_dim_out, 0, 0},
	{0}
};

static PyMethodDef Preconditioner_methods[] = {
	{"inverse", (PyCFunction)Preconditioner_inverse, METH_VARARGS|METH_KEYWORDS, Preconditioner_inverse_doc},
	{"logjacobian", (PyCFunction)Preconditioner_logjacobian, METH_VARARGS|METH_KEYWORDS, Preconditioner_logjacobian_doc},
	{0}
};

PyTypeObject Preconditioner_type = {
	PyObject_HEAD_INIT(0)
	0,                                  /*ob_size*/
	"cmt.Preconditioner",               /*tp_name*/
	sizeof(PreconditionerObject),       /*tp_basicsize*/
	0,                                  /*tp_itemsize*/
	(destructor)Preconditioner_dealloc, /*tp_dealloc*/
	0,                                  /*tp_print*/
	0,                                  /*tp_getattr*/
	0,                                  /*tp_setattr*/
	0,                                  /*tp_compare*/
	0,                                  /*tp_repr*/
	0,                                  /*tp_as_number*/
	0,                                  /*tp_as_sequence*/
	0,                                  /*tp_as_mapping*/
	0,                                  /*tp_hash */
	(ternaryfunc)Preconditioner_call,   /*tp_call*/
	0,                                  /*tp_str*/
	0,                                  /*tp_getattro*/
	0,                                  /*tp_setattro*/
	0,                                  /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,                 /*tp_flags*/
	Preconditioner_doc,                 /*tp_doc*/
	0,                                  /*tp_traverse*/
	0,                                  /*tp_clear*/
	0,                                  /*tp_richcompare*/
	0,                                  /*tp_weaklistoffset*/
	0,                                  /*tp_iter*/
	0,                                  /*tp_iternext*/
	Preconditioner_methods,             /*tp_methods*/
	0,                                  /*tp_members*/
	Preconditioner_getset,              /*tp_getset*/
	0,                                  /*tp_base*/
	0,                                  /*tp_dict*/
	0,                                  /*tp_descr_get*/
	0,                                  /*tp_descr_set*/
	0,                                  /*tp_dictoffset*/
	(initproc)Preconditioner_init,      /*tp_init*/
	0,                                  /*tp_alloc*/
	Preconditioner_new,                 /*tp_new*/
};

static PyGetSetDef AffinePreconditioner_getset[] = {
	{"mean_in", (getter)AffinePreconditioner_mean_in, 0, 0},
	{"mean_out", (getter)AffinePreconditioner_mean_out, 0, 0},
	{"pre_in", (getter)AffinePreconditioner_pre_in, 0, 0},
	{"pre_out", (getter)AffinePreconditioner_pre_out, 0, 0},
	{"predictor", (getter)AffinePreconditioner_predictor, 0, 0},
	{0}
};

static PyMethodDef AffinePreconditioner_methods[] = {
	{"__reduce__", (PyCFunction)AffinePreconditioner_reduce, METH_NOARGS, 0},
	{"__setstate__", (PyCFunction)AffinePreconditioner_setstate, METH_VARARGS, 0},
	{0}
};

PyTypeObject AffinePreconditioner_type = {
	PyObject_HEAD_INIT(0)
	0,                                   /*ob_size*/
	"cmt.AffinePreconditioner",          /*tp_name*/
	sizeof(AffinePreconditionerObject),  /*tp_basicsize*/
	0,                                   /*tp_itemsize*/
	(destructor)Preconditioner_dealloc,  /*tp_dealloc*/
	0,                                   /*tp_print*/
	0,                                   /*tp_getattr*/
	0,                                   /*tp_setattr*/
	0,                                   /*tp_compare*/
	0,                                   /*tp_repr*/
	0,                                   /*tp_as_number*/
	0,                                   /*tp_as_sequence*/
	0,                                   /*tp_as_mapping*/
	0,                                   /*tp_hash */
	0,                                   /*tp_call*/
	0,                                   /*tp_str*/
	0,                                   /*tp_getattro*/
	0,                                   /*tp_setattro*/
	0,                                   /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,                  /*tp_flags*/
	AffinePreconditioner_doc,            /*tp_doc*/
	0,                                   /*tp_traverse*/
	0,                                   /*tp_clear*/
	0,                                   /*tp_richcompare*/
	0,                                   /*tp_weaklistoffset*/
	0,                                   /*tp_iter*/
	0,                                   /*tp_iternext*/
	AffinePreconditioner_methods,        /*tp_methods*/
	0,                                   /*tp_members*/
	AffinePreconditioner_getset,         /*tp_getset*/
	&Preconditioner_type,                /*tp_base*/
	0,                                   /*tp_dict*/
	0,                                   /*tp_descr_get*/
	0,                                   /*tp_descr_set*/
	0,                                   /*tp_dictoffset*/
	(initproc)AffinePreconditioner_init, /*tp_init*/
	0,                                   /*tp_alloc*/
	Preconditioner_new,                  /*tp_new*/
};

static PyMethodDef AffineTransform_methods[] = {
	{"__reduce__", (PyCFunction)AffineTransform_reduce, METH_NOARGS, 0},
	{0}
};


PyTypeObject AffineTransform_type = {
	PyObject_HEAD_INIT(0)
	0,                                  /*ob_size*/
	"cmt.AffineTransform",              /*tp_name*/
	sizeof(AffineTransformObject),      /*tp_basicsize*/
	0,                                  /*tp_itemsize*/
	(destructor)Preconditioner_dealloc, /*tp_dealloc*/
	0,                                  /*tp_print*/
	0,                                  /*tp_getattr*/
	0,                                  /*tp_setattr*/
	0,                                  /*tp_compare*/
	0,                                  /*tp_repr*/
	0,                                  /*tp_as_number*/
	0,                                  /*tp_as_sequence*/
	0,                                  /*tp_as_mapping*/
	0,                                  /*tp_hash */
	0,                                  /*tp_call*/
	0,                                  /*tp_str*/
	0,                                  /*tp_getattro*/
	0,                                  /*tp_setattro*/
	0,                                  /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,                 /*tp_flags*/
	AffineTransform_doc,                /*tp_doc*/
	0,                                  /*tp_traverse*/
	0,                                  /*tp_clear*/
	0,                                  /*tp_richcompare*/
	0,                                  /*tp_weaklistoffset*/
	0,                                  /*tp_iter*/
	0,                                  /*tp_iternext*/
	AffineTransform_methods,            /*tp_methods*/
	0,                                  /*tp_members*/
	0,                                  /*tp_getset*/
	&AffinePreconditioner_type,         /*tp_base*/
	0,                                  /*tp_dict*/
	0,                                  /*tp_descr_get*/
	0,                                  /*tp_descr_set*/
	0,                                  /*tp_dictoffset*/
	(initproc)AffineTransform_init,     /*tp_init*/
	0,                                  /*tp_alloc*/
	Preconditioner_new,                 /*tp_new*/
};

PyTypeObject WhiteningPreconditioner_type = {
	PyObject_HEAD_INIT(0)
	0,                                      /*ob_size*/
	"cmt.WhiteningPreconditioner",          /*tp_name*/
	sizeof(WhiteningPreconditionerObject),  /*tp_basicsize*/
	0,                                      /*tp_itemsize*/
	(destructor)Preconditioner_dealloc,     /*tp_dealloc*/
	0,                                      /*tp_print*/
	0,                                      /*tp_getattr*/
	0,                                      /*tp_setattr*/
	0,                                      /*tp_compare*/
	0,                                      /*tp_repr*/
	0,                                      /*tp_as_number*/
	0,                                      /*tp_as_sequence*/
	0,                                      /*tp_as_mapping*/
	0,                                      /*tp_hash */
	0,                                      /*tp_call*/
	0,                                      /*tp_str*/
	0,                                      /*tp_getattro*/
	0,                                      /*tp_setattro*/
	0,                                      /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,                     /*tp_flags*/
	WhiteningPreconditioner_doc,            /*tp_doc*/
	0,                                      /*tp_traverse*/
	0,                                      /*tp_clear*/
	0,                                      /*tp_richcompare*/
	0,                                      /*tp_weaklistoffset*/
	0,                                      /*tp_iter*/
	0,                                      /*tp_iternext*/
	0,                                      /*tp_methods*/
	0,                                      /*tp_members*/
	0,                                      /*tp_getset*/
	&AffinePreconditioner_type,             /*tp_base*/
	0,                                      /*tp_dict*/
	0,                                      /*tp_descr_get*/
	0,                                      /*tp_descr_set*/
	0,                                      /*tp_dictoffset*/
	(initproc)WhiteningPreconditioner_init, /*tp_init*/
	0,                                      /*tp_alloc*/
	Preconditioner_new,                     /*tp_new*/
};

PyTypeObject WhiteningTransform_type = {
	PyObject_HEAD_INIT(0)
	0,                                  /*ob_size*/
	"cmt.WhiteningTransform",           /*tp_name*/
	sizeof(WhiteningTransformObject),   /*tp_basicsize*/
	0,                                  /*tp_itemsize*/
	(destructor)Preconditioner_dealloc, /*tp_dealloc*/
	0,                                  /*tp_print*/
	0,                                  /*tp_getattr*/
	0,                                  /*tp_setattr*/
	0,                                  /*tp_compare*/
	0,                                  /*tp_repr*/
	0,                                  /*tp_as_number*/
	0,                                  /*tp_as_sequence*/
	0,                                  /*tp_as_mapping*/
	0,                                  /*tp_hash */
	0,                                  /*tp_call*/
	0,                                  /*tp_str*/
	0,                                  /*tp_getattro*/
	0,                                  /*tp_setattro*/
	0,                                  /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,                 /*tp_flags*/
	WhiteningTransform_doc,             /*tp_doc*/
	0,                                  /*tp_traverse*/
	0,                                  /*tp_clear*/
	0,                                  /*tp_richcompare*/
	0,                                  /*tp_weaklistoffset*/
	0,                                  /*tp_iter*/
	0,                                  /*tp_iternext*/
	0,                                  /*tp_methods*/
	0,                                  /*tp_members*/
	0,                                  /*tp_getset*/
	&AffineTransform_type,              /*tp_base*/
	0,                                  /*tp_dict*/
	0,                                  /*tp_descr_get*/
	0,                                  /*tp_descr_set*/
	0,                                  /*tp_dictoffset*/
	(initproc)WhiteningTransform_init,  /*tp_init*/
	0,                                  /*tp_alloc*/
	Preconditioner_new,                 /*tp_new*/
};

static PyGetSetDef PCAPreconditioner_getset[] = {
	{"eigenvalues", (getter)PCAPreconditioner_eigenvalues, 0, "Eigenvalues of the covariance of the input."},
	{0}
};

static PyMethodDef PCAPreconditioner_methods[] = {
	{"__reduce__", (PyCFunction)PCAPreconditioner_reduce, METH_NOARGS, 0},
	{0}
};

PyTypeObject PCAPreconditioner_type = {
	PyObject_HEAD_INIT(0)
	0,                                  /*ob_size*/
	"cmt.PCAPreconditioner",            /*tp_name*/
	sizeof(PCAPreconditionerObject),    /*tp_basicsize*/
	0,                                  /*tp_itemsize*/
	(destructor)Preconditioner_dealloc, /*tp_dealloc*/
	0,                                  /*tp_print*/
	0,                                  /*tp_getattr*/
	0,                                  /*tp_setattr*/
	0,                                  /*tp_compare*/
	0,                                  /*tp_repr*/
	0,                                  /*tp_as_number*/
	0,                                  /*tp_as_sequence*/
	0,                                  /*tp_as_mapping*/
	0,                                  /*tp_hash */
	0,                                  /*tp_call*/
	0,                                  /*tp_str*/
	0,                                  /*tp_getattro*/
	0,                                  /*tp_setattro*/
	0,                                  /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,                 /*tp_flags*/
	PCAPreconditioner_doc,              /*tp_doc*/
	0,                                  /*tp_traverse*/
	0,                                  /*tp_clear*/
	0,                                  /*tp_richcompare*/
	0,                                  /*tp_weaklistoffset*/
	0,                                  /*tp_iter*/
	0,                                  /*tp_iternext*/
	PCAPreconditioner_methods,          /*tp_methods*/
	0,                                  /*tp_members*/
	PCAPreconditioner_getset,           /*tp_getset*/
	&AffinePreconditioner_type,         /*tp_base*/
	0,                                  /*tp_dict*/
	0,                                  /*tp_descr_get*/
	0,                                  /*tp_descr_set*/
	0,                                  /*tp_dictoffset*/
	(initproc)PCAPreconditioner_init,   /*tp_init*/
	0,                                  /*tp_alloc*/
	Preconditioner_new,                 /*tp_new*/
};

static PyGetSetDef PCATransform_getset[] = {
	{"eigenvalues", (getter)PCATransform_eigenvalues, 0, "Eigenvalues of the covariance of the input."},
	{0}
};

static PyMethodDef PCATransform_methods[] = {
	{"__reduce__", (PyCFunction)PCATransform_reduce, METH_NOARGS, 0},
	{0}
};

PyTypeObject PCATransform_type = {
	PyObject_HEAD_INIT(0)
	0,                                  /*ob_size*/
	"cmt.PCATransform",                 /*tp_name*/
	sizeof(PCATransformObject),         /*tp_basicsize*/
	0,                                  /*tp_itemsize*/
	(destructor)Preconditioner_dealloc, /*tp_dealloc*/
	0,                                  /*tp_print*/
	0,                                  /*tp_getattr*/
	0,                                  /*tp_setattr*/
	0,                                  /*tp_compare*/
	0,                                  /*tp_repr*/
	0,                                  /*tp_as_number*/
	0,                                  /*tp_as_sequence*/
	0,                                  /*tp_as_mapping*/
	0,                                  /*tp_hash */
	0,                                  /*tp_call*/
	0,                                  /*tp_str*/
	0,                                  /*tp_getattro*/
	0,                                  /*tp_setattro*/
	0,                                  /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,                 /*tp_flags*/
	PCATransform_doc,                   /*tp_doc*/
	0,                                  /*tp_traverse*/
	0,                                  /*tp_clear*/
	0,                                  /*tp_richcompare*/
	0,                                  /*tp_weaklistoffset*/
	0,                                  /*tp_iter*/
	0,                                  /*tp_iternext*/
	PCATransform_methods,               /*tp_methods*/
	0,                                  /*tp_members*/
	PCATransform_getset,                /*tp_getset*/
	&AffineTransform_type,              /*tp_base*/
	0,                                  /*tp_dict*/
	0,                                  /*tp_descr_get*/
	0,                                  /*tp_descr_set*/
	0,                                  /*tp_dictoffset*/
	(initproc)PCATransform_init,        /*tp_init*/
	0,                                  /*tp_alloc*/
	Preconditioner_new,                 /*tp_new*/
};

static PyMethodDef cmt_methods[] = {
	{"random_select", (PyCFunction)random_select, METH_VARARGS|METH_KEYWORDS, random_select_doc},
	{"generate_data_from_image", (PyCFunction)generate_data_from_image, METH_VARARGS|METH_KEYWORDS, generate_data_from_image_doc},
	{"generate_data_from_video", (PyCFunction)generate_data_from_video, METH_VARARGS|METH_KEYWORDS, generate_data_from_video_doc},
	{"sample_image", (PyCFunction)sample_image, METH_VARARGS|METH_KEYWORDS, sample_image_doc},
	{"sample_video", (PyCFunction)sample_video, METH_VARARGS|METH_KEYWORDS, sample_video_doc},
	{"fill_in_image", (PyCFunction)fill_in_image, METH_VARARGS|METH_KEYWORDS, fill_in_image_doc},
	{"fill_in_image_map", (PyCFunction)fill_in_image_map, METH_VARARGS|METH_KEYWORDS, 0},
	{0}
};

PyMODINIT_FUNC initcmt() {
	// set random seed
	timeval time;
	gettimeofday(&time, 0);
	srand(time.tv_usec * time.tv_sec);

	// initialize NumPy
	import_array();

	// create module object
	PyObject* module = Py_InitModule3("cmt", cmt_methods, cmt_doc);

	// initialize types
	if(PyType_Ready(&AffinePreconditioner_type) < 0)
		return;
	if(PyType_Ready(&AffineTransform_type) < 0)
		return;
	if(PyType_Ready(&Bernoulli_type) < 0)
		return;
	if(PyType_Ready(&CD_type) < 0)
		return;
	if(PyType_Ready(&Distribution_type) < 0)
		return;
	if(PyType_Ready(&GLM_type) < 0)
		return;
	if(PyType_Ready(&LogisticFunction_type) < 0)
		return;
	if(PyType_Ready(&MCBM_type) < 0)
		return;
	if(PyType_Ready(&MCGSM_type) < 0)
		return;
	if(PyType_Ready(&Nonlinearity_type) < 0)
		return;
	if(PyType_Ready(&PatchMCBM_type) < 0)
		return;
	if(PyType_Ready(&PCAPreconditioner_type) < 0)
		return;
	if(PyType_Ready(&PCATransform_type) < 0)
		return;
	if(PyType_Ready(&Preconditioner_type) < 0)
		return;
	if(PyType_Ready(&UnivariateDistribution_type) < 0)
		return;
	if(PyType_Ready(&WhiteningPreconditioner_type) < 0)
		return;
	if(PyType_Ready(&WhiteningTransform_type) < 0)
		return;

	// initialize Eigen
	Eigen::initParallel();

	// add types to module
	Py_INCREF(&AffinePreconditioner_type);
	Py_INCREF(&AffineTransform_type);
	Py_INCREF(&Bernoulli_type);
	Py_INCREF(&CD_type);
	Py_INCREF(&Distribution_type);
	Py_INCREF(&GLM_type);
	Py_INCREF(&LogisticFunction_type);
	Py_INCREF(&MCBM_type);
	Py_INCREF(&MCGSM_type);
	Py_INCREF(&Nonlinearity_type);
	Py_INCREF(&PCAPreconditioner_type);
	Py_INCREF(&PCATransform_type);
	Py_INCREF(&PatchMCBM_type);
	Py_INCREF(&Preconditioner_type);
	Py_INCREF(&UnivariateDistribution_type);
	Py_INCREF(&WhiteningPreconditioner_type);
	Py_INCREF(&WhiteningTransform_type);

	PyModule_AddObject(module, "AffinePreconditioner", reinterpret_cast<PyObject*>(&AffinePreconditioner_type));
	PyModule_AddObject(module, "AffineTransform", reinterpret_cast<PyObject*>(&AffineTransform_type));
	PyModule_AddObject(module, "Bernoulli", reinterpret_cast<PyObject*>(&Bernoulli_type));
	PyModule_AddObject(module, "ConditionalDistribution", reinterpret_cast<PyObject*>(&CD_type));
	PyModule_AddObject(module, "Distribution", reinterpret_cast<PyObject*>(&Distribution_type));
	PyModule_AddObject(module, "GLM", reinterpret_cast<PyObject*>(&GLM_type));
	PyModule_AddObject(module, "LogisticFunction", reinterpret_cast<PyObject*>(&LogisticFunction_type));
	PyModule_AddObject(module, "MCBM", reinterpret_cast<PyObject*>(&MCBM_type));
	PyModule_AddObject(module, "MCGSM", reinterpret_cast<PyObject*>(&MCGSM_type));
	PyModule_AddObject(module, "Nonlinearity", reinterpret_cast<PyObject*>(&Nonlinearity_type));
	PyModule_AddObject(module, "PCAPreconditioner", reinterpret_cast<PyObject*>(&PCAPreconditioner_type));
	PyModule_AddObject(module, "PCATransform", reinterpret_cast<PyObject*>(&PCATransform_type));
	PyModule_AddObject(module, "PatchMCBM", reinterpret_cast<PyObject*>(&PatchMCBM_type));
	PyModule_AddObject(module, "Preconditioner", reinterpret_cast<PyObject*>(&Preconditioner_type));
	PyModule_AddObject(module, "UnivariateDistribution", reinterpret_cast<PyObject*>(&UnivariateDistribution_type));
	PyModule_AddObject(module, "WhiteningPreconditioner", reinterpret_cast<PyObject*>(&WhiteningPreconditioner_type));
	PyModule_AddObject(module, "WhiteningTransform", reinterpret_cast<PyObject*>(&WhiteningTransform_type));
}
