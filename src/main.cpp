/*******************************************************************************************
*
*   raylib [core] example - Basic window
*
*   Welcome to raylib!
*
*   To test examples, just press F6 and execute raylib_compile_execute script
*   Note that compiled executable is placed in the same folder as .c file
*
*   You can find all basic examples on C:\raylib\raylib\examples folder or
*   raylib official webpage: www.raylib.com
*
*   Enjoy using raylib. :)
*
*   Example originally created with raylib 1.0, last time updated with raylib 1.0
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2013-2022 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include <cstdio>
#include <memory>
#include <vector>
#include "raylib.h"
#include "FastNoiseLite.h"
#include "bezier.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

void UpdateDrawFrame();

int two_numbers[2];
extern "C" {
	// extern int *my_js();
	extern int get_window_height();
	extern int get_window_width();

	int *testing() {
		return two_numbers;
	}
}

enum TileType {
	TILE_EMPTY,
	TILE_TEXTURE,
};	

struct Tile {
	TileType type;
	int textureId;
	int x;
	int y;
	int width;
	int height;
	float rotation;
	int alpha;
};

struct GameState {
	bool coral_activated;
	int coral_animation_x;
	int coral_animation_y;
	int coral_animation_done;

	Texture2D dolphinSheet;
	int dolphinSheetHorizontalCount;
	int dolphinSheetWidth;
	int dolphinSheetHorizontalStep;
};

#define TILE_MAP_SIZE 40
#define CORAL_ANIMATION_SPEED 5
// #define TILE_MAP_STEP 60

Shader bgShader;
int window_width;
int window_height;
GameState state;
std::vector<Texture2D> tileTextures;
std::vector<Tile> tiles;
Tile tileMap[TILE_MAP_SIZE][TILE_MAP_SIZE];
FastNoiseLite noise;

void setup()
{
	const char *bgVShader =
		"#version 100                       \n"
		"attribute vec3 vertexPosition;     \n"
		"attribute vec2 vertexTexCoord;     \n"
		"attribute vec4 vertexColor;        \n"
		"varying vec2 fragTexCoord;         \n"
		"varying vec4 fragColor;            \n"
		"uniform mat4 mvp;                  \n"
		"void main()                        \n"
		"{                                  \n"
		"    fragTexCoord = vertexTexCoord; \n"
		"    fragColor = vertexColor;       \n"
		"    gl_Position = mvp*vec4(vertexPosition, 1.0); \n"
		"}                                  \n";

	const char *bgFShader = 
		"#version 100                       \n"
		"precision mediump float;           \n"     // Precision required for OpenGL ES2 (WebGL)
		"varying vec2 fragTexCoord;         \n"
		"varying vec4 fragColor;            \n"
		"uniform sampler2D texture0;        \n"
		"uniform vec4 colDiffuse;           \n"
		"void main()                        \n"
		"{                                  \n"
		"    vec2 uv = gl_FragCoord.xy / vec2(1667, 1043);                               \n"
		"    vec4 blue = vec4(0.23, 0.82, 0.82, 1.0);   \n"
		// "    blue *= vec4(1, 1, 1, uv.y);   \n"
		"    vec4 yellow = vec4(0.43, 0.40, 0.29, 1.0);   \n"
		// "    yellow *= vec4(1, 1, 1, 1.0 - uv.y);   \n"
		"    vec4 texelColor = texture2D(texture0, fragTexCoord); \n"
		"    //gl_FragColor = mix(yellow, blue, uv.y);            \n"
		"    gl_FragColor = blue;            \n"
		"}                                  \n";

	bgShader = LoadShaderFromMemory(bgVShader, bgFShader);
}
// Program main entry point
int main(void)
{
    // Initialization
    const int screenWidth = 800;
    const int screenHeight = 450;


	noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

	// fnl_state noise2 = fnlCreateState();
	// noise = fnlCreateState();
	// noise.noise_type = FNL_NOISE_OPENSIMPLEX2;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

	window_height = get_window_height();
	window_width = get_window_width();
	SetWindowSize(window_width - 15, window_height - 20);
	setup();


	state = {};

	Image dImage = LoadImage("assets/images/dolphinpod_sprite.png");
	if(dImage.date != NULL) {
		state.dolphinSheet = LoadTextureFromImage(dImage);
	}


	// tileTextureList.textures = (Texture2D*)malloc(100 * sizeof(Texture2D));
	// tileTextureList.capacity = 100;


	FilePathList files = LoadDirectoryFiles("assets/images");
	for(int i = 0; i < files.count; i++) {
		printf("%s\n", files.paths[i]);
		Image image = LoadImage(files.paths[i]);
		if(image.data != NULL) {
			tileTextures.push_back(LoadTextureFromImage(image));
			// tileTextureList.textures[i] = LoadTextureFromImage(image);
			// tileTextureList.count++;



			UnloadImage(image);

		}
	}
	printf("tile texture amount:%d\n", tileTextures.size());

	

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
	// emscripten_run_script("alert('hi')");
#else
    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second




    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    CloseWindow();        // Close window and OpenGL context

    return 0;
}

// Vector2 GetMouseTouchPosition() {
// }
float pow(float f, int e) {
	if(e == 0)
		return 1;

	if(e < 0)
		return f;

	float result = f;
	for(int i = 0; i < e-1; i++) {
		result *= f;
	}

	return result;
}
float GetBezier(float t, Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4) {
	// float t = 0.5f; // given example value
	// x = (1 - t) * (1 - t) * p[0].x + 2 * (1 - t) * t * p[1].x + t * t * p[2].x;
	// y = (1 - t) * (1 - t) * p[0].y + 2 * (1 - t) * t * p[1].y + t * t * p[2].y;
// By = (1-t)^3 * P0x + 
//      3t(1-t)^2 * P1x + 
//      3t^2(1-t) * P2x + 
//      t^3 * P3x

	float y = pow((1-t), 3)*p1.x + 3*t*pow((1-t),2)*p2.x + 3*pow(t,2)*(1-t)*p3.x + pow(t,3) * p4.x;
	// float x = pow((1-t), 3)*p1.y + 3*pow((1-t),2)*t*p2.y + 3*pow((1-t),2)*t*p3.y + pow(t, 3) * p4.y;

	return y;
}

float GetBezierForCorals(float t) {
	Vector2 p1 = {0.0f, 1.0f};
	Vector2 p4 = {1.0f, 1.0f};

	Vector2 p2 = {0.5f, 0.0f};
	Vector2 p3 = {0.9f, 1.0f};

	float result =  GetBezier(t, p1, p2, p3, p4);

	printf("bez:%f\n", result);

	return result;
}

void DrawCoral()
{
	Vector2 pos = GetMousePosition();

	int tile_map_x = pos.x / 50;
	int tile_map_y = pos.y / 50;

	for(int x = -2; x < 4; x++) {
		for(int y = -2; y < 4; y++) {

			int pos_x = x + tile_map_x;
			int pos_y = y + tile_map_y;

			if(pos_x >= 0 &&
			   pos_x < TILE_MAP_SIZE &&
			   pos_y >= 0 &&
			   pos_y < TILE_MAP_SIZE) {
				const char *id = TextFormat("%f, %f\n", pos.x, pos.y);
				Tile tile;
				int rnd = GetRandomValue(0, tileTextures.size()-1);
				tile.textureId = rnd;
				float noiseFreq = 7;
				float n = noise.GetNoise((float)pos_x * noiseFreq, (float)pos_y * noiseFreq);
				n += 1;
				n *= 0.5;
				n = GetBezierForCorals(n);

				tile.alpha = (int)(n * 256);
				if(n < 0.0f) {
					tile.type = TILE_EMPTY;
				}
				else {
					tile.type = TILE_TEXTURE;
					int size = GetRandomValue(100, 150);
					tile.width = size;
					tile.height = size;
					tile.rotation = (float)GetRandomValue(0, 360);
					tileMap[pos_x][pos_y] = tile;
				}
			}
		}
	}
}

void RunCoralAnimation()
{
	for(int i = 0; i < CORAL_ANIMATION_SPEED; i++) {

		Tile tile;

		int rnd = GetRandomValue(0, tileTextures.size()-1);
		tile.textureId = rnd;

		float noiseFreq = 7;
		float n = noise.GetNoise((float)state.coral_animation_x * noiseFreq, (float)state.coral_animation_y * noiseFreq);
		n += 1;
		n *= 0.5;
		tile.alpha = (int)(n * 256);

		if(n < 0.2f) {
			tile.type = TILE_EMPTY;
		}
		else {

			float step_width = window_width / TILE_MAP_SIZE;
			float step_height = window_height / TILE_MAP_SIZE;
			tile.type = TILE_TEXTURE;
			tile.x = state.coral_animation_x * step_width;
			tile.y = (TILE_MAP_SIZE - state.coral_animation_y) * step_height;
			int size = GetRandomValue(50, 200);
			tile.width = size;
			tile.height = size;
			tile.rotation = (float)GetRandomValue(0, 360);
			tiles.push_back(tile);
		}

		// ANIMATION TICK
		if(state.coral_animation_x < TILE_MAP_SIZE) {
			state.coral_animation_x++;
		}
		else {
			if(state.coral_animation_y < TILE_MAP_SIZE) {
				state.coral_animation_x = 0;
				state.coral_animation_y++;
			}
			else {
				// animation done
				state.coral_animation_done = true;
				return;
			}
		}
	}

}

void UpdateDrawFrame()
{
	if(IsMouseButtonDown(0)) {
		// DrawCoral();

		if(!state.coral_activated) {
			state.coral_activated = true;
			state.coral_animation_done = false;
		}
	}

	if(state.coral_activated && !state.coral_animation_done) {
		RunCoralAnimation();
	}


	BeginDrawing();
	ClearBackground(PURPLE);

	BeginShaderMode(bgShader);
	DrawRectangle(0, 0, window_width, window_height, BLUE);
	EndShaderMode();


	for(int i = 0; i < tiles.size(); i++) {
		// DrawTexture(tileTextures[tiles[i].textureId], tiles[i].x, tiles[i].y, WHITE);
		if(tiles[i].type != TILE_EMPTY) {
			// printf("%d\n", tiles[i].alpha);

			Texture2D *texture = &tileTextures[tiles[i].textureId];
			Rectangle rectSrc;
			rectSrc.width = texture->width;
			rectSrc.height = texture->height;
			rectSrc.x = 0;
			rectSrc.y = 0;

			Rectangle rectDst;
			rectDst.width = tiles[i].width; 
			rectDst.height = tiles[i].height; 
			rectDst.x = tiles[i].x;
			rectDst.y = tiles[i].y;

			Vector2 origin = Vector2{(float)tiles[i].width / 2, (float)tiles[i].height / 2};

			Color color = WHITE;
			color.a = tiles[i].alpha;
			DrawTexturePro(*texture, rectSrc, rectDst, origin, tiles[i].rotation, color);
		}
	}

	// for(int x = 0; x < TILE_MAP_SIZE; x++) {
	// 	for(int y = 0; y < TILE_MAP_SIZE; y++) {
	// 		Tile *tile = &tileMap[x][y];
	// 		if(tile->type != TILE_EMPTY) {
	// 			Texture2D *texture = &tileTextures[tile->textureId];
	// 			Rectangle rectSrc;
	// 			rectSrc.width = texture->width;
	// 			rectSrc.height = texture->height;
	// 			rectSrc.x = 0;
	// 			rectSrc.y = 0;

	// 			Rectangle rectDst;
	// 			rectDst.width = tile->width; 
	// 			rectDst.height = tile->height; 
	// 			rectDst.x = x * TILE_MAP_STEP;
	// 			rectDst.y = y * TILE_MAP_STEP;

	// 			Vector2 origin = Vector2{(float)tile->width / 2, (float)tile->height / 2};

	// 			Color color = WHITE;
	// 			color.a = tile->alpha;
	// 			DrawTexturePro(*texture, rectSrc, rectDst, origin, tile->rotation, color);
	// 		}
	// 	}
	// }

	EndDrawing();
}
