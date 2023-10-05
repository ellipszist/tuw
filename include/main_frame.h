#pragma once
#include <vector>
#include <string>
#include "rapidjson/document.h"
#include "component.h"
#include "json_utils.h"
#include "ui.h"

// Main window
class MainFrame {
 private:
    rapidjson::Document m_definition;
    int m_definition_id;
    rapidjson::Document m_config;
    uiWindow* m_mainwin;

    std::vector<Component*> m_components;
    uiGrid* m_grid;
    uiButton* m_run_button;

    void CreateFrame();
    void CreateMenu();
    json_utils::JsonResult CheckDefinition(rapidjson::Document& definition);
    void UpdateConfig();
    void ShowSuccessDialog(const std::string& msg, const std::string& title = "Success");
    void ShowErrorDialog(const std::string& msg, const std::string& title = "Error");
    void JsonLoadFailed(const std::string& msg, rapidjson::Document& definition);

 public:
    explicit MainFrame(const rapidjson::Document& definition =
                           rapidjson::Document(rapidjson::kObjectType),
                       const rapidjson::Document& config =
                           rapidjson::Document(rapidjson::kObjectType));
    void UpdatePanel(int definition_id);
    void OpenURL(int id);
    std::string GetCommand();
    void RunCommand();
    void GetDefinition(rapidjson::Document& json);
    void SaveConfig();
    void Fit();
    void Close() {
        if (m_mainwin == NULL) return;
        uiControlDestroy(uiControl(m_mainwin));
    }
};

void MainFrameDisableDialog();
