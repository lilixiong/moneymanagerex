/*******************************************************
Copyright (C) 2014 Gabriele-V

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
********************************************************/

#include "mmSimpleDialogs.h"
#include "constants.h"
#include "mmex.h"
#include "paths.h"
#include "util.h"

#include "model/Model_Account.h"
#include "model/Model_Setting.h"

#include <wx/richtooltip.h>

//mmSingleChoiceDialog
mmSingleChoiceDialog::mmSingleChoiceDialog()
{
}
mmSingleChoiceDialog::mmSingleChoiceDialog(wxWindow *parent, const wxString& message,
    const wxString& caption, const wxArrayString& choices)
{
    wxSingleChoiceDialog::Create(parent, message, caption, choices);
    fix_translation();
}
mmSingleChoiceDialog::mmSingleChoiceDialog(wxWindow* parent, const wxString& message,
    const wxString& caption, const Model_Account::Data_Set& accounts)
{
    wxArrayString choices;
    for (const auto & item : accounts) choices.Add(item.ACCOUNTNAME);
    wxSingleChoiceDialog::Create(parent, message, caption, choices);
    fix_translation();
}
void mmSingleChoiceDialog::fix_translation()
{
    wxButton* ok = (wxButton*)FindWindow(wxID_OK);
    if (ok) ok->SetLabel(_("&OK "));
    wxButton* ca = (wxButton*)FindWindow(wxID_CANCEL);
    if (ca) ca->SetLabel(_("&Cancel "));
}

//  mmDialogComboBoxAutocomplete
mmDialogComboBoxAutocomplete::mmDialogComboBoxAutocomplete()
{
}
mmDialogComboBoxAutocomplete::mmDialogComboBoxAutocomplete(wxWindow *parent, const wxString& message, const wxString& caption,
    const wxString& defaultText, const wxArrayString& choices)
{
    long style = wxCAPTION | wxRESIZE_BORDER | wxCLOSE_BOX;
    Default = defaultText;
    Choices = choices;
    Message = message;
    Create(parent, wxID_STATIC, caption, wxDefaultPosition, wxSize(300, 100), style);
}

