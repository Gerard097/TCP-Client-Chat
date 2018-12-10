#include "TronGame.h"

TronGame::TronGame()
{

}

TronGame::~TronGame()
{
	if (m_map != nullptr)
		delete[] m_map;
}

void TronGame::Init(SOCKET* serverSocket, bool* playing, std::string* finalmessage, const std::string& color)
{
	m_window.create(sf::VideoMode(820, 640), "Tron", sf::Style::None);
	m_mapSize.x = 204;
	m_mapSize.y = 120;
	m_window.setPosition(sf::Vector2i(0, 0));
	std::string newRow;
	
	m_map = new char[m_mapSize.x * m_mapSize.y]{0};

	if (!m_map)
		exit(1);
	

	m_font.loadFromFile("font.ttf");
	m_playerColor.setFont(m_font);
	m_playerColor.setCharacterSize(32);
	if (color == "rojo") {
		m_playerColor.setFillColor(sf::Color(255, 10, 10));
		ThisPlayer = P0;
	}
	else {
		m_playerColor.setFillColor(sf::Color(10, 10, 255));
		ThisPlayer = P1;
	}
	m_playerColor.setString("Eres el jugador " + color + ".");
	m_playerColor.setPosition(m_window.getSize().x / 2 - m_playerColor.getGlobalBounds().width / 2, 550);

	m_timer.setFont(m_font);
	m_timer.setPosition(sf::Vector2f(m_window.getSize()) * 0.5f + sf::Vector2f(-20,-150));
	m_timer.setCharacterSize(64);
	m_timer.setFillColor(sf::Color::White);

	m_players[P0].color = sf::Color::Red;
	m_players[P1].color = sf::Color::Blue;
	m_players[P0].position = { 1 , int(m_mapSize.y / 2) };
	m_players[P1].position = { int(m_mapSize.x - 2) , int(m_mapSize.y / 2) };
	m_players[P0].direction = sf::Keyboard::Right;
	m_players[P1].direction = sf::Keyboard::Left;

	m_playerTrack.setFillColor(sf::Color::Black);
	m_playerTrack.setSize(sf::Vector2f(m_squareSize,m_squareSize));
	m_playerShape.setRadius(m_squareSize * 0.5f);

	m_finalmessage = finalmessage;
	
	m_playing = playing;
	m_serverSocket = serverSocket;

}

void TronGame::Play()
{
	HandleInputs();
	if(!m_endOfGame) ReceiveData();
	if (m_gameStarted) Update();
	if (!*m_playing) {
		m_window.close();
		return;
	}
	Draw();
}

void TronGame::HandleInputs()
{

	sf::Event evnt;
	while (m_window.pollEvent(evnt)) {

		if (evnt.type == sf::Event::Closed) {
			*m_playing = false;
			return;
		}
		else if (evnt.type == sf::Event::KeyPressed && (m_gameStarted || m_endOfGame)) {
			if (m_endOfGame) {
				if (evnt.key.code == sf::Keyboard::Return)
					*m_playing = false;
				break;
			}
			switch (evnt.key.code)
			{
			case sf::Keyboard::Up:  
				if (!(m_players[ThisPlayer].direction == sf::Keyboard::Down)) {
					m_sendData.push_back('M');
					m_sendData.push_back(evnt.key.code);
					send(*m_serverSocket, m_sendData.data(), m_sendData.length() + 1, NULL);
					m_sendData.clear();
				} break;
			case sf::Keyboard::Down: 
				if (!(m_players[ThisPlayer].direction == sf::Keyboard::Up)) {
					m_sendData.push_back('M');
					m_sendData.push_back(evnt.key.code);
					send(*m_serverSocket, m_sendData.data(), m_sendData.length() + 1, NULL);
					m_sendData.clear();
				} break;
			case sf::Keyboard::Left: 
				if (!(m_players[ThisPlayer].direction == sf::Keyboard::Right)) {
					m_sendData.push_back('M');
					m_sendData.push_back(evnt.key.code);
					send(*m_serverSocket, m_sendData.data(), m_sendData.length() + 1, NULL);
					m_sendData.clear();				
				} break;
			case sf::Keyboard::Right: 
				if (!(m_players[ThisPlayer].direction == sf::Keyboard::Left)) {
					m_sendData.push_back('M');
					m_sendData.push_back(evnt.key.code);
					send(*m_serverSocket, m_sendData.data(), m_sendData.length() + 1, NULL);
					m_sendData.clear();
				} break;
			default:
				break;
			}
		}
	}

}

void TronGame::ReceiveData()
{
	if (m_gameStarted)
		recv(*m_serverSocket, m_buffer, sizeof(m_buffer), NULL);
	else {
		short time;
		recv(*m_serverSocket, (char*)&time, sizeof(time), NULL);
		m_timer.setString(std::to_string(time));
		if (time <= 0) 
			m_gameStarted = true;
	}
	
}

