#pragma once
#include <wx/wx.h>
#include <wx/filepicker.h>
#pragma once
#include <nlohmann/json.hpp>
#include "Component.h"



//Main window
class MainFrame : public wxFrame
{
private:
    nlohmann::json definition;
    nlohmann::json sub_definition;
    nlohmann::json config;

    std::vector<Component> components;
    wxPanel* mainPanel;

    void LoadDefinition();
    int UpdatePanel(wxPanel* panel);
    void LoadConfig();
    void UpdateConfig();
    void SaveConfig();
    void ShowErrorDialog(std::string msg);
    void ShowSuccessDialog(std::string msg);

public:
    //std::array<std::string, 2> names = { "Frame1", "Frame2" };
    //wxFilePickerCtrl* filePicker;
    //wxFilePickerCtrl* filePicker2;
    //wxDirPickerCtrl* folderPicker;
    //wxChoice* choice;
    MainFrame();
    virtual ~MainFrame();

    void OnClose(wxCloseEvent& event);
    void OpenURL(wxCommandEvent& event);
    void UpdateFrame(wxCommandEvent& event);
    void RunCommand(wxCommandEvent& event);
};
