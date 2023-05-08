#include <iostream>
#include <cstdlib>
#include <vector>
#include "SFML/Graphics.hpp"
#include <SFML/Audio.hpp>

/*  
	����������: W - �����, S - ����, A - �����, D - ������.

	Escape - �����, ���������� ����������� ����� � ����. 

	������� ����� ���� �� Enter
*/

const int FIELD_CELL_TYPE_WALL = -2; // �������� ��� ��������
const int FIELD_CELL_TYPE_APPLE = -1; // �������� ��� ������
const int FIELD_CELL_TYPE_NONE = 0; // ������� ��������� ������ ���� ��� ��������
const int SNAKE_DIRECTION_UP = 0; // ����������� �����
const int SNAKE_DIRECTION_RIGHT = 1; // ����������� ������
const int SNAKE_DIRECTION_DOWN = 2; // ���������� ����
const int SNAKE_DIRECTION_LEFT = 3; // ����������� �����

const int size_x = 35; // ������
const int size_y = 25; // ������
const int size_cell = 32; // ������ ������
const int window_width = size_x * size_cell; // ������ ���� � ������
const int window_height = size_y * size_cell; // ������ ���� � ������

int field[size_y][size_x];// ���� ��� �������� ���������� ��� ��������� � ������
int snake_pos_x = (size_x / 2); // ������� ������ �� x
int snake_pos_y = (size_y / 2); // ������� ������ �� y
int snake_length = 3; // ����� ������ � ������ ������
int snake_direction = SNAKE_DIRECTION_RIGHT; // �������� ������ ���������� �������� �� WASD ��� ���
int score = 0; // ���� ����������
int current_menu_item_index = 0; // ������� ����� ������ ����, ���������� ���������� ������ �����

bool game_over = false;
bool game_paused = true;

sf::Texture snake_texture; // �������� ����
sf::Sprite snake;
sf::Texture none_texture; // �������� ������ �����
sf::Sprite none;
sf::Texture apple_texture; // �������� ������
sf::Sprite apple;
sf::Texture wall_texture; // �������� �����
sf::Sprite wall;

sf::SoundBuffer sb_ate_apple; // ����������� ����� �������� ������
sf::Sound sound_ate_apple;
sf::SoundBuffer sb_died_the_wall; // ����������� ����� ���������� �� ������
sf::Sound sound_died_the_wall;
sf::SoundBuffer sb_died_himself; // ����������� ����� ���������� �� ����
sf::Sound sound_died_himself;

sf::Font font_score; // ����
sf::Text text_score; // ����
sf::Font font_gameover; // ����� ����
sf::Text text_gameover; // ����� ����

sf::Font font_menu; // ����
std::vector<sf::Text> text_menu_items; // ������ ������ ��� �������� ������ � �������� "resume", "start new game", "quit"
std::vector<std::string> menu_items = { "Resume", "Quit" };


void init_game() { // ��� ���� ����� ��� ������� �������� �� ���������� 10 ��� � �������, ������� �� ������������� ��� �� �������

	snake_texture.loadFromFile("images/snake.png"); // �������� ����
	snake.setTexture(snake_texture);

	none_texture.loadFromFile("images/none.png"); // �������� ������ �����
	none.setTexture(none_texture);

	apple_texture.loadFromFile("images/apple.png"); // �������� ������
	apple.setTexture(apple_texture);

	wall_texture.loadFromFile("images/wall.png"); // �������� �����
	wall.setTexture(wall_texture);

	sb_ate_apple.loadFromFile("sounds/collect_point.wav"); // ���� ��������� ������
	sound_ate_apple.setBuffer(sb_ate_apple);

	sb_died_the_wall.loadFromFile("sounds/explosion_01.wav"); // ���� ���������� �� ������
	sound_died_the_wall.setBuffer(sb_died_the_wall);

	sb_died_himself.loadFromFile("sounds/explosion_00.wav"); // ���� ���������� �� ����
	sound_died_himself.setBuffer(sb_died_himself);

	font_score.loadFromFile("fonts/Quinquefive-ALoRM.ttf"); // ����� �����
	text_score.setFont(font_score); 

	font_gameover.loadFromFile("fonts/Quinquefive-ALoRM.ttf"); // ����� GAME OVER
	text_gameover.setFont(font_gameover);

	text_gameover.setString("GAME OVER"); // �������
	text_gameover.setCharacterSize(90); // ������
	text_gameover.setFillColor(sf::Color::Red);// ����
	text_gameover.setPosition(75, 350); // ������� �� ������

	font_menu.loadFromFile("fonts/Quinquefive-ALoRM.ttf"); // ����� ����
	for (int i = 0; i < menu_items.size(); ++i) {
		text_menu_items.emplace_back(sf::Text());
		text_menu_items.back().setString(menu_items.at(i));
		text_menu_items.back().setFont(font_menu);
		text_menu_items.back().setCharacterSize(40);
	}
}

