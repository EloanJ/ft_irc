#pragma once
#include <iostream>
#include <string>
#include <sys/socket.h>
#include "Channel.hpp"

# define BOT 0
# define HI 1
# define TREE 2
# define COLOR 3
# define TABLE 4

class Bot
{
	private:
		std::string _name;
	public:
		Bot();
		~Bot();
		void	exclamationCommand(Channel *ch, std::string cmd);
		void	executeCommand(Channel *ch, int type);
};
