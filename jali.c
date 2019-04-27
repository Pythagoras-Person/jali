#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

//changeable
#define DBF 
#define MX  520
#define MY  520
#define MZ  30

//automatic
#define AX MX / 4
#define AY MY / 27
#define SX MX / 8
#define SY MY / 8
#define EY MY - SY

//this creates a Texture from a file
SDL_Texture *mkTex(SDL_Renderer *r, char *p) {
	SDL_Surface *tmp = IMG_Load(p);
	SDL_Texture *rtn = SDL_CreateTextureFromSurface(r, tmp);
	SDL_FreeSurface(tmp);
	return rtn;
}

//this makes the 4 lines wich always show up
void dbl(SDL_Renderer *rend) {
	SDL_SetRenderDrawColor(rend, 0, 0, 0, 0);
	for (int i = 0; i < 4; i++) {
		int plus = i * AX;
		SDL_RenderDrawLine(rend, SX + plus, SY, SX + plus, EY);
	}
	SDL_SetRenderDrawColor(rend, 255, 255, 255, 0);
}

//this holy complicated pice of shit just reads the mapfile and store the values in a static int array
int (*loadmap(char *fn))[3][MZ][2] {
	FILE *fp = fopen(fn, "r");
	static int lm[3][MZ][2];
	char ch;
	char toc[2];
	int nc = 0; 
	int lc = 0;
	int ln[3] = {0, 0, 0};
	int ns[3] = {0, 0, 0};
	while ((ch = fgetc(fp)) != EOF) {
		if (ch != ',' && ch != '\n') {
			toc[nc] = ch;
			nc++;
		} else if (ch == ',' || ch == '\n') {
			if (nc == 1) {
				toc[1] = toc[0];
				toc[0] = '0';
			}
			ln[lc] = atoi(toc);
			nc = 0;
			lc++;
		} if (ch == '\n') {
			lm[ln[0]][ns[ln[0]]][0] = ln[1];
			lm[ln[0]][ns[ln[0]]][1] = ln[2];
			ns[ln[0]]++;
			lc = 0;
		}
	}
	fclose(fp);
	return &lm;
}

//this funktion moves an objekt to an position
int moveto(SDL_Rect *target, int *move[6]) {
	int erg;
	float f;
	if ((*move)[0] == (*move)[4 + (*move)[1]] || -(*move)[0] == (*move)[4 + (*move)[1]]) return 1;
	f = ((float)(*move)[0] / (float)(*move)[4 + (*move)[1]]) * (float)(*move)[5 - (*move)[1]];
	if (f - (int)f < .5) {
		erg = (int)f;
	} else {
		erg = (int)f + 1;
	}
	if ((*move)[1]) {
		if ((*move)[5] < 0) {
			target->y--;
		} else {
			target->y++;
		}
		target->x = (*move)[2] + erg;
	} else {
		if ((*move)[4] < 0) {
			target->x--;
		} else {
			target->x++;
		}
		target->y = (*move)[3] + erg;
	}
	if ((*move)[4 + (*move)[1]] < 0) (*move)[0]--; else (*move)[0]++;
	return 0;
}	

//generate a array wich is importend for thread moving
int *mkmove(int x1, int y1, int xs, int ys) {
	static int lo[6];
//	int xs = x2 - x1;
//	int ys = y2 - y1;
	int vs = 0;
	int xa, ya;
	if (xs < 0) xa = -xs; else xa = xs;
	if (ys < 0) ya = -ys; else ya = ys;
	if (xa < ya) vs = 1;
	lo[0] = 0;
	lo[1] = vs;
	lo[2] = x1;
	lo[3] = y1;
	lo[4] = xs;
	lo[5] = ys;
	return lo;
}

void cbl(SDL_Renderer *rend, int (*mapa)[3][MZ][2]) {
	SDL_SetRenderDrawColor(rend, 0, 0, 0, 0);
	int cnt, plus;
	int *ch;
	for (int i = 0; i < 3; i++) {
		cnt = 0;
		plus = (AX * i);
		while (1) {
			ch = (*mapa)[i][cnt];
			if (!ch[0] && !ch[1]) break;
			SDL_RenderDrawLine(rend, SX + plus, SY + (ch[0] * AY), SX + plus + AX, SY + (ch[1] * AY));
			cnt++;
		}
	}
	SDL_SetRenderDrawColor(rend, 255, 255, 255, 0);
}

