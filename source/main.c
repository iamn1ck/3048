#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <stdbool.h>
#include "HID.h"
#include "memory.h"
#include "main.h"
#include "utils.h"
#include "draw.h"



#define SIZE 4
int score = 0, oldscore = 0;

struct Board
{
	int value;
	int xpos, ypos;
};

void init_board(struct Board board1[4][4])
{
	int i,j,x=92, y=229;
	for (i = 0; i < 4; i++){
		for (j = 0; j < 4; j++){
			board1[i][j].value = 0;
			board1[i][j].ypos = y;
			board1[i][j].xpos = x;
			x = x + 58;
		}
		x = 92;
		y = y - 58;
	}

}


void drawBoard(struct Board board1[4][4]) {
	int x, y;

	char c, blockscore[8], gamescore[8], oldgamescore[8];

	int i, j;
	for (i = 0; i < 4; i++){
		for (j = 0; j < 4; j++){
			draw_fillrect(board1[i][j].xpos, board1[i][j].ypos, board1[i][j].xpos + 50, board1[i][j].ypos - 50, 255, 255, 0, TOP_SCREEN);
			if (board1[i][j].value != 0)
			{
				itoa(board1[i][j].value, blockscore);
				draw_string(blockscore, board1[i][j].xpos + 23, board1[i][j].ypos - 26, 0, 0, 255, TOP_SCREEN);
			}
		}
	}
	itoa(score, gamescore);
	draw_fillrect(70, 20, 70, 30, 90, 100, 225, BOTTOM_SCREEN);
	draw_string("score: ", 20, 20, 0, 0, 0, BOTTOM_SCREEN);
	draw_string(gamescore, 70, 20, 0, 0, 0, BOTTOM_SCREEN);
}


