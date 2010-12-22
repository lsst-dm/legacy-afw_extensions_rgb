#if !defined(LSST_AFW_EXTENSION_RGB_RGB_H)
#define LSST_AFW_EXTENSION_RGB_RGB_H 1

#include <string>
#include "boost/shared_ptr.hpp"
#include "boost/gil/gil_all.hpp"
#include "boost/gil/extension/io/tiff_io.hpp"
#include "lsst/pex/exceptions.h"

namespace lsst { namespace afw { namespace extension { namespace rgb {

struct rgb_traits {
    typedef boost::gil::rgb8_image_t image;
    typedef boost::gil::rgb8_view_t view;
    typedef boost::gil::rgb8_pixel_t pixel;
    typedef boost::gil::channel_type<image>::type channel;
    static channel max() { return 255; }
};

/************************************************************************************************************/

template<typename PixelT>
class RgbMapping {
public:
    virtual ~RgbMapping() {}
    
    virtual rgb_traits::pixel operator()(PixelT r, PixelT g, PixelT b) const = 0;

    /**
     * Convert a set of (red, green, blue) intensities to a pixel, preserving the ratios of the bands
     *
     * The output values are all in the range [0, rgb_traits::max()].  If all the input values are in that
     * range this routine simple converts them to an rgb_traits::pixel; if some are out of the range the
     * values are truncated at 0, and then the largest is set to rgb_traits::max() and the others reduced
     * proportionally
     */
    rgb_traits::pixel trueColorPixel(float const r, float const g, float const b) const {
        double const channelMax = rgb_traits::max();

        float valR = (r < 0) ? 0 : r;
        float valG = (g < 0) ? 0 : g;
        float valB = (b < 0) ? 0 : b;
            
        if(valR > valG) {
            if(valR > valB) {
                if(valR >= channelMax) {
                    valG *= channelMax/valR;
                    valB *= channelMax/valR;
                    valR = channelMax;
                }
            } else {
                if(valB >= channelMax) {
                    valR *= channelMax/valB;
                    valG *= channelMax/valB;
                    valB = channelMax;
                }
            }
        } else {
            if(valG > valB) {
                if(valG >= channelMax) {
                    valR *= channelMax/valG;
                    valB *= channelMax/valG;
                    valG = channelMax;
                }
            } else {
                if(valB >= channelMax) {
                    valR *= channelMax/valB;
                    valG *= channelMax/valB;
                    valB = channelMax;
                }
            }
        }

        return rgb_traits::pixel(valR, valG, valB);
    }        
};

/************************************************************************************************************/

template<typename ImageT>
class RgbImage {
public:
    typedef boost::shared_ptr<RgbImage> Ptr;
    typedef boost::shared_ptr<RgbImage const> ConstPtr;

    RgbImage(ImageT const& rim, ImageT const& gim, ImageT const& bim,
             RgbMapping<typename ImageT::Pixel> const& rgbMap);

    void writeTiff(std::string const& fileName) {
        boost::gil::tiff_write_view(fileName, _view);
    }
private:
    boost::shared_ptr<rgb_traits::image> _image;
    rgb_traits::view _view;
};

template<typename ImageT>
lsst::afw::extension::rgb::RgbImage<ImageT>::RgbImage(ImageT const& rim,
                                                      ImageT const& gim,
                                                      ImageT const& bim,
                                                      RgbMapping<typename ImageT::Pixel> const& rgbMap
                                                     ) :
    _image(new rgb_traits::image(rim.getWidth(), rim.getHeight())),
    _view(view(*_image))
{
    if (rim.getDimensions() != gim.getDimensions() || rim.getDimensions() != bim.getDimensions()) {
        throw LSST_EXCEPT(lsst::pex::exceptions::LengthErrorException,
                           (boost::format("Each of the RGB must be the same size: %dx%d %dx%d %dx%d")
                            % rim.getWidth() % rim.getHeight()
                            % gim.getWidth() % gim.getHeight()
                            % bim.getWidth() % bim.getHeight()).str());
    }

    for (int y = 0; y != rim.getHeight(); ++y) {
        rgb_traits::view::x_iterator tptr = _view.row_begin(_view.height() - y - 1);
        int x = 0;
        for (typename ImageT::x_iterator
                 rptr = rim.row_begin(y), gptr = gim.row_begin(y), bptr = bim.row_begin(y),
                 rend = rim.row_end(y); rptr != rend; ) {

            x++;
            *tptr++ = rgbMap(*rptr++, *gptr++, *bptr++);
        }
    }
}

/************************************************************************************************************/

template<typename PixelT>
struct Sum {
    PixelT operator()(PixelT r, PixelT g, PixelT b) const {
        return (r + g + b)/3;
    }
};

/************************************************************************************************************/
/**
 * A mapping for an asinh stretch (preserving colours independent of brightness)
 *
 *  x = asinh(Q (I - min)/range)/Q
 *
 * This reduces to a linear stretch if Q == 0
 */
template<typename PixelT, typename Intensity=Sum<PixelT> >
class AsinhMapping : public RgbMapping<PixelT> {
public:
    AsinhMapping(double min,            // minimum value of intensity
                 double range,          // range of intensities to use if Q == 0
                 double Q=0.0           // softening parameter for asinh stretch
                );
    virtual rgb_traits::pixel operator()(PixelT r, PixelT g, PixelT b) const;
private:
    double _min, _soften, _slope;
};
                
}}}}

#endif
