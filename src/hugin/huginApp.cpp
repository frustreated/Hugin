// -*- c-basic-offset: 4 -*-

/** @file huginApp.cpp
 *
 *  @brief implementation of huginApp Class
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

#include <config.h>

#include "panoinc_WX.h"

#include "panoinc.h"


#include "hugin/config_defaults.h"
#include "hugin/huginApp.h"
#include "hugin/PanoPanel.h"

#include <tiffio.h>


// utility functions
bool str2double(wxString s, double & d)
{
    if (!utils::stringToDouble(std::string(s.mb_str()), d)) {
        wxLogError(_("Value must be numeric."));
        return false;
    }
    return true;
}

#ifdef __WXMAC__
wxString MacGetPathTOBundledResourceFile(CFStringRef filename)
{
    wxString theResult = wxT("");

    CFBundleRef mainbundle = CFBundleGetMainBundle();
    if(mainbundle == NULL)
    {
        DEBUG_INFO("Mac: Not bundled");
    }
    else
    {
        CFURLRef XRCurl = CFBundleCopyResourceURL(mainbundle, filename, NULL, NULL);
        if(XRCurl == NULL)
        {
            DEBUG_INFO("Mac: Cannot locate the file in bundle.");
        }
        else
        {
            CFStringRef pathInCFString = CFURLCopyFileSystemPath(XRCurl, kCFURLPOSIXPathStyle);
            if(pathInCFString == NULL)
            {
                DEBUG_INFO("Mac: Failed to get URL in CFString");
            }
            else
            {
                CFRetain( pathInCFString );
                theResult = wxMacCFStringHolder(pathInCFString).AsString(wxLocale::GetSystemEncoding());
                DEBUG_INFO("Mac: returned the resource file's path in the application bundle");
            }
            CFRelease( XRCurl );
        }
    }
    return theResult;
}

wxString MacGetPathTOBundledExecutableFile(CFStringRef filename)
{
    wxString theResult = wxT("");

    CFBundleRef mainbundle = CFBundleGetMainBundle();
    if(mainbundle == NULL)
    {
        DEBUG_INFO("Mac: Not bundled");
    }
    else
    {
        CFURLRef PTOurl = CFBundleCopyAuxiliaryExecutableURL(mainbundle, filename);
        if(PTOurl == NULL)
        {
            DEBUG_INFO("Mac: Cannot locate the file in the bundle.");
        }
        else
        {
            CFURLRef PTOAbsURL = CFURLCopyAbsoluteURL( PTOurl );
            if(PTOAbsURL == NULL)
            {
                DEBUG_INFO("Mac: Cannot convert the file path to abosolute");
            }
            else
            {
                CFStringRef pathInCFString = CFURLCopyFileSystemPath(PTOAbsURL, kCFURLPOSIXPathStyle);
                if(pathInCFString == NULL)
                {
                    DEBUG_INFO("Mac: Failed to get URL in CFString");
                }
                else
                {
                    CFRetain( pathInCFString );
                    theResult =  wxMacCFStringHolder(pathInCFString).AsString(wxLocale::GetSystemEncoding());
                    DEBUG_INFO("Mac: returned the executable's full path in the application bundle");
                }
                CFRelease( PTOAbsURL );
            }
            CFRelease( PTOurl );
        }
    }
    return theResult;
}
#endif


// make wxwindows use this class as the main application
IMPLEMENT_APP(huginApp)

huginApp::huginApp()
{
    DEBUG_TRACE("ctor");
    m_this=this;
}

huginApp::~huginApp()
{
    DEBUG_TRACE("dtor");
    // delete temporary dir
//    if (!wxRmdir(m_workDir)) {
//        DEBUG_ERROR("Could not remove temporary directory");
//    }

//    delete frame;
    DEBUG_TRACE("dtor end");
}

bool huginApp::OnInit()
{
    DEBUG_TRACE("=========================== huginApp::OnInit() begin ===================");
    SetAppName(wxT("hugin"));

    wxString m_huginPath;
    wxFileName::SplitPath( argv[0], &m_huginPath, NULL, NULL );

    // DEBUG_INFO( GetAppName().c_str() )
    // DEBUG_INFO( wxFileName::GetCwd().c_str() )
    // DEBUG_INFO( wxFileName::GetHomeDir().c_str() )
    DEBUG_INFO( "hugin path:" << m_huginPath.mb_str() )


    // here goes and comes configuration
    wxConfigBase * config = wxConfigBase::Get();

    config->SetRecordDefaults(TRUE);

    if ( config->IsRecordingDefaults() ) {
      char e_dbg[128] = "writes in config: ";
      sprintf ( e_dbg ,"%s %d\n", e_dbg, config->GetNumberOfEntries() );
      DEBUG_INFO(e_dbg);
    }
    config->Flush();

    // initialize i18n
    int localeID = config->Read(wxT("language"), (long) HUGIN_LANGUAGE);
	DEBUG_TRACE("localeID: " << localeID);
    {
	bool bLInit;
	bLInit = locale.Init(localeID);
	if (bLInit) {
	  DEBUG_TRACE("locale init OK");
	  DEBUG_TRACE("System Locale: " << locale.GetSysName().mb_str())
	  DEBUG_TRACE("Canonical Locale: " << locale.GetCanonicalName().mb_str())
	} else {
	  DEBUG_TRACE("locale init failed");
	}
	}
	

    // add local Paths
    locale.AddCatalogLookupPathPrefix(m_huginPath + wxT("/locale"));
    locale.AddCatalogLookupPathPrefix(wxT(INSTALL_LOCALE_DIR));
    DEBUG_INFO("add locale path: " << INSTALL_LOCALE_DIR)
    // add path from config file
    if (config->HasEntry(wxT("locale_path"))){
        locale.AddCatalogLookupPathPrefix(  config->Read(wxT("locale_path")).c_str() );
    }

#ifdef __WXMAC__
    wxString thePath = MacGetPathTOBundledResourceFile(CFSTR("locale"));
    if(thePath != wxT(""))
        locale.AddCatalogLookupPathPrefix(thePath);
#endif

    // set the name of locale recource to look for
    locale.AddCatalog(wxT("hugin"));

    // initialize image handlers
    wxInitAllImageHandlers();

    // Initialize all the XRC handlers.
    wxXmlResource::Get()->InitAllHandlers();

    // load all XRC files.
#ifdef _INCLUDE_UI_RESOURCES
    InitXmlResource();
#else
    // try local xrc files first
    wxString xrcPrefix;
    // testing for xrc file location
    if ( wxFile::Exists(m_huginPath + wxT("/xrc/main_frame.xrc")) ) {
        DEBUG_INFO("using local xrc files");
        // wxString currentDir = wxFileName::GetCwd();
        xrcPrefix = m_huginPath + wxT("/xrc/");
    } else if ( wxFile::Exists((wxString)wxT(INSTALL_XRC_DIR) + wxT("/main_frame.xrc")) ) {
        DEBUG_INFO("using installed xrc files");
        xrcPrefix = (wxString)wxT(INSTALL_XRC_DIR) + wxT("/");
//    } else if ( wxFile::Exists("/usr/local/share/hugin/xrc/main_frame.xrc") ) {
//        DEBUG_INFO("using installed xrc files in standard path")
//        xrcPrefix = "/usr/local/share/hugin/xrc/";
    } else {
        DEBUG_INFO("using xrc prefix from config")
        xrcPrefix = config->Read(wxT("xrc_path")) + wxT("/");
    }

#ifdef __WXMAC__
    wxString thePath2 = MacGetPathTOBundledResourceFile(CFSTR("xrc"));
    if( thePath2 != wxT(""))
        xrcPrefix = thePath2 + wxT("/");
#endif

    wxXmlResource::Get()->Load(xrcPrefix + wxT("image_center.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("nona_panel.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("ptstitcher_panel.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("cp_list_frame.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("preview_frame.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("run_optimizer_frame.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("edit_script_dialog.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("run_stitcher_frame.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("anchor_orientation.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("main_menu.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("main_tool.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("edit_text.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("about.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("help.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("keyboard_help.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("pref_dialog.xrc"));
#ifdef USE_WX253
    wxXmlResource::Get()->Load(xrcPrefix + wxT("cp_editor_panel-2.5.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("images_panel-2.5.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("lens_panel-2.5.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("main_frame-2.5.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("optimize_panel-2.5.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("pano_panel-2.5.xrc"));
#else
    wxXmlResource::Get()->Load(xrcPrefix + wxT("cp_editor_panel.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("images_panel.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("lens_panel.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("main_frame.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("optimize_panel.xrc"));
    wxXmlResource::Get()->Load(xrcPrefix + wxT("pano_panel.xrc"));
#endif

#endif

#ifdef __WXMAC__
    // If hugin is starting with file opening AppleEvent, MacOpenFile will be called on first wxYield().
    // Those need to be initialised before first call of Yield which happens in Mainframe constructor.
    m_macInitDone=false;
    m_macOpenFileOnStart=false;
#endif
    // create main frame
    frame = new MainFrame(NULL, pano);
    SetTopWindow(frame);

    wxString cwd = wxFileName::GetCwd();
    config->Write( wxT("startDir"), cwd );

    m_workDir = config->Read(wxT("tempDir"),wxT(""));
    // FIXME, make secure against some symlink attacks
    // get a temp dir
#ifdef __WXMSW__
    DEBUG_DEBUG("figuring out windows temp dir");
    if (m_workDir == wxT("")) {
        /* added by Yili Zhao */
        wxChar buffer[MAX_PATH];
        GetTempPath(MAX_PATH, buffer);
        m_workDir = buffer;
    }