//get the coords of the next horizontal line
int *getLine(int pos[2], int (*mapa)[3][MZ][2]) {
	static int lia[3];
	int a, b = 64, c = 0;
	if (pos[0] == 0) {
		lia[0] = 0;
	} else if (pos[0] == MZ - 1) {
		lia[0] = 1;
	} else {
		for (int d = 0; d < 2; d++) {
			for (int i = 0; i < MZ; i++) {
				a = (*mapa)[pos[0] - d][i][d];
				if (a == 0) {
					break;
				} else if (a < b && a > pos[1]) {
					c = i;
					b = a;
					lia[0] = d;
				}
			}
		}
	}
	if (b == 64) { 
		for (int i = 0; i < MZ; i++) {
			a = (*mapa)[pos[0] - lia[0]][i][lia[0]];
			if (a == 0) {
				break;
			} else if (a < b && a > pos[1]) {
				c = i;
				b = a;
			}
		}
	}
	lia[1] = b;
	lia[2] = (*mapa)[pos[0] - lia[0]][c][1 - lia[0]];
	return lia;
}

//well this is the funktion wich will be executed first ... you know
int main(int argc, char *argv[]) {
	//errorexeption... beehh
	if (argc != 3 && argc != 2) {perror("fuck you ... thats not the right number of args!"); exit(1);}
	int stpo = 0;
	if (argc == 3) {
		if (strlen(argv[2]) > 1) {perror("come on ... write only one single char!"); exit(1);}
		if ((int)argv[2][0] < 48 || (int)argv[2][0] > 58) {perror("dude ... numbers doesnt work like that..."); exit(1);}
		if (atoi(argv[2]) > 3) {perror("holy crap ... this number is even with one char to big, you ashole!"); exit(1);}
		stpo = atoi(argv[2]);
	}
	//init
		//system
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	SDL_Window *mwin = SDL_CreateWindow("Japanisches Glücksspiel", 0, 0, MX, MY, 0);
	SDL_Renderer *render = SDL_CreateRenderer(mwin, -1, 0);
		//vars
	int i = AY;
	int slide = 0;
	int imgsize = AX / 2;
	int himgs = imgsize / 2;
	int run = 1;
	SDL_Event e;
	int (*map)[3][MZ][2] = loadmap(argv[1]);
	int spot[2] = {stpo, 0};
	int *nline = getLine(spot, map);
	int *move;
	int gvl = 1;
		//images
	SDL_Texture *img = mkTex(render, "tux.png");
		//Rects
	SDL_Rect pos = {SX + (AX * spot[0]),  SY};
	SDL_Rect ros = {pos.x - himgs, pos.y - himgs, imgsize, imgsize};
	//main ##############################################################################
	SDL_SetRenderDrawColor(render, 255, 255, 255, 0);
	while (1) {
		//event handling
		SDL_PollEvent(&e);
		switch (e.type) {
		case SDL_QUIT:
			run = 0;
		case SDL_KEYDOWN:
			switch (e.key.keysym.sym) {
			case SDLK_ESCAPE:
				run = 0;
			}
		}
		if (!run) break;
		//actual programm
		if (pos.y < EY) {
			if (slide) {
				if (moveto(&pos, &move)) {
					slide = 0;
					if (nline[0]) spot[0]--; else spot[0]++;
					spot[1] = nline[2];
					nline = getLine(spot, map);
				}
			} else {
				if (i == 0) {
					spot[1]++;
					if (nline[1] == spot[1]) {
						if (nline[0]) gvl = -AX; else gvl = AX;
						move = mkmove(pos.x, pos.y, gvl, AY * (nline[2] - spot[1]));
						slide = 1;
					}
					i = AY;
				} else {
					i--;
					pos.y++;
				}
			}
		}
		//graphic handling
		SDL_RenderClear(render);
		dbl(render);
		cbl(render, map);
		ros.x = pos.x - himgs;
		ros.y = pos.y - himgs;
		SDL_RenderCopy(render, img, NULL, &ros);
		SDL_RenderPresent(render);
		SDL_Delay(20);
	// end main #########################################################################
	}
	//quit
	SDL_DestroyWindow(mwin);
	SDL_DestroyRenderer(render);
	SDL_DestroyTexture(img);
	SDL_Quit();
	return 0;
}
