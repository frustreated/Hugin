// -*- c-basic-offset: 4 -*-

/** @file PanoramaMemento.cpp
 *
 *  @brief implementation of PanoramaMemento Class
 *
 *  @author Pablo d'Angelo <pablo.dangelo@web.de>
 *
 *  $Id$
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

#include "panoinc.h"
#include <jhead/jhead.h>

using namespace PT;
using namespace std;


PanoramaMemento::~PanoramaMemento()
{

}

void PT::fillVariableMap(VariableMap & vars)
{

    vars.insert(pair<const char*, Variable>("y",Variable("y",0)));
    vars.insert(pair<const char*, Variable>("r",Variable("r",0)));
    vars.insert(pair<const char*, Variable>("p",Variable("p",0)));

    // Lens variables
    vars.insert(pair<const char*, Variable>("v",Variable("v",51)));
    vars.insert(pair<const char*, Variable>("a",Variable("a",0)));
    vars.insert(pair<const char*, Variable>("b",Variable("b",0)));
    vars.insert(pair<const char*, Variable>("c",Variable("c",0)));
    vars.insert(pair<const char*, Variable>("d",Variable("d",0)));
    vars.insert(pair<const char*, Variable>("e",Variable("e",0)));
};


ostream & Variable::print(ostream & o) const
{
    return o << name << value;
}

std::ostream & LensVariable::printLink(std::ostream & o,
                                       unsigned int linkImage) const
{
    return o << name << "=" << linkImage;
}

#if 0
std::ostream & ImageVariables::print(std::ostream & o, bool printLinks) const
{
    for (const_iterator it = begin()
    yaw.print(o, printLinks) << " ";
    roll.print(o, printLinks) << " ";
    pitch.print(o, printLinks) << " ";
    HFOV.print(o, printLinks) << " ";
    a.print(o, printLinks) << " ";
    b.print(o, printLinks) << " ";
    c.print(o, printLinks) << " ";
    d.print(o, printLinks) << " ";
    e.print(o, printLinks);
    return o;
};
#endif

#if 0
void ImageVariables::updateValues(const ImageVariables & vars)
{
    yaw.setValue(vars.yaw.getValue());
    roll.setValue(vars.roll.getValue());
    pitch.setValue(vars.pitch.getValue());
    HFOV.setValue(vars.HFOV.getValue());
    a.setValue(vars.a.getValue());
    b.setValue(vars.b.getValue());
    c.setValue(vars.c.getValue());
    d.setValue(vars.d.getValue());
    e.setValue(vars.e.getValue());
}

#endif


/*
QString PT::getAttrib(QDomNamedNodeMap map, QString name)
{
    return map.namedItem(name).nodeValue();
}
*/

//=========================================================================
//=========================================================================


Lens::Lens()
    : projectionFormat(RECTILINEAR),
      isLandscape(false),
      focalLengthConversionFactor(1),
      sensorWidth(36.0), sensorHeight(24.0),
      sensorRatio(1.5)
{
    variables.insert(pair<const char*, LensVariable>("v",LensVariable("v",50 , true)));
    variables.insert(pair<const char*, LensVariable>("a",LensVariable("a", 0.0, true )));
    variables.insert(pair<const char*, LensVariable>("b",LensVariable("b",-0.01, true)));
    variables.insert(pair<const char*, LensVariable>("c",LensVariable("c", 0.0, true)));
    variables.insert(pair<const char*, LensVariable>("d",LensVariable("d", 0.0)));
    variables.insert(pair<const char*, LensVariable>("e",LensVariable("e", 0.0)));
}

char *PT::Lens::variableNames[] = { "v", "a", "b", "c", "d", "e", 0};

#if 0
void Lens::toXML(std::ostream & o)
{
    o << "<lens type=\"" << prjToString(projection) << "\">" << endl
      << root.setAttribute("focal_length", focalLength)
      << root.setAttribute("focal_length_conv_factor", focalLengthConversionFactor)
      << root.setAttribute("HFOV", HFOV)
      << "<distortion a=\"" << a
      << "\" b=\"" << b
      << "\" c=\"" << c
      << "\" d=\"" << d
      << "\" e=\"" << e << "\"/>" << endl;
}


