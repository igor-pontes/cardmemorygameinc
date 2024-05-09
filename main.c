#include <assert.h>
#include <stdlib.h>
#include <raylib.h>

#define DARKYELLOW CLITERAL(Color) { 253, 249, 0, 230 } // DarkYellow
#define ROWS 4
#define COLUMNS 4
#define TEXTCOLOR WHITE

Rectangle *createCards(int rows, int columns, int height, int width, float padding, Vector2 position) {
  Rectangle *cards = (Rectangle*) malloc(rows * columns * sizeof(Rectangle));
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++)
      cards[i * columns + j] = (Rectangle) { .x = position.x + j * width + j * padding , .y = position.y, .width = width, .height = height };
    position.y += height + padding;
  }  
  return cards;
};

void resetCards(Rectangle* cards) {
    for (int i = 0; i < ROWS * COLUMNS; i++)
      DrawText("?", cards[i].x + cards[i].width/2 - 10, cards[i].y + cards[i].height/2 - 10, 50, DARKGRAY);
}

int main() {
  int w_width = 1000;
  int w_height = 1000;
  InitWindow(w_width, w_height, "Card Memory Game");
  Rectangle content = { .x =  w_width/2 - 800./2, .y = w_height/2 - 350, .width = 800, .height = 800 };
  Rectangle *cards = createCards(ROWS, COLUMNS, 187, 187, 10., (Vector2) { .x = 10. + content.x, .y = 10. + content.y });
  while (true) {
    BeginDrawing();
    ClearBackground(BLACK);
    DrawRectangleRounded(content, .03, 1, DARKGRAY);
    for (int i = 0; i < ROWS * COLUMNS; i++)
      DrawRectangleRounded(cards[i], .03, 1, WHITE);
    // Rendering Text
    resetCards(cards);
    DrawText("PLAY", 430, 50, 50, TEXTCOLOR);
    DrawText("80s", 170, 50, 50, TEXTCOLOR);
    DrawText("0", 700, 50, 50, TEXTCOLOR);
    DrawText("0", 800, 50, 50, TEXTCOLOR);
    EndDrawing();
  };
  free(cards);
  return 0;
}

