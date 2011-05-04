/**
 * \file
 */
#include <cmath>
#include "boost/regex.hpp"
#include "boost/format.hpp"
#include "lsst/afw/image/Image.h"
#include "lsst/afw/extension/rgb/Rgb.h"

namespace afwRgb = lsst::afw::extension::rgb;

template<typename ImageT>
void afwRgb::RgbImage<ImageT>::write(std::string const& fileName) const
{
    static const boost::regex pngRE("\\.png$");
    static const boost::regex tiffRE("\\.tiff$");

    if (boost::regex_search(fileName.c_str(), pngRE)) {
        writePng(fileName);
    } else if (boost::regex_search(fileName.c_str(), tiffRE)) {
        writeTiff(fileName);
    } else {
        throw LSST_EXCEPT(lsst::pex::exceptions::NotFoundException,
                          (boost::format("Unrecognised file type: %s") % fileName).str());
    }
}

/************************************************************************************************************/

template<typename PixelT, typename Intensity>
void afwRgb::AsinhMapping<PixelT, Intensity>::_init(
        double minR,                    // minimum value of intensity in R
        double minG,                    // minimum value of intensity in G
        double minB,                    // minimum value of intensity in B
        double range,                   // range of intensities to use if Q == 0
        double Q                        // softening parameter for asinh stretch
                                                     )
{
    _min[0] = minR;
    _min[1] = minG;
    _min[2] = minB;

    double const alpha = 1/range;

    if (fabs(Q) < std::numeric_limits<float>::epsilon()) {
        Q = 0.1;
    } else {
        double const Qmax = 1e10;
        if (Q > Qmax) {
            Q = Qmax;
        }
    }

#if 0
    _slope = afwRgb::rgb_traits::max()/Q; // gradient at origin is _slope
#else
    {
        float const frac = 0.1;         // gradient estimated using frac*range is _slope
        _slope = frac*afwRgb::rgb_traits::max()/asinh(frac*Q);
    }

#endif
    _soften = alpha*Q;
}

template<typename PixelT, typename Intensity>
afwRgb::rgb_traits::pixel
afwRgb::AsinhMapping<PixelT, Intensity>::operator()(PixelT r, PixelT g, PixelT b) const
{
    r -= _min[0];
    g -= _min[1];
    b -= _min[2];

    double const I = Intensity()(r, g, b);
    if (I <= 0.0) {
        return afwRgb::rgb_traits::pixel(0, 0, 0);
    }
    
    double const fac = ::asinh(I*_soften)*_slope/I;
    float valR = r*fac, valG = g*fac, valB = b*fac;

    return this->trueColorPixel(valR, valG, valB);
}

template class afwRgb::RgbImage<lsst::afw::image::Image<float> >;
template class afwRgb::AsinhMapping<float>;
