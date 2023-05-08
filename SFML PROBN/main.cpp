#include <iostream>
#include <cstdlib>
#include <vector>
#include "SFML/Graphics.hpp"
#include <SFML/Audio.hpp>

/*  
	Управление: W - вверх, S - вниз, A - влево, D - вправо.

	Escape - пауза, управление стрелочками вверх и вниз. 

	Выбрать пункт меню на Enter
*/

const int FIELD_CELL_TYPE_WALL = -2; // значение для преграды
const int FIELD_CELL_TYPE_APPLE = -1; // значение для яблока
const int FIELD_CELL_TYPE_NONE = 0; // создаем константу равную нулю для удобства
const int SNAKE_DIRECTION_UP = 0; // направление вверх
const int SNAKE_DIRECTION_RIGHT = 1; // направление вправо
const int SNAKE_DIRECTION_DOWN = 2; // напрвление вниз
const int SNAKE_DIRECTION_LEFT = 3; // направление влево

const int size_x = 35; // ширина
const int size_y = 25; // высота
const int size_cell = 32; // размер клетки
const int window_width = size_x * size_cell; // размер окна в ширину
const int window_height = size_y * size_cell; // размер окна в высоту

int field[size_y][size_x];// поле для хранения информации что находится в клетке
int snake_pos_x = (size_x / 2); // позиция змейки по x
int snake_pos_y = (size_y / 2); // позиция змейки по y
int snake_length = 3; // длина змейки с самого начала
int snake_direction = SNAKE_DIRECTION_RIGHT; // движение змейки независимо нажимаем мы WASD или нет
int score = 0; // счёт изначально
int current_menu_item_index = 0; // текущий выбор пункта меню, изначально выбирается первый пункт

bool game_over = false;
bool game_paused = true;

sf::Texture snake_texture; // текстура змеи
sf::Sprite snake;
sf::Texture none_texture; // текстура пустых полей
sf::Sprite none;
sf::Texture apple_texture; // текстура яблока
sf::Sprite apple;
sf::Texture wall_texture; // текстура стены
sf::Sprite wall;

sf::SoundBuffer sb_ate_apple; // подключение звука съедания яблока
sf::Sound sound_ate_apple;
sf::SoundBuffer sb_died_the_wall; // подключение звука разрушения об стенку
sf::Sound sound_died_the_wall;
sf::SoundBuffer sb_died_himself; // подключение звука разрушения об себя
sf::Sound sound_died_himself;

sf::Font font_score; // счёт
sf::Text text_score; // счёт
sf::Font font_gameover; // конец игры
sf::Text text_gameover; // конец игры

sf::Font font_menu; // меню
std::vector<sf::Text> text_menu_items; // создаём вектор для удобства работы с кнопками "resume", "start new game", "quit"
std::vector<std::string> menu_items = { "Resume", "Quit" };


void init_game() { // для того чтобы при запуске картинки не выводились 10 раз в секунду, создаем их инициализацию при их запуске

	snake_texture.loadFromFile("images/snake.png"); // текстура змеи
	snake.setTexture(snake_texture);

	none_texture.loadFromFile("images/none.png"); // текстура пустых полей
	none.setTexture(none_texture);

	apple_texture.loadFromFile("images/apple.png"); // текстура яблока
	apple.setTexture(apple_texture);

	wall_texture.loadFromFile("images/wall.png"); // текстура стены
	wall.setTexture(wall_texture);

	sb_ate_apple.loadFromFile("sounds/collect_point.wav"); // звук собирания яблока
	sound_ate_apple.setBuffer(sb_ate_apple);

	sb_died_the_wall.loadFromFile("sounds/explosion_01.wav"); // звук разрушения об стенку
	sound_died_the_wall.setBuffer(sb_died_the_wall);

	sb_died_himself.loadFromFile("sounds/explosion_00.wav"); // звук разрушения об себя
	sound_died_himself.setBuffer(sb_died_himself);

	font_score.loadFromFile("fonts/Quinquefive-ALoRM.ttf"); // текст счёта
	text_score.setFont(font_score); 

	font_gameover.loadFromFile("fonts/Quinquefive-ALoRM.ttf"); // текст GAME OVER
	text_gameover.setFont(font_gameover);

	text_gameover.setString("GAME OVER"); // надпись
	text_gameover.setCharacterSize(90); // размер
	text_gameover.setFillColor(sf::Color::Red);// цвет
	text_gameover.setPosition(75, 350); // позиция на экране

	font_menu.loadFromFile("fonts/Quinquefive-ALoRM.ttf"); // текст меню
	for (int i = 0; i < menu_items.size(); ++i) {
		text_menu_items.emplace_back(sf::Text());
		text_menu_items.back().setString(menu_items.at(i));
		text_menu_items.back().setFont(font_menu);
		text_menu_items.back().setCharacterSize(40);
	}
}