void TronGame::Update()
{
	if (strcmp(m_buffer,"Error") == 0) {
		*m_finalmessage = "Se perdio la conexion con el usuario rival.";
		*m_playing = false;
		return;
	}

	sf::Vector2i lastPosition[2] = { m_players[P0].position, m_players[P1].position };

	if (m_buffer[1] == 71 || m_buffer[1] == 72 || m_buffer[1] == 73 || m_buffer[1] == 74){
		m_players[P0].direction = static_cast<sf::Keyboard::Key>(m_buffer[1]);
	}
	if (m_buffer[2] == 71 || m_buffer[2] == 72 || m_buffer[2] == 73 || m_buffer[2] == 74) {
		m_players[P1].direction = static_cast<sf::Keyboard::Key>(m_buffer[2]);
	}

	for (int i = 0; i < 2; i++) {
		switch (m_players[i].direction)
		{
		case sf::Keyboard::Up: m_players[i].position.y--; break;
		case sf::Keyboard::Down: m_players[i].position.y++; break;
		case sf::Keyboard::Left: m_players[i].position.x--; break;
		case sf::Keyboard::Right: m_players[i].position.x++; break;
		default:
			break;
		}
		if (lastPosition[i].x >= 0 && lastPosition[i].x < (int)m_mapSize.x &&
			lastPosition[i].y >= 0 && lastPosition[i].y < (int)m_mapSize.y) {
			m_map[lastPosition[i].x + lastPosition[i].y * m_mapSize.x] = i + 1;
		}	
	}

	int lose = 0;

	for (int i = 0; i < 2; i++) {
		if (m_players[i].position.x < 1 || m_players[i].position.x >= (int)(m_mapSize.x-1) ||
			m_players[i].position.y < 1 || m_players[i].position.y >= (int)(m_mapSize.y-1)) {
			lose += i + 1;
		}
		else if (m_map[m_players[i].position.x + m_players[i].position.y * m_mapSize.x]) {
			lose += i + 1;
		}
	}
	if (m_players[P0].position == m_players[P1].position)
	{
		lose = 3;
	}
	if (lose) {
		if (lose == 1) {
			ThisPlayer == P0 ? *m_finalmessage = "Mala suerte, perdiste!" : *m_finalmessage = "Buen trabajo, ganaste!";
		}
		else if (lose == 2) {
			ThisPlayer == P0 ? *m_finalmessage = "Buen trabajo, ganaste!" : *m_finalmessage = "Mala suerte, perdiste!";
		}
		else {
			*m_finalmessage = "Ni tu ni el, Empate!";
		}
		m_endOfGame = true;
		m_gameStarted = false;
		m_timer.setFillColor(sf::Color(sf::Color::White));
		m_timer.setCharacterSize(32);
		m_timer.setString(*m_finalmessage + "\n\nPresiona ENTER para regresar al lobby.");
		m_timer.setPosition(30, 300);
		m_sendData = "F";
		send(*m_serverSocket,m_sendData.data(),m_sendData.length() + 1,NULL);
	}

}

void TronGame::DrawPlayers()
{
	sf::Vector2f postion{ 0,0 };
	for (int i = 0; i < 2; i++) {
		m_playerShape.setFillColor(m_players[i].color);
		postion.x = m_players[i].position.x * m_squareSize;
		postion.y = m_players[i].position.y * m_squareSize;
		m_playerShape.setPosition(postion);
		m_window.draw(m_playerShape);
	}
}

void TronGame::DrawBorders()
{
	m_playerTrack.setFillColor(sf::Color::White);

	for (size_t i = 0; i < m_mapSize.x; i++) {
		m_playerTrack.setPosition(i * m_squareSize, m_mapSize.y * m_squareSize);
		m_window.draw(m_playerTrack);
		m_playerTrack.setPosition(i * m_squareSize, 0);
		m_window.draw(m_playerTrack);
	}

	for (size_t i = 0; i < (m_mapSize.y + 1); i++) {
		m_playerTrack.setPosition(0, i * m_squareSize);
		m_window.draw(m_playerTrack);
		m_playerTrack.setPosition((m_mapSize.x - 1) * m_squareSize, i * m_squareSize);
		m_window.draw(m_playerTrack);
	}
}

void TronGame::DrawTracks()
{
	sf::Vector2f position{ 0,0 };
	
	for (size_t i = 0; i < m_mapSize.x * m_mapSize.y; i++) {
		
		if (!m_map[i])
			continue;
		else if (m_map[i] == 1)
			m_playerTrack.setFillColor(sf::Color::Red);
		else
			m_playerTrack.setFillColor(sf::Color::Blue);
		
		int row = int(i / m_mapSize.x);
		int col = i - row * m_mapSize.x;
		m_playerTrack.setPosition(col * m_squareSize, row * m_squareSize);
		m_window.draw(m_playerTrack);
	
	}


}

void TronGame::Draw()
{
	m_window.clear(sf::Color::Black);
	DrawTracks();
	DrawPlayers();
	DrawBorders();
	m_window.draw(m_playerColor);
	if (!m_gameStarted || m_endOfGame)
		m_window.draw(m_timer);
	m_window.display();
}
