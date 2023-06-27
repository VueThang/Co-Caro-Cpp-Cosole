#include <iostream>
#include <conio.h>
#include <Windows.h>
#include <fstream>
#include <string>
#include <stack>
#include <vector>
#include <cstdio>
using namespace std;
HANDLE hStdin; 
DWORD fdwSaveOldMode;
VOID ErrorExit (LPSTR lpszMessage);
#define MAX_SIZE  16
struct Point
{
	char val;
	int x;
	int y;
};
stack<Point> UNDO_STACK;
stack<Point> REDO_STACK;

const char PLAYER_X = 'X';
const char PLAYER_O = 'O';
const int WIDTH = 3, HEIGHT = 1; // độ rộng, cao của một ô vuông trên lưới ô vuông màn hình console
const int ROWS = 14; // rows 14
const int COLS = 16; // cols 16
char board[ROWS][COLS];
int move_X = 0, move_Y = 0;
int preMove_X = -1, preMove_Y = -1;
int SoQuanCoWin = 5;
bool chanHaiDau = false;
bool playWithBot = false;

enum Coordinates
{
	START_X = 3,
	START_Y = 1,
	END_X = COLS * (WIDTH + 1) + START_X,//4 * COLS + WIDTH,
	END_Y = ROWS * (HEIGHT + 1) + START_Y//START_Y + ROWS * 2
};
enum keyControls
{
	UP = 72,
	DOWN = 80,
	LEFT = 75,
	RIGHT = 77,
	SPACE = 32,
	ENTER = 13,
	ESC = 27,
	CTRL_U = 21,
	CTRL_Z = 26,
	CTRL_S = 19,
	CTRL_D = 4,
	BACKSPACE = 8
};
enum Colors
{
	RED = 12,
	GREEN = 10,
	NORMAL_COLOR = 7,
	WHITE = 15,
	PURPLE = 91,
	YELLOW_BACKGROUND = 110
};

void textcolor(int x)
{
	HANDLE mau;
	mau = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(mau,x);
}

void gotoxy(int x,int y)
{    
	HANDLE hConsoleOutput;    
	COORD Cursor_an_Pos = {x-1,y-1};   
	hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);    
	SetConsoleCursorPosition(hConsoleOutput , Cursor_an_Pos);
}

void clear(int x = 0, int y = 0) {
    COORD topLeft  = { x, y };
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO screen;
    DWORD written;

    GetConsoleScreenBufferInfo(console, &screen);
    FillConsoleOutputCharacterA(
        console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written
    );
    FillConsoleOutputAttribute(
        console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
        screen.dwSize.X * screen.dwSize.Y, topLeft, &written
    );
    SetConsoleCursorPosition(console, topLeft);
}

void clearScreen(int x = 0, int y = 0, int y_end = 0) {
    COORD topLeft  = { x, y };
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO screen;
    DWORD written;

    GetConsoleScreenBufferInfo(console, &screen);
    // Chiều rộng của vùng cần xóa bằng chiều rộng của màn hình console
    int width = screen.dwSize.X;
    // Duyệt qua các hàng từ y đến y_end
    for (int i = y; i <= y_end; i++) {
        // Đặt con trỏ tại vị trí (x, i)
        topLeft.X = x;
        topLeft.Y = i;
        SetConsoleCursorPosition(console, topLeft);
        // Xóa các ký tự từ (x, i) đến cuối hàng
        FillConsoleOutputCharacterA(
            console, ' ', width, topLeft, &written
        );
        FillConsoleOutputAttribute(
            console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
            width, topLeft, &written
        );
    }
    // Đặt con trỏ tại vị trí ban đầu
    topLeft.X = x;
    topLeft.Y = y;
    SetConsoleCursorPosition(console, topLeft);
}

void clear2(int x = 0, int y = 0, int x_end = 0, int y_end = 0) {
    COORD topLeft  = { x, y };
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO screen;
    DWORD written;

    GetConsoleScreenBufferInfo(console, &screen);
    // Chiều rộng của vùng cần xóa
    int width = x_end - x + 1;
    // Nếu chiều rộng nhỏ hơn hoặc bằng 0 thì không làm gì
    if (width <= 0) return;
    // Duyệt qua các hàng từ y đến y_end
    for (int i = y; i <= y_end; i++) {
        // Đặt con trỏ tại vị trí (x, i)
        topLeft.X = x;
        topLeft.Y = i;
        SetConsoleCursorPosition(console, topLeft);
        // Xóa các ký tự từ (x, i) đến (x_end, i)
        FillConsoleOutputCharacterA(
            console, ' ', width, topLeft, &written
        );
        FillConsoleOutputAttribute(
            console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
            width, topLeft, &written
        );
    }
    // Đặt con trỏ tại vị trí ban đầu
    topLeft.X = x;
    topLeft.Y = y;
    SetConsoleCursorPosition(console, topLeft);
}

// Hiển thị
void gameLogo()
{
	ifstream file_in;
	file_in.open("Resources\\game_logo.txt", ios_base:: in);
	gotoxy(10, 20);
	textcolor(11);
	while (!file_in.eof())
	{
		string str;
		getline(file_in, str);
		cout << "   " << str << endl;
	}
	textcolor(NORMAL_COLOR);
	file_in.close();
}
void caroLogo()
{
	int x = END_X + 3, y = 20;
	int color = 139;
	ifstream file_in("Resources\\logo_caro.txt");
	string str;
	textcolor(color);
	while (!file_in.eof())
	{
		gotoxy(x, y++);
		getline(file_in, str);
		cout << str;
	}
	textcolor(NORMAL_COLOR);
	file_in.close();
}
void xWins()
{
	for (int colors = 10; colors <= 15; colors++)
	{
		if(colors == 15)
			textcolor(RED);
		else
			textcolor(colors);
		int x = 70, y = 16;
		ifstream file_in("Resources\\x_wins.txt");
		string str;
		while (!file_in.eof())
		{
			gotoxy(x, y++);
			getline(file_in, str);
			cout << str;
		}
		file_in.close();
		Sleep(50);
	}
	
}
void oWins()
{
	for (int colors = 10; colors <= 15; colors++)
	{
		if (colors == 15)
			textcolor(GREEN);
		else
			textcolor(colors);
		int x = 70, y = 16;
		ifstream file_in("Resources\\o_wins.txt");
		string str;
		while (!file_in.eof())
		{
			gotoxy(x, y++);
			getline(file_in, str);
			cout << str;
		}
		file_in.close();
		Sleep(50);
	}
}