int get_random_empty_cell() { // функция для поиска пустой ячейки. int потому что мы возвращаем последовательно номер пустой клетки начиная с нуля
	int empty_cell_count = 0;

	for (int j = 0; j < size_y; ++j) {
		for (int i = 0; i < size_x; ++i) {
			if (field[j][i] == FIELD_CELL_TYPE_NONE) { // если текущая клетка пустая, то мы меняем счетчик(инкриминтируем)
				empty_cell_count++;
			}
		}
	}
	int target_empty_cell_index = std::rand() % empty_cell_count; // делим рандомное число на количество пустых клеток и получаем номер целевой клетки
	int empty_cell_index = 0;

	for (int j = 0; j < size_y; ++j) { // отсчитываем выбранную клетку и определяем её координату
		for (int i = 0; i < size_x; ++i) {
			if (field[j][i] == FIELD_CELL_TYPE_NONE) {
				if (empty_cell_index == target_empty_cell_index) { // если мы дошли до текущей клетки то,
					return j * size_x + i;// возвращаем координаты в ввиде произведения
				}
				empty_cell_index++;
			}
		}
	}
	return -1;
}

void add_apple() { // функция для добавления яблока по координатам пустых клеток
	int apple_pos = get_random_empty_cell();
	if (apple_pos != -1) {
		field[apple_pos / size_x][apple_pos % size_x] = FIELD_CELL_TYPE_APPLE;
	}
	field[apple_pos / size_x][apple_pos % size_x] = FIELD_CELL_TYPE_APPLE;
}

void clear_field() { // очистка полей змейки для её обработки
	for (int j = 0; j < size_y; ++j) {
		for (int i = 0; i < size_x; ++i) {
			field[j][i] = FIELD_CELL_TYPE_NONE;
		}
	}
	for (int i = 0; i < snake_length; i++) { // увеличиваем длину змейки когда сьест яблоко
		field[snake_pos_y][snake_pos_x - i] = snake_length - i;
	}
	for (int i = 0; i < size_y; i++) { // создаем цикл для отрисовки боковых стен
		if (i < 8 || size_y - i - 1 < 8) { // 8 стен
			field[i][0] = FIELD_CELL_TYPE_WALL; // выдаем стенам координаты
			field[i][size_x - 1] = FIELD_CELL_TYPE_WALL;
		}
	}
	for (int i = 0; i < size_x; i++) { // создаем цикл для отрисовки стен
		if (i < 8 || size_x - i - 1 < 8) { // 8 стен 
			field[0][i] = FIELD_CELL_TYPE_WALL; // выдаем стенам координаты
			field[size_y - 1][i] = FIELD_CELL_TYPE_WALL;
		}
	}
	add_apple();
}

