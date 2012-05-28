/*
 * GameChat.h
 *
 *  Created on: May 27, 2012
 *      Author: 100397561
 */

#ifndef GAMECHAT_H_
#define GAMECHAT_H_

#include <string>
#include <vector>
#include "../util/game_basic_structs.h"

struct GameState;

class GameChat {
public:
	void step(GameState* gs);
	void draw(GameState* gs);
	GameChat();
private:
	/*Represents a coloured message in chat*/
	struct ChatMessage {
		std::string sender, message;
		Colour sender_colour, message_colour;
		ChatMessage(const std::string& sender, const std::string& message,
				const Colour& sender_colour = Colour(255, 255, 255),
				const Colour& message_colour = Colour(255, 255, 255)) :
				sender(sender), message(message), sender_colour(sender_colour), message_colour(
						message_colour) {
		}
	};

	void draw_player_chat(GameState* gs);

	std::vector<ChatMessage> messages;
	bool show_chat;
	float fade_out;
};

#endif /* GAMECHAT_H_ */