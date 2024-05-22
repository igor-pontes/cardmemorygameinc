#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <raylib.h>
#include <stdint.h>

#define WIDTH  1000;
#define HEIGHT 1000;
#define DARKGRAY2  CLITERAL(Color){ 0, 0, 0, 100 }
#define DARKDARK   CLITERAL(Color){ 0, 0, 0, 150 }
#define DARKYELLOW CLITERAL(Color) { 253, 249, 0, 230 }
#define TEXTCOLOR WHITE
#define ROWS 4
#define COLUMNS 4
#define PAIR_SIZE ROWS + COLUMNS
#define ROUNDNESS 0.03
#define SEGMENTS 1

const Color colors[PAIR_SIZE] = { ORANGE, PINK, RED, GREEN, BLUE, MAROON, PURPLE, YELLOW };
const char letters[PAIR_SIZE] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H' };

typedef enum { 
  GAMEOVER = 0,
  RUNNING  = 1,
} GameState;

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

typedef struct ActivePair {
  bool active;
  Card *card1;
  Card *card2;
  struct timespec delay;
} ActivePair;

typedef struct GameManager {
  GameState status;
  int misses;
  int hits;
  time_t start;
  struct timespec sig_int;
  ActivePair active_pairs[PAIR_SIZE];
} GameManager;

void cardOnRelease(Card *card, CardPair **pairs, GameManager *gm);

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
    cards[last]->opened = false;
    pairs[i]->card1 = &*cards[last];
    last -= 1;
    current = rand() % (last+1);
    if (last != current) swapCards(cards, current, last);
    cards[last]->pair_index = i;
    cards[last]->opened = false;
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

void updateCardsText(Card **cards, CardPair **pairs) {
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

void updateCards(Card** cards, CardPair **pairs, GameManager *gm) {
  for (int i = 0; i < ROWS * COLUMNS; i++) {
    Vector2 m = GetMousePosition();
    if (checkHit(m, cards[i]->shape)) {
      if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        drawCard(*cards[i], DARKGRAY2, modifiedColor(pairs[cards[i]->pair_index]->color, 1./8));
      } else {
        drawCard(*cards[i], LIGHTGRAY, modifiedColor(pairs[cards[i]->pair_index]->color, 1./4));
        if (gm->status != GAMEOVER) cardOnRelease(cards[i], pairs, gm);
      }
    } else {
      drawCard(*cards[i], WHITE, modifiedColor(pairs[cards[i]->pair_index]->color, 1./2));
    };
  }
  updateCardsText(cards, pairs);
}
void resetActivePair(ActivePair *ap) {
  ap->active = false;
  ap->card1 = NULL;
  ap->card2 = NULL;
  ap->delay = (struct timespec) { .tv_sec = 0, .tv_nsec = 0 };
}

void cardOnRelease(Card *card, CardPair **pairs, GameManager *gm) {
  if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && !card->opened) {
    card->opened = true;
    for (int i = 0; i < PAIR_SIZE; i++) {
      if (gm->active_pairs[i].card1 == NULL) {
        gm->active_pairs[i].card1 = card;
        return;
      } 
      if (gm->active_pairs[i].card2 != NULL) continue;
      if (gm->active_pairs[i].card1->pair_index == card->pair_index) {
        gm->hits += 1;
        resetActivePair(&gm->active_pairs[i]);
        return;
      } 
      gm->active_pairs[i].active = true;
      gm->misses += 1;
      gm->active_pairs[i].card2 = card;
      clock_gettime(CLOCK_MONOTONIC, &gm->active_pairs[i].delay);
      gm->active_pairs[i].delay.tv_nsec += 1E8;
      return;
    } 
  }
}

void updateHeader(Card **cards, CardPair **pairs, GameManager *gm) {
  if (gm->status == GAMEOVER ) {
    DrawText("PLAY", 430, 50, 50, TEXTCOLOR);
    if (checkHit(GetMousePosition(), (Rectangle) { .x = 430, .y = 50, .width = 100, .height = 50 })) {
      DrawText("PLAY", 430, 50, 50, GRAY);
      if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        gm->start = time(NULL);
        gm->hits = 0;
        gm->misses = 0;
        gm->status = RUNNING;
        resetPairs(cards, pairs);
        return;
      } 
    }
  }
  DrawText(TextFormat("%d H", gm->hits), 700, 50, 50, TEXTCOLOR);
  DrawText(TextFormat("%d M", gm->misses), 800, 50, 50, TEXTCOLOR);
}

void updateTimer(GameManager *gm) {
  if (gm->status != GAMEOVER) {
    int elapsed = (int) difftime(time(NULL), gm->start);
    if (elapsed == 80) gm->status = GAMEOVER;
    DrawText(TextFormat("%ds", elapsed), 170, 50, 50, TEXTCOLOR);
    return;
  } 
  DrawText("80s", 170, 50, 50, TEXTCOLOR);
}

double get_interval(struct timespec end, struct timespec start) {
  return (double)(end.tv_sec - start.tv_sec) * 1.0e9 + (double)(end.tv_nsec - start.tv_nsec);
}

void resetActivePairs(GameManager *gm) {
  for (int i = 0; i < PAIR_SIZE; i++) {
    double interval = get_interval(gm->sig_int, gm->active_pairs[i].delay);
    if (interval > 0 && gm->active_pairs[i].active) {
      Card* a = gm->active_pairs[i].card1;
      Card* b = gm->active_pairs[i].card2;
      resetActivePair(&gm->active_pairs[i]);
      a->opened = false;
      b->opened = false;
      continue;
    }
  }
}

void drawBoard(Card **cards, CardPair **pairs, GameManager *gm, Rectangle board) {
  DrawRectangleRounded(board, .03, 1, DARKGRAY);
  updateCards(cards, pairs, gm);
  if (gm->status == GAMEOVER) {
    DrawRectangleRounded(board, .03, 1, DARKDARK);
    if (gm->hits == PAIR_SIZE) {
      DrawText("CONGRATULATIONS", 250, 480, 50, TEXTCOLOR);
    }
    if (gm->misses != 0 && gm->hits < PAIR_SIZE) {
      DrawText("GAME OVER", 320, 480, 50, TEXTCOLOR);
    }
  }
}

int main() {
  int width = WIDTH;
  int height = HEIGHT;
  InitWindow(width, height, "Card Memory Game");
  GameManager gm = (GameManager) { .status = GAMEOVER, .misses = 0, .hits = 0, .sig_int = (struct timespec) { .tv_sec = 0, .tv_nsec = 0} };
  for (int i = 0; i < PAIR_SIZE; i++) 
    gm.active_pairs[i] = (ActivePair) { .active = false, .card1 = NULL, .card2 = NULL, .delay = (struct timespec) { .tv_sec = 0, .tv_nsec = 0 } };
  Rectangle board = { .x =  width/2. - 800./2., .y = height/2. - 350., .width = 800, .height = 800 };
  Card **cards = createCards(187, 187, 10., (Vector2) { .x = 10. + board.x, .y = 10. + board.y });
  CardPair **pairs = createPairs();
  resetPairs(cards, pairs);
  while (true) {
    if (gm.hits == PAIR_SIZE) gm.status = GAMEOVER;
    clock_gettime(CLOCK_MONOTONIC, &gm.sig_int);
    BeginDrawing();
    resetActivePairs(&gm); 
    ClearBackground(BLACK);
    drawBoard(cards, pairs, &gm, board);
    updateHeader(cards, pairs, &gm);
    updateTimer(&gm);
    EndDrawing();
  };
  // TODO: free array?
  return 0;
}