void Lens::setFromXML(const QDomNode & node)
{
    DEBUG_DEBUG("LensSettings::setFromXML");
    Q_ASSERT(node.nodeName() == "lens");
    QDomNamedNodeMap attr = node.attributes();
    projectionFormat = (ProjectionFormat) getAttrib(attr, "projection").toUInt();
    focalLength = getAttrib(attr, "focal_length").toDouble();
    focalLengthConversionFactor = getAttrib(attr, "focal_length_conv_factor").toDouble();
    HFOV = getAttrib(attr, "HFOV").toDouble();
    a = getAttrib(attr, "a").toDouble();
    b = getAttrib(attr, "b").toDouble();
    c = getAttrib(attr, "c").toDouble();
    d = getAttrib(attr, "d").toDouble();
    e = getAttrib(attr, "e").toDouble();
    exifFocalLength = getAttrib(attr, "exif_focal_length").toDouble();
    exifFocalLengthConversionFactor = getAttrib(attr, "exif_focal_length_conv_factor").toDouble();
    exifHFOV = getAttrib(attr, "exif_HFOV").toDouble();
}
#endif


bool Lens::readEXIF(const std::string & filename)
{

    double HFOV = 0;

    std::string::size_type idx = filename.rfind('.');
    if (idx == std::string::npos) {
        DEBUG_DEBUG("could not find extension in filename");
        return false;
    }
    std::string ext = filename.substr( idx+1 );
    if (ext != "jpg" && ext != "JPG" && ext != "jpeg" && ext != "JPEG") {
        DEBUG_NOTICE("can only read lens data from jpg files, current ext:"
                     << ext);
        return false;
    }

    ImageInfo_t exif;
    ResetJpgfile();

    // Start with an empty image information structure.
    memset(&exif, 0, sizeof(exif));
    exif.FlashUsed = -1;
    exif.MeteringMode = -1;

    if (!ReadJpegFile(exif,filename.c_str(), READ_EXIF)){
        DEBUG_DEBUG("Could not read jpg info");
        return false;
    }
    ShowImageInfo(exif);

    DEBUG_DEBUG("exif dimensions: " << exif.Width << "x" << exif.Height);

    sensorWidth = exif.CCDWidth;
    if (sensorWidth <= 0) {
        DEBUG_ERROR("EXIF does not contain sensor width, assuming 36 x 24 mm film");
        sensorWidth = 36.0;
    }

    // FIXME, assumes square pixels
    //sensorHeight = sensorWidth / ratio;
    sensorHeight = exif.CCDHeight;
    if (sensorHeight <= 0) {
        DEBUG_ERROR("EXIF does not contain sensor height, assuming 36 x 24 mm film");
        sensorHeight = 24.0;
    }

    // update the sensor ratio to fit this
    sensorRatio = sensorWidth / sensorHeight;
    if (sensorWidth < sensorHeight) {
        DEBUG_WARN("support for sensors probably buggy");
    }
    focalLengthConversionFactor = sqrt(36.0*36.0 + 24.0*24.0) /
                                  sqrt(sensorWidth * sensorWidth + sensorHeight * sensorHeight);

    if (isLandscape) {
        HFOV = 2.0 * atan((sensorWidth/2)/exif.FocalLength) * 180.0/M_PI;
    } else {
        HFOV = 2.0 * atan((sensorHeight/2)/exif.FocalLength) * 180.0/M_PI;
    }

    if ( HFOV <= 0.0)
        HFOV = 50;
    DEBUG_DEBUG("CCD size: " << sensorWidth << "," << sensorHeight << " mm");
    DEBUG_DEBUG("real focal length: " << exif.FocalLength << ", 35mm equiv: "
                << exif.FocalLength * focalLengthConversionFactor
                << " crop factor: " << focalLengthConversionFactor
                << " HFOV: " << HFOV);

    map_get(variables,"v").setValue(HFOV);

    return true;
}


void Lens::setFLFactor(double factor)
{
    focalLengthConversionFactor = factor;

    // calculate corrosponding diagonal on our sensor
    double d = sqrt(36.0*36.0 + 24.0*24.0) / focalLengthConversionFactor;
    // calculate the sensor width and height that fit the ratio
    // the ratio is determined by the size of our image.
    sensorHeight = d / sqrt(sensorRatio*sensorRatio +1);
    sensorWidth = sensorHeight * sensorRatio;
    DEBUG_DEBUG("factor: " << factor << "ratio: " << sensorRatio << " --> New sensor size: " << sensorWidth << "," << sensorHeight)
}