int get_random_empty_cell() { // ������� ��� ������ ������ ������. int ������ ��� �� ���������� ��������������� ����� ������ ������ ������� � ����
	int empty_cell_count = 0;

	for (int j = 0; j < size_y; ++j) {
		for (int i = 0; i < size_x; ++i) {
			if (field[j][i] == FIELD_CELL_TYPE_NONE) { // ���� ������� ������ ������, �� �� ������ �������(��������������)
				empty_cell_count++;
			}
		}
	}
	int target_empty_cell_index = std::rand() % empty_cell_count; // ����� ��������� ����� �� ���������� ������ ������ � �������� ����� ������� ������
	int empty_cell_index = 0;

	for (int j = 0; j < size_y; ++j) { // ����������� ��������� ������ � ���������� � ����������
		for (int i = 0; i < size_x; ++i) {
			if (field[j][i] == FIELD_CELL_TYPE_NONE) {
				if (empty_cell_index == target_empty_cell_index) { // ���� �� ����� �� ������� ������ ��,
					return j * size_x + i;// ���������� ���������� � ����� ������������
				}
				empty_cell_index++;
			}
		}
	}
	return -1;
}

void add_apple() { // ������� ��� ���������� ������ �� ����������� ������ ������
	int apple_pos = get_random_empty_cell();
	if (apple_pos != -1) {
		field[apple_pos / size_x][apple_pos % size_x] = FIELD_CELL_TYPE_APPLE;
	}
	field[apple_pos / size_x][apple_pos % size_x] = FIELD_CELL_TYPE_APPLE;
}

void clear_field() { // ������� ����� ������ ��� � ���������
	for (int j = 0; j < size_y; ++j) {
		for (int i = 0; i < size_x; ++i) {
			field[j][i] = FIELD_CELL_TYPE_NONE;
		}
	}
	for (int i = 0; i < snake_length; i++) { // ����������� ����� ������ ����� ����� ������
		field[snake_pos_y][snake_pos_x - i] = snake_length - i;
	}
	for (int i = 0; i < size_y; i++) { // ������� ���� ��� ��������� ������� ����
		if (i < 8 || size_y - i - 1 < 8) { // 8 ����
			field[i][0] = FIELD_CELL_TYPE_WALL; // ������ ������ ����������
			field[i][size_x - 1] = FIELD_CELL_TYPE_WALL;
		}
	}
	for (int i = 0; i < size_x; i++) { // ������� ���� ��� ��������� ����
		if (i < 8 || size_x - i - 1 < 8) { // 8 ���� 
			field[0][i] = FIELD_CELL_TYPE_WALL; // ������ ������ ����������
			field[size_y - 1][i] = FIELD_CELL_TYPE_WALL;
		}
	}
	add_apple();
}