void draw_field(sf::RenderWindow & window) { // отрисовка текстур

	for (int j = 0; j < size_y; j++) {
		for (int i = 0; i < size_x; i++) {
			switch (field[j][i]) { 
			case FIELD_CELL_TYPE_NONE : // если в клетке ничего, то мы выводим изображение пустой клетки
				none.setPosition(float(i * size_cell), float(j * size_cell)); // положение пустой клетки расчитываем как:
				window.draw(none);
				break;
			case FIELD_CELL_TYPE_APPLE: // если в клетке ничего, то мы выводим изображение яблока
				apple.setPosition(float(i * size_cell), float(j * size_cell));
				window.draw(apple);
				break;
			case FIELD_CELL_TYPE_WALL: // вывод стены на экран
				wall.setPosition(float(i * size_cell), float(j * size_cell));
				window.draw(wall);
				break;
			default:
				snake.setPosition(float(i * size_cell), float(j * size_cell)); // положение пустой клетки расчитываем как:
				window.draw(snake);				
			}
		}
	}
	text_score.setString("Score:" + std::to_string(score)); // надпись
	text_score.setCharacterSize(24); // размер
	text_score.setFillColor(sf::Color::Black);// цвет
	text_score.setPosition(window_width - text_score.getLocalBounds().width - 59, 1); // позиция на экране
	window.draw(text_score); // вывод на экран
}

void draw_menu(sf::RenderWindow &window) { // функция создавающая меню по нажатию ESC
	float const menu_filling_horizontal = 60; // добавление черных отступов для заполнения меню
	float const menu_filling_vertical = 60; // добавление черных отступов для заполнения меню
	float const menu_item_interval = 20; // разделение между пунктами меню
	float menu_item_max_width = 0; // максимальная ширина пунктов
	float current_menu_item_offset_y = 0; // переменная для Y позиции
	for (int i = 0; i < text_menu_items.size(); i++) {
		text_menu_items.at(i).setPosition(0, current_menu_item_offset_y); // позиция для функций
		text_menu_items.at(i).setFillColor(current_menu_item_index == i ? sf::Color(224, 224, 224) : sf::Color(128, 128, 128));// цвет функций
		current_menu_item_offset_y += text_menu_items.at(i).getLocalBounds().height + menu_item_interval;
		menu_item_max_width = std::max(menu_item_max_width, text_menu_items.at(i).getLocalBounds().width);
	}

	float const menu_width = menu_item_max_width + menu_filling_horizontal * 2; // ширина меню
	float const menu_height = current_menu_item_offset_y - menu_item_interval + menu_filling_vertical * 2; // высота меню

	float const menu_pos_x = 350; // позиция меню по x
	float const menu_pos_y = 275; // позиция меню по y

	sf::RectangleShape main_menu_window(sf::Vector2f(menu_width, menu_height)); // создаём фигуру
	main_menu_window.setPosition(menu_pos_x, menu_pos_y); // позиция меню
	main_menu_window.setFillColor(sf::Color(0, 0, 0, 224)); // цвет меню
	window.draw(main_menu_window);

	for (int i = 0; i < text_menu_items.size(); ++i) {
		text_menu_items.at(i).move(menu_pos_x + menu_filling_horizontal, menu_pos_y + menu_filling_vertical); // смещаем из левого верхнего угла (0, 0) в то место где находится вычисленное меню
		window.draw(text_menu_items.at(i));
	}
}

void grow_snake() { // рост змейки
	for (int j = 0; j < size_y; j++) {
		for (int i = 0; i < size_x; i++) {
			if (field[j][i] > FIELD_CELL_TYPE_NONE) { // если в ячейке что-то больше нуля, то мы увеличиваем размер змейки на 1 
				field[j][i]++;
			}
		}
	}
}

