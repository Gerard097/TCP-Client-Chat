#pragma once
#ifndef CLIENT_TCP_H
#define CLIENT_TCP_H

#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "TronGame.h"

class ClientTCP {
private:
	enum TextCoord { CHAT, WRITE };
	enum FontColor { GREEN = 0x0002, RED = 0x0004, YELLOW = 0x0006 , PURPLE = 0x0005, WHITE = 0x000F,
					 BLUE = 0x0001, GREY = 0x0008, WHITEL = 0x0007, INVERTWB = 0x00F0, AQUA = 0x0003};
	struct NoBlockSet {
		fd_set fdRead;
		timeval tv;
	};
public:
	ClientTCP();
	~ClientTCP();
	bool Init();
	void Start();
private:
	void GotoConsoleXY(COORD);
	void SetFontColor(FontColor,bool foregroundIntensity = true);
	void ManageInputs();
	void ProcessGame();
	void ProcessMessages();
	void GetNickname();
	void SendMessageToServer(const std::string& message);
	void Close();
private:
	COORD m_coords[2];
	NoBlockSet m_noBlockSet;
	std::string m_nickname{ "" };
	std::string m_message{ "" };
	std::string m_plyColor{""};
	SOCKET m_serverSocket{ INVALID_SOCKET };
	bool m_connected{ false };
	bool m_playing{ false };
};


#endif // !CLIENT_TCP_H