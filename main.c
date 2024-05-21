#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <raylib.h>
#include <stdint.h>

#define DARKGRAY2 CLITERAL(Color){ 0, 0, 0, 1 }
#define DARKYELLOW CLITERAL(Color) { 253, 249, 0, 230 }
#define ROWS 4
#define COLUMNS 4
#define PAIR_SIZE ROWS + COLUMNS
#define TEXTCOLOR WHITE
#define ROUNDNESS 0.03
#define SEGMENTS 1

const Color colors[PAIR_SIZE] = { ORANGE, PINK, RED, GREEN, BLUE, MAROON, PURPLE, YELLOW };
const char letters[PAIR_SIZE] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H' };

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
  Rectangle shape;
  bool opened;
  int pair_index;
} Card;

typedef struct CardPair {
  Color color;
  Card *card1;
  Card *card2;
  char letter[2];
} CardPair;

CardPair **createPairs() {
  // Same reasoning as "createCards"
  CardPair **pairs = (CardPair **) malloc(PAIR_SIZE * sizeof(CardPair*));
  CardPair *temp = (CardPair *) malloc(PAIR_SIZE * sizeof(CardPair));
  for (int i = 0; i < PAIR_SIZE; i++) {
    temp[i].letter[1] = '\0';
    pairs[i] = &temp[i];
  }
  return pairs;
}

void swapCards(Card **cards, int a, int b) {
  cards[a] = (Card *) ((intptr_t) cards[a] ^ (intptr_t) cards[b]);
  cards[b] = (Card *) ((intptr_t) cards[b] ^ (intptr_t) cards[a]);
  cards[a] = (Card *) ((intptr_t) cards[a] ^ (intptr_t) cards[b]);
}

void swapPairs(CardPair **pairs, int a, int b) {
  pairs[a] = (CardPair *) ((intptr_t) pairs[a] ^ (intptr_t) pairs[b]);
  pairs[b] = (CardPair *) ((intptr_t) pairs[b] ^ (intptr_t) pairs[a]);
  pairs[a] = (CardPair *) ((intptr_t) pairs[a] ^ (intptr_t) pairs[b]);
}

void resetPairs(Card **cards, CardPair** pairs) {
  int last = PAIR_SIZE - 1;
  for (int i = 0; i < PAIR_SIZE; i++) {
    int current = rand() % (last+1);
    if (current != last) swapPairs(pairs, current, last);
    pairs[last]->color = colors[i];
    last -= 1;
  }

  for (int i = 0; i < PAIR_SIZE; i++)
   pairs[i]->letter[0] = letters[i];

  last = COLUMNS * ROWS - 1;
  for (int i = 0; i < PAIR_SIZE; i++) {
    int current = rand() % (last+1);
    if (last != current) swapCards(cards, current, last);
    cards[last]->pair_index = i;
    pairs[i]->card1 = &*cards[last];
    last -= 1;
    current = rand() % (last+1);
    if (last != current) swapCards(cards, current, last);
    cards[last]->pair_index = i;
    pairs[i]->card2 = &*cards[last];
    last -= 1;
  }
}

Card **createCards(int height, int width, float padding, Vector2 position) {
  // This way we anly need to allocate once, increasing performance.
  // To free this block of memory, check lowest point in the cards array.
  Card **cards = (Card **) malloc(ROWS * COLUMNS * sizeof(Card *));
  Card *temp = (Card *) malloc(ROWS * COLUMNS * sizeof(Card));
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLUMNS; j++) {
      int index = i * COLUMNS + j;
      temp[index].shape = (Rectangle) { .x = position.x + j * width + j * padding , .y = position.y, .width = width, .height = height };
      temp[index].opened = false; 
      cards[index] = &temp[index];
    }
    position.y += height + padding;
  }
  return cards;
};

void updateCards(Card **cards, CardPair **pairs) {
  for (int i = 0; i < ROWS * COLUMNS; i++) {
    if (cards[i]->opened) {
      DrawText(pairs[cards[i]->pair_index]->letter, cards[i]->shape.x + cards[i]->shape.width/2 - 12, cards[i]->shape.y + cards[i]->shape.height/2 - 12, 50, pairs[cards[i]->pair_index]->color);
    } else {
      DrawText("?", cards[i]->shape.x + cards[i]->shape.width/2 - 12, cards[i]->shape.y + cards[i]->shape.height/2 - 12, 50, DARKGRAY);
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

void cardOnHover(Card** cards, CardPair **pairs) {
  for (int i = 0; i < ROWS * COLUMNS; i++) {
    Vector2 m = GetMousePosition();
    if (checkHit(m, cards[i]->shape)) {
      if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        DrawRectangleRounded(cards[i]->shape, ROUNDNESS, SEGMENTS, DARKGRAY2);
      } else {
        drawCard(*cards[i], LIGHTGRAY, modifiedColor(pairs[cards[i]->pair_index]->color, 1./4));
      }
    } else {
      drawCard(*cards[i], WHITE, modifiedColor(pairs[cards[i]->pair_index]->color, 1./2));
    };
  }
}

void cardOnRelease(Card** cards) {
  for (int i = 0; i < ROWS * COLUMNS; i++) {
    Vector2 m = GetMousePosition();
    if (checkHit(m, cards[i]->shape) && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
      cards[i]->opened = true;
    }
  }
}

int main() {
  int w_width = 1000;
  int w_height = 1000;
  InitWindow(w_width, w_height, "Card Memory Game");
  Rectangle content = { .x =  w_width/2. - 800./2., .y = w_height/2. - 350., .width = 800, .height = 800 };
  Card **cards = createCards(187, 187, 10., (Vector2) { .x = 10. + content.x, .y = 10. + content.y });
  CardPair **pairs = createPairs();
  resetPairs(cards, pairs);
  BeginDrawing();
  ClearBackground(BLACK);
  DrawRectangleRounded(content, .03, 1, DARKGRAY);
  for (int i = 0; i < ROWS * COLUMNS; i++)
    drawCard(*cards[i], WHITE, WHITE);
  while (true) {
    cardOnHover(cards, pairs);
    cardOnRelease(cards);
    updateCards(cards, pairs);
    DrawText("PLAY", 430, 50, 50, TEXTCOLOR);
    DrawText("80s", 170, 50, 50, TEXTCOLOR);
    DrawText("0", 700, 50, 50, TEXTCOLOR);
    DrawText("0", 800, 50, 50, TEXTCOLOR);
    EndDrawing();
  };
  free(cards);
  return 0;
}

