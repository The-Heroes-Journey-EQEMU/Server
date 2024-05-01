#include "../client.h"

void command_disable_seasonal(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;

	if (!c->IsSeasonal()) {
		c->Message(Chat::White, "This character is not a seasonal character, and cannot use this command");
	}

	if (arguments && strcmp(sep->arg[1], "confirm") == 0) {
		c->DisableSeasonal();
	} else {
		c->Message(Chat::White, "Usage: #disable_seasonal confirm - Permanently remove this character from this Season. This CANNOT be reversed.");
		return;
	}
}

void command_seasoninfo(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	
}