void move() {
	switch (snake_direction) {
	case SNAKE_DIRECTION_UP:
		snake_pos_y--;
		if (snake_pos_y < 0) { // чтобы змейка когда уходила за пределы экрана перемещалась с другой стороны равномерно
			snake_pos_y = size_y - 1;
		}
		break;
	case SNAKE_DIRECTION_DOWN:
		snake_pos_y++;
		if (snake_pos_y > size_y - 1) { // чтобы змейка когда уходила за пределы экрана перемещалась с другой стороны равномерно
			snake_pos_y = 0;
		}
		break;
	case SNAKE_DIRECTION_LEFT:
		snake_pos_x--;
		if (snake_pos_x < 0) { // чтобы змейка когда уходила за пределы экрана перемещалась с другой стороны равномерно
			snake_pos_x = size_x - 1;
		}
		break;
	case SNAKE_DIRECTION_RIGHT:
		snake_pos_x++;
		if (snake_pos_x > size_x - 1) { // чтобы змейка когда уходила за пределы экрана перемещалась с другой стороны равномерно
			snake_pos_x = 0;
		}
		break;
	}

	if(field[snake_pos_y][snake_pos_x] != FIELD_CELL_TYPE_NONE) // проверка на none.png
		switch (field[snake_pos_y][snake_pos_x]) { // у нас может быть либо змея либо яблоко
		case FIELD_CELL_TYPE_APPLE: // если мы нашли преграду в виде яблока, то мы увеличиваем длину змеи
			sound_ate_apple.play(); // добавляем звук съедания
			snake_length++; // увеличиваем змейку
			score++;
			grow_snake();
			add_apple();
			break;
		case FIELD_CELL_TYPE_WALL:
			sound_died_the_wall.play();
			game_over = true;
			break;
		default: // в остальных столкновениях мы завершаем игру
			if (field[snake_pos_y][snake_pos_x] > 1) {
				sound_died_himself.play();
				game_over = true;
			}
		}
	if (!game_over) { // чтобы змейка не входила в границы стены, а врезалась в неё

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
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) { // кнопка W
		if (snake_direction != SNAKE_DIRECTION_DOWN) { // делаем проверку чтобы нельзя было идти в обратную сторону пока ползешь по прямой
			snake_direction = SNAKE_DIRECTION_UP;
		}
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) { // кнопка S
		if (snake_direction != SNAKE_DIRECTION_UP) { // делаем проверку чтобы нельзя было идти в обратную сторону пока ползешь по прямой
			snake_direction = SNAKE_DIRECTION_DOWN;
		}
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) { // кнопка A
		if (snake_direction != SNAKE_DIRECTION_RIGHT) { // делаем проверку чтобы нельзя было идти в обратную сторону пока ползешь по прямой
			snake_direction = SNAKE_DIRECTION_LEFT;
		}
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) { // кнопка D
		if (snake_direction != SNAKE_DIRECTION_LEFT) { // делаем проверку чтобы нельзя было идти в обратную сторону пока ползешь по прямой
			snake_direction = SNAKE_DIRECTION_RIGHT;
		}
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
		game_over = true;
	}
} ПЛОХО РАБОТАЮЩИЙ КОД ДЛЯ ДВИЖЕНИЯ ЗМЕЙКИ
*/