#else
    DEBUG_DEBUG("on unix or mac");
    if (m_workDir == wxT("")) {
        // try to read environment variable
        if (!wxGetEnv(wxT("TMPDIR"), &m_workDir)) {
            // still no tempdir, use /tmp
            m_workDir = wxT("/tmp");
        }
    }
#endif

    if (!wxFileName::DirExists(m_workDir)) {
        DEBUG_DEBUG("creating temp dir: " << m_workDir.mb_str());
        if (!wxMkdir(m_workDir)) {
            DEBUG_ERROR("Tempdir could not be created: " << m_workDir.mb_str());
        }
    }
    if (!wxSetWorkingDirectory(m_workDir)) {
        DEBUG_ERROR("could not change to temp. dir: " << m_workDir.mb_str());
    }
    DEBUG_DEBUG("using temp dir: " << m_workDir.mb_str());

    // show the frame.
    frame->Show(TRUE);

    // restore layout
    frame->RestoreLayoutOnNextResize();

    // setup main frame size, after it has been created.
    bool maximized = config->Read(wxT("/MainFrame/maximized"), 0l) != 0;
    if (maximized) {
        frame->Maximize();
        // explicitly layout after maximize
    } else {
        //size
        int w = config->Read(wxT("/MainFrame/width"),-1l);
        int h = config->Read(wxT("/MainFrame/height"),-1l);
        if (w >0) {
            frame->SetClientSize(w,h);
        } else {
            frame->Fit();
        }
        //position
        int x = config->Read(wxT("/MainFrame/positionX"),-1l);
        int y = config->Read(wxT("/MainFrame/positionY"),-1l);
        if ( y > 0) {
            frame->Move(x, y);
        } else {
            frame->Move(0, 44);
        }
    }



    // TODO: check if we need to load images.
    if (argc == 2) {
        wxString filename(argv[1]);
        if (! wxIsAbsolutePath(filename)) {
            filename.Prepend(wxFileName::GetPathSeparator());
            filename.Prepend(cwd);
        }
        frame->LoadProjectFile(filename);
    }
