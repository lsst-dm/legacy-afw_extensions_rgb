#include "lsst/afw/image/Image.h"
#include "boost/gil/extension/io/tiff_io.hpp"

namespace afwGeom = lsst::afw::geom;
namespace afwImage = lsst::afw::image;

int main() {
    typedef float Pixel;
    const int ncol = 100;
    const int nrow = 256;
    afwImage::Image<Pixel> im(afwGeom::ExtentI(ncol, nrow));

    for (int y = 0; y != nrow; ++y) {
        for (afwImage::Image<Pixel>::x_iterator ptr = im.row_begin(y); ptr != im.row_end(y); ++ptr) {
            *ptr = 0 + y;
        }
    }

    typedef boost::gil::rgb8_image_t IoImage;
    typedef boost::gil::rgb8_view_t IoView;
    IoImage tim(ncol, nrow);
    IoView tview = view(tim);
    
    for (int y = 0; y != nrow; ++y) {
        IoView::x_iterator tptr = tview.row_begin(y);
        for (afwImage::Image<Pixel>::x_iterator ptr = im.row_begin(y); ptr != im.row_end(y); ++ptr, ++tptr) {
            *tptr = boost::gil::rgb8_pixel_t(*ptr, 0, 0);
        }
    }
    
    boost::gil::tiff_write_view("out.tiff", tview);
}
