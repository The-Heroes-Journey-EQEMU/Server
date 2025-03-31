#include "../client.h"

void command_attackmode(Client *c, const Seperator *sep)
{
	if (!c || !sep) {
		return;
	}

	std::string usage = "Usage: #attackmode [ranged or melee].";

	if (sep->argnum == 0) {
		Client::AttackMode current_mode = c->GetAttackMode();
		std::string mode_str = current_mode == Client::RANGED ? "ranged" : "melee";
		c->Message(Chat::White, "Your current attack mode is: %s", mode_str.c_str());
		c->Message(Chat::White, "%s", usage.c_str());
		return;
	}

	if (sep->argnum > 0) {
		std::string arg = sep->arg[1];

		if (Strings::ToLower(arg) == "ranged") {
			c->SetAttackMode(Client::RANGED);
			c->Message(Chat::White, "Attack mode changed to: ranged");
		}
		else if (Strings::ToLower(arg) == "melee") {
			c->SetAttackMode(Client::MELEE);
			c->Message(Chat::White, "Attack mode changed to: melee");
		}
		else {
			c->Message(Chat::White, "Invalid attack mode: %s", arg.c_str());
			c->Message(Chat::White, "%s", usage.c_str());
		}
	}
}