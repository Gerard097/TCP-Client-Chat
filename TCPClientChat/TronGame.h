#pragma once
#ifndef TRON_GAME_H
#define TRON_GAME_H
#include <SFML\Graphics.hpp>
#include <vector>
#include <string>
#include <winsock.h>

class TronGame
{
	enum {P0,P1}ThisPlayer;
	struct Player
	{
		sf::Vector2i position;
		sf::Keyboard::Key direction;
		sf::Color color;
	};
public:
	TronGame();
	~TronGame();
	void Init(SOCKET* serverSocket, bool* playing, std::string* finalmessage, const std::string& color);
	void Play();
private:
	void HandleInputs();
	void ReceiveData();
	void Update();
	void DrawPlayers();
	void DrawBorders();
	void DrawTracks();
	void Draw();
private:
	SOCKET* m_serverSocket;
	Player m_players[2];
	sf::Text m_timer;
	sf::Text m_playerColor;
	sf::Font m_font;
	sf::RenderWindow m_window;
	sf::RectangleShape m_playerTrack;
	sf::CircleShape m_playerShape;
	std::string m_sendData;
	char* m_map;
	std::string* m_finalmessage;
	sf::Vector2u m_mapSize;
	bool* m_playing;
	bool m_endOfGame{ false };
	bool m_gameStarted{ false };
	float m_squareSize{ 4.f };
	char m_buffer[16]{ "" };
};


#endif // !TRON_GAME_H