void Lens::setRatio(double ratio)
{
    sensorRatio = ratio;
    setFLFactor(focalLengthConversionFactor);
}


void Lens::update(const Lens & l)
{
    focalLengthConversionFactor = l.focalLengthConversionFactor;
    projectionFormat = l.projectionFormat;
    sensorWidth = l.sensorWidth;
    sensorHeight = l.sensorHeight;
    variables = l.variables;
}


double Lens::calcFocalLength(double HFOV) const
{
    if (isLandscape) {
        return (sensorWidth/2.0) / tan(HFOV/180.0*M_PI/2);
    } else {
        return (sensorHeight/2.0) / tan(HFOV/180.0*M_PI/2);
    }
}


double Lens::calcHFOV(double focalLength) const
{
    if (isLandscape) {
        return 2.0 * atan((sensorWidth/2.0)/focalLength) * 180.0/M_PI;
    } else {
        return 2.0 * atan((sensorHeight/2.0)/focalLength) * 180.0/M_PI;
    }
}

//=========================================================================
//=========================================================================

#if 0
ControlPoint::ControlPoint(Panorama & pano, const QDomNode & node)
{
    setFromXML(node,pano);
}
#endif


#if 0
QDomNode ControlPoint::toXML(QDomDocument & doc) const
{
    QDomElement elem = doc.createElement("control_point");
    elem.setAttribute("image1", image1->getNr());
    elem.setAttribute("x1", x1);
    elem.setAttribute("y1", y1);
    elem.setAttribute("image2", image2->getNr());
    elem.setAttribute("x2", x2);
    elem.setAttribute("y2", y2);
    elem.setAttribute("mode", mode);
    elem.setAttribute("distance",error);
    return elem;
}

void ControlPoint::setFromXML(const QDomNode & elem, Panorama & pano)
{
    DEBUG_DEBUG("ControlPoint::setFromXML");
    Q_ASSERT(elem.nodeName() == "control_point");
    QDomNamedNodeMap attrs = elem.attributes();
    image1 = pano.getImage(getAttrib(attrs,"image1").toUInt());
    x1 = getAttrib(attrs,"x1").toUInt();
    y1 = getAttrib(attrs,"y1").toUInt();
    image2 = pano.getImage(getAttrib(attrs,"image2").toUInt());
    x2 = getAttrib(attrs,"x2").toUInt();
    y2 = getAttrib(attrs,"y2").toUInt();
    error = getAttrib(attrs,"distance").toDouble();
    mode = (OptimizeMode) getAttrib(attrs, "mode").toUInt();
}

#endif

void ControlPoint::mirror()
{
    unsigned int ti;
    double td;
    ti =image1Nr; image1Nr = image2Nr, image2Nr = ti;
    td = x1; x1 = x2 ; x2 = td;
    td = y1; y1 = y2 ; y2 = td;
}

std::string ControlPoint::modeNames[] = { "x_y", "x", "y" };


const std::string & ControlPoint::getModeName(OptimizeMode mode) const
{
    return modeNames[mode];
}

//=========================================================================
//=========================================================================


#if 0
QDomNode PanoramaOptions::toXML(QDomDocument & doc) const
{
    QDomElement elem = doc.createElement("output");
    elem.setAttribute("projection", projectionFormat);
    elem.setAttribute("HFOV", HFOV);
    elem.setAttribute("width", width);
    elem.setAttribute("height", height);
    elem.setAttribute("output", outfile);
    elem.setAttribute("format", outputFormat);
    elem.setAttribute("jpg_quality", quality);
    elem.setAttribute("progressive", progressive);
    elem.setAttribute("color_correction", colorCorrection);
    elem.setAttribute("color_ref_image", colorReferenceImage);
    elem.setAttribute("gamma", gamma);
    elem.setAttribute("interpolator", interpolator);
    return elem;
}

