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
	std::string bot[6] = {"  .------.\r\n",
						  " /        \\\r\n",
						  "|  []  []  |\r\n",
						  "|    △     |\r\n",
						 " \\  ----- /\r\n",
						  "  '------'\r\n"};
	int i = rand()%101;
	switch (type)
	{
		case BOT :
			for (size_t i = 0; i < 6; i++)
			{
				ts = ":Bob!Bot@localhost PRIVMSG "+ch->getChName()+" :";
				ts += bot[i];
				ch->sendToAll(ts, -1);
			}
			break;
		case HI :
			ts += "Hi, I'm Bob ヾ(^ ∇ ^).\r\n";
			ch->sendToAll(ts, -1);
			break;
		case TREE :
			ts += "↟↟𓂃 ོ ☼𓂃↟ 𖠰𓂃 ོ𓂃\r\n";
			ch->sendToAll(ts, -1);
			break;
		case COLOR :
			ts += "To use color with IRSSI press ctrl + c and enter a number before yout message [C2Your message] 0->blanc, 1->noir, 2->bleu, 3->vert, 4->rouge clair, 5->rouge, 6->violet, 7->orange, 8->jaune, 9->vert clair, 10->cyan, 11->cyan clair, 12->bleu clair, 13->rose, 14->gris, 15->gris clair\r\n";
			ch->sendToAll(ts, -1);
			break;
		case TABLE :
			if (i % 2)
				ts += "(╯° □ °)╯︵ ┻━┻\r\n";
			else
				ts += "┬─┬ノ(ಠ_ಠノ)\r\n";
			ch->sendToAll(ts, -1);
			break;
		default :
			ts += "I'm sorry, i don't understand humans...\r\n";
			ch->sendToAll(ts, -1);
			break;
	}
}