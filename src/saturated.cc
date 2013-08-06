/**
 * \file
 *
 * Handle saturated pixels when making colour images
 */
#include "boost/format.hpp"
#include "lsst/afw/detection.h"
#include "lsst/afw/image/MaskedImage.h"
#include "lsst/afw/extension/rgb/Rgb.h"

namespace lsst { namespace afw {
namespace extension { namespace rgb {

namespace {
    template <typename ImageT>
    class SetPixels : public detection::FootprintFunctor<ImageT> {
    public:
        SetPixels(ImageT const& img     // The image the source lives in
                 ) : detection::FootprintFunctor<ImageT>(img), _value(0) {}

        void setValue(float value) { _value = value; }

        // method called for each pixel by apply()
        void operator()(typename ImageT::xy_locator loc,        // locator pointing at the pixel
                        int,                                    // column-position of pixel
                        int                                     // row-position of pixel
                       ) {
            *loc = _value;
        }
    private:
        float _value;
    };
}

template<typename ImageT>
void
replaceSaturatedPixels(ImageT & rim,    // R image (e.g. i)
                       ImageT & gim,    // G image (e.g. r)
                       ImageT & bim,    // B image (e.g. g)
                       int borderWidth,	// width of border used to estimate colour of saturated regions
                       float saturatedPixelValue // the brightness of a saturated pixel, once fixed
                      )
{
    SetPixels<typename ImageT::Image>
        setR(*rim.getImage()),
        setG(*gim.getImage()),
        setB(*bim.getImage()); // functors used to set pixel values

    // Find all the saturated pixels in any of the three image
    int const npixMin = 1;              // minimum number of pixels in an object
    int const SAT = rim.getMask()->getPlaneBitMask("SAT");
    detection::Threshold const satThresh(SAT, detection::Threshold::BITMASK);

    detection::FootprintSet       sat(*rim.getMask(), satThresh, npixMin);
    sat.merge(detection::FootprintSet(*gim.getMask(), satThresh, npixMin));
    sat.merge(detection::FootprintSet(*bim.getMask(), satThresh, npixMin));
    // go through the list of saturated regions, determining the mean colour of the surrounding pixels
    typedef detection::FootprintSet::FootprintList FootprintList;
    PTR(FootprintList) feet = sat.getFootprints();
    for (FootprintList::iterator ptr = feet->begin(), end = feet->end(); ptr != end; ++ptr) {
        PTR(detection::Footprint) foot = *ptr;
        PTR(detection::Footprint) bigFoot = growFootprint(*foot, borderWidth);

        float sumR = 0, sumG = 0, sumB = 0;

        for (detection::Footprint::SpanList::const_iterator sptr = bigFoot->getSpans().begin(),
                 send = bigFoot->getSpans().end(); sptr != send; ++sptr) {
            PTR(detection::Span) const span = *sptr;

            int const y = span->getY();
            int const sx0 = span->getX0();
            int const sx1 = span->getX1();

            for (typename ImageT::iterator
                     rptr = rim.at(sx0, y),
                     rend = rim.at(sx1 + 1, y),
                     gptr = gim.at(sx0, y),
                     bptr = bim.at(sx0, y); rptr != rend; ++rptr, ++gptr, ++bptr) {
                if (!((rptr.mask() | gptr.mask() | bptr.mask()) & SAT)) {
                    sumR += rptr.image();
                    sumG += gptr.image();
                    sumB += bptr.image();
                }
            }
        }
        // OK, we have the mean fluxes for the pixels surrounding this set of saturated pixels
        // so we can figure out the proper values to use for the saturated ones
        float R = 0, G = 0, B = 0;      // mean intensities
        if(sumR + sumB + sumG > 0) {
            if(sumR > sumG) {
                if(sumR > sumB) {
                    R = saturatedPixelValue;
                    G = (saturatedPixelValue*sumG)/sumR;
                    B = (saturatedPixelValue*sumB)/sumR;
                } else {
                    R = (saturatedPixelValue*sumR)/sumB;
                    G = (saturatedPixelValue*sumG)/sumB;
                    B = saturatedPixelValue;
                }
            } else {
                if(sumG > sumB) {
                    R = (saturatedPixelValue*sumR)/sumG;
                    G = saturatedPixelValue;
                    B = (saturatedPixelValue*sumB)/sumG;
                } else {
                    R = (saturatedPixelValue*sumR)/sumB;
                    G = (saturatedPixelValue*sumG)/sumB;
                    B = saturatedPixelValue;
                }
            }
        }
        // Now that we know R, G, and B we can fix the values
        setR.setValue(R); setR.apply(*foot);
        setG.setValue(G); setG.apply(*foot);
        setB.setValue(B); setB.apply(*foot);
    }
}

template
void
replaceSaturatedPixels(image::MaskedImage<float> & rim,
                       image::MaskedImage<float> & gim,
                       image::MaskedImage<float> & bim,
                       int borderWidth,
                       float saturatedPixelValue
                      );

}}}}