void PanoramaOptions::setFromXML(const QDomNode & elem)
{
    DEBUG_DEBUG("PanoramaOptions::setFromXML");
    Q_ASSERT(elem.nodeName() == "output");
    QDomNamedNodeMap attrs = elem.attributes();
    projectionFormat = (ProjectionFormat)getAttrib(attrs,"projection").toUInt();
    HFOV = getAttrib(attrs,"HFOV").toDouble();
    width = getAttrib(attrs,"width").toUInt();
    height = getAttrib(attrs,"height").toUInt();
    outfile = getAttrib(attrs,"output");
    outputFormat = getAttrib(attrs,"format");
    quality = getAttrib(attrs,"jpg_quality").toUInt();
    progressive = getAttrib(attrs,"height").toUInt() != 0;
    colorCorrection = (ColorCorrection) getAttrib(attrs, "color_correction").toUInt();
    colorReferenceImage = getAttrib(attrs, "color_ref_image").toUInt();
    gamma = getAttrib(attrs, "gamma").toDouble();
    interpolator = (Interpolator) getAttrib(attrs, "interpolator").toUInt();
}

#endif

const std::string & PanoramaOptions::getFormatName(FileFormat f)
{
    assert((int)f <= (int)QTVR);
    return fileformatNames[(int) f];
}

PanoramaOptions::FileFormat PanoramaOptions::getFormatFromName(const std::string & name)
{
    int max = (int) QTVR;
    int i;
    for (i=0; i<max; i++) {
        if (name == fileformatNames[i]) {
            break;
        }
    }
    if (i == max) {
        DEBUG_ERROR("could not parse format " << name );
        return TIFF;
    }
    return (FileFormat) i;
}


void PanoramaOptions::printScriptLine(std::ostream & o) const
{
    o << "p f" << projectionFormat << " w" << width << " h" << getHeight()
      << " v" << HFOV << " ";

    switch (colorCorrection) {
    case NONE:
        break;
    case BRIGHTNESS_COLOR:
        o << " k" << colorReferenceImage;
        break;
    case BRIGHTNESS:
        o << " b" << colorReferenceImage;
        break;
    case COLOR:
        o << " d" << colorReferenceImage;
        break;
    }

    o << " n\"" << getFormatName(outputFormat);
    if ( outputFormat == JPEG ) {
        o << " q" << quality;
    }
    o << "\"";
    o << std::endl;

    // misc options
    o << "m g" << gamma << " i" << interpolator << std::endl;
}

unsigned int PanoramaOptions::getHeight() const
{
    switch (projectionFormat) {
    case RECTILINEAR:
    {
        return (int) ( width * tan(DEG_TO_RAD(VFOV)/2.0) / tan(DEG_TO_RAD(HFOV)/2.0));
        break;
    }
    case CYLINDRICAL:
    {
        double f = (double)width / DEG_TO_RAD(HFOV);
        return (int) (2.0 * tan(DEG_TO_RAD(VFOV)/2.0) * f);
        break;
    }
    case EQUIRECTANGULAR:
        return (int) (width * VFOV/HFOV);
    }
    return 0;

}

const string PanoramaOptions::fileformatNames[] =
{
    "JPEG",
    "PNG",
    "TIFF",
    "TIFF_m",
    "TIFF_mask",
    "PICT",
    "PSD",
    "PSD_m",
    "PSD_mask",
    "PAN",
    "IVR",
    "IVR_java",
    "VRML",
    "QTVR"
};