// Kiểm tra chiến thắng Game
bool KiemTraHangDong(char player, int row, int col)
{
	int DemHaiDau = 0;
	int cnt = 0;
	for (int j = col; j >= 0 ; --j)
	{
		if(board[row][j] == player)
		{
			cnt++;
			if (!chanHaiDau)
			{
				if(cnt == SoQuanCoWin)
				return true;
			}
		}
		else 
		{
			if (chanHaiDau)
			{
				if (board[row][j] != ' ' && board[row][j] != NULL)
					DemHaiDau++;
			}
			break;
		}
	}

	for (int j = col + 1; j < COLS; ++j)
	{
		if(board[row][j] == player)
		{
			cnt++;
			if (!chanHaiDau)
			{
				if(cnt == SoQuanCoWin)
					return true;
			}
		}
		else 
		{
			if (chanHaiDau)
			{
				if (board[row][j] != ' ' && board[row][j] != NULL)
					DemHaiDau++;
			}
			break;
		}
	}

	if (!chanHaiDau)
		return false;
	if(DemHaiDau >= 2)
		return false;
	if(cnt >= SoQuanCoWin)
		return true;
	return false;
}
bool KiemTraCot(char player, int row, int col)
{
	int DemHaiDau = 0;
	int cnt = 0;
	for (int i = row; i >= 0; --i)
	{
		if (board[i][col] == player)
		{
			cnt++;
			if (!chanHaiDau)
			{
				if(cnt == SoQuanCoWin)
					return true;
			}
		}
		else 
		{
			if (chanHaiDau)
			{
				if (board[i][col] != ' ' && board[i][col] != NULL)
					DemHaiDau++;
			}
			break;
		}
	}

	for (int i = row + 1; i < ROWS; ++i)
	{
		if (board[i][col] == player)
		{
			cnt++;
			if (!chanHaiDau)
			{
				if(cnt == SoQuanCoWin)
					return true;
			}
		}
		else 
		{
			if (chanHaiDau)
			{
				if (board[i][col] != ' ' && board[i][col] != NULL)
					DemHaiDau++;
			}
			break;
		}
	}

	if (!chanHaiDau)
		return false;
	if (DemHaiDau >= 2)
		return false;
	if (cnt >= SoQuanCoWin)
		return true;
	return false;
}
bool KiemTraDuongCheoChinh(char player, int row, int col)
{
	int i = row, j = col;
	int cnt = 0;
	int DemHaiDau = 0;

	while (i >= 0 && j >= 0)
	{
		if (board[i][j] == player)
		{
			cnt++;
			if (!chanHaiDau)
			{
				if(cnt == SoQuanCoWin)
					return true;
			}
		}
		else
		{
			if (chanHaiDau)
			{
				if (board[i][j] != ' ' && board[i][j] != NULL)
					DemHaiDau ++;
			}
			break;
		}
		--i;
		--j;
	}

	i = row + 1;
	j = col + 1;
	while (i < ROWS && j < COLS)
	{
		if (board[i][j] == player)
		{
			cnt++;
			if (!chanHaiDau)
			{
				if(cnt == SoQuanCoWin)
					return true;
			}
		}
		else
		{
			if (chanHaiDau)
			{
				if (board[i][j] != ' ' && board[i][j] != NULL)
					DemHaiDau ++;
			}
			break;
		}
		++i;
		++j;
	}

	if(!chanHaiDau)
		return false;
	if (DemHaiDau >= 2)
		return false;
	if (cnt == SoQuanCoWin)
		return true;
	return false;
}
bool KiemTraDuongCheoPhu(char player, int row, int col)
{
	int i = row, j = col;
	int cnt = 0;
	int DemHaiDau = 0;

	while (i >= 0 && j < COLS)
	{
		if(board[i][j] == player)
		{
			cnt++;
			if (!chanHaiDau)
			{
				if(cnt == SoQuanCoWin)
					return true;
			}
		}
		else
		{
			if (chanHaiDau)
			{
				if (board[i][j] != ' ' && board[i][j] != NULL)
					DemHaiDau++;
			}
			break;
		}
		--i;
		++j;
	}

	i = row + 1;
	j = col - 1;
	while (i < ROWS && j >= 0)
	{
		if(board[i][j] == player)
		{
			cnt++;
			if (!chanHaiDau)
			{
				if(cnt == SoQuanCoWin)
					return true;
			}
		}
		else
		{
			if (chanHaiDau)
			{
				if (board[i][j] != ' ' && board[i][j] != NULL)
					DemHaiDau++;
			}
			break;
		}
		++i;
		--j;
	}
	if (!chanHaiDau)
		return false;
	if (DemHaiDau >= 2)
		return false;
	if (cnt >= SoQuanCoWin)
		return true;
	return false;
}
void HieuUngChienThangDong(char , int , int );
void HieuUngChienThangCot(char , int, int);
void HieuUngChienThangCheoChinh(char, int, int);
void HieuUngChienThangCheoPhu(char, int, int);
bool checkWinner(char player, int row, int col);