bool mmDialogComboBoxAutocomplete::Create(wxWindow* parent, wxWindowID id,
    const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
{
    wxDialog::Create(parent, id, caption, pos, size, style);
    const wxSizerFlags flags = wxSizerFlags().Align(wxALIGN_CENTER | wxALIGN_CENTER_VERTICAL).Border(wxLEFT | wxRIGHT, 15);

    wxBoxSizer* Sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(Sizer);

    Sizer->AddSpacer(10);
    wxStaticText* headerText = new wxStaticText(this, wxID_STATIC, Message);
    Sizer->Add(headerText, flags);
    Sizer->AddSpacer(15);
    cbText_ = new wxComboBox(this, wxID_STATIC, Default, wxDefaultPosition, wxSize(150, -1), Choices);
    cbText_->AutoComplete(Choices);
    Sizer->Add(cbText_, wxSizerFlags(flags).Expand());
    Sizer->AddSpacer(20);
    wxSizer* Button = CreateButtonSizer(wxOK | wxCANCEL);
    Sizer->Add(Button, flags);
    Sizer->AddSpacer(10);

    cbText_->SetFocus();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
    return true;
}

const wxString mmDialogs::selectLanguageDlg(wxWindow *parent, const wxString &langPath, bool verbose)
{
    wxString lang;

    wxArrayString lang_files;
    wxFileName fn(langPath, "");
    fn.AppendDir("en");
    size_t cnt = wxDir::GetAllFiles(fn.GetPath(), &lang_files, "*.mo");

    if (!cnt)
    {
        if (verbose)
        {
            wxString s = wxString::Format("Can't find language files (.mo) at \"%s\"", fn.GetPath());

            wxMessageDialog dlg(parent, s, "Error", wxOK | wxICON_ERROR);
            dlg.ShowModal();
        }

        return lang;
    }

    for (size_t i = 0; i < cnt; ++i)
    {
        wxFileName fname(lang_files[i]);
        lang_files[i] = fname.GetName().Left(1).Upper() + fname.GetName().SubString(1, fname.GetName().Len());
    }

    lang_files.Sort(CaseInsensitiveCmp);
    lang = wxGetSingleChoice("Please choose language", "Languages", lang_files, parent);

    return lang.Lower();
}

/*
locale.AddCatalog(lang) calls wxLogWarning and returns true for corrupted .mo file,
so I should use locale.IsLoaded(lang) also.
*/
const wxString mmDialogs::mmSelectLanguage(mmGUIApp *app, wxWindow* window, bool forced_show_dlg, bool save_setting)
{
    wxString lang;

    const wxString langPath = mmex::getPathShared(mmex::LANG_DIR);
    wxLocale &locale = app->getLocale();

    if (wxDir::Exists(langPath))
    {
        locale.AddCatalogLookupPathPrefix(langPath);
    }
    else
    {
        if (forced_show_dlg)
        {
            wxMessageDialog dlg(window
                , wxString::Format(_("Directory of language files does not exist:\n%s"), langPath)
                , _("Error"), wxOK | wxICON_ERROR);
            dlg.ShowModal();
        }

        return lang;
    }

    if (!forced_show_dlg)
    {
        lang = Model_Setting::instance().GetStringSetting(LANGUAGE_PARAMETER, "english");
        if (!lang.empty() && locale.AddCatalog(lang) && locale.IsLoaded(lang))
        {
            mmOptions::instance().language_ = lang;
            return lang;
        }
    }

    lang = selectLanguageDlg(window, langPath, forced_show_dlg);

    if (save_setting && !lang.empty())
    {
        bool ok = locale.AddCatalog(lang) && locale.IsLoaded(lang);
        if (!ok)  lang.clear(); // bad .mo file
        mmOptions::instance().language_ = lang;
        Model_Setting::instance().Set(LANGUAGE_PARAMETER, lang);
    }

    return lang;
}

/* Error Messages --------------------------------------------------------*/
void mmErrorDialogs::MessageError(wxWindow *parent
    , const wxString &message, const wxString &messageheader)
{
    wxMessageDialog msgDlg(parent, message, messageheader, wxOK | wxICON_ERROR);
    msgDlg.ShowModal();
}

void mmErrorDialogs::MessageWarning(wxWindow *parent
    , const wxString &message, const wxString &messageheader)
{
    wxMessageDialog msgDlg(parent, message, messageheader, wxOK | wxICON_WARNING);
    msgDlg.ShowModal();
}

void mmErrorDialogs::MessageInvalid(wxWindow *parent, const wxString &message)
{
    const wxString& msg = wxString::Format(_("Entry %s is invalid"), message);
    MessageError(parent, msg, _("Invalid Entry"));
}

void mmErrorDialogs::InvalidCategory(wxWindow *button)
{
    wxRichToolTip tip(_("Invalid Category"),
        _("Please use this button for category selection\nor use the 'Split' checkbox for multiple categories.")
        + "\n");
    tip.SetIcon(wxICON_WARNING);
    tip.ShowFor(button);
}

void mmErrorDialogs::InvalidFile(wxWindow *object, bool open)
{
    const wxString& errorHeader = open ? _("Unable to open file.") : _("File name is empty.");
    wxString errorMessage = _("Please select the file for this operation.");

    const wxString errorTips = _("Selection can be made by using Search button.");
    errorMessage = errorMessage + "\n\n" + errorTips + "\n";

    wxRichToolTip tip(errorHeader, errorMessage);
    tip.SetIcon(wxICON_WARNING);
    tip.ShowFor(object);
}

void mmErrorDialogs::InvalidAccount(wxWindow *object, bool transfer)
{
    const wxString& errorHeader = _("Invalid Account");
    wxString errorMessage;
    if (!transfer)
        errorMessage = _("Please select the account for this transaction.");
    else
        errorMessage = _("Please specify which account the transfer is going to.");

    wxString errorTips = _("Selection can be made by using the dropdown button.");
    errorMessage = errorMessage + "\n\n" + errorTips + "\n";

    wxRichToolTip tip(errorHeader, errorMessage);
    tip.SetIcon(wxICON_WARNING);
    tip.ShowFor(object);
}

void mmErrorDialogs::InvalidPayee(wxWindow *object)
{
    const wxString& errorHeader = _("Invalid Payee");
    const wxString& errorMessage = (_("Please type in a new payee,\nor make a selection using the dropdown button.")
        + "\n");
    wxRichToolTip tip(errorHeader, errorMessage);
    tip.SetIcon(wxICON_WARNING);
    tip.ShowFor(object);
}

void mmErrorDialogs::InvalidName(wxTextCtrl *textBox)
{
    const wxString& errorHeader = _("Invalid Name");
    const wxString& errorMessage = (_("Please type in a non empty name.")
        + "\n");
    wxRichToolTip tip(errorHeader, errorMessage);
    tip.SetIcon(wxICON_WARNING);
    tip.ShowFor((wxWindow*)textBox);
}