#ifdef __WXMAC__
    m_macInitDone = true;
    if(m_macOpenFileOnStart) {frame->LoadProjectFile(m_macFileNameToOpenOnStart);}
    m_macOpenFileOnStart = false;
#endif

    //load tip startup preferences (tips will be started after splash terminates)
	int nValue = config->Read(wxT("/MainFrame/ShowStartTip"), 1l);
		
	//show tips if needed now
	if(nValue > 0)
	{
		wxCommandEvent dummy;
		frame->OnTipOfDay(dummy);
	}

    // suppress tiff warnings
    TIFFSetWarningHandler(0);

    DEBUG_TRACE("=========================== huginApp::OnInit() end ===================");
    return true;
}

int huginApp::OnExit()
{
    DEBUG_TRACE("");
    return wxApp::OnExit();
}

huginApp * huginApp::Get()
{
    if (m_this) {
        return m_this;
    } else {
        DEBUG_FATAL("huginApp not yet created");
        DEBUG_ASSERT(m_this);
        return 0;
    }
}

#ifdef __WXMAC__
void huginApp::MacOpenFile(const wxString &fileName)
{
    if(!m_macInitDone)
    {
        m_macOpenFileOnStart=true;
        m_macFileNameToOpenOnStart = fileName;
        return;
    }

    if(frame) frame->MacOnOpenFile(fileName);
}
#endif

huginApp * huginApp::m_this = 0;
