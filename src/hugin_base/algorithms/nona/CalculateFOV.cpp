// -*- c-basic-offset: 4 -*-
/** @file PanoramaDataLegacySupport.h
 *
 *  @author Pablo d'Angelo <pablo.dangelo@web.de>
 *
 *  $Id: Panorama.h 1947 2007-04-15 20:46:00Z dangelo $
 *
 * !! from Panorama.h 1947 
 *
 *  This is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this software; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "CalculateFOV.h"

#include <algorithm>
#include <vigra/impex.hxx>
#include <nona/RemappedPanoImage.h>

namespace HuginBase {

    FDiff2D CalculateFOV::calcFOV(const PanoramaData& panorama)
    {
        vigra::Size2D panoSize(360,180);
        
        // remap into minature pano.
        PanoramaOptions opts;
        opts.setHFOV(360);
        opts.setProjection(PanoramaOptions::EQUIRECTANGULAR);
        opts.setWidth(360);
        opts.setHeight(180);
        
        // remap image
        // DGSW - make sure the type is correct
        vigra::BImage panoAlpha(panoSize.x, panoSize.y,static_cast< unsigned char >(0));
        //    vigra::BImage panoAlpha(panoSize.x, panoSize.y,0);
        Nona::RemappedPanoImage<vigra::BImage, vigra::BImage> remapped;
        UIntSet activeImgs = panorama.getActiveImages();
        for (UIntSet::iterator it = activeImgs.begin(); it != activeImgs.end(); ++it) {
            //    for (unsigned int imgNr=0; imgNr < getNrOfImages(); imgNr++) {
            // DGSW FIXME - Unreferenced
            //	        const PanoImage & img = getImage(*it);
            remapped.setPanoImage(panorama.getSrcImage(*it), opts);
            //remapped.setPanoImage(*this, *it, vigra::Size2D(img.getWidth(), img.getHeight()), opts);
            // calculate alpha channel
            remapped.calcAlpha();
            // copy into global alpha channel.
            vigra::copyImageIf(vigra_ext::applyRect(remapped.boundingBox(),
                                                    vigra_ext::srcMaskRange(remapped)),
                               vigra_ext::applyRect(remapped.boundingBox(),
                                                    vigra_ext::srcMask(remapped)),
                               vigra_ext::applyRect(remapped.boundingBox(),
                                                    destImage(panoAlpha)));
            //        vigra::ImageExportInfo imge2("c:/hugin_calcfov_alpha.png");
            //        exportImage(vigra::srcImageRange(panoAlpha), imge2);
            }
        
        // get field of view
        FDiff2D ul,lr;
        bool found = false;
        ul.x = DBL_MAX;
        ul.y = DBL_MAX;
        lr.x = -DBL_MAX;
        lr.y = -DBL_MAX;
        for (int v=0; v< 180; v++) {
            for (int h=0; h < 360; h++) {
                if (panoAlpha(h,v)) {
                    // pixel is valid
                    if ( ul.x > h ) {
                        found=true;
                        ul.x = h;
                    }
                    if ( ul.y > v ) {
                        found=true;
                        ul.y = v;
                    }
                    if ( lr.x < h) {
                        found=true;
                        lr.x = h;
                    }
                    if ( lr.y < v) {
                        found=true;
                        lr.y = v;
                    }
                }
            }
        }
        if (!found) {
            // if nothing found, return current fov
            return FDiff2D(panorama.getOptions().getHFOV(), panorama.getOptions().getVFOV());
        }
        ul.x = ul.x - 180;
        ul.y = ul.y - 90;
        lr.x = lr.x - 180;
        lr.y = lr.y - 90;
        FDiff2D fov (2*std::max(fabs(ul.x), fabs(lr.x)), 2*std::max(fabs(ul.y), fabs(lr.y)));
        return fov;
    }
        
}
        
        