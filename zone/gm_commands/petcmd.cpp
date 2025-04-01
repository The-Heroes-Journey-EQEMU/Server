#include "../client.h"

void command_petcmd(Client *c, const Seperator *sep) {
    if (!c || !sep) {
        return;
    }

    std::string usage = "Usage: #petcmd [attack, qattack, follow, guard, sit, stop, taunt (on\\off), hold (on\\off), ghold (on\\off), spellhold (on\\off), focus (on\\off), back, regroup (on\\off)] [all, mag, bst, nec, enc, shm, dru, brd, shd].";

    // Check if we have at least one argument
    if (sep->argnum < 1) {
        c->Message(Chat::White, usage.c_str());
        return;
    }

    // Get the command type from first argument
    std::string command_str = sep->arg[1];
    int command_code = -1;
    bool toggle_command = false;

    // Map command string to command code
    if (command_str == "attack") {
        command_code = PET_ATTACK;
    } else if (command_str == "qattack") {
        command_code = PET_QATTACK;
    } else if (command_str == "follow") {
        command_code = PET_FOLLOWME;
    } else if (command_str == "guard") {
        command_code = PET_GUARDHERE;
    } else if (command_str == "sit") {
        command_code = PET_SIT;
    } else if (command_str == "stop") {
        command_code = PET_STOP;
    } else if (command_str == "taunt") {
        command_code = PET_TAUNT;
        toggle_command = true;
    } else if (command_str == "hold") {
        command_code = PET_HOLD;
        toggle_command = true;
    } else if (command_str == "ghold") {
        command_code = PET_GHOLD;
        toggle_command = true;
    } else if (command_str == "spellhold") {
        command_code = PET_SPELLHOLD;
        toggle_command = true;
    } else if (command_str == "focus") {
        command_code = PET_FOCUS;
        toggle_command = true;
    } else if (command_str == "back") {
        command_code = PET_BACKOFF;
    } else if (command_str == "regroup") {
        command_code = PET_REGROUP;
        toggle_command = true;
    }

    if (command_code == -1) {
        c->Message(Chat::White, usage.c_str());
        return;
    }

    // Parse toggle value (on/off) if it's a toggle command
    int target_arg_start = 2;

    if (toggle_command && sep->argnum >= 2) {
        std::string toggle = sep->arg[2];
        if (toggle == "on") {
            // Map to the appropriate ON command code
            if (command_code == PET_SIT) command_code = PET_SITDOWN;
            else if (command_code == PET_STOP) command_code = PET_STOP_ON;
            else if (command_code == PET_TAUNT) command_code = PET_TAUNT_ON;
            else if (command_code == PET_HOLD) command_code = PET_HOLD_ON;
            else if (command_code == PET_GHOLD) command_code = PET_GHOLD_ON;
            else if (command_code == PET_SPELLHOLD) command_code = PET_SPELLHOLD_ON;
            else if (command_code == PET_FOCUS) command_code = PET_FOCUS_ON;
            else if (command_code == PET_REGROUP) command_code = PET_REGROUP_ON;
            target_arg_start = 3;
        } else if (toggle == "off") {
            // Map to the appropriate OFF command code
            if (command_code == PET_SIT) command_code = PET_STANDUP;
            else if (command_code == PET_STOP) command_code = PET_STOP_OFF;
            else if (command_code == PET_TAUNT) command_code = PET_TAUNT_OFF;
            else if (command_code == PET_HOLD) command_code = PET_HOLD_OFF;
            else if (command_code == PET_GHOLD) command_code = PET_GHOLD_OFF;
            else if (command_code == PET_SPELLHOLD) command_code = PET_SPELLHOLD_OFF;
            else if (command_code == PET_FOCUS) command_code = PET_FOCUS_OFF;
            else if (command_code == PET_REGROUP) command_code = PET_REGROUP_OFF;
            target_arg_start = 3;
        }
    }

    // Parse class targets
    std::vector<uint8> class_targets;
    bool all_classes = false;

    // If no target specified, default to all classes
    if (sep->argnum < target_arg_start) {
        all_classes = true;
    } else {
        // Check if the target is "all"
        if (strcmp(sep->arg[target_arg_start], "all") == 0) {
            all_classes = true;
        } else {
            // Process class abbreviations
            bool found_any_class = false;
            for (int i = target_arg_start; i <= sep->argnum; i++) {
                if (strcmp(sep->arg[i], "mag") == 0) {
                    class_targets.push_back(Class::Magician);
                    found_any_class = true;
                } else if (strcmp(sep->arg[i], "bst") == 0) {
                    class_targets.push_back(Class::Beastlord);
                    found_any_class = true;
                } else if (strcmp(sep->arg[i], "nec") == 0) {
                    class_targets.push_back(Class::Necromancer);
                    found_any_class = true;
                } else if (strcmp(sep->arg[i], "enc") == 0) {
                    class_targets.push_back(Class::Enchanter);
                    found_any_class = true;
                } else if (strcmp(sep->arg[i], "shm") == 0) {
                    class_targets.push_back(Class::Shaman);
                    found_any_class = true;
                } else if (strcmp(sep->arg[i], "dru") == 0) {
                    class_targets.push_back(Class::Druid);
                    found_any_class = true;
                } else if (strcmp(sep->arg[i], "brd") == 0) {
                    class_targets.push_back(Class::Bard);
                    found_any_class = true;
                } else if (strcmp(sep->arg[i], "shd") == 0) {
                    class_targets.push_back(Class::ShadowKnight);
                    found_any_class = true;
                }
            }

            // If no valid class identifier was found, default to 'all'
            if (!found_any_class) {
                all_classes = true;
            }
        }
    }

	for (auto pet : c->GetAllPets()) {
		auto pet_class_id = pet->CastToNPC()->GetPetOriginClass();
		if (all_classes || std::find(class_targets.begin(), class_targets.end(), pet_class_id) != class_targets.end()) {
			pet->CastToNPC()->DoPetCommand(command_code, c->GetTarget());
		}
	}
}