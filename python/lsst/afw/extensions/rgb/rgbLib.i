// -*- lsst-c++ -*-
%define rgbLib_DOCSTRING
"
Python bindings for classes describing the the geometry of a mosaic camera
"
%enddef

%feature("autodoc", "1");
%module(package="lsst.afw.extension.rgb", docstring=rgbLib_DOCSTRING) rgbLib

%{
#include "lsst/pex/logging.h"
#include "lsst/afw/geom.h"
#include "lsst/afw/image.h"
#include "lsst/afw/cameraGeom.h"
#include "lsst/afw/extension/rgb/Rgb.h"
%}

%include "lsst/p_lsstSwig.i"
%import  "lsst/afw/image/imageLib.i" 

%lsst_exceptions();

%include "lsst/afw/extension/rgb/Rgb.h"

%template(rgbMappingF) lsst::afw::extension::rgb::RgbMapping<float>;
%template(asinhMappingF) lsst::afw::extension::rgb::AsinhMapping<float>;
%template(RgbImageF) lsst::afw::extension::rgb::RgbImage<lsst::afw::image::Image<float> >;
%template(replaceSaturatedPixels)
        lsst::afw::extension::rgb::replaceSaturatedPixels<lsst::afw::image::MaskedImage<float> >;