void draw_field(sf::RenderWindow & window) { // ��������� �������

	for (int j = 0; j < size_y; j++) {
		for (int i = 0; i < size_x; i++) {
			switch (field[j][i]) { 
			case FIELD_CELL_TYPE_NONE : // ���� � ������ ������, �� �� ������� ����������� ������ ������
				none.setPosition(float(i * size_cell), float(j * size_cell)); // ��������� ������ ������ ����������� ���:
				window.draw(none);
				break;
			case FIELD_CELL_TYPE_APPLE: // ���� � ������ ������, �� �� ������� ����������� ������
				apple.setPosition(float(i * size_cell), float(j * size_cell));
				window.draw(apple);
				break;
			case FIELD_CELL_TYPE_WALL: // ����� ����� �� �����
				wall.setPosition(float(i * size_cell), float(j * size_cell));
				window.draw(wall);
				break;
			default:
				snake.setPosition(float(i * size_cell), float(j * size_cell)); // ��������� ������ ������ ����������� ���:
				window.draw(snake);				
			}
		}
	}
	text_score.setString("Score:" + std::to_string(score)); // �������
	text_score.setCharacterSize(24); // ������
	text_score.setFillColor(sf::Color::Black);// ����
	text_score.setPosition(window_width - text_score.getLocalBounds().width - 59, 1); // ������� �� ������
	window.draw(text_score); // ����� �� �����
}

void draw_menu(sf::RenderWindow &window) { // ������� ����������� ���� �� ������� ESC
	float const menu_filling_horizontal = 60; // ���������� ������ �������� ��� ���������� ����
	float const menu_filling_vertical = 60; // ���������� ������ �������� ��� ���������� ����
	float const menu_item_interval = 20; // ���������� ����� �������� ����
	float menu_item_max_width = 0; // ������������ ������ �������
	float current_menu_item_offset_y = 0; // ���������� ��� Y �������
	for (int i = 0; i < text_menu_items.size(); i++) {
		text_menu_items.at(i).setPosition(0, current_menu_item_offset_y); // ������� ��� �������
		text_menu_items.at(i).setFillColor(current_menu_item_index == i ? sf::Color(224, 224, 224) : sf::Color(128, 128, 128));// ���� �������
		current_menu_item_offset_y += text_menu_items.at(i).getLocalBounds().height + menu_item_interval;
		menu_item_max_width = std::max(menu_item_max_width, text_menu_items.at(i).getLocalBounds().width);
	}

	float const menu_width = menu_item_max_width + menu_filling_horizontal * 2; // ������ ����
	float const menu_height = current_menu_item_offset_y - menu_item_interval + menu_filling_vertical * 2; // ������ ����

	float const menu_pos_x = 350; // ������� ���� �� x
	float const menu_pos_y = 275; // ������� ���� �� y

	sf::RectangleShape main_menu_window(sf::Vector2f(menu_width, menu_height)); // ������ ������
	main_menu_window.setPosition(menu_pos_x, menu_pos_y); // ������� ����
	main_menu_window.setFillColor(sf::Color(0, 0, 0, 224)); // ���� ����
	window.draw(main_menu_window);

	for (int i = 0; i < text_menu_items.size(); ++i) {
		text_menu_items.at(i).move(menu_pos_x + menu_filling_horizontal, menu_pos_y + menu_filling_vertical); // ������� �� ������ �������� ���� (0, 0) � �� ����� ��� ��������� ����������� ����
		window.draw(text_menu_items.at(i));
	}
}

void grow_snake() { // ���� ������
	for (int j = 0; j < size_y; j++) {
		for (int i = 0; i < size_x; i++) {
			if (field[j][i] > FIELD_CELL_TYPE_NONE) { // ���� � ������ ���-�� ������ ����, �� �� ����������� ������ ������ �� 1 
				field[j][i]++;
			}
		}
	}
}

