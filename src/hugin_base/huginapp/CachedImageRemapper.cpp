// -*- c-basic-offset: 4 -*-

/** @file ImageCache.cpp
 *
 *  @brief implementation of ImageCache Class
 *
 *  @author Pablo d'Angelo <pablo.dangelo@web.de>
 *
 *  $Id: ImageCache.cpp 1997 2007-05-09 21:14:15Z dangelo $
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this software; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "CachedImageRemapper.h"

//#include <config.h>
//#include "panoinc_WX.h"
//
//#include "panoinc.h"
//
//#include <fstream>
//
//#include <vigra/basicimage.hxx>
#include <vigra/basicimageview.hxx>
//#include <vigra/rgbvalue.hxx>
//#include <vigra/impex.hxx>
//#include <vigra/error.hxx>
//#include <vigra_ext/utils.h>
//#include <vigra_ext/impexalpha.hxx>
//#include <vigra_ext/Pyramid.h>
//#include <vigra_ext/ImageTransforms.h>
//#include <vigra_ext/FunctorAccessor.h>
//#include <PT/Stitcher.h>
//#include <vigra/functorexpression.hxx>
//
//#include "hugin/ImageCache.h"
//#include "hugin/config_defaults.h"
//
//#include <vigra/sized_int.hxx>


namespace HuginBase {

//using namespace std;
using namespace vigra;
//using namespace vigra_ext;
//using namespace hugin_utils;
//using namespace functor;



SmallRemappedImageCache::~SmallRemappedImageCache()
{
    invalidate();
}

SmallRemappedImageCache::MRemappedImage *
SmallRemappedImageCache::getRemapped(const PanoramaData& pano,
                                    const PanoramaOptions & popts,
                                    unsigned int imgNr,
                                    AppBase::MultiProgressDisplay& progress)
{
    // always map to HDR mode. curve and exposure is applied in preview window, for speed
    PanoramaOptions opts = popts;
    opts.outputMode = PanoramaOptions::OUTPUT_HDR;
    opts.outputExposureValue = 0.0;

    // return old image, if already in cache and if it has changed since the last rendering
    if (set_contains(m_images, imgNr)) {
        // return cached image if the parameters of the image have not changed
        SrcPanoImage oldParam = m_imagesParam[imgNr];
        if (oldParam == pano.getSrcImage(imgNr)
                && m_panoOpts[imgNr].getHFOV() == opts.getHFOV()
                && m_panoOpts[imgNr].getWidth() == opts.getWidth()
                && m_panoOpts[imgNr].getHeight() == opts.getHeight()
                && m_panoOpts[imgNr].getProjection() == opts.getProjection()
                && m_panoOpts[imgNr].getProjectionParameters() == opts.getProjectionParameters()
           )
        {
            DEBUG_DEBUG("using cached remapped image " << imgNr);
            return m_images[imgNr];
        }
    }

    // WARNING: this might invalidate images that are stored somewhere..
    ImageCache::getInstance().softFlush();
    
    typedef  BasicImageView<RGBValue<unsigned char> > BRGBImageView;

//    typedef NumericTraits<PixelType>::RealPromote RPixelType;

    // remap image
    DEBUG_DEBUG("remapping image " << imgNr);

    // load image
    const PanoImage & img = pano.getImage(imgNr);
    const ImageOptions & iopts = img.getOptions();

    ImageCache::EntryPtr e = ImageCache::getInstance().getSmallImage(img.getFilename().c_str());
    if ( (e->image8->width() == 0) && (e->imageFloat->width() == 0) ) {
        throw std::runtime_error("could not retrieve small source image for preview generation");
    }
    Size2D srcImgSize;
    if (e->image8->width() > 0)
        srcImgSize = e->image8->size();
    else
        srcImgSize = e->imageFloat->size();

    MRemappedImage *remapped = new MRemappedImage;
    SrcPanoImage srcPanoImg = pano.getSrcImage(imgNr);
    // adjust distortion parameters for small preview image
    srcPanoImg.resize(srcImgSize);

    FImage srcFlat;
    // use complete image, by supplying an empty mask image
    BImage srcMask;

    if (iopts.m_vigCorrMode & ImageOptions::VIGCORR_FLATFIELD) {
        ImageCache::EntryPtr e = ImageCache::getInstance().getSmallImage(iopts.m_flatfield.c_str());
        if (!e) {
            throw std::runtime_error("could not retrieve flatfield image for preview generation");
        }
        if (e->image8->width()) {
            srcFlat.resize(e->image8->size());
            copyImage(srcImageRange(*(e->image8),
                             RGBToGrayAccessor<RGBValue<UInt8> >()),
                             destImage(srcFlat));
        } else {
            srcFlat.resize(e->imageFloat->size());
            copyImage(srcImageRange(*(e->imageFloat),
                             RGBToGrayAccessor<RGBValue<float> >()),
                             destImage(srcFlat));
        }
    }
    progress.pushTask(AppBase::ProgressTask("remapping", "", 0));

    if (e->imageFloat->width()) {
        // remap image
        remapImage(*(e->imageFloat),
                   srcMask,
                   srcFlat,
                   srcPanoImg,
                   opts,
                   *remapped,
                   progress);
    } else {
        remapImage(*(e->image8),
                     srcMask,
                     srcFlat,
                     srcPanoImg,
                     opts,
                     *remapped,
                     progress);
    }

    progress.popTask();

    m_images[imgNr] = remapped;
    m_imagesParam[imgNr] = pano.getSrcImage(imgNr);
    m_panoOpts[imgNr] = opts;
    return remapped;
}


void SmallRemappedImageCache::invalidate()
{
    DEBUG_DEBUG("Clear remapped cache");
    for(std::map<unsigned int, MRemappedImage*>::iterator it = m_images.begin();
        it != m_images.end(); ++it)
    {
        delete (*it).second;
    }
    // remove all images
    m_images.clear();
    m_imagesParam.clear();
}

void SmallRemappedImageCache::invalidate(unsigned int imgNr)
{
    DEBUG_DEBUG("Remove " << imgNr << " from remapped cache");
    if (set_contains(m_images, imgNr)) {
        delete (m_images[imgNr]);
        m_images.erase(imgNr);
        m_imagesParam.erase(imgNr);
    }
}


} //namespace