// Tính năng phụ
void mainMenu();
void Undo(char);
void Redo(char);
bool SaveGame(char, int, int);
int LoadGame(char&, int&, int&);
int printFiles(vector<string>, int, int);
void drawPreviousSteps();
void Clock()
{
	int hr = 0, minu = 0, sec = 0;
	gotoxy(END_X + 18, 14);
	textcolor(YELLOW_BACKGROUND);
	if(hr < 10)
		cout << '0' << hr;
	else
		cout << hr;
	cout << " : ";
	
	if (minu < 10)
		cout << '0' << minu;
	else
		cout << minu;
	cout << " : ";
	if (sec < 10)
		cout << '0' << sec;
	else
		cout << sec;
	textcolor(NORMAL_COLOR);
}
void Help()
{
	textcolor(138);
	gotoxy(48, 2);
	cout << "  Controls  ";
	gotoxy(35, 4);
	textcolor(GREEN);
	cout << " TOP";
	gotoxy(35, 5);
	textcolor(106);
	cout << "  W  ";
	

	gotoxy(72, 4);
	textcolor(GREEN);
	cout << " TOP";
	gotoxy(72, 5);
	textcolor(106);
	cout << "  " << (char)24 << "  ";

	gotoxy(24, 7);
	textcolor(GREEN);
	cout << "LEFT  ";
	textcolor(106);
	cout << "  A  ";

	gotoxy(60, 7);
	textcolor(GREEN);
	cout << "LEFT  ";
	textcolor(106);
	cout << "  " << (char)27 << "  ";

	gotoxy(40, 7);
	textcolor(106);
	cout << "  D  ";
	textcolor(GREEN);
	cout << "  RIGHT";

	gotoxy(77, 7);
	textcolor(106);
	cout << "  " << (char)26 << "  ";
	textcolor(GREEN);
	cout << "  RIGHT";

	gotoxy(35, 9);
	textcolor(106);
	cout << "  S  ";
	gotoxy(35, 10);
	textcolor(GREEN);
	cout << " DOWN ";
	
	gotoxy(72, 9);
	textcolor(106);
	cout << "  " << (char)25 << "  ";
	gotoxy(72, 10);
	textcolor(GREEN);
	cout << " DOWN ";

	textcolor(138);
	gotoxy(50, 12);
	cout << "  Done  ";
	textcolor(NORMAL_COLOR);
	_getch();
	clear();
	mainMenu();
}
void About()
{
	textcolor(GREEN);
	cout << "TAC GIA: VU A THANG" << endl;
	cout << "GMAIL: thangvu407@gmail.com" << endl;
	textcolor(NORMAL_COLOR);
	_getch();
	clear();
	mainMenu();
}
void Settings()
{
	textcolor(GREEN);
	int idx_contents = 0;
	vector<string> contents;
	contents.push_back("So luong quan lien tiep de duoc chien thang     ");
	contents.push_back("Chan hai dau quan co se khong chien thang     ");
	contents.push_back("Choi choi voi may     ");
	contents.push_back("Kich thuoc ban co     ");

	int x = 20, y = 3;
	int startY = y;

	int length = contents.size();
	
	int SoQuanCoWins[] = {5, 6, 4, 3}, idx_SoQuanWins = 0, lenghtSoQuanWins = 4;

	int idx_BoardSizes = 0;
	pair<int, int> boardSize;
	vector<pair<int, int>> boardSizes;
	boardSize.first = 14;
	boardSize.second = 16;
	boardSizes.push_back(boardSize);

	boardSize.first = 14;
	boardSize.second = 14;
	boardSizes.push_back(boardSize);

	boardSize.first = 10;
	boardSize.second = 10;
	boardSizes.push_back(boardSize);

	boardSize.first = 5;
	boardSize.second = 5;
	boardSizes.push_back(boardSize);

	for (int i = 0; i < length; i++)
	{
		gotoxy(x, y);
		if (i == 0)
			cout << contents[i] << "< " << SoQuanCoWins[idx_SoQuanWins] << " >";
		else if(i == 1)
		{
			cout << contents[i] << "< " << "off" << " >";
		}
		else if(i == 2)
			cout << contents[i] << "< " << "off" << " >";
		else if (i == 3)
			cout << contents[i] << "< " << boardSizes[0].first << " x " << boardSizes[0].second << " >";
		
		y += 2;
	}
	int endY = y;

	// control's
	x = 20; y = 3;
	gotoxy(x + contents[idx_contents].length() + 1, y);
	int contensSize = contents.size();
	while (true)
	{
		char c = _getch();
		if (c == UP || c == 'w' || c == 'W')
		{
			if(idx_contents > 0)
				idx_contents--;
			if (y > startY)
				gotoxy(x + contents[idx_contents].length() + 1, y -= 2);
		}
		else if (c == DOWN || c == 's' || c == 'S')
		{
			if (idx_contents < contensSize - 1)
				idx_contents++;
			if (y < endY - 2)
				gotoxy(x + contents[idx_contents].length() + 1, y += 2);
		}
		else if (c == LEFT || c == 'a' || c == 'A')
		{
			if (idx_contents == 0)
			{
				if(idx_SoQuanWins > 0)
					idx_SoQuanWins--;
				gotoxy(x + contents[idx_contents].length() + 2, y);
				cout << SoQuanCoWins[idx_SoQuanWins];
				SoQuanCoWin = SoQuanCoWins[idx_SoQuanWins];
			}
			else if (idx_contents == 1)
			{
				chanHaiDau == false ? chanHaiDau = true : chanHaiDau = false;
				gotoxy(x + contents[idx_contents].length() + 2, y);
				if (chanHaiDau)
					cout << "on ";
				else
					cout << "off";
			}
			else if (idx_contents == 3)
			{
				if (idx_BoardSizes > 0)
					idx_BoardSizes--;
				clear2(x + contents[idx_contents].length() + 2, y - 1, x + contents[idx_contents].length() + 8, y);
				gotoxy(x + contents[idx_contents].length() + 2, y);
				cout << boardSizes[idx_BoardSizes].first << " x " << boardSizes[idx_BoardSizes].second;
			}
		}
		else if (c == RIGHT || c == 'd' || c == 'D')
		{
			if (idx_contents == 0)
			{
				if (idx_SoQuanWins < lenghtSoQuanWins - 1)
					idx_SoQuanWins++;
				gotoxy(x + contents[idx_contents].length() + 2, y);
				cout << SoQuanCoWins[idx_SoQuanWins];
				SoQuanCoWin = SoQuanCoWins[idx_SoQuanWins];
			}
			else if (idx_contents == 1)
			{
				chanHaiDau == false ? chanHaiDau = true : chanHaiDau = false;
				gotoxy(x + contents[idx_contents].length() + 2, y);
				if (chanHaiDau)
					cout << "on ";
				else
					cout << "off";
			}
			else if (idx_contents == 3)
			{
				int sizeOfBoard = boardSizes.size();
				if (idx_BoardSizes < sizeOfBoard - 1)
					idx_BoardSizes++;
				clear2(x + contents[idx_contents].length() + 2, y - 1, x + contents[idx_contents].length() + 8, y);
				gotoxy(x + contents[idx_contents].length() + 2, y);
				cout << boardSizes[idx_BoardSizes].first << " x " << boardSizes[idx_BoardSizes].second;
			}
		}
		else if(c == ESC)
		{
			clear();
			break;
		}
	}
	textcolor(NORMAL_COLOR);
	mainMenu();
}
void Border(int, int, int, int);
void drawBoard();
bool validateMouseClick(int x, int y);
int controlMenu();
void playerBox(char, int, int, int, int);
bool continueGame();
bool quitGame();
int playerControl(char);
int processCore(char, int, int);
void mainGame(int);
void ResetGame();

int main()
{
	 // Get the standard input handle. 
    hStdin = GetStdHandle(STD_INPUT_HANDLE); 
    if (hStdin == INVALID_HANDLE_VALUE) 
        ErrorExit("GetStdHandle"); 
 
    // Save the current input mode, to be restored on exit. 
    if (! GetConsoleMode(hStdin, &fdwSaveOldMode) ) 
        ErrorExit("GetConsoleMode"); 

	mainMenu();
	return 0;
}

