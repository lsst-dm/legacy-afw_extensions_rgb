#!/usr/bin/env python
"""
Tests for RGB Images

Run with:
   ./rgb.py
or
   python
   >>> import rgb; rgb.run()
"""

import math, os, sys
import numpy as np
import unittest

import lsst.utils.tests as utilsTests
import lsst.pex.exceptions as pexExceptions
import lsst.afw.detection as afwDetect
import lsst.afw.image as afwImage
import lsst.afw.geom as afwGeom
import lsst.afw.math as afwMath
import lsst.afw.display.ds9 as ds9
import lsst.afw.extensions.rgb as afwRgb
try:
    type(display)
except NameError:
    display = False

#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

R, G, B = 2, 1, 0

class RgbTestCase(unittest.TestCase):
    """A test case for Rgb"""
    def setUp(self):
        width, height = 85, 75
        self.images = []
        self.images.append(afwImage.ImageF(afwGeom.ExtentI(width, height)))
        self.images.append(afwImage.ImageF(afwGeom.ExtentI(width, height)))
        self.images.append(afwImage.ImageF(afwGeom.ExtentI(width, height)))

        for (x, y, A, g_r, r_i) in [(15, 15, 1000,  1.0,  2.0),
                                    (50, 45, 5500, -1.0, -0.5),
                                    (30, 30,  600,  1.0,  2.5),
                                    (45, 15, 20000,  1.0,  1.0),
                                    ]:
            for i in range(len(self.images)):
                if i == B:
                    amp = A
                elif i == G:
                    amp = A*math.pow(10, 0.4*g_r)
                elif i == R:
                    amp = A*math.pow(10, 0.4*r_i)

                self.images[i].set(x, y, amp)

        psf = afwMath.AnalyticKernel(15, 15, afwMath.GaussianFunction2D(2.5, 1.5, 0.5))

        convolvedImage = type(self.images[0])(self.images[0].getDimensions())
        randomImage = type(self.images[0])(self.images[0].getDimensions())
        rand = afwMath.Random()
        for i in range(len(self.images)):
            afwMath.convolve(convolvedImage, self.images[i], psf, True, True)
            afwMath.randomGaussianImage(randomImage, rand)
            randomImage *= 2
            convolvedImage += randomImage
            self.images[i] <<= convolvedImage
        del convolvedImage; del randomImage

    def tearDown(self):
        for im in self.images:
            del im
        del self.images

    def writeFile(self, fileName):
        if True:
            min, range, Q = 0, 5, 20  # asinh
        else:
            min, range, Q = 0, 5, 0    # linear

        asinh = afwRgb.asinhMappingF(min, range, Q)
        rgb = afwRgb.RgbImageF(self.images[R], self.images[G], self.images[B], asinh)
        if False:
            ds9.mtv(self.images[B], frame=0, title="B")
            ds9.mtv(self.images[G], frame=1, title="G")
            ds9.mtv(self.images[R], frame=2, title="R")

        rgb.write(fileName)


    def testStars(self):
        for ext in ("png", "tiff"):
            fileName = "rgb.%s" % ext
            self.writeFile(fileName)

            if False:
                if os.system("open %s > /dev/null 2>&1" % fileName) != 0:
                    print "Not removing %s" % (fileName)
            else:
                os.remove(fileName)

        def tst():
            self.writeFile("rgb.unknown")
        utilsTests.assertRaisesLsstCpp(self, pexExceptions.NotFoundException, tst)

    def testSaturated(self):
        """Test interpolating saturated pixels"""

        feet = {}
        for f in [R, G, B]:
            self.images[f] = afwImage.makeMaskedImage(self.images[f])

            ds = afwDetect.FootprintSet(self.images[f], afwDetect.Threshold(1000), "SAT")
            feet[f] = ds.getFootprints()

            arr = self.images[f].getImage().getArray()
            arr[np.where(arr >= 1000)] = np.nan

        afwRgb.replaceSaturatedPixels(self.images[R], self.images[G], self.images[B], 1, 2000)
        #
        # Check that we replaced those NaNs with some reasonable value
        #
        f0 = [k for k, v in feet.items() if v][0] # find a filter with a saturated region
        foot = feet[f0][0]
        s = foot.getSpans()[0]
        for f in [R, G, B]:
            val = self.images[f].getImage().get(s.getX0(), s.getY())
            self.assertTrue(np.isfinite(val))
        
        if False:
            ds9.mtv(self.images[B], frame=0, title="B")
            ds9.mtv(self.images[G], frame=1, title="G")
            ds9.mtv(self.images[R], frame=2, title="R")
        #
        # Prepare for generating an output file
        #
        for f in [R, G, B]:
            self.images[f] = self.images[f].getImage()

        fileName = "sat.png"
        self.writeFile(fileName)

        if False:
            if os.system("open %s > /dev/null 2>&1" % fileName) != 0:
                print "Not removing %s" % (fileName)
        else:
            os.remove(fileName)
        
#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

def suite():
    """Returns a suite containing all the test cases in this module."""

    utilsTests.init()

    suites = []
    suites += unittest.makeSuite(RgbTestCase)
    suites += unittest.makeSuite(utilsTests.MemoryTestCase)
    return unittest.TestSuite(suites)

def run(shouldExit=False):
    """Run the tests"""
    utilsTests.run(suite(), shouldExit)

if __name__ == "__main__":
    run(True)