int findTarget(struct Board board[SIZE], int x, int stop) {
	int t;
	// if the position is already on the first, don't evaluate
	if (x == 0) {
		return x;
	}
	for (t = x - 1; t >= 0; t--) {
		if (board[t].value != 0) {
			if (board[t].value != board[x].value) {
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

bool slideArray(struct Board board[4]) {
	bool success = false;
	int x, t, stop = 0;

	for (x = 0; x<4; x++) {
		if (board[x].value != 0) {
			t = findTarget(board, x, stop);
			// if target is not original position, then move or merge
			if (t != x) {
				// if target is not zero, set stop to avoid double merge
				if (board[t].value != 0) {
					oldscore = score;
					score = score + board[t].value + board[x].value;
					stop = t + 1;
				}
				board[t].value += board[x].value;
				board[x].value = 0;
				success = true;
			}
		}
	}
	return success;
}

void rotateBoard(struct Board board1[4][4]) {
	int i, j, n = 4;
	int tmp;
	for (i = 0; i<n / 2; i++){
		for (j = i; j<n - i - 1; j++){
			tmp = board1[i][j].value;
			board1[i][j].value = board1[j][n - i - 1].value;
			board1[j][n - i - 1].value = board1[n - i - 1][n - j - 1].value;
			board1[n - i - 1][n - j - 1].value = board1[n - j - 1][i].value;
			board1[n - j - 1][i].value = tmp;
		}
	}
}

//we rotate the board instead of determining the logic to move left right or down because this is easier

bool moveUp(struct Board board[4][4]) {
	bool success = false;
	int x;
	for (x = 0; x<4; x++) {
		success |= slideArray(board[x]);
	}
	return success;
}

bool moveLeft(struct Board board[4][4]) {
	bool success;
	rotateBoard(board);
	success = moveUp(board);
	rotateBoard(board);
	rotateBoard(board);
	rotateBoard(board);
	return success;
}

bool moveDown(struct Board board[4][4]) {
	bool success;
	rotateBoard(board);
	rotateBoard(board);
	success = moveUp(board);
	rotateBoard(board);
	rotateBoard(board);
	return success;
}

bool moveRight(struct Board board[4][4]) {
	bool success;
	rotateBoard(board);
	rotateBoard(board);
	rotateBoard(board);
	success = moveUp(board);
	rotateBoard(board);
	return success;
}


bool findPairDown(struct Board board1[4][4]) {
	bool success = false;
	int x, y;
	for (x = 0; x<4; x++) {
		for (y = 0; y<4 - 1; y++) {
			if (board1[x][y].value == board1[x][y + 1].value) return true;
		}
	}
	return success;
}

int countEmpty(struct Board board[4][4]) {
	int x, y;
	int count = 0;
	for (x = 0; x<4; x++) {
		for (y = 0; y<4; y++) {
			if (board[x][y].value == 0) {
				count++;
			}
		}
	}
	return count;
}

bool gameEnded(struct Board board[4][4]) {
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

void addRandom(struct Board board1[4][4]) {
	static bool initialized = false;
	int x, y;
	int r, len = 0;
	int n, eligpiece[16][2]; //can put new piece here



	for (x = 0; x<4; x++) {
		for (y = 0; y<4; y++) {
			if (board1[x][y].value == 0) {
				eligpiece[len][0] = x;
				eligpiece[len][1] = y;
				len++;
			}
		}
	}

	if (len>0) {
		r = TIMER % len;
		x = eligpiece[r][0];
		y = eligpiece[r][1];
		n = ((TIMER % 10) / 9 + 1) * 2;
		board1[x][y].value = n;
	}
}

char key_press(){
	int button_press;
	button_press = read_word(HID);
	if ((!(button_press & BUTTON_LEFT) | !(button_press & BUTTON_Y))){
		return 'l';
	}
	if ((!(button_press & BUTTON_RIGHT) | !(button_press & BUTTON_A))){
		return 'r';
	}
	if ((!(button_press & BUTTON_UP) | !(button_press & BUTTON_X))){
		return 'u';
	}
	if ((!(button_press & BUTTON_DOWN) | !(button_press & BUTTON_B))){
		return 'd';
	}
	if (!(button_press & BUTTON_START)){
		return 'q';
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


int main(){

	clearscreen(TOP_SCREEN | BOTTOM_SCREEN);
	initscreens();
	struct Board board1[4][4];
	init_board(board1);
	addRandom(board1);
	addRandom(board1);
	drawBoard(board1);
	char HID_new;
	char buffer[32];
	bool success;
	while (true) {
		HID_new = key_press();
		switch (HID_new) {
		case 'l':	// left arrow
			success = moveUp(board1);  break;
		case 'r':	// right arrow
			success = moveDown(board1); break;
		case 'u':	// up arrow
			success = moveRight(board1);    break;
		case 'd':	// down arrow
			success = moveLeft(board1);  break;
		default: success = false;
		}

		if (success) {
			drawBoard(board1);
			addRandom(board1);
			drawBoard(board1);
			if (gameEnded(board1)) {
				initscreens();
				draw_string("new game?", 50, 50, 0, 0, 255, BOTTOM_SCREEN);
				initscreens(board1);
				init_board(board1);
				score = 0;
				oldscore = 0;
				addRandom(board1);
				addRandom(board1);
				drawBoard(board1);
			}
		}
		if (HID_new == 'q') {
			draw_string("new game? start", 50, 50, 0, 0, 255, BOTTOM_SCREEN);
			draw_string("continue    A/R", 50, 60, 0, 0, 255, BOTTOM_SCREEN);
			HID_new = '0';
			while (true) {
				HID_new = key_press();
				if (HID_new == 'q'){
					initscreens(board1);
					init_board(board1);
					addRandom(board1);
					addRandom(board1);
					drawBoard(board1);
					break;
				}
				if (HID_new == 'r') {
					drawBoard(board1);
					break;
				}

			}
		}
	}
	
		return 0;
}

void initscreens(){
	draw_fillrect(0, 0, 320, 240, 90, 100, 225, BOTTOM_SCREEN);
	draw_fillrect(0, 0, 400, 240, 90, 100, 225, TOP_SCREEN);
	draw_fillrect(84, 1, 324, 235, 0, 255, 0, TOP_SCREEN);
}