bool PanoramaMemento::loadPTScript(std::istream &i, const std::string &prefix)
{
    DEBUG_TRACE("");
#ifdef __unix__
    // set numeric locale to C, for correct number output
    char * old_locale = setlocale(LC_NUMERIC,NULL);
    setlocale(LC_NUMERIC,"C");
#endif

    PTFileFormat fileformat = PTFILE_HUGIN;

    PTParseState state;
    string line;

    // indicate PTGui's dummy image
    bool ptGUIDummyImage = false;

    // PTGui & PTAssembler image names.
    string nextFilename = "";
    int nextWidth = 0;
    int nextHeight = 0;

    bool firstOptVecParse = true;
    unsigned int lineNr = 0;
    while (!i.eof()) {
        std::getline(i, line);
        lineNr++;
        DEBUG_DEBUG(lineNr << ": " << line);
        // check for a known line
        switch(line[0]) {
        case 'p':
        {
            DEBUG_DEBUG("p line: " << line);
            string format;
            int i;
            getParam(i,line,"f");
            options.projectionFormat = (PanoramaOptions::ProjectionFormat) i;
            getParam(options.width, line, "w");
            getParam(options.HFOV, line, "v");
            int height;
            getParam(height, line, "h");


            switch (options.projectionFormat) {
            case PanoramaOptions::RECTILINEAR:
                options.VFOV = 2.0 * atan( (double)height * tan(DEG_TO_RAD(options.HFOV)/2.0) / options.width);
                options.VFOV = RAD_TO_DEG(options.VFOV);
                break;
            case PanoramaOptions::CYLINDRICAL:
            { 
		// equations: w = f * v (f: focal length, in pixel)
                double f = options.width / DEG_TO_RAD(options.HFOV);
                options.VFOV = 2*atan(height/(2.0*f));
                options.VFOV = RAD_TO_DEG(options.VFOV);
                break;
            }
            case PanoramaOptions::EQUIRECTANGULAR:
                options.VFOV = options.HFOV * height / options.width;
                break;
            }


            DEBUG_DEBUG("options.VFOV: " << options.VFOV << " ratio: "
                        << (double) height / options.width);
            // this is fragile.. hope nobody adds additional whitespace
            // and other arguments than q...
            // n"JPEG q80"
            getPTStringParam(format,line,"n");
            int t = format.find(' ');
            // FIXME. add argument parsing for output formats
            options.outputFormat = options.getFormatFromName(format.substr(0,t));
            // "parse" jpg quality
            unsigned int q = format.find('q',t);
            if (q != string::npos) {
                DEBUG_DEBUG("found jpg quality: " << format.substr(q+1));
                options.quality = utils::lexical_cast<int, string>(format.substr(q+1));
            }

            int cRefImg = 0;
            if (getParam(cRefImg, line,"k")) {
                options.colorCorrection = PanoramaOptions::BRIGHTNESS_COLOR;
            } else if (getParam(cRefImg, line,"b")) {
                options.colorCorrection = PanoramaOptions::BRIGHTNESS;
            } else if (getParam(cRefImg, line,"d")) {
                options.colorCorrection = PanoramaOptions::COLOR;
            } else {
                options.colorCorrection = PanoramaOptions::NONE;
            }
            options.colorReferenceImage=cRefImg;
            break;

        }
        case 'm':
        {
            DEBUG_DEBUG("m line: " << line);
            // parse misc options
            int i;
            getParam(i,line,"i");
            options.interpolator = (PanoramaOptions::Interpolator) i;
            getParam(options.gamma,line,"g");
            break;
        }
        case 'i':
            // ptgui & ptasm scripts have 'o' image lines...
        case 'o':
        {
            // ugly hack to load PTGui script files
            if (ptGUIDummyImage) {
                DEBUG_DEBUG("loading default PTGUI line: " << line);
                Lens l;
                // skip ptgui's dummy image
                // load parameters into default lens...
                for (LensVarMap::iterator it = l.variables.begin();
                 it != l.variables.end();
                 ++it)
                {
                    DEBUG_DEBUG("reading default lens variable " << it->first);
                    int link;
                    bool ok = readVar(it->second, link, line);
                    DEBUG_ASSERT(ok);
                    DEBUG_ASSERT(link == -1);
                }
                lenses.push_back(l);

                ptGUIDummyImage = false;
                break;
            }
            DEBUG_DEBUG("i line: " << line);
            // parse image lines

            bool ok;

            // read the variables & decide if to create a new lens or not
            VariableMap vars;
            fillVariableMap(vars);
            int link;
            ok = readVar(map_get(vars, "r"), link, line);
            if (!ok) {
#ifdef __unix__
                // reset locale
                setlocale(LC_NUMERIC,old_locale);
#endif
                return false;
            }
            DEBUG_ASSERT(link == -1);
            ok = readVar(map_get(vars, "p"), link, line);
            if (!ok){
#ifdef __unix__
                // reset locale
                setlocale(LC_NUMERIC,old_locale);
#endif

                return false;
            }
            DEBUG_ASSERT(link == -1);
            ok = readVar(map_get(vars, "y"), link, line);
            if (!ok) {
#ifdef __unix__
                // reset locale
                setlocale(LC_NUMERIC,old_locale);
#endif

                return false;
            }
            DEBUG_ASSERT(link == -1);

            string file;
            int width, height;
            // use previously known filename, if provided..
            if (nextFilename != "") {
                file = nextFilename;
                nextFilename = "";
                width = nextWidth;
                height = nextHeight;
            } else {
                getPTStringParam(file,line,"n");

                getParam(width, line, "w");
                getParam(height, line, "h");

            }
            // add prefix if only a relative path.
            // FIXME, make this more robust. it breaks if one saves the project in a different dir
            // as the images
            if (file.find_first_of("\\/") == string::npos) {
                file.insert(0, prefix);
            }
            DEBUG_DEBUG("filename: " << file);

            Lens l;

            l.isLandscape = width > height;

            int anchorImage = -1;
            int lensNr = -1;
            for (LensVarMap::iterator it = l.variables.begin();
                 it != l.variables.end();
                 ++it)
            {
                DEBUG_DEBUG("reading variable " << it->first);
                ok = readVar(it->second, link, line);
                if (!ok) {
#ifdef __unix__
                    // reset locale
                    setlocale(LC_NUMERIC,old_locale);
#endif

                    return false;
                }
                if (link !=-1) {
                    // linked variable
                    if ( anchorImage < 0) {
                        // first occurance of a link for this image.
                        // special case for PTGUI script files
                        if (fileformat == PTFILE_PTGUI && images.size() <= 0
                            && lenses.size() == 1)
                        {
                            DEBUG_DEBUG("PTGUI special case for first image");
                            // use the first lens
                            DEBUG_ASSERT(link == 0);
                            lenses[link].setRatio(((double)width)/height);
                            lensNr = link;

                        } else if ((int) images.size() <= link) {
                            DEBUG_ERROR("variables must be linked to an image with a lower number" << endl
                                        << "number links: " << link << " images: " << images.size() << endl
                                        << "error on line " << lineNr << ":" << endl
                                        << line);
#ifdef __unix__
                            // reset locale
                            setlocale(LC_NUMERIC,old_locale);
#endif
                            return false;
                        } else {
                            DEBUG_DEBUG("anchored to image " << link);
                            anchorImage = link;
                            // existing lens
                            lensNr = images[anchorImage].getLensNr();
                            DEBUG_DEBUG("using lens nr " << lensNr);
                            map_get(lenses[lensNr].variables,it->first).setLinked(true);
                        }
                    } else if (anchorImage != link) {
                        // conflict, link parameters do not match!
                        DEBUG_ERROR("cannot process images whos variables are linked "
                                    "to different anchor images, on line " << lineNr
                                    << ":\n" << line);
#ifdef __unix__
                        // reset locale
                        setlocale(LC_NUMERIC,old_locale);
#endif
                        return false;
                    }
                    DEBUG_ASSERT(lensNr >= 0);
                    // get variable value of the link target
                    double val = map_get(lenses[lensNr].variables, it->first).getValue();
                    map_get(vars, it->first).setValue(val);
                    it->second.setValue(val);
                } else {
                    DEBUG_DEBUG("anchored to image " << link);
                    // not linked
                    // copy value to image variable.
                    map_get(vars,it->first).setValue(it->second.getValue());
                }
            }
            variables.push_back(vars);


            DEBUG_DEBUG("lensNr after scanning " << lensNr);
            int lensProjInt;
            getParam(lensProjInt, line, "f");
            l.projectionFormat = (Lens::LensProjectionFormat) lensProjInt;

            if (lensNr != -1) {
//                lensNr = images[anchorImage].getLensNr();
                if (l.projectionFormat != lenses[lensNr].projectionFormat) {
                    DEBUG_ERROR("cannot link images with different projections");
#ifdef __unix__
                    // reset locale
                    setlocale(LC_NUMERIC,old_locale);
#endif
                    return false;
                }
            }



            if (lensNr == -1) {
                if (width > height) {
                    l.setRatio(((double)width)/height);
                } else {
                    l.setRatio(((double)height)/width);
                }
                // no links -> create a new lens
                // create a new lens.
                lenses.push_back(l);
                lensNr = lenses.size()-1;
            } else {
                // check if the lens uses landscape as well..
                if (lenses[(unsigned int) lensNr].isLandscape != l.isLandscape) {
                    DEBUG_ERROR("Landscape and portrait images can't share a lens" << endl
                                << "error on script line " << lineNr << ":" << line);
                }
                // check if the ratio is equal
            }

            DEBUG_ASSERT(lensNr >= 0);
            DEBUG_DEBUG("adding image with lens " << lensNr);
            images.push_back(PanoImage(file,width, height, (unsigned int) lensNr));

            ImageOptions opts = images.back().getOptions();
            getParam(opts.featherWidth, line, "u");
            images.back().setOptions(opts);

            state = P_IMAGE;

            break;
        }
        case 'v':
	{
            DEBUG_DEBUG("v line: " << line);
            if (firstOptVecParse) {
                optvec = OptimizeVector(images.size());
		firstOptVecParse = false;
            }
	    std::stringstream optstream;
            optstream << line.substr(1);
            string var;
            while (!(optstream >> std::ws).eof()) {
                optstream >> var;
                if (var.length() < 2) {
                    if (fileformat == PTFILE_PTGUI) {
                        // special case for PTGUI
                        var += "0";
                        break;
                    } else {
                        DEBUG_ERROR("short option read");
                        continue;
                    }
                }
		unsigned int nr = utils::lexical_cast<unsigned int>(var.substr(1));
		DEBUG_ASSERT(nr < optvec.size());
		optvec[nr].insert(var.substr(0,1));
		DEBUG_DEBUG("parsing opt: >" << var << "< : var:" << var[0] << " image:" << nr);
	    }
            break;
	}
        case 'c':
        {
            DEBUG_DEBUG("c line: " << line);
            int t;
            // read control points
            ControlPoint point;
            getParam(point.image1Nr, line, "n");
            getParam(point.image2Nr, line, "N");
            getParam(point.x1, line, "x");
            getParam(point.x2, line, "X");
            getParam(point.y1, line, "y");
            getParam(point.y2, line, "Y");
            getParam(t, line, "t");
            point.mode = (ControlPoint::OptimizeMode) t;
            ctrlPoints.push_back(point);
            state = P_CP;
            break;
        }
        case '#':
            if (line.substr(0,20) == "# ptGui project file") {
                DEBUG_DEBUG("loading PTGui project file");
                // PTGUI
                fileformat = PTFILE_PTGUI;
            }

            if (line.substr(0,41) ==  "# Script file for Panorama Tools stitcher") {
                DEBUG_DEBUG("loading PTAssembler project file");
                // PTAssembler
                fileformat = PTFILE_PTA;
            }

            // PTGui and PTAssember project files:
            // #-imgfile 960 1280 "D:\data\bruno\074-098\087.jpg"
            if (line.substr(0,10) == "#-imgfile ") {

                // arghhh. I like string processing without regexps.
                int b = line.find_first_not_of(" ",9);
                int e = line.find_first_of(" ",b);
                DEBUG_DEBUG(" width:" << line.substr(b,e-b)<<":")
                nextWidth = utils::lexical_cast<int,string>(line.substr(b,e-b));
                DEBUG_DEBUG("next width " << nextWidth);
                b = line.find_first_not_of(" ",e);
                e = line.find_first_of(" ",b);
                DEBUG_DEBUG(" height:" << line.substr(b,e-b)<<":")
                nextHeight = utils::lexical_cast<int, string>(line.substr(b,e-b));
                DEBUG_DEBUG("next height " << nextHeight);
                b = line.find_first_not_of(" \"",e);
                e = line.find_first_of("\"",b);
                nextFilename = line.substr(b,e-b);
                DEBUG_DEBUG("next filename " << nextFilename);
            }

            if (line.substr(0,12) == "#-dummyimage") {
                ptGUIDummyImage = true;
            }

            // parse our special options
            if (line.substr(0,14) == "#hugin_options") {
                DEBUG_DEBUG("parsing special line");
                getParam(options.optimizeReferenceImage, line, "r");
            }
            break;
        default:
            // ignore line..
            break;
        }
    }
#ifdef __unix__
    // reset locale
    setlocale(LC_NUMERIC,old_locale);
#endif
    return true;
}
