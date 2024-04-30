#include "../client.h"

void command_award(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #award [Character Name] [Amount]");
		return;
	}

	const auto& l = CharacterDataRepository::GetWhere(
		*this,
		fmt::format(
			"`name` = '{}'",
			Strings::Escape(sep->arg[1])
		)
	);

	if (l.empty()) {
		c->Message(Chat::White, "Unable to find character %s", Strings::Escape(sep->arg[1]));
		return;
	}

	auto& e = l.front();
	
	DataBucketKey k;
	k.character_id = e.id;
	k.key = "EoM-Award";
	k.value = sep->arg[2];

	c->Message(Chat::White, "Awarded %d EoM to %s.", sep->arg[1], sep->arg[2]);
}
