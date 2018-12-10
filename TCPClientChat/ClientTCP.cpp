#include "ClientTCP.h"
#include <iostream>
#include <conio.h>

ClientTCP::ClientTCP() {
	
	m_coords[WRITE] = COORD{ 75, 0 };
	m_coords[CHAT] = COORD{ 0, 7 };
	m_noBlockSet.tv.tv_sec = 0;
	m_noBlockSet.tv.tv_usec = 0;
}

bool ClientTCP::Init() {

	SetConsoleTitle(static_cast<LPCSTR>("Cliente"));

	WSADATA wsaData;//----
	ADDRINFO hints, *pAddrResult;
	
	int result;
	
	GetNickname();

	std::cout << "Introduce la";
	SetFontColor(YELLOW);
	std::cout << " ip ";
	SetFontColor(WHITE, false);
	std::cout << "del servidor: ";
	std::string serverIP = "";
	std::getline(std::cin, serverIP);

	result = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (result)
	{
		std::cout << "Error al inicializar los Sockets de Window en su version 2" << std::endl;
		return false;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_socktype = SOCK_STREAM;

	result = GetAddrInfo(static_cast<LPCTSTR>(serverIP.c_str()), "3995", &hints, &pAddrResult); //----

	if (result) {
		std::cout << "al buscar protocolo de comunicaciones en la maquina local" << std::endl;
		WSACleanup();
		return false;
	}

	char szBuffer[10] = "";

	do{

		m_serverSocket = socket(pAddrResult->ai_family, pAddrResult->ai_socktype, pAddrResult->ai_protocol);

		if (m_serverSocket == INVALID_SOCKET) {
			printf("Error en el socket: %ld\n", WSAGetLastError());
			freeaddrinfo(pAddrResult);
			WSACleanup();
			return false;
		}

		result = connect(m_serverSocket, pAddrResult->ai_addr, (int)pAddrResult->ai_addrlen);

		if (result == SOCKET_ERROR) {
			closesocket(m_serverSocket);
			freeaddrinfo(pAddrResult);
			WSACleanup();
			return false;
		}
	
		SendMessageToServer(m_nickname);

		recv(m_serverSocket, szBuffer, sizeof(szBuffer), NULL);

		if (!strcmp(szBuffer,"ACEPTADO")) break;

		else {
			SetFontColor(RED);
			GotoConsoleXY(COORD{ 0,0 });
			std::cout << "Error: ";
			SetFontColor(WHITE);
			std::cout << "el nickname que eligiste ya esta en uso, favor de elgir uno nuevo.";
			_getch();
			GotoConsoleXY(COORD{ 0,0 });
			std::cout << "                                                                          ";
			GetNickname();
			m_serverSocket = INVALID_SOCKET;
		}

	} while (true);

	SetFontColor(GREEN);
	std::cout << "Servidor encontrado.\n";
	SetFontColor(WHITE);
	std::cout << "Escribe '/' para ver la lista de comandos disponibles.";
	GotoConsoleXY(m_coords[TextCoord::WRITE]);
	SetFontColor(AQUA, false);
	std::cout << "Escribe tu texto aqui:" << std::endl;
	m_connected = true;

	freeaddrinfo(pAddrResult);

	return m_connected;
}

void ClientTCP::Start() {

	while (m_connected) {

		if (!m_playing) {
			ManageInputs();
			ProcessMessages();
		}
		else
			ProcessGame();

	}

	Close();

}

void ClientTCP::GotoConsoleXY(COORD coord) {

	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(handle, coord);

}

void ClientTCP::SetFontColor(FontColor color, bool foregroundIntensity){

	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	WORD fontcolor = static_cast<WORD>(color);
	if (foregroundIntensity) fontcolor |= FOREGROUND_INTENSITY;
	SetConsoleTextAttribute(handle, fontcolor);

}

void ClientTCP::ManageInputs() {

	
	if (_kbhit()) {
		SetFontColor(WHITE);
		GotoConsoleXY(m_coords[TextCoord::WRITE]);
		GotoConsoleXY(COORD{ m_coords[TextCoord::WRITE].X - 5 ,
							 m_coords[TextCoord::WRITE].Y + 1 });
		std::cout << "                                    ";

		char t = _getch();
		switch (t)
		{
		case '\b':
		{
			if (m_message.length())
			{
				m_message = m_message.substr(0, m_message.length() - 1);		
			}

		} break;
		case '\r':
		{
			if (m_message.length()) {
				if (m_message[0] != '/')
				{
					SetFontColor(FontColor::WHITE, false);
					GotoConsoleXY(m_coords[TextCoord::CHAT]);
					std::cout << m_nickname << ": " << m_message;
					m_coords[TextCoord::CHAT].Y++;
				}
				SendMessageToServer(m_message);
				if (m_message == "/exit") m_connected = false;
				m_message.clear();
			}

		} break;
		default:
			m_message += t;
			break;
		}

		GotoConsoleXY(COORD{ m_coords[TextCoord::WRITE].X - 5 , 
							 m_coords[TextCoord::WRITE].Y + 1 });
		SetFontColor(FontColor::WHITE);
		std::cout << m_message;
	}
	
}

void ClientTCP::ProcessGame()
{
	std::string message;
	TronGame newGame;
	newGame.Init(&m_serverSocket, &m_playing, &message, m_plyColor);
	while (m_playing) {
		newGame.Play();
	}

	GotoConsoleXY(m_coords[CHAT]);
	SetFontColor(GREEN);
	std::cout << message;
	m_coords[CHAT].Y++;

}

void ClientTCP::ProcessMessages() {

	static char szBuffer[256] = "";
	FD_ZERO(&m_noBlockSet.fdRead);
	FD_SET(m_serverSocket, &m_noBlockSet.fdRead);

	if (select(NULL,&m_noBlockSet.fdRead,NULL,NULL,&m_noBlockSet.tv)) {
		int res = recv(m_serverSocket, szBuffer, sizeof(szBuffer), NULL);
		if (res <= 0) {
			m_connected = false;
			SetFontColor(RED);
			GotoConsoleXY(COORD{ 0,2 });
			std::cout << "Se perdio la conexion con el servidor" << std::endl;
		}
		if (szBuffer[0] == '/') {
			szBuffer[0] = '-';
			szBuffer[1] == 'P' ? SetFontColor(YELLOW, false) : SetFontColor(YELLOW);
			std::string cpy = szBuffer;
			if (cpy.find("comenzando") != std::string::npos) {
				m_playing = true;
				m_plyColor = "rojo";
			}
			else if (cpy.find("empezando") != std::string::npos) {
				m_playing = true;
				m_plyColor = "azul";
			}
		}
		else if (!strcmp(szBuffer, "---Lista de Comandos---")) {
			SetFontColor(GREEN);
		}
		else if (!strcmp(szBuffer, "---Personas Online---")) {
			SetFontColor(GREEN);
		}
		else {
			std::string cpy = szBuffer;
			if (cpy.find(':') == std::string::npos) {
				if (cpy.find("desconectado") != std::string::npos ||
					cpy.find("ingresado") != std::string::npos)
					SetFontColor(GREY);
				else
					SetFontColor(WHITE);
			}
			else
				SetFontColor(WHITE);
		}
			
		GotoConsoleXY(m_coords[TextCoord::CHAT]);
		m_coords[TextCoord::CHAT].Y++;
		std::cout << szBuffer;
	}

}

void ClientTCP::GetNickname()
{
	SetFontColor(WHITE);
	GotoConsoleXY(COORD{ 0,0 });
	do {
		SetFontColor(WHITE, false);
		std::cout << "Introduce tu ";
		SetFontColor(YELLOW);
		std::cout << "nickname ";
		SetFontColor(WHITE, false);
		std::cout << "(max 10 char y sin espacios) : ";
		std::getline(std::cin, m_nickname);
		for (auto& c : m_nickname) {
			if (!isalnum(c)) {
				SetFontColor(RED);
				std::cout << "Tu nombre solo puede contener letras, numeros y estar sin espacios." << std::endl;
				m_nickname.clear();
				break;
			}
		}
	} while (!m_nickname.length());

	do {
		size_t space = m_nickname.find(' ');
		if (space != std::string::npos)
			m_nickname.erase(m_nickname.begin() + space);
		else
			break;
	} while (true);

	if (m_nickname.length() > 10)
		m_nickname = m_nickname.substr(0, 10);

	
}

void ClientTCP::SendMessageToServer(const std::string& message) {

	send(m_serverSocket, message.c_str(), message.length() + 1, NULL);
		
}

void ClientTCP::Close() {
	SetFontColor(AQUA);
	GotoConsoleXY(m_coords[TextCoord::CHAT]);
	std::cout << "Sesion finalizada.";
	m_coords[TextCoord::CHAT].Y++;
	GotoConsoleXY(m_coords[TextCoord::CHAT]);
	std::cout << "presione ENTER para salir...";
#ifdef max
#undef max
#endif
	std::cin.clear();
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	closesocket(m_serverSocket);
	WSACleanup();

}

ClientTCP::~ClientTCP() {



}