// -*- c-basic-offset: 4 -*-
/** @file PreviewPanel.h
 *
 *  @author Pablo d'Angelo <pablo.dangelo@web.de>
 *
 *  $Id$
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

#ifndef _PREVIEWPANEL_H
#define _PREVIEWPANEL_H

#include <vector>

#include "wx/frame.h"
#include "wx/dnd.h"

#include <PT/ImageTransforms.h>

class wxImage;

/** A preview panel that renders the pictures using the panotools library
 *
 *  Lets hope this works out fine..
 */
class PreviewPanel : public wxPanel, public PT::PanoramaObserver
{
    typedef PT::RemappedPanoImage<vigra::BRGBImage, vigra::BImage> RemappedImage;
    typedef std::vector<RemappedImage *> RemappedVector;
public:

    /** ctor.
     */
    PreviewPanel(wxWindow *parent, PT::Panorama * pano );

    /** dtor.
     */
    virtual ~PreviewPanel();

    void panoramaChanged(PT::Panorama &pano);
    void panoramaImagesChanged(PT::Panorama &pano, const PT::UIntSet & imgNr);

    void SetAutoUpdate(bool enabled);

    // forces an update of all images.
    void ForceUpdate();

    // select which images should be shown.
    void SetDisplayedImages(const PT::UIntSet &images);

private:

    // draw the preview directly onto the canvas
    void DrawPreview(wxDC & dc);

    // remaps the images, called automatically if autopreview is enabled.
    void updatePreview();

    void mapPreviewImage(unsigned int imgNr);

    /** recalculate panorama to fit the panel */
    void OnResize(wxSizeEvent & e);
    void OnDraw(wxPaintEvent & event);
    void OnMouse(wxMouseEvent & e);
    void OnUpdatePreview(wxCommandEvent & e);
    void DrawOutline(const std::vector<FDiff2D> & points, wxDC & dc, int offX, int offY);

    /** the model */
    PT::Panorama &pano;

    bool m_autoPreview;

    vigra::Diff2D m_panoImgSize;

    PT::UIntSet m_displayedImages;

    RemappedVector m_remapped;
    wxBitmap * m_panoBitmap;
    // currently updating the preview.

    PT::UIntSet m_dirtyImgs;

    // panorama options
    PT::PanoramaOptions opts;

    wxWindow * parentWindow;

    DECLARE_EVENT_TABLE()
};



#endif // _PREVIEWPANEL_H