bool checkWinner(char player, int row, int col)
{
	bool flag = false;
	flag = KiemTraHangDong(player, row, col);
	if(flag == true) 
	{
		if(player == PLAYER_O)
		{
			clear2(69, 17, 120, 30);
			textcolor(GREEN);
			oWins();
			HieuUngChienThangDong(player, row, col);
			_getch();
		}
		else
		{
			clear2(69, 17, 120, 30);
			textcolor(RED);
			xWins();
			HieuUngChienThangDong(player, row, col);
			_getch();
		}
		textcolor(NORMAL_COLOR);
		return false;
	}

	flag = KiemTraCot(player, row, col);
	if(flag == true) 
	{
		if(player == PLAYER_O)
		{
			clear2(69, 17, 120, 30);
			textcolor(GREEN);
			oWins();
			HieuUngChienThangCot(player, row, col);
			_getch();
		}
		else
		{
			clear2(69, 17, 120, 30);
			textcolor(RED);
			xWins();
			HieuUngChienThangCot(player, row, col);
			_getch();
		}
		textcolor(NORMAL_COLOR);
		return false;
	}

	flag = KiemTraDuongCheoChinh(player, row, col);
	if(flag == true) 
	{
		if(player == PLAYER_O)
		{
			clear2(69, 17, 120, 30);
			textcolor(GREEN);
			oWins();
			HieuUngChienThangCheoChinh(player, row, col);
			_getch();
		}
		else
		{
			clear2(69, 17, 120, 30);
			textcolor(RED);
			xWins();
			HieuUngChienThangCheoChinh(player, row, col);
			_getch();
		}
		textcolor(NORMAL_COLOR);
		return false;
	}

	flag = KiemTraDuongCheoPhu(player, row, col);
	if(flag == true) 
	{
		if(player == PLAYER_O)
		{
			clear2(69, 17, 120, 30);
			textcolor(GREEN);
			oWins();
			HieuUngChienThangCheoPhu(player, row, col);
			_getch();
		}
		else
		{
			clear2(69, 17, 120, 30);
			textcolor(RED);
			xWins();
			HieuUngChienThangCheoPhu(player, row, col);
			_getch();
		}
		textcolor(NORMAL_COLOR);
		return false;
	}
	return true;
}

void Border(int x, int y, int width = 20, int height = 14)
{
	textcolor(11);
	gotoxy(x, y);
	cout << (char)201;
	for (int i = 0; i < width; i++)
	{
		cout << (char)205;
	}
	cout << (char)187;

	for (int i = 0; i < height; i++)
	{
		gotoxy(x, ++y);
		cout << (char)186;
		for (int j = 0; j < width; j++)
		{
			cout << ' ';
		}
		cout << (char)186;
	}

	gotoxy(x, y);
	cout << (char)200;
	for (int i = 0; i < width; i++)
	{
		cout << (char)205;
	}
	cout << (char)188;
	textcolor(NORMAL_COLOR);
}

int controlMenu()
{
	int point_X = 40, point_Y = 5;
	gotoxy(point_X, point_Y);
	
	int i = 1;
	while (true)
	{
		char x = _getch();
		if(x == 'w' || x == 'W' || x == UP)
		{
			if(point_Y > 5)
			{
				--i;
				gotoxy(point_X, point_Y -= 2);
			}
		}
		else if(x == 's' || x == 'S' || x == DOWN)
		{
			if(point_Y < 14)
			{
				++i;
				gotoxy(point_X, point_Y += 2);
			}
		}
		else if(x == ENTER || x == SPACE)
			return i;
	}
}

void mainMenu()
{
	Border(35, 3);
	int point_X = 40, point_Y = 5;
	gotoxy(point_X, point_Y);
	cout << " NEW GAME ";
	gotoxy(point_X, point_Y += 2);
	cout << " LOAD GAME ";
	gotoxy(point_X, point_Y += 2);
	cout << " SETTINGS ";
	gotoxy(point_X, point_Y += 2);
	cout << " HELP ";
	gotoxy(point_X, point_Y += 2);
	cout << " ABOUT ";
	gotoxy(point_X, point_Y += 2);
	cout << " EXIT ";
	gameLogo();
	int options = controlMenu();
	switch (options)
	{
	case 1:
		clear();
		mainGame(0);
		break;
	case 2:
		clear();
		mainGame(1);
		break;
	case 3:
		clear();
		Settings();
		break;
	case 4:
		clear();
		Help();
		break;
	case 5:
		clear();
		About();
		break;
	case 6:
		clear();
		return;
	}
}

void drawBoard()
{
	gotoxy(START_X, START_Y);
	// Vẽ đường ngang trên
	cout << (char)218;// ┌
	for (int i = 0; i < COLS; i++)
	{
		if(i >= 1)
			cout << (char)194;
		for (int j = 0; j < WIDTH; j++)
			cout << (char)196; // ─
	}
	cout << (char)191; // ┐

	for (int i = 0; i < ROWS * 2; i++)
	{
		gotoxy(START_X, START_Y + i + 1);
		cout << (char)179;
		for (int j = 0; j < COLS; j++)
		{
			for (int k = 0; k < WIDTH; k++)
				cout << ' ';
			cout << (char)179;
		}

		if(i < ROWS * 2 - 1)
		{
			++i;
			gotoxy(START_X, START_Y + i + 1);
			cout << (char)195;
			for (int k = 0; k < COLS; k++)
			{
				for (int l = 0; l < WIDTH; l++)
					cout << (char)196;
				if(k < COLS - 1)
					cout << (char)197;
				else
					cout << (char)180;
			}
		}
	}

	// Vẽ đường ngang dưới
	gotoxy(START_X, START_Y + ROWS * 2);
	cout << (char)192; // └
	for (int i = 0; i < COLS; i++)
	{
		if(i >= 1)
			cout << (char)193;
		for (int j = 0; j < WIDTH; j++)
			cout << (char)196; // ─
	}
	cout << (char)217; // ┘
}

void playerBox(char player, int color, int countXO, int point_X = END_X + 3, int point_Y = START_Y)
{
	gotoxy(END_X + 3 + 21, 5);
	textcolor(RED);
	cout << "PLAYER";
	gotoxy(END_X + 3 + 23, 7);
	textcolor(11);
	cout << "VS";
	gotoxy(END_X + 3 + 21, 9);
	textcolor(GREEN);
	cout << "PLAYER";

	int pre_x = point_X;
	int pre_y = point_Y;
	textcolor(11);
	gotoxy(point_X, point_Y);
	cout << (char)201;
	for (int i = 0; i < 18; i++) // width
		cout << (char)205;
	cout << (char)187;
	
	for (int i = 0; i < 10; i++) // heigth
	{
		gotoxy(point_X, ++point_Y);
		cout << (char)186;
		for (int j = 0; j < 18; j++)
		{
			cout << ' ';
		}
		cout << (char)186;
	}

	gotoxy(point_X, ++point_Y);
	cout << (char)200;
	for (int i = 0; i < 18; i++)
		cout << (char)205;
	cout << (char)188;
	textcolor(NORMAL_COLOR);

	point_X = pre_x;
	point_Y = pre_y;
	ifstream file_in;
	if(player == 'X')
	{
		file_in.open("Resources\\logo_x.txt", ios_base:: in);
		textcolor(color);
		while (!file_in.eof())
		{
			string str;
			getline(file_in, str);
			gotoxy(++point_X, ++point_Y);
			cout << str;
		}
		gotoxy(END_X + 3 + 5, 11);
		textcolor(NORMAL_COLOR);
		cout << "move's: ";
		textcolor(RED);
		cout << countXO;
	}
	else
	{
		file_in.open("Resources\\logo_o.txt", ios_base:: in);
		textcolor(color);
		while (!file_in.eof())
		{
			string str;
			getline(file_in, str);
			gotoxy(++point_X, ++point_Y);
			cout << str;
		}
		gotoxy(END_X + 3 + 35, 11);
		textcolor(NORMAL_COLOR);
		cout << "move's: ";
		textcolor(GREEN);
		cout << countXO;
		textcolor(NORMAL_COLOR);
	}
	file_in.close();
}

