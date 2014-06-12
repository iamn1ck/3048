#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>
#include "HID.h"



#define SIZE 4
uint32_t score = 0;

void getColor(uint16_t value, char *color, size_t length) {
	uint8_t original[] = { 8, 255, 1, 255, 2, 255, 3, 255, 4, 255, 5, 255, 6, 255, 7, 255, 9, 0, 10, 0, 11, 0, 12, 0, 13, 0, 14, 0, 255, 0, 255, 0 };
	//uint8_t blackwhite[] = {232,255,234,255,236,255,238,255,240,255,242,255,244,255,246,0,248,0,249,0,250,0,251,0,252,0,253,0,254,0,255,0};
	//uint8_t bluered[] = {235,255,63,255,57,255,93,255,129,255,165,255,201,255,200,255,199,255,198,255,197,255,196,255,196,255,196,255,196,255,196,255};
	uint8_t *scheme = original;
	uint8_t *background = scheme + 0;
	uint8_t *foreground = scheme + 1;
	if (value > 0) while (value >>= 1) {
		if (background + 2<scheme + sizeof(original)) {
			background += 2;
			foreground += 2;
		}
	}
	snprintf(color, length, "\033[38;5;%d;48;5;%dm", *foreground, *background);
}

struct board
{
	int xcoord;
	int ycoord;

	bool isupdate = 1;
}

void drawBoard(uint16_t board[SIZE][SIZE]) {
	int8_t x, y;
	char color[40], reset[] = "\033[m";
	printf("\033[H");

	printf("2048.c %17d pts\n\n", score);

	draw_fillrect(400, 20, 600, 220, 0, 255, 0, TOP_SCREEN);

	int x = 400;
	int y = 220;

	for (i = 0; i < 4; ++i){}
		for (j = 0; j < 4; ++j){
			draw_fillrect(x, y, x + 50, y - 50, 255, 255, 0, TOP_SCREEN);
			x = x + 50;
		}
		y = y - 50;
	}

	for (y = 0; y<SIZE; y++) {
		for (x = 0; x<SIZE; x++) {
			getColor(board[x][y], color, 40);
			printf("%s", color);
			printf("       ");
			printf("%s", reset);
		}
		printf("\n");
		for (x = 0; x<SIZE; x++) {
			getColor(board[x][y], color, 40);
			printf("%s", color);
			if (board[x][y] != 0) {
				char s[8];
				snprintf(s, 8, "%u", board[x][y]);
				int8_t t = 7 - strlen(s);
				printf("%*s%s%*s", t - t / 2, "", s, t / 2, "");
			}
			else {
				printf("   ·   ");
			}
			printf("%s", reset);
		}
		printf("\n");
		for (x = 0; x<SIZE; x++) {
			getColor(board[x][y], color, 40);
			printf("%s", color);
			printf("       ");
			printf("%s", reset);
		}
		printf("\n");
	}
	printf("\n");
	printf("        ←,↑,→,↓ or q        \n");
	printf("\033[A");
}

int8_t findTarget(uint16_t array[SIZE], int8_t x, int8_t stop) {
	int8_t t;
	// if the position is already on the first, don't evaluate
	if (x == 0) {
		return x;
	}
	for (t = x - 1; t >= 0; t--) {
		if (array[t] != 0) {
			if (array[t] != array[x]) {
				// merge is not possible, take next position
				return t + 1;
			}
			return t;
		}
		else {
			// we should not slide further, return this one
			if (t == stop) {
				return t;
			}
		}
	}
	// we did not find a
	return x;
}

bool slideArray(uint16_t array[SIZE]) {
	bool success = false;
	int8_t x, t, stop = 0;

	for (x = 0; x<SIZE; x++) {
		if (array[x] != 0) {
			t = findTarget(array, x, stop);
			// if target is not original position, then move or merge
			if (t != x) {
				// if target is not zero, set stop to avoid double merge
				if (array[t] != 0) {
					score += array[t] + array[x];
					stop = t + 1;
				}
				array[t] += array[x];
				array[x] = 0;
				success = true;
			}
		}
	}
	return success;
}

void rotateBoard(uint16_t board[SIZE][SIZE]) {
	int8_t i, j, n = SIZE;
	uint16_t tmp;
	for (i = 0; i<n / 2; i++){
		for (j = i; j<n - i - 1; j++){
			tmp = board[i][j];
			board[i][j] = board[j][n - i - 1];
			board[j][n - i - 1] = board[n - i - 1][n - j - 1];
			board[n - i - 1][n - j - 1] = board[n - j - 1][i];
			board[n - j - 1][i] = tmp;
		}
	}
}

bool moveUp(uint16_t board[SIZE][SIZE]) {
	bool success = false;
	int8_t x;
	for (x = 0; x<SIZE; x++) {
		success |= slideArray(board[x]);
	}
	return success;
}

