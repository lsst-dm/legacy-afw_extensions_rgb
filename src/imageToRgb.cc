/**
 * \file
 */
#include <cmath>
#include "lsst/afw/image/Image.h"
#include "lsst/afw/extension/rgb/Rgb.h"

namespace afwImage = lsst::afw::image;
namespace afwRgb = lsst::afw::extension::rgb;

template<typename PixelT, typename Intensity>
afwRgb::AsinhMapping<PixelT, Intensity>::AsinhMapping(double min, double range, double Q) : _min(min)
{
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
    double const I = Intensity()(r, g, b) - _min;
    if (I <= 0.0) {
        return afwRgb::rgb_traits::pixel(0, 0, 0);
    }
    
    double const fac = ::asinh(I*_soften)*_slope/I;
    float valR = r*fac, valG = g*fac, valB = b*fac;

    return this->trueColorPixel(valR, valG, valB);
}

template class afwRgb::RgbImage<afwImage::Image<float> >;
template class afwRgb::AsinhMapping<float>;