int main()
{
	init_game();
	std::srand(time(NULL)); // инициализируем рандом в коде
	// sf - пространство имён SFML
	sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Snake", sf::Style::Close); // создаём основное окно
	
	clear_field(); // вызываем цикл заранее

	std::vector<int> snake_direction_queue; // вектор хранящий изменение направления змейки

	while (window.isOpen()) // запуск основного процесса
	{
		sf::Event event; // обработка событий 
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close(); // закрытие окна
			if (event.type == sf::Event::KeyPressed) { // создаём обработку событий нажатий клавиш
				if (game_paused) { // если игра на паузе, то мы выполняем эти функции, если не на паузе, то движение змейки WASD
					switch (event.key.code) {
					case sf::Keyboard::Up:
						current_menu_item_index--; // двигаться по меню вверх, почему "--"? потому что начинаем с 0 индекса (Resume)
						if (current_menu_item_index < 0) {
							current_menu_item_index = text_menu_items.size() - 1;
						}
						break;
					case sf::Keyboard::Down:
						current_menu_item_index++; // двигаться по меню вниз
						if (current_menu_item_index > text_menu_items.size() - 1) {
							current_menu_item_index = 0;
						}
						break;
					case sf::Keyboard::Enter:
						if (menu_items.at(current_menu_item_index) == "Resume") { // если мы выбираем первую позицию (начало сверху), то снимаем с паузы (возобновляем игру)
							game_paused = false;
						}
						if (menu_items.at(current_menu_item_index) == "Quit") { // если мы выбираем треть позицию (начало сверху), то выходим из игры
							window.close();
						}
					case sf::Keyboard::Escape: // во время паузы ESC снимает паузу
						game_paused = false;
						break;
					}
				} else {
					int snake_direction_last = snake_direction_queue.empty() ? snake_direction : snake_direction_queue.at(0); // если направление пустое, то мы берем направление змейки, 
					// а иначе берем направление находящиеся в под нулевым индексом (первое)
					switch (event.key.code) {
					case sf::Keyboard::W:
						if (snake_direction_last != SNAKE_DIRECTION_DOWN) { // делаем проверку чтобы нельзя было идти в обратную сторону пока ползешь по прямой
							if (snake_direction_queue.size() < 2) { // лимит на очередь кравиш
								snake_direction_queue.insert(snake_direction_queue.begin(), SNAKE_DIRECTION_UP); // присваивание нового направления
							}
						}
						break;
					case sf::Keyboard::S:
						if (snake_direction_last != SNAKE_DIRECTION_UP) { // делаем проверку чтобы нельзя было идти в обратную сторону пока ползешь по прямой
							if (snake_direction_queue.size() < 2) { // лимит на очередь кравиш
								snake_direction_queue.insert(snake_direction_queue.begin(), SNAKE_DIRECTION_DOWN); // присваивание нового направления
								break;
							}
						}
					case sf::Keyboard::A:
						if (snake_direction_last != SNAKE_DIRECTION_RIGHT) { // делаем проверку чтобы нельзя было идти в обратную сторону пока ползешь по прямой
							if (snake_direction_queue.size() < 2) { // лимит на очередь кравиш
								snake_direction_queue.insert(snake_direction_queue.begin(), SNAKE_DIRECTION_LEFT); // присваивание нового направления
								break;
							}
						}
					case sf::Keyboard::D:
						if (snake_direction_last != SNAKE_DIRECTION_LEFT) { // делаем проверку чтобы нельзя было идти в обратную сторону пока ползешь по прямой
							if (snake_direction_queue.size() < 2) { // лимит на очередь кравиш
								snake_direction_queue.insert(snake_direction_queue.begin(), SNAKE_DIRECTION_RIGHT); // присваивание нового направления
								break;
							}
						}
					case sf::Keyboard::Escape: // во время игры ESC ставит на паузу
						game_paused = true;
						break;
					}
				}
				
			}
		}
		if (!snake_direction_queue.empty()) { // если направление змейки не пустое
			snake_direction = snake_direction_queue.back(); // возвращаем последний элемент из вектора 
			snake_direction_queue.pop_back(); // затем удаляем последний элемент
		}

		if (!game_paused) {
			move(); // подключаем движение
		}

				window.clear(sf::Color(183, 212, 168)); // очищает экран
				draw_field(window); // подключаем картинки
				if (game_over) { // подключаем закрытие окна
					window.draw(text_gameover); // вывод на экран GAME OVER
					window.display(); // выводит все на экран
					sf::sleep(sf::seconds(2));
					window.close();
				}

			//	handle_keyboard(); - прошлое подключение неудобного управления змейки
				if (game_paused) {
					draw_menu(window);
				}
				sf::sleep(sf::milliseconds(100)); // скорость змейки
				window.display(); // выводит все на экран
			}
			return 0;
	}