void move() {
	switch (snake_direction) {
	case SNAKE_DIRECTION_UP:
		snake_pos_y--;
		if (snake_pos_y < 0) { // ����� ������ ����� ������� �� ������� ������ ������������ � ������ ������� ����������
			snake_pos_y = size_y - 1;
		}
		break;
	case SNAKE_DIRECTION_DOWN:
		snake_pos_y++;
		if (snake_pos_y > size_y - 1) { // ����� ������ ����� ������� �� ������� ������ ������������ � ������ ������� ����������
			snake_pos_y = 0;
		}
		break;
	case SNAKE_DIRECTION_LEFT:
		snake_pos_x--;
		if (snake_pos_x < 0) { // ����� ������ ����� ������� �� ������� ������ ������������ � ������ ������� ����������
			snake_pos_x = size_x - 1;
		}
		break;
	case SNAKE_DIRECTION_RIGHT:
		snake_pos_x++;
		if (snake_pos_x > size_x - 1) { // ����� ������ ����� ������� �� ������� ������ ������������ � ������ ������� ����������
			snake_pos_x = 0;
		}
		break;
	}

	if(field[snake_pos_y][snake_pos_x] != FIELD_CELL_TYPE_NONE) // �������� �� none.png
		switch (field[snake_pos_y][snake_pos_x]) { // � ��� ����� ���� ���� ���� ���� ������
		case FIELD_CELL_TYPE_APPLE: // ���� �� ����� �������� � ���� ������, �� �� ����������� ����� ����
			sound_ate_apple.play(); // ��������� ���� ��������
			snake_length++; // ����������� ������
			score++;
			grow_snake();
			add_apple();
			break;
		case FIELD_CELL_TYPE_WALL:
			sound_died_the_wall.play();
			game_over = true;
			break;
		default: // � ��������� ������������� �� ��������� ����
			if (field[snake_pos_y][snake_pos_x] > 1) {
				sound_died_himself.play();
				game_over = true;
			}
		}
	if (!game_over) { // ����� ������ �� ������� � ������� �����, � ��������� � ��

		for (int j = 0; j < size_y; ++j) {
			for (int i = 0; i < size_x; ++i) {
				if (field[j][i] > FIELD_CELL_TYPE_NONE) {
					field[j][i]--;
				}
			}
		}
		field[snake_pos_y][snake_pos_x] = snake_length; // 
	}
}

/*void handle_keyboard() {
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) { // ������ W
		if (snake_direction != SNAKE_DIRECTION_DOWN) { // ������ �������� ����� ������ ���� ���� � �������� ������� ���� ������� �� ������
			snake_direction = SNAKE_DIRECTION_UP;
		}
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) { // ������ S
		if (snake_direction != SNAKE_DIRECTION_UP) { // ������ �������� ����� ������ ���� ���� � �������� ������� ���� ������� �� ������
			snake_direction = SNAKE_DIRECTION_DOWN;
		}
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) { // ������ A
		if (snake_direction != SNAKE_DIRECTION_RIGHT) { // ������ �������� ����� ������ ���� ���� � �������� ������� ���� ������� �� ������
			snake_direction = SNAKE_DIRECTION_LEFT;
		}
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) { // ������ D
		if (snake_direction != SNAKE_DIRECTION_LEFT) { // ������ �������� ����� ������ ���� ���� � �������� ������� ���� ������� �� ������
			snake_direction = SNAKE_DIRECTION_RIGHT;
		}
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
		game_over = true;
	}
} ����� ���������� ��� ��� �������� ������
*/

