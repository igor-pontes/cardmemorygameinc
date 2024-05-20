#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <raylib.h>

#define DARKGRAY2 CLITERAL(Color){ 0, 0, 0, 1 }
#define DARKYELLOW CLITERAL(Color) { 253, 249, 0, 230 }
#define ROWS 4
#define COLUMNS 4
#define TEXTCOLOR WHITE
#define ROUNDNESS 0.03
#define SEGMENTS 1

typedef enum { 
  GAMEOVER,
  RUNNING,
} GameState;

typedef struct GameManager {
  bool running;
  int misses;
  int hits;
} GameManager;

typedef struct Card {
  char letter;
  Color color;
  Rectangle shape;
  bool opened;
} Card;

Card *createCards(int rows, int columns, int height, int width, float padding, Vector2 position) {
  Card *cards = (Card*) malloc(rows * columns * sizeof(Rectangle));
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++)
      cards[i * columns + j] = (Card) { .letter = 'A', .shape.x = position.x + j * width + j * padding , .shape.y = position.y, .shape.width = width, .shape.height = height, .opened = false, .color = ORANGE };
    position.y += height + padding;
  }  
  return cards;
};

void resetCards(Card* cards) { }

void updateCards(Card* cards) {
  for (int i = 0; i < ROWS * COLUMNS; i++) {
    if (cards[i].opened) {
      DrawText("A", cards[i].shape.x + cards[i].shape.width/2 - 12, cards[i].shape.y + cards[i].shape.height/2 - 12, 50, cards[i].color);
    } else {
      DrawText("?", cards[i].shape.x + cards[i].shape.width/2 - 12, cards[i].shape.y + cards[i].shape.height/2 - 12, 50, DARKGRAY);
    }
  }
}

bool checkHit(Vector2 mouse, Rectangle card) {
  return mouse.x >= card.x && mouse.x <= card.x + card.width && mouse.y >= card.y && mouse.y <= card.y + card.height;
}

Color modifiedColor(Color color, float modifier) {
  Color new_color = color;
  new_color.r *= modifier;
  new_color.g *= modifier;
  new_color.b *= modifier;
  return new_color;
}

void drawCard(Card card, Color a, Color b) {
  DrawRectangleRounded(card.shape, ROUNDNESS, SEGMENTS, a);
  if (card.opened)
    DrawRectangleRounded(card.shape, ROUNDNESS, SEGMENTS, b);
}

void cardOnHover(Card* cards) {
  for (int i = 0; i < ROWS * COLUMNS; i++) {
    Vector2 m = GetMousePosition();
    if (checkHit(m, cards[i].shape)) {
      if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        DrawRectangleRounded(cards[i].shape, ROUNDNESS, SEGMENTS, DARKGRAY2);
      } else {
        drawCard(cards[i], LIGHTGRAY, modifiedColor(cards[i].color, 1./4));
      }
    } else {
      drawCard(cards[i], WHITE, modifiedColor(cards[i].color, 1./2));
    };
  }
}

void cardOnRelease(Card* cards) {
  for (int i = 0; i < ROWS * COLUMNS; i++) {
    Vector2 m = GetMousePosition();
    if (checkHit(m, cards[i].shape) && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
      cards[i].opened = true;
    }
  }
}

int main() {
  int w_width = 1000;
  int w_height = 1000;
  InitWindow(w_width, w_height, "Card Memory Game");
  Rectangle content = { .x =  w_width/2. - 800./2., .y = w_height/2. - 350., .width = 800, .height = 800 };
  Card *cards = createCards(ROWS, COLUMNS, 187, 187, 10., (Vector2) { .x = 10. + content.x, .y = 10. + content.y });
  BeginDrawing();
  ClearBackground(BLACK);
  DrawRectangleRounded(content, .03, 1, DARKGRAY);
  for (int i = 0; i < ROWS * COLUMNS; i++)
    drawCard(cards[i], WHITE, WHITE);
  while (true) {
    cardOnHover(cards);
    cardOnRelease(cards);
    updateCards(cards);
    DrawText("PLAY", 430, 50, 50, TEXTCOLOR);
    DrawText("80s", 170, 50, 50, TEXTCOLOR);
    DrawText("0", 700, 50, 50, TEXTCOLOR);
    DrawText("0", 800, 50, 50, TEXTCOLOR);
    EndDrawing();
  };
  free(cards);
  return 0;
}

