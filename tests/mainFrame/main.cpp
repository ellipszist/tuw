// Tests for MainFrame.cpp
// Todo: Write more tests

#include <gtest/gtest.h>
#include <wx/app.h>
#include <wx/modalhook.h>
#include "MainFrame.h"

char const * json_file;

// Hook to skip message dialogues
class DialogSkipper : public wxModalDialogHook
{
protected:
    virtual int Enter(wxDialog* dialog)
    {
        if ( wxDynamicCast(dialog, wxMessageDialog) )
        {
            return wxID_CANCEL;
        }
        return wxID_NONE;
    }
    virtual void Exit(wxDialog* dialog){}
};

int main (int argc, char * argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    assert(argc == 2);

    json_file = argv[1];

    // Make dummy app
    wxApp* app = new wxApp();

    // Initialize app
    wxEntryStart(argc, argv);

    // Make hook to skip message dialogues
    DialogSkipper* hook = new DialogSkipper();
    hook->Register();

    return RUN_ALL_TESTS();
}

TEST(MainFrameTest, MakeDefaultMainFrame) {
    MainFrame* mainFrame = new MainFrame();
    EXPECT_EQ(jsonUtils::default_definition(), mainFrame->GetDefinition()["gui"][0]);
}

nlohmann::json GetTestJson(){
    nlohmann::json test_json = jsonUtils::loadJson(json_file);
    EXPECT_NE(nlohmann::json({}), test_json);
    return test_json;
}

TEST(MainFrameTest, InvalidDefinition) {
    nlohmann::json test_json = GetTestJson();
    test_json["gui"][1]["components"][4]["default"] = "number";
    MainFrame* mainFrame = new MainFrame(nlohmann::json(test_json));
    EXPECT_EQ(jsonUtils::default_definition(), mainFrame->GetDefinition()["gui"][0]);
    EXPECT_TRUE(mainFrame->GetDefinition().contains("help"));
}

TEST(MainFrameTest, InvalidHelp) {
    nlohmann::json test_json = GetTestJson();
    test_json["help"][0]["url"] = 1;
    MainFrame* mainFrame = new MainFrame(nlohmann::json(test_json));
    EXPECT_FALSE(mainFrame->GetDefinition().contains("help"));
    EXPECT_EQ(test_json["gui"][0]["components"], mainFrame->GetDefinition()["gui"][0]["components"]);
}

TEST(MainFrameTest, RunCommandSuccess) {
    nlohmann::json test_json = GetTestJson();
    MainFrame* mainFrame = new MainFrame(nlohmann::json(test_json));

    //The json file should not be fixed by app.
    ASSERT_EQ(test_json["help"], mainFrame->GetDefinition()["help"]);
    ASSERT_EQ(test_json["gui"][0]["components"], mainFrame->GetDefinition()["gui"][0]["components"]);

    std::array<std::string, 2> msg = mainFrame->RunCommand();
    EXPECT_STRNE("", msg[0].c_str());
    EXPECT_STREQ("", msg[1].c_str());
}

TEST(MainFrameTest, RunCommandFail) {
    nlohmann::json test_json = GetTestJson();
    test_json["gui"][0]["command"] = "I want errors";
    MainFrame* mainFrame = new MainFrame(nlohmann::json(test_json));

    std::array<std::string, 2> msg = mainFrame->RunCommand();
    std::string errmsg = msg[1];
    EXPECT_STREQ("", msg[0].c_str());
    EXPECT_STREQ("Json format error(Can not make command)", msg[1].c_str());
}

TEST(MainFrameTest, RunCommandFail2) {
    nlohmann::json test_json = GetTestJson();
    test_json["gui"][0] = jsonUtils::default_definition();
    test_json["gui"][0]["command"] = "I'll fail";
    MainFrame* mainFrame = new MainFrame(nlohmann::json(test_json));

    std::array<std::string, 2> msg = mainFrame->RunCommand();
    std::string errmsg = msg[1];
    EXPECT_STREQ("", msg[0].c_str());
    EXPECT_STRNE("", msg[1].c_str());
}

TEST(MainFrameTest, DeleteFrame){
    nlohmann::json test_json = GetTestJson();
    MainFrame* mainFrame = new MainFrame(nlohmann::json(test_json));
    wxDELETE(mainFrame);
}