bool quitGame()
{
	gotoxy(END_X + 3 + 10, 26);
	textcolor(PURPLE);
	cout << "Are your sure to quit game ?";
	int x = END_X + 3 + 15, y = 27;
	gotoxy(x, y);
	cout << "YES           NO"; // 12
	bool flag = 1;
	gotoxy(x, y);
	while (true)
	{
		char c;
		c = _getch();
		if (c == 'a' || c == 'A' || c == LEFT)
		{
			if(x > END_X + 3 + 15)
			{
				gotoxy(x -= 14, y);
			}
			flag = 1;
		}
		else if(c == 'd' || c == 'D' || c == RIGHT)
		{
			if(x == END_X + 3 + 15)
			{
				gotoxy(x += 14, y);
			}
			flag = 0;
		}
		else if(c == SPACE || c == ENTER)
		{
			if(flag == true)
			{
				clear();
				return true;
			}
			clear2(END_X, 25, 110, 26);
			textcolor(NORMAL_COLOR);
			break;
		}
	}
	return false;
}

bool continueGame()
{
	textcolor(PURPLE);
	gotoxy(78, 25);
	cout << "Are your want to continue ?";
	int x = 83, y = 26;
	gotoxy(x, y);
	cout << "YES        NO"; // 9
	gotoxy(x, y);
	textcolor(NORMAL_COLOR);
	bool flag = true;
	while (true)
	{
		char c;
		c = _getch();
		if(c == 'a' || c == 'A' || c == 75) // left
		{
			if(x > 83)
				gotoxy(x -= 10, y);
			flag = true;
		}
		else if(c == 'd' || c == 'D' || c == 77) // right
		{
			if(x == 83)
				gotoxy(x += 10, y);
			flag = false;
		}
		else if(c == 32 || c == 13)
		{
			if(flag)
				return true;
			break;
		}
	}
	return false;
}

