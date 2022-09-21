#include "JsonUtils.h"

namespace jsonUtils {
    nlohmann::json loadJson(std::string file) {
        std::ifstream istream(file);
        nlohmann::json json;
        if (!istream) {
            return nlohmann::json({});
        }
        try {
            istream >> json;
            istream.close();
        }
        catch (...) {
            json = nlohmann::json({});
        }
        return json;
    }

    bool saveJson(nlohmann::json json, std::string file) {
        std::ofstream ostream(file);

        if (!ostream) {
            return false;
        }
        ostream << std::setw(4) << json << std::endl;
        ostream.close();
        return true;
    }

    std::string GetLabel(std::string label, std::string key) {
        std::string msg = "['" + key + "']";
        if (label != "") {
            msg = "['" + label + "']" + msg;
        }
        return msg;
    }
    void checkContain(nlohmann::json j, std::string key, std::string label = "") {
        if (!j.contains(key)) {
            std::string msg = GetLabel(label, key) + " not found.";
            throw std::runtime_error(msg);
        }
    }

    enum class JsonType {
        STRING,
        NUMBER,
        BOOLEAN,
        ARRAY
    };

    void raise(std::string msg) {
        throw std::runtime_error(msg);
    }

    void checkJsonType(nlohmann::json j, std::string key, JsonType type, std::string label="", bool canSkip=false) {
        if (canSkip && !j.contains(key)) {
            return;
        }
        checkContain(j, key, label);

        bool valid = false;
        std::string type_name;
        switch (type) {
        case JsonType::STRING:
            valid = j[key].is_string();
            type_name = "a string";
            break;
        case JsonType::NUMBER:
            valid = j[key].is_number();
            type_name = "an int";
            break;
        case JsonType::BOOLEAN:
            valid = j[key].is_boolean();
            type_name = "a boolean";
            break;
        case JsonType::ARRAY:
            valid = j[key].is_array();
            type_name = "an array";
            break;
        }
        if (!valid) {
            raise(GetLabel(label, key) + " should be " + type_name + ".");
        }
    }


    //get default definition of gui
    nlohmann::json default_definition() {
        nlohmann::json def =
        {
                {"label", "Default GUI"},
    #ifdef _WIN32
                {"command", {"dir"} },
                {"button", "run 'dir'"},
    #else
                {"command", {"ls"} },
                {"button", "run 'ls'"},
    #endif
                {"components",nlohmann::json::array({})}
        };
        return def;
    }

    void checkItemsValues(nlohmann::json& c) {
        std::string label = c["label"];
        if (!c.contains("items")) {
            if (c.contains("item")) {
                c["items"] = c["item"];
            }
            else {
                raise(GetLabel(label, "items") + " not found.");
            }
        }
        checkJsonType(c, "items", JsonType::ARRAY, label);
        if (!c.contains("values") && c.contains("value")) {
            c["values"] = c["value"];
        }
        std::string key = "values";
        checkJsonType(c, key, JsonType::ARRAY, label, true);
        if (c.contains(key)) {
            if (c[key].size() != c["items"].size()) {
                raise(GetLabel(label, key) + " and " + GetLabel(label, "items") + " should have the same size.");
            }
        }
    }
    
    std::vector<std::string> split(const std::string& s, const char delimiter)
    {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        bool store = true;
        while (std::getline(tokenStream, token, delimiter))
        {
            if (store) {
                tokens.push_back(token);
                store = false;
            }
            else {
                store = true;
            }
        }
        return tokens;
    }

    void checkDefinition(nlohmann::json& definition) {
        checkJsonType(definition, "gui", JsonType::ARRAY);
        for (nlohmann::json& sub_d : definition["gui"]){
            checkSubDefinition(sub_d);
        }
    }

    const std::string COMMAND = "command";
    const std::string COMPONENTS = "components";

    void checkSubDefinition(nlohmann::json& sub_definition) {
        //check is_string
        checkJsonType(sub_definition, "label", JsonType::STRING);
        checkJsonType(sub_definition, "button", JsonType::STRING, "", true);
        checkJsonType(sub_definition, "window_name", JsonType::STRING, "", true);

        checkContain(sub_definition, COMMAND, "");
        if (sub_definition[COMMAND].is_string()) {
            sub_definition[COMMAND] = split(sub_definition[COMMAND], '%');
        }
        checkJsonType(sub_definition, COMMAND, JsonType::ARRAY);

        //check is_boolean
        checkJsonType(sub_definition, "show_last_line", JsonType::BOOLEAN, "", true);

        //check is_array
        checkJsonType(sub_definition, COMPONENTS, JsonType::ARRAY);

        //check components
        std::vector<std::string> subkeys = {};
        std::string label;
        std::string type;
        for (nlohmann::json& c : sub_definition[COMPONENTS]) {
            //check if type and label exist
            checkJsonType(c, "type", JsonType::STRING, COMPONENTS);
            checkJsonType(c, "label", JsonType::STRING, COMPONENTS);

            label = c["label"];
            type = c["type"];
            if (type == "file") {
                checkJsonType(c, "extention", JsonType::STRING, label, true);
            }
            else if (type == "choice") {
                checkItemsValues(c);
                subkeys = { "width", "default" };
                for (std::string key : subkeys) {
                    checkJsonType(c, key, JsonType::NUMBER, label, true);
                }
            }
            else if (type == "check") {
                checkJsonType(c, "value", JsonType::STRING, label, true);
            }
            else if (type == "checks" || type == "check_array") {
                checkItemsValues(c);
                std::string key = "default";
                checkJsonType(c, key, JsonType::ARRAY, label, true);
                if (c.contains(key) && c[key].size() != c["items"].size()) {
                    raise(GetLabel(label, key) + " and " + GetLabel(label, "items") + " should have the same size.");
                }
            }
            else if (type == "text" || type == "text_box") {
                checkJsonType(c, "default", JsonType::STRING, label, true);
            }
            checkJsonType(c, "add_quotes", JsonType::BOOLEAN, label, true);
            checkJsonType(c, "empty_message", JsonType::STRING, label, true);
        }
    }

    void checkHelpURLs(nlohmann::json& definition) {
        checkJsonType(definition, "help", JsonType::ARRAY);
        std::vector<std::string> keys = { "type", "label", "url" };
        for (nlohmann::json h : definition["help"]) {
            for (std::string key : keys) {
                checkJsonType(h, key, JsonType::STRING);
            }
        }
    }
}
