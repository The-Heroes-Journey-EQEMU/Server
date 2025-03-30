#include "../client.h"
#include "../object.h"

/*
Skill ID	Skill Name
8	Backstab
10	Bash
16	Disarm
21	Dragon Punch
23	Eagle Strike
26	Flying Kick
30	Kick
38	Round Kick
52	Tiger Claw
74	Frenzy
*/

void command_autoskill(Client *c, const Seperator *sep)
{
    if (!c || !sep) {
        return;
    }

	std::string usage = "Usage: #autoskill [skill id or name] [enable/disable]";

    // Check if we have at least one argument
    if (sep->argnum < 1) {
        c->Message(Chat::Skills, usage.c_str());
        return;
    }

    // Define a map of proper display names to skill IDs
    static std::map<std::string, int> skill_display_map = {
        {"Backstab", 8},
        {"Bash", 10},
        {"Disarm", 16},
        {"Dragon Punch", 21},
        {"Eagle Strike", 23},
        {"Flying Kick", 26},
        {"Kick", 30},
        {"Round Kick", 38},
        {"Tiger Claw", 52},
        {"Frenzy", 74}
    };

    // Create a lowercase lookup map and a reverse ID map
    static std::map<std::string, int> skill_lookup_map;
    static std::map<int, std::string> id_to_name_map;

    if (skill_lookup_map.empty()) {
        // Populate the lookup map with lowercase versions
        for (const auto& pair : skill_display_map) {
            // Add standard version
            std::string lowercase = Strings::ToLower(pair.first);
            skill_lookup_map[lowercase] = pair.second;

            // Add version without spaces
            std::string no_spaces = lowercase;
            Strings::FindReplace(no_spaces, " ", "");
            if (no_spaces != lowercase) {
                skill_lookup_map[no_spaces] = pair.second;
            }

            // Add to reverse map (ID to display name)
            id_to_name_map[pair.second] = pair.first;
        }
    }

    // First determine if the last argument is an enable/disable parameter
    bool has_enable_param = false;
    bool enable = false;
    std::string last_arg = sep->arg[sep->argnum];

    if (last_arg.empty()) {
        c->Message(Chat::Skills, usage.c_str());
        return;
    }

    // Check if last argument is a valid enable/disable parameter
    if (Strings::ToBool(last_arg) ||
        Strings::ToLower(last_arg) == "false" ||
        Strings::ToLower(last_arg) == "off" ||
        Strings::ToLower(last_arg) == "disable" ||
        Strings::ToLower(last_arg) == "disabled" ||
        last_arg == "0") {

        has_enable_param = true;
        enable = Strings::ToBool(last_arg);
    }

    // Now determine the skill name or ID based on whether we have an enable param
    int skill_id = -1;
    std::string skill_name_input;

    if (has_enable_param && sep->argnum < 2) {
        c->Message(Chat::Skills, "Autoskill configuration failed. Missing skill name or ID.");
        return;
    }

    if (has_enable_param) {
        // If we have enable/disable param, everything before it is the skill
        // Combine all arguments except the last one for skill name
        for (int i = 1; i < sep->argnum; i++) {
            skill_name_input += sep->arg[i];
            if (i < sep->argnum - 1) {
                skill_name_input += " ";
            }
        }
    } else {
        // If no enable/disable param, all arguments are the skill name
        for (int i = 1; i <= sep->argnum; i++) {
            skill_name_input += sep->arg[i];
            if (i < sep->argnum) {
                skill_name_input += " ";
            }
        }
    }

    // Check if input is a number (skill ID)
    if (Strings::IsNumber(skill_name_input)) {
        skill_id = Strings::ToInt(skill_name_input);

        // Verify the ID exists in our map
        if (id_to_name_map.find(skill_id) == id_to_name_map.end()) {
            skill_id = -1;  // ID not found
        }
    } else {
        // It's a name, look it up in our map
        std::string skill_name_lower = Strings::ToLower(skill_name_input);
        auto it = skill_lookup_map.find(skill_name_lower);

        if (it != skill_lookup_map.end()) {
            skill_id = it->second;
        }
    }

    // Check if we found a valid skill ID
    if (skill_id == -1) {
        c->Message(Chat::Skills, "Autoskill configuration failed. Invalid skill name or ID.");
        return;
    }

    // Output based on whether we have an enable/disable parameter
    if (has_enable_param) {
        c->Message(Chat::Skills, "Auto-skill %s (%d) %s.",
            id_to_name_map[skill_id].c_str(),
            skill_id,
            enable ? "enabled" : "disabled");

        // Additional implementation for enabling/disabling would go here
    } else {
        c->Message(Chat::Skills, "Auto-skill: %s (ID: %d)",
            id_to_name_map[skill_id].c_str(),
            skill_id);

        // When state data is available, you would output the current state here
    }
}