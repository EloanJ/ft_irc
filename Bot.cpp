#include "Bot.hpp"

Bot::Bot() : _name("Bob") {}

Bot::~Bot() {}

void Bot::exclamationCommand(Channel *ch, std::string cmd)
{
	std::cout<<GREY<<"BOT CALLED WITH "<<cmd<<std::endl;
	std::string cmds[5] = {"BOT", "HI", "TREE", "COLOR", "TABLE"};
	std::string sub_cmd = cmd.substr(1, cmd.size() - 1);
	std::cout<<GREY<<"BOT CALLED WITH ["<<cmd<<"]\nSUBSTR : ["<<sub_cmd<<"]\n"<<RST<<std::endl;
	for (size_t i = 0; i < 5; i++)
		if (cmds[i] == sub_cmd)
			return this->executeCommand(ch, i);
	return this->executeCommand(ch, -1);
}

void Bot::executeCommand(Channel *ch, int type)
{
	std::cout<<GREY<<"BOT CALLED WITH ["<<type<<"]\nCHANNEL : ["<<ch->getChName()<<"]\n"<<RST<<std::endl;
	std::string ts = ":Bob!Bot@localhost PRIVMSG "+ch->getChName()+" :";
	switch (type)
	{
		case BOT :
			ts += "  .------.\n"
				 " /        \\\n"
				 "|  []  []  |\n"
				 "|    △     |\n"
				 " \\  ----- /\n"
				 "  '------'\n\r\n";
			std::cout<<GREY<<ts<<std::endl<<RST;
			ch->sendToAll(ts, -1);
			break;
		case HI :
			break;
		case TREE :
			break;
		case COLOR :
			break;
		case TABLE :
			ts += "(╯°□°)╯︵ ┻━┻\r\n";
			ch->sendToAll(ts, -1);
		default :
			break;
	}
}