int main()
{
	init_game();
	std::srand(time(NULL)); // �������������� ������ � ����
	// sf - ������������ ��� SFML
	sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Snake", sf::Style::Close); // ������ �������� ����
	
	clear_field(); // �������� ���� �������

	std::vector<int> snake_direction_queue; // ������ �������� ��������� ����������� ������

	while (window.isOpen()) // ������ ��������� ��������
	{
		sf::Event event; // ��������� ������� 
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close(); // �������� ����
			if (event.type == sf::Event::KeyPressed) { // ������ ��������� ������� ������� ������
				if (game_paused) { // ���� ���� �� �����, �� �� ��������� ��� �������, ���� �� �� �����, �� �������� ������ WASD
					switch (event.key.code) {
					case sf::Keyboard::Up:
						current_menu_item_index--; // ��������� �� ���� �����, ������ "--"? ������ ��� �������� � 0 ������� (Resume)
						if (current_menu_item_index < 0) {
							current_menu_item_index = text_menu_items.size() - 1;
						}
						break;
					case sf::Keyboard::Down:
						current_menu_item_index++; // ��������� �� ���� ����
						if (current_menu_item_index > text_menu_items.size() - 1) {
							current_menu_item_index = 0;
						}
						break;
					case sf::Keyboard::Enter:
						if (menu_items.at(current_menu_item_index) == "Resume") { // ���� �� �������� ������ ������� (������ ������), �� ������� � ����� (������������ ����)
							game_paused = false;
						}
						if (menu_items.at(current_menu_item_index) == "Quit") { // ���� �� �������� ����� ������� (������ ������), �� ������� �� ����
							window.close();
						}
					case sf::Keyboard::Escape: // �� ����� ����� ESC ������� �����
						game_paused = false;
						break;
					}
				} else {
					int snake_direction_last = snake_direction_queue.empty() ? snake_direction : snake_direction_queue.at(0); // ���� ����������� ������, �� �� ����� ����������� ������, 
					// � ����� ����� ����������� ����������� � ��� ������� �������� (������)
					switch (event.key.code) {
					case sf::Keyboard::W:
						if (snake_direction_last != SNAKE_DIRECTION_DOWN) { // ������ �������� ����� ������ ���� ���� � �������� ������� ���� ������� �� ������
							if (snake_direction_queue.size() < 2) { // ����� �� ������� ������
								snake_direction_queue.insert(snake_direction_queue.begin(), SNAKE_DIRECTION_UP); // ������������ ������ �����������
							}
						}
						break;
					case sf::Keyboard::S:
						if (snake_direction_last != SNAKE_DIRECTION_UP) { // ������ �������� ����� ������ ���� ���� � �������� ������� ���� ������� �� ������
							if (snake_direction_queue.size() < 2) { // ����� �� ������� ������
								snake_direction_queue.insert(snake_direction_queue.begin(), SNAKE_DIRECTION_DOWN); // ������������ ������ �����������
								break;
							}
						}
					case sf::Keyboard::A:
						if (snake_direction_last != SNAKE_DIRECTION_RIGHT) { // ������ �������� ����� ������ ���� ���� � �������� ������� ���� ������� �� ������
							if (snake_direction_queue.size() < 2) { // ����� �� ������� ������
								snake_direction_queue.insert(snake_direction_queue.begin(), SNAKE_DIRECTION_LEFT); // ������������ ������ �����������
								break;
							}
						}
					case sf::Keyboard::D:
						if (snake_direction_last != SNAKE_DIRECTION_LEFT) { // ������ �������� ����� ������ ���� ���� � �������� ������� ���� ������� �� ������
							if (snake_direction_queue.size() < 2) { // ����� �� ������� ������
								snake_direction_queue.insert(snake_direction_queue.begin(), SNAKE_DIRECTION_RIGHT); // ������������ ������ �����������
								break;
							}
						}
					case sf::Keyboard::Escape: // �� ����� ���� ESC ������ �� �����
						game_paused = true;
						break;
					}
				}
				
			}
		}
		if (!snake_direction_queue.empty()) { // ���� ����������� ������ �� ������
			snake_direction = snake_direction_queue.back(); // ���������� ��������� ������� �� ������� 
			snake_direction_queue.pop_back(); // ����� ������� ��������� �������
		}

		if (!game_paused) {
			move(); // ���������� ��������
		}

				window.clear(sf::Color(183, 212, 168)); // ������� �����
				draw_field(window); // ���������� ��������
				if (game_over) { // ���������� �������� ����
					window.draw(text_gameover); // ����� �� ����� GAME OVER
					window.display(); // ������� ��� �� �����
					sf::sleep(sf::seconds(2));
					window.close();
				}

			//	handle_keyboard(); - ������� ����������� ���������� ���������� ������
				if (game_paused) {
					draw_menu(window);
				}
				sf::sleep(sf::milliseconds(100)); // �������� ������
				window.display(); // ������� ��� �� �����
			}
			return 0;
	}