int playerControl(char player)
{
	DWORD fdwMode; 
	COORD coord;
	DWORD Events;
	INPUT_RECORD InputRecord;
	/* 
       Step-1:
       Disable 'Quick Edit Mode' option
    */
	fdwMode = ENABLE_EXTENDED_FLAGS;
       if (! SetConsoleMode(hStdin, fdwMode) )
           ErrorExit("SetConsoleMode");

	    /* 
       Step-2:
       Enable the window and mouse input events,
       after you have already applied that 'ENABLE_EXTENDED_FLAGS'
       to disable 'Quick Edit Mode'
    */
    // Enable the window and mouse input events. 
    fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT; 
    if (! SetConsoleMode(hStdin, fdwMode) ) 
        ErrorExit("SetConsoleMode"); 
	// insert mouse code in here
	char playerMove;
	while (true)
	{
		int x, y;
		// Wait for the mouse events. 
		if (GetAsyncKeyState(VK_LBUTTON))
		{
			FlushConsoleInputBuffer(hStdin);
			 if ( !ReadConsoleInput(hStdin, &InputRecord, 1, &Events) )
				ErrorExit("ReadConsoleInput");

			 if (InputRecord.Event.MouseEvent.dwEventFlags == 0)
			 {
				 if(InputRecord.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
				 {
					 coord.X = InputRecord.Event.MouseEvent.dwMousePosition.X;
					 coord.Y = InputRecord.Event.MouseEvent.dwMousePosition.Y;
					 if(validateMouseClick(coord.X + 1, coord.Y + 1))
					 {
						move_X =  (coord.X - START_X) / (WIDTH  + 1);
						move_Y = (coord.Y- START_Y) / (HEIGHT + 1);
						x = START_X + WIDTH / 2 + 1 + move_X * (WIDTH + 1);
						y = START_Y + HEIGHT / 2 + 1 + move_Y * (HEIGHT + 1);
						gotoxy(x, y);
						goto GOTO_CLICK_MOUSE;
					 }
				 }
			 }
		}

		if(_kbhit())
		{
			playerMove = _getch();
			if(playerMove == 'w' || playerMove == 'W' || playerMove == UP)
			{
				if(move_Y > 0)
					move_Y--;
				x = START_X + WIDTH / 2 + 1 + move_X * (WIDTH + 1);
				y = START_Y + HEIGHT / 2 + 1 + move_Y * (HEIGHT + 1);
				gotoxy(x, y);
			}
			else if(playerMove == 's' || playerMove == 'S' || playerMove == DOWN)
			{
				if(move_Y < ROWS - 1)
					move_Y++;
				x = START_X + WIDTH / 2 + 1 + move_X * (WIDTH + 1);
				y = START_Y + HEIGHT / 2 + 1 + move_Y * (HEIGHT + 1);
				gotoxy(x, y);
			}
			else if(playerMove == 'a' || playerMove == 'A' || playerMove == LEFT)
			{
				if(move_X > 0)
					move_X--;
				x = START_X + WIDTH / 2 + 1 + move_X * (WIDTH + 1);
				y = START_Y + HEIGHT / 2 + 1 + move_Y * (HEIGHT + 1);
				gotoxy(x, y);
			}
			else if(playerMove == 'd' || playerMove == 'D' || playerMove == RIGHT)
			{
				if(move_X < COLS - 1)
					move_X++;
				x = START_X + WIDTH / 2 + 1 + move_X * (WIDTH + 1);
				y = START_Y + HEIGHT / 2 + 1 + move_Y * (HEIGHT + 1);
				gotoxy(x, y);
			}
			else if(playerMove == SPACE || playerMove == ENTER)
			{
GOTO_CLICK_MOUSE:
				
				if(board[move_Y][move_X] != PLAYER_O && board[move_Y][move_X] != PLAYER_X)
				{
					if(preMove_X != -1 && preMove_Y != -1)
					{
						x = START_X + WIDTH / 2 + 1 + preMove_X * (WIDTH + 1);
						y = START_Y + HEIGHT / 2 + 1 + preMove_Y * (HEIGHT + 1);
						gotoxy(x, y);
						if(player == PLAYER_X)
						{
							textcolor(GREEN);
							cout << PLAYER_O;
						}
						else 
						{
							textcolor(RED);
							cout << PLAYER_X;
						}
						x = START_X + WIDTH / 2 + 1 + move_X * (WIDTH + 1);
						y = START_Y + HEIGHT / 2 + 1 + move_Y * (HEIGHT + 1);
						gotoxy(x, y);
					}
					Point Step;

					preMove_X = move_X;
					preMove_Y = move_Y;

					Step.val = player;
					Step.x = move_Y;
					Step.y = move_X;
					UNDO_STACK.push(Step);

					board[move_Y][move_X] = player;

					if(player == PLAYER_X)
						textcolor(RED + 32);
					else 
						textcolor(GREEN + 32);
					cout << player;
					textcolor(NORMAL_COLOR);
					break;
				}
			}
			else if(playerMove == CTRL_U)
			{
				if(!UNDO_STACK.empty())
				{
					Undo(player);
					return 1;
				}
			}
			else if(playerMove == CTRL_Z)
			{
				if(!REDO_STACK.empty())
				{
					Redo(player);
					return 2;
				}
			}
			else if (playerMove == CTRL_S)
				return 3; // save game
			else if(playerMove == ESC)
			{
				if(quitGame())
					return -1;
				x = START_X + WIDTH / 2 + 1 + move_X * (WIDTH + 1);
				y = START_Y + HEIGHT / 2 + 1 + move_Y * (HEIGHT + 1);
				gotoxy(x, y);
			}
		}
	}
	return 0;
}
// Trung tâm xử lí các logic game
int processCore(char player, int count_x, int count_o)
{
	bool gameOver = true;
	while (gameOver)
	{
		//Clock();
		// xét tọa độ con trỏ ban đầu x, y nằm tại ô 0, 0
		int x = START_X + WIDTH / 2 + 1 + move_X * (WIDTH + 1);
		int y = START_Y + HEIGHT / 2 + 1 + move_Y * (HEIGHT + 1);
		gotoxy(x, y);
		int flag = playerControl(player);

		if(flag == -1) // nguoi choi thoat game
		{
			ResetGame();
			textcolor(NORMAL_COLOR);
			SetConsoleMode(hStdin, fdwSaveOldMode);
			return 1; // stop play game
		}
		gameOver = checkWinner(player, move_Y, move_X);

		if(flag == 1 && gameOver) // Undo
		{
			player == PLAYER_O ? player = PLAYER_X : player = PLAYER_O;
			if (player == PLAYER_X)
			{
				--count_x;
				playerBox(PLAYER_O, WHITE, count_o, END_X + 3 + 28);
				playerBox(PLAYER_X, RED, count_x);
			}
			else
			{
				--count_o;
				playerBox(PLAYER_X,WHITE, count_x);
				playerBox(PLAYER_O,GREEN, count_o, END_X + 3 + 28);
			}
		}
		else if(flag == 2 && gameOver) // Redo
		{
			if(player == PLAYER_O)
			{
				++count_x;
				player = PLAYER_X;
				playerBox(PLAYER_O, WHITE, count_o, END_X + 3 + 28);
				playerBox(PLAYER_X, RED, count_x);
			}
			else
			{
				++count_o;
				player = PLAYER_O;
				playerBox(PLAYER_X,WHITE, count_x);
				playerBox(PLAYER_O,GREEN, count_o, END_X + 3 + 28);
			}
		}
		else if(flag == 3)
		{
			if(SaveGame(player, count_o, count_x))
			{
				clear();
				ResetGame();
				SetConsoleMode(hStdin, fdwSaveOldMode);
				return -1;
			}
		}
		else
		{
			player == PLAYER_O ? player = PLAYER_X : player = PLAYER_O;
			if(player == PLAYER_X && gameOver)
			{
				count_o++;
				playerBox(PLAYER_O, WHITE, count_o, END_X + 3 + 28);
				playerBox(PLAYER_X, RED, count_x);
			}
			else if(player == PLAYER_O && gameOver)
			{
				count_x++;
				playerBox(PLAYER_X,WHITE, count_x);
				playerBox(PLAYER_O,GREEN, count_o, END_X + 3 + 28);
			}
		}
	}
	SetConsoleMode(hStdin, fdwSaveOldMode);
	return 0;
}

// mode = 0 => New Game
// mode = 1 => Load Game
void mainGame(int modeGame = 0)
{
	 while (true)
	{
		char player = PLAYER_O;
		int count_x = 0, count_o = 0;

		if(modeGame == 1)
		{
			int flag = LoadGame(player, count_o, count_x);
			if(flag == 0)
			{
				clear();
				break ; // comback to main menu
			}
			else if(flag == 2)
			{
				clear();
				break; // cancel
			}
			clear();
		}

		drawBoard();
		if (modeGame == 1)
			drawPreviousSteps();

		if(player == PLAYER_O)
		{
			playerBox(PLAYER_O, GREEN, count_o, END_X + 3 + 28);
			playerBox(PLAYER_X, WHITE, count_x);
		}
		else
		{
			playerBox(PLAYER_O, WHITE, count_o, END_X + 3 + 28);
			playerBox(PLAYER_X, RED, count_x);
		}
		caroLogo();
		textcolor(YELLOW_BACKGROUND);
		gotoxy(END_X + 3, 16);
		cout << "UNDO: CTRL + U";
		gotoxy(END_X + 3 * 9 + 2, 16);
		cout << "REDO: CTRL + Z";
		gotoxy(END_X + 3, 18);
		cout << "QUIT GAME: ESC";
		gotoxy(END_X + 3 * 9 + 2, 18);
		cout << "SAVE GAME: CTRL + S";

		int flag = processCore(player, count_x, count_o);

		if(flag == -1) // load game fail
			break;
		else if(flag == 1) // quit game
			break;

		if(continueGame()) // tiep tuc choi
		{
			clear();
			modeGame = 0;
			ResetGame();
		}
		else // thoat ra menu chinh
		{
			clear();
			ResetGame();
			break;
		}
	}
	mainMenu();
}

bool validateMouseClick(int x, int y)
{
	if ( (x >= START_X && x <= END_X) && ( y >= START_Y && y <= END_Y))
	{
		if ((x - START_X) % (WIDTH  + 1) != 0 && (y - START_Y) % (HEIGHT + 1) != 0)
		{
			return true;
		}
	}
	return false;
}

void ResetGame()
{
	while (!UNDO_STACK.empty())
		UNDO_STACK.pop();
	while (!REDO_STACK.empty())
		REDO_STACK.pop();
	move_X = move_Y = 0;
	preMove_X = preMove_Y = -1; 
	fill((char *)board, (char *)board + sizeof(board), ' ');
}
void Undo(char player)
{
	Point temp = UNDO_STACK.top();
	REDO_STACK.push(temp);
	UNDO_STACK.pop();
	move_X = temp.y;
	move_Y = temp.x;

	// xoa diem hien tai di
	int x = START_X + WIDTH / 2 + 1 + move_X * (WIDTH + 1);
	int y = START_Y + HEIGHT / 2 + 1 + move_Y * (HEIGHT + 1);
	gotoxy(x, y);
	cout << ' ';
	board[move_Y][move_X] = ' ';

	if (!UNDO_STACK.empty())
	{
		temp = UNDO_STACK.top();
		preMove_X = temp.y;
		preMove_Y = temp.x;
		move_X = preMove_X;
		move_Y = preMove_Y;

		x = START_X + WIDTH / 2 + 1 + move_X * (WIDTH + 1);
		y = START_Y + HEIGHT / 2 + 1 + move_Y * (HEIGHT + 1);
		gotoxy(x, y);
		if (player == PLAYER_O) // X
		{
			textcolor(GREEN + 32);
			cout << PLAYER_O;
		}
		else // O
		{
			textcolor(RED + 32);
			cout << PLAYER_X;
		}
	}
	else
	{
		preMove_X = -1;
		preMove_Y = -1;
	}

}
VOID ErrorExit (LPSTR lpszMessage) 
{ 
    fprintf(stderr, "%s\n", lpszMessage); 
    // Restore input mode on exit.
    SetConsoleMode(hStdin, fdwSaveOldMode);
    ExitProcess(0); 
}
void Redo(char player)
{
	if(!REDO_STACK.empty())
	{
		Point temp = REDO_STACK.top();
		UNDO_STACK.push(temp);
		REDO_STACK.pop();
		board[temp.x][temp.y] = temp.val;

		int x = START_X + WIDTH / 2 + 1 + move_X * (WIDTH + 1);
		int y = START_Y + HEIGHT / 2 + 1 + move_Y * (HEIGHT + 1);
		gotoxy(x, y);

		if(player == PLAYER_O)
		{
			textcolor(RED);
			cout << PLAYER_X;
		}
		else
		{
			textcolor(GREEN);
			cout << PLAYER_O;
		}

		move_X = temp.y;
		move_Y = temp.x;
		preMove_X = move_X;
		preMove_Y = move_Y;
		x = START_X + WIDTH / 2 + 1 + move_X * (WIDTH + 1);
		y = START_Y + HEIGHT / 2 + 1 + move_Y * (HEIGHT + 1);
		gotoxy(x, y);
	
		if(temp.val == PLAYER_O)
			textcolor(GREEN + 32);
		else
			textcolor(RED + 32);
		cout << temp.val;
		textcolor(NORMAL_COLOR);
	}
}
bool SaveGame(char player, int count_o, int count_x)
{
	vector<string> files;
	ifstream fin("Resources\\saveGames.txt");
	while (!fin.eof())
	{
		string str;
		getline(fin, str);

		if(str != "")
			files.push_back(str);
	}
	fin.close();

	string fileName;
	int x = END_X + 3;
	int y = 26;
	gotoxy(x, y);
	string content = "Enter save file name (*.txt): ";
	cout << content;
	x += content.length();

	while (true)
	{
		char c = _getch();

		if(c == ESC)
		{
			clear2(END_X + 1, y - 1, END_X + 3 * 18, y + 3);
			int x = START_X + WIDTH / 2 + 1 + move_X * (WIDTH + 1);
			int y = START_Y + HEIGHT / 2 + 1 + move_Y * (HEIGHT + 1);
			gotoxy(x, y);

			return false;
		}
		if (c == ENTER)
		{
			if(!fileName.empty())
			{
				int len = fileName.length();
				if(len > 6 && fileName[len - 1] == 't' && fileName[len - 2] == 'x' 
					&& fileName[len - 3] == 't' && fileName[len - 4] == '.')
				{
					if(files.empty())
						break;
					else
					{
						int size = files.size();
						bool flag = false;
						for (int i = 0; i < size; i++)
						{
							if(files[i] == fileName)
							{
								gotoxy(END_X + 3, y + 2);
								textcolor(RED);
								cout << "Bi trung ten file vui long nhap lai!";
								textcolor(NORMAL_COLOR);
								Sleep(1000);
								clear2(END_X + 2, y + 1, END_X + 3 * 15, y + 3);
								gotoxy(x, y);
								flag = true;
								break;
							}
						}
						if(!flag) break;
					}
				}
				else
				{
					gotoxy(END_X + 3, y + 2);
					textcolor(RED);
					cout << "Nhap ten file khong hop le!";
					textcolor(NORMAL_COLOR);
					Sleep(1000);
					clear2(END_X + 2, y + 1, END_X + 3 * 15, y + 3);
					gotoxy(x, y);
				}
			}	
		}

		int length = 0;
		if(!fileName.empty())
			length = fileName.length();

		if(((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '.') && length <= 15)
		{
			gotoxy(x++, y);
			cout << c;
			fileName.push_back(c);
		}

		if(c == BACKSPACE)
		{
			if(!fileName.empty())
			{
				gotoxy(--x, y);
				cout << ' ';
				fileName.pop_back();
			}
		}
	}
	string linkFile = "Resources\\";
	linkFile += fileName;
	ofstream fout(linkFile);

	fout << player << endl;
	fout << count_o << ' ' << count_x << endl;
	fout << preMove_Y << ' ' << preMove_X << endl;

	for (int i = 0; i < ROWS * COLS; ++i)
	{
		if(board[i / COLS][i % COLS] == 'X' || board[i / COLS][i % COLS] == 'O')
			fout << board[i / COLS][i % COLS];
		else 
			fout << 0;
		if((i + 1) % COLS == 0)
			fout << "\n";
	}
	fout.close();
	fout.open("Resources\\saveGames.txt", ios:: app);
	fout << "\n" << fileName;
	fout.close();
	return true;
}
void drawPreviousSteps()
{
	for (int i = 0; i < ROWS; ++i)
	{
		for (int j = 0; j < COLS; ++j)
		{
			if(board[i][j] == PLAYER_O || board[i][j] == PLAYER_X)
			{
				int x = START_X + WIDTH / 2 + 1 + j * (WIDTH + 1);
				int y = START_Y + HEIGHT / 2 + 1 + i * (HEIGHT + 1);
				gotoxy(x, y);
				if(board[i][j] == 'O')
				{
					if(i == preMove_Y && j == preMove_X)
						textcolor(GREEN + 32);
					else
						textcolor(GREEN);
					cout << PLAYER_O;
				}
				else
				{
					if(i == preMove_Y && j == preMove_X)
						textcolor(RED + 32);
					else
						textcolor(RED);
					cout << PLAYER_X;
				}
			}
		}
	}
}
int printFiles(vector<string> files, int x, int y)
{
	int length = files.size();
	for (int i = 0; i < length; i++)
	{
		gotoxy(x, y);
		cout << files[i];
		y += 2;
	}
	return y;
}
int LoadGame(char &player, int &count_o, int &count_x)
{
	vector<string> files;
	ifstream fin("Resources\\saveGames.txt");
	while (!fin.eof())
	{
		string str;
		getline(fin, str);
		if(str != "")
			files.push_back(str);
	}
	fin.close();

	// Draw logo
	fin.open("Resources\\loadGameLogo.txt", ios:: in);
	string str;
	int myX = 30, myY = 0;
	textcolor(139);
	while (!fin.eof())
	{
		getline(fin, str);
		gotoxy(myX, myY++);
		cout << str;
	}
	textcolor(NORMAL_COLOR);
	fin.close();

	int x = 43, y = 9, width = 25, height = 15;
	Border(x, y, width, height);

	x = 48, y = 11;
	
	if (!files.empty())
	{
		int temp_x = x, temp_y = y;
		int rangeStartY = y;
		int rangeEndY = printFiles(files, x, y);
		gotoxy(x, y);
		int idx_move = 0;

		int length = files.size();
		// control 
		while (true)
		{
			char c = _getch();
			if (c == UP || c == 'W' || c == 'w')
			{
				if(idx_move > 0)
					--idx_move;
				if (temp_y > rangeStartY)
				{
					temp_y -= 2;
					gotoxy(temp_x, temp_y);
				}
			}
			else if(c == DOWN || c == 'S' || c == 's')
			{
				if(idx_move < length - 1)
					++idx_move;
				if (temp_y < rangeEndY - 2)
				{
					temp_y += 2;
					gotoxy(temp_x, temp_y);
				}
			}
			else if(c == ENTER || c == SPACE)
			{
				string linkFile = "Resources\\";
				linkFile += files[idx_move];
				fin.open(linkFile, ios:: in);
				break;
			}
			else if(c == CTRL_D) // delete file
			{
				// Hỏi người dùng có chắc chắn muốn xóa file đó hay không?
				// YES     NO
				string linkFile = "Resources\\";
				linkFile += files[idx_move];
				int check = remove(linkFile.c_str());
				if(check == 0)
				{
					int temp_length = files.size();

					files.erase(files.begin() + idx_move);
					ofstream fout("Resources\\saveGames.txt", ios:: out);
					for (auto it = files.begin(); it != files.end(); ++it)
						fout << *it << endl;
					fout.close();

					if(files.empty())
						return 2;

					clear2(x - 1, y - 1, 60, 20);
					rangeEndY = printFiles(files, x, y);
					length = files.size();
					gotoxy(x, y);
				}
			}
			else if(c == ESC)
				return 2; // cancel
		}

		fin >> player;
		fin >> count_o >> count_x;
		fin >> preMove_Y >> preMove_X;

		preMove_X != -1 ? move_X = preMove_X : move_X = 0;
		preMove_Y != -1 ? move_Y = preMove_Y : move_Y = 0;
		for (int i = 0; i < ROWS * COLS; i++)
			fin >> board[i / COLS][i % COLS];
		fin.close();
		return 1;
	}

	gotoxy(x, y);
	textcolor(RED);
	cout << "Empty file!";
	textcolor(NORMAL_COLOR);
	_getch();
	return 0;
}
void HieuUngChienThangDong(char player, int row, int col)
{
	for (int color = 10; color <= 15; color++)
	{
		textcolor(color);
		for (int j = col; j >= 0 ; --j)
		{
			if(board[row][j] == player)
			{
				int x = START_X + WIDTH / 2 + 1 + j * (WIDTH + 1);
				int y = START_Y + HEIGHT / 2 + 1 + row * (HEIGHT + 1);
				gotoxy(x, y);
				cout << player;
			}
			else break;
		}

		for (int j = col + 1; j < COLS; ++j)
		{
			if(board[row][j] == player)
			{
				int x = START_X + WIDTH / 2 + 1 + j * (WIDTH + 1);
				int y = START_Y + HEIGHT / 2 + 1 + row * (HEIGHT + 1);
				gotoxy(x, y);
				cout << player;
			}
			else break;
		}
		Sleep(200);
	}
}
void HieuUngChienThangCot(char player, int row, int col)
{
	for (int color = 10; color <= 15; color++)
	{
		textcolor(color);
		for (int i = row; i >= 0; --i)
		{
			if (board[i][col] == player)
			{
				int x = START_X + WIDTH / 2 + 1 + col * (WIDTH + 1);
				int y = START_Y + HEIGHT / 2 + 1 + i * (HEIGHT + 1);
				gotoxy(x, y);
				cout << player;
			}
			else break;
		}

		for (int i = row + 1; i < ROWS; ++i)
		{
			if (board[i][col] == player)
			{
				int x = START_X + WIDTH / 2 + 1 + col * (WIDTH + 1);
				int y = START_Y + HEIGHT / 2 + 1 + i * (HEIGHT + 1);
				gotoxy(x, y);
				cout << player;
			}
			else break;
		}
		Sleep(200);
	}
}
void HieuUngChienThangCheoChinh(char player, int row, int col)
{
	for (int color = 10; color <= 15; color++)
	{
		textcolor(color);
		int i = row, j = col;
		while (i >= 0 && j >= 0)
		{
			if (board[i][j] == player)
			{
				int x = START_X + WIDTH / 2 + 1 + j * (WIDTH + 1);
				int y = START_Y + HEIGHT / 2 + 1 + i * (HEIGHT + 1);
				gotoxy(x, y);
				cout << player;
			}
			else
				break;
			--i;
			--j;
		}

		i = row + 1;
		j = col + 1;
		while (i < ROWS && j < COLS)
		{
			if (board[i][j] == player)
			{
				int x = START_X + WIDTH / 2 + 1 + j * (WIDTH + 1);
				int y = START_Y + HEIGHT / 2 + 1 + i * (HEIGHT + 1);
				gotoxy(x, y);
				cout << player;
			}
			else
				break;
			++i;
			++j;
		}
		Sleep(200);
	}
}
void HieuUngChienThangCheoPhu(char player, int row, int col)
{
	for (int color = 10; color <= 15; color++)
	{
		textcolor(color);
		int i = row, j = col;
		while (i >= 0 && j < COLS)
		{
			if(board[i][j] == player)
			{
				int x = START_X + WIDTH / 2 + 1 + j * (WIDTH + 1);
				int y = START_Y + HEIGHT / 2 + 1 + i * (HEIGHT + 1);
				gotoxy(x, y);
				cout << player;
			}
			else
				break;
			--i;
			++j;
		}

		i = row + 1;
		j = col - 1;
		while (i < ROWS && j >= 0)
		{
			if(board[i][j] == player)
			{
				int x = START_X + WIDTH / 2 + 1 + j * (WIDTH + 1);
				int y = START_Y + HEIGHT / 2 + 1 + i * (HEIGHT + 1);
				gotoxy(x, y);
				cout << player;
			}
			else
				break;
			++i;
			--j;
		}
		Sleep(200);
	}
}