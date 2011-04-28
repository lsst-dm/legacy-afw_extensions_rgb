#include <cmath>
#include "lsst/afw/image/Image.h"
#include "boost/gil/extension/io/tiff_io.hpp"

namespace afwGeom = lsst::afw::geom;
namespace afwImage = lsst::afw::image;

template<typename Pixel>
void addStar(afwImage::Image<Pixel> *im, float const amp, float const xcen, float const ycen)
{
    int hsize = 7;
    float alpha = 1.0;

    int const ix =  static_cast<int>(xcen);  float const dx = xcen - ix;
    int const iy =  static_cast<int>(ycen);  float const dy = ycen - iy;

    for (int y = iy - hsize; y != iy + hsize + 1; ++y) {
        if (y < 0) {
            continue;
        } else if(y >= im->getHeight()) {
            break;
        }
        
        typename afwImage::Image<Pixel>::iterator ptr = im->at(ix - hsize, y);
        for (int x = ix - hsize; x != ix + hsize + 1; ++x) {
            if (x < 0) {
                continue;
            } else if (x >= im->getWidth()) {
                break;
            }
            *ptr++ = amp*::exp(-((pow((x - ix) + dx, 2) + pow((y - iy) + dy, 2)))/(2*pow(alpha, 2)));
        }
    }
}

int main() {
    typedef float Pixel;
    const int ncol = 50;
    const int nrow = 50;
    afwImage::Image<Pixel> R(afwGeom::ExtentI(ncol, nrow));
    afwImage::Image<Pixel> G(afwGeom::ExtentI(ncol, nrow));
    afwImage::Image<Pixel> B(afwGeom::ExtentI(ncol, nrow));

    R = 0; G = 0; B = 0;
    
    float xcen = 20.0;
    float ycen = 25.0;
    addStar(&R, 255, xcen, ycen);
    addStar(&G, 175, xcen, ycen);
    addStar(&B, 100, xcen, ycen);

    typedef boost::gil::rgb8_image_t IoImage;
    typedef boost::gil::rgb8_view_t IoView;
    IoImage tim(ncol, nrow);
    IoView tview = view(tim);
    
    for (int y = 0; y != nrow; ++y) {
        IoView::x_iterator tptr = tview.row_begin(y);
        for (afwImage::Image<Pixel>::x_iterator
                 rptr = R.row_begin(y), gptr = G.row_begin(y), bptr = B.row_begin(y); rptr != R.row_end(y);) {
            *tptr++ = boost::gil::rgb8_pixel_t(*rptr++, *gptr++, *bptr++);
        }
    }
    
    boost::gil::tiff_write_view("out.tiff", tview);
}