bool moveLeft(uint16_t board[SIZE][SIZE]) {
	bool success;
	rotateBoard(board);
	success = moveUp(board);
	rotateBoard(board);
	rotateBoard(board);
	rotateBoard(board);
	return success;
}

bool moveDown(uint16_t board[SIZE][SIZE]) {
	bool success;
	rotateBoard(board);
	rotateBoard(board);
	success = moveUp(board);
	rotateBoard(board);
	rotateBoard(board);
	return success;
}

bool moveRight(uint16_t board[SIZE][SIZE]) {
	bool success;
	rotateBoard(board);
	rotateBoard(board);
	rotateBoard(board);
	success = moveUp(board);
	rotateBoard(board);
	return success;
}

bool findPairDown(uint16_t board[SIZE][SIZE]) {
	bool success = false;
	int8_t x, y;
	for (x = 0; x<SIZE; x++) {
		for (y = 0; y<SIZE - 1; y++) {
			if (board[x][y] == board[x][y + 1]) return true;
		}
	}
	return success;
}

int16_t countEmpty(uint16_t board[SIZE][SIZE]) {
	int8_t x, y;
	int16_t count = 0;
	for (x = 0; x<SIZE; x++) {
		for (y = 0; y<SIZE; y++) {
			if (board[x][y] == 0) {
				count++;
			}
		}
	}
	return count;
}

bool gameEnded(uint16_t board[SIZE][SIZE]) {
	bool ended = true;
	if (countEmpty(board)>0) return false;
	if (findPairDown(board)) return false;
	rotateBoard(board);
	if (findPairDown(board)) ended = false;
	rotateBoard(board);
	rotateBoard(board);
	rotateBoard(board);
	return ended;
}

void addRandom(uint16_t board[SIZE][SIZE]) {
	static bool initialized = false;
	int8_t x, y;
	int16_t r, len = 0;
	uint16_t n, list[SIZE*SIZE][2];

	if (!initialized) {
		srand(time(NULL));
		initialized = true;
	}

	for (x = 0; x<SIZE; x++) {
		for (y = 0; y<SIZE; y++) {
			if (board[x][y] == 0) {
				list[len][0] = x;
				list[len][1] = y;
				len++;
			}
		}
	}

	if (len>0) {
		r = rand() % len;
		x = list[r][0];
		y = list[r][1];
		n = ((rand() % 10) / 9 + 1) * 2;
		board[x][y] = n;
	}
}



void wait(int t){
	int i = 0;
	int j = 0;
	int z = 0;

	for (i = 0; i < t; i++) {
		for (j = 0; j < t; j++) {
			z = i / 33;
		}
	}

}

char key_press(){

	button_press = read_word(HID);
	if ( (!(button_press & BUTTON_LEFT) | !(button_press & BUTTON_Y)) ){
		return 'l';
	}
	if ( (!(button_press & BUTTON_RIGHT) | !(button_press & BUTTON_A)) ){
		return 'r';
	}
	if ( (!(button_press & BUTTON_UP) | !(button_press & BUTTON_X)) ){
		return 'u';
	}
	if ( (!(button_press & BUTTON_DOWN) | !(button_press & BUTTON_B)) ){
		return 'd';
	}
	if (!(button_press & BUTTON_START)){
		return 's';
	}
	if (!(button_press & BUTTON_Select)){
		return 'n';
	}


}

int main()
{
	uint16_t board[SIZE][SIZE];
	char currkey;
	bool success;
	

	memset(board, 0, sizeof(board));
	addRandom(board);
	addRandom(board);
	drawBoard(board);
	while (true) {
		currkey = keypress();
		switch (currkey) {
		case 'l':	// left arrow
			success = moveLeft(board);  break;
		case 'r':	// right arrow
			success = moveRight(board); break;
		case 'u':	// up arrow
			success = moveUp(board);    break;
		case 'd':	// down arrow
			success = moveDown(board);  break;
		default: success = false;
		}
		if (success) {
			drawBoard(board);
			wait(5000); //make this wait function
			addRandom(board);
			drawBoard(board);
			if (gameEnded(board)) {
				printf("         GAME OVER          \n"); // draw game over or soemthing
				break;
			}
		}
		if (currkey == 'q') {
			printf("        QUIT? (a/b)         \n"); //bot screen
			while (true) {
				c = keypress();
				if (c == 'a'){
					printf("\033[?25h");
					exit(0);
				}
				else {
					drawBoard(board);
					break;
				}
			}
		}
		if (c == 'n') {
			printf("       RESTART? (y/n)       \n"); //bot screen
			while (true) {
				c = getchar();
				if (c == 'a'){
					memset(board, 0, sizeof(board));
					addRandom(board);
					addRandom(board);
					drawBoard(board);
					break;
				}
				else {
					drawBoard(board);
					break;
				}
			}
		}
	}

	return 0;
}