
#include <TFT_eSPI.h>
#include <SPI.h>
#include <Helvetica6pt7b.h>
#include <helvetica_light12pt7b.h>
#include <helvetica_light16pt7b.h>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite screenSprite = TFT_eSprite(&tft);
TFT_eSprite iconSprite   = TFT_eSprite(&tft);

const char* mouseIcon[] = {
  "    ,d88b.         ",
  " ,8P'    `8,       ",
  " 8'       -.8.-    ",
  "8       /    |    \\",
  "       /     [-]    \\",
  "      |-----'-----|.",
  "      | jgs        |.",
  "      |;            .|",
  "       \\            /",
  "         \\' .-.  '/.",
  "           '-..-'  ",
  nullptr
};

const char* keyboardIcon[] = {
  "               jgs  ",
  " ---------------    ",
  "| |-||-||-||-||-|  |",
  "| |-||-||-||-||-|  |",
  "|  ,----------,  |  ",
  "|  |----------|  |  ",
  " ---------------    ",
  nullptr
};

const char* folderIcon[] = {
",----,                 ",
"|     ------------,    ",
"|    ,--------------,  ",
"|  /                    /",
"| /  mga          /    ",
"|/--------------/      ",
nullptr
};

const char* solitaireIcon[] = {
" -------------- ",
"| A               |",
"|    ,-,   ,-,     |",
"|   /   \\/    \\     |",
"|   \\         /     |",
"|     `.   .'       |",
"|        V        |",
"|               A |",
" --------------- ",
nullptr
};

const char* windowsIcon[] = {
"        ..--``--.. ",
"'-...-'|     ||    |",
"'-...-'|..--''--..|",
"'-...-'|     ||    |",
"'-...-'|..--''--..|",
"              jgs",
nullptr
};


const char* appleIcon[] = {
"    .--------------------.",
"    | .-------------. |==|",
"    | |                    | |  |",
"    | |                    | |  |",
"    | |                    | |  |",
"    | |  jgs             | | ,|",
"    | !--------------! |(c|",
"    !-----------------!--!",
"    |     -=      -=     | ,|",
"    |    [-]       [-]     |(c|",
"    !-----------------!--!",
"  /  [][][][][][][][][][][][]   \\",
" /  [][][][][][][][][][][][][]   \\",
"(  [][][][----------][][][]   )",
" \\ ---------------------- /",
nullptr
};

const char** icons[]     = { mouseIcon, keyboardIcon, folderIcon, solitaireIcon, windowsIcon };
const int    NUM_ICONS   = 5;
const char** currentIcon = windowsIcon;

// ---- MESSAGES ----
const char* mouseMsg =
  "The mouse can be used as a pointing device "
  "by sliding it over the work surface. "
  "Move it left and right to position the cursor.";

const char* backspaceMsg =
  "The BS (backspace) key erases the last character "
  "you typed. Use it to correct mistakes as you go. "
  "Practice makes perfect!";

const char* folderMsg = "Documents contain information that an application program "
  "needs in order to execute properly. For example, the MacWrite™ word processor is considered an application, but a file "
  "that contains a memo or letter is considered a document.";

const char* solitaireMsg = "If you're looking for a fun challenge, try the games that come with Windows 95, for example, Solitaire, one of the most popular card games of all time.";

const char* windowsMsg = "Screen savers can save wear and tear on your screen and protect your work while you're away. Several screen savers come with Windows 95.";

const char* messages[]   = { mouseMsg, backspaceMsg, folderMsg, solitaireMsg, windowsMsg };
const int   NUM_MESSAGES = 5;
const char* currentMsg   = windowsMsg;

// ---- SCROLL STATE ----
float scrollY     = 0;
float scrollSpeed = 1.5f;
const int SCROLL_GAP = 60;

// ---- WRAPPED LINES ----
#define MAX_LINES      48
#define MAX_LINE_CHARS 32
char wrappedLines[MAX_LINES][MAX_LINE_CHARS];
int  numWrappedLines = 0;
int  lineH           = 0;
int  totalTextH      = 0;

// ---- ICON STATE ----
int   iconW = 0, iconH = 0;
float ix, iy, ivx, ivy;
const float ICON_SPEED = 0.8f;
bool  hasChangedMsg = false;

// ---- INTRO ----
static unsigned long lastIntro = 0;
const char* oss[] = { "Apple Macintosh", "Windows 95", "Xerox Alto", "Amiga 500", nullptr };

// ------------------------------------------------------------------
// FONT HELPERS
// ------------------------------------------------------------------
void setScrollFont() {
  tft.setFreeFont(&helvetica_light16pt7b);   // must clear free-font or setTextSize is ignored
//   tft.setTextSize(3);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
}

void setScrollFontOnSprite(TFT_eSprite& spr) {
  spr.setFreeFont(&helvetica_light16pt7b);
//   spr.setTextSize(3);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(TFT_WHITE, TFT_BLACK);
}

// ------------------------------------------------------------------
// WORD WRAP  (call setScrollFont() on tft first)
// ------------------------------------------------------------------
int wordWrap(const char* text, int maxW,
             char lines[][MAX_LINE_CHARS], int maxLines) {
  int lineCount = 0;
  int textLen   = strlen(text);
  int i = 0;

  while (i < textLen && lineCount < maxLines) {
    char tmp[MAX_LINE_CHARS];
    int  lineLen   = 0;
    int  lastSpace = -1;
    int  j         = i;

    while (j < textLen && lineLen < MAX_LINE_CHARS - 1) {
      tmp[lineLen]     = text[j];
      tmp[lineLen + 1] = '\0';
      if (tft.textWidth(tmp) > maxW) break;
      if (text[j] == ' ') lastSpace = lineLen;
      lineLen++;
      j++;
    }

    if (j >= textLen) {
      strncpy(lines[lineCount], text + i, lineLen);
      lines[lineCount][lineLen] = '\0';
      lineCount++;
      break;
    }

    int breakAt = (lastSpace > 0) ? lastSpace : lineLen;
    strncpy(lines[lineCount], text + i, breakAt);
    lines[lineCount][breakAt] = '\0';
    lineCount++;

    i += breakAt;
    while (i < textLen && text[i] == ' ') i++;
  }
  return lineCount;
}

// ------------------------------------------------------------------
// BUILD WRAPPED TEXT
// ------------------------------------------------------------------
void buildWrappedText(const char* text) {
  setScrollFont();
  numWrappedLines = wordWrap(text, tft.width(), wrappedLines, MAX_LINES);
  lineH      = tft.fontHeight();
  totalTextH = numWrappedLines * lineH;
  scrollY    = tft.height();
}

// ------------------------------------------------------------------
// ICON SPRITE
// ------------------------------------------------------------------

void createIconSprite(const char** lines, uint16_t color = TFT_BLUE) {
  iconSprite.deleteSprite();
  iconSprite.setFreeFont(&Helvetica6pt7b);

  iconW = 0;
  int n = 0;
  for (int i = 0; lines[i] != nullptr; i++) {
    int w = iconSprite.textWidth(lines[i]);
    if (w > iconW) iconW = w;
    n++;
  }
  iconH = n * iconSprite.fontHeight();

  iconSprite.setColorDepth(16);
  iconSprite.createSprite(iconW, iconH);
  iconSprite.fillSprite(TFT_BLACK);
  iconSprite.setFreeFont(&Helvetica6pt7b);
  iconSprite.setTextColor(color, TFT_BLACK);

  int lh = iconSprite.fontHeight();
  for (int i = 0; lines[i] != nullptr; i++) {
    iconSprite.drawString(lines[i], 0, i * lh);
  }

  // randomize position now that iconW/iconH are known
  ix = random(20, max((int)tft.width()  - iconW - 20, 21));
  iy = random(20, max((int)tft.height() - iconH - 20, 21));
}

// ------------------------------------------------------------------
// VELOCITY PERTURBATION
// ------------------------------------------------------------------
void perturbVelocity(float& vx, float& vy, float amount) {
  vx += random(-100, 100) / 100.0f * amount;
  vy += random(-100, 100) / 100.0f * amount;
  float mag = sqrtf(vx * vx + vy * vy);
  if (mag > 0.01f) {
    vx = (vx / mag) * ICON_SPEED;
    vy = (vy / mag) * ICON_SPEED;
  }
  // prevent nearly-vertical or nearly-horizontal trajectories
  if (fabsf(vx) < ICON_SPEED * 0.4f) vx = copysignf(ICON_SPEED * 0.4f, vx);
  if (fabsf(vy) < ICON_SPEED * 0.4f) vy = copysignf(ICON_SPEED * 0.4f, vy);
  // re-normalize after clamping
  mag = sqrtf(vx * vx + vy * vy);
  vx = (vx / mag) * ICON_SPEED;
  vy = (vy / mag) * ICON_SPEED;
}

// ------------------------------------------------------------------
// SWITCH MESSAGE
// ------------------------------------------------------------------
void switchMessage(bool showCornerHit = true) {
  int idx;
  do { idx = random(NUM_MESSAGES); } while (messages[idx] == currentMsg);

  // save these so we have them before they change in createIconSprite()
  float curX = ix;
  float curY = iy;

  if (showCornerHit){
    tft.fillScreen(TFT_BLUE);

    createIconSprite(currentIcon, TFT_BLACK);  // white text, but bg is still TFT_BLACK
    // fill icon background blue to match screen
    iconSprite.fillRect(0, 0, iconW, iconH, TFT_BLUE);
    iconSprite.setFreeFont(&Helvetica6pt7b);
    iconSprite.setTextColor(TFT_BLACK, TFT_BLUE);
    int lh = iconSprite.fontHeight();
    for (int i = 0; currentIcon[i] != nullptr; i++) {
        iconSprite.drawString(currentIcon[i], 0, i * lh);
    }
    // push the blue sprite at the same location as the corner hit
    iconSprite.pushSprite((int)curX, (int)curY);
  
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.drawString("corner", tft.width()  / 2, tft.height()  / 2 - tft.fontHeight());
    tft.drawString("hit!", tft.width()  / 2, tft.height()  / 2);
    tft.drawString("next tip..", tft.width()  / 2, tft.height()  / 2 + tft.fontHeight());
    //   tft.drawString("tip...", tft.width()  / 2, tft.height()  / 2 + 2 * tft.fontHeight());
    delay(1000);
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_TRANSPARENT);
  }

  currentMsg  = messages[idx];
  // Note that this assumes same num icons as messages (and that their indecies match up)
  currentIcon = icons[idx];

  buildWrappedText(currentMsg);
  createIconSprite(currentIcon);
  setScrollFont();

  if (ix + iconW > tft.width())  ix = tft.width()  - iconW;
  if (iy + iconH > tft.height()) iy = tft.height() - iconH;
}

// ------------------------------------------------------------------
// INTRO
// ------------------------------------------------------------------
void showIntro(bool isSetup = false) {
  tft.setFreeFont(&helvetica_light12pt7b);
  tft.setTextDatum(MC_DATUM);

  int cx = tft.width()  / 2;
  int cy = tft.height() / 2;
  int fh = tft.fontHeight();

  createIconSprite(appleIcon);

  // draw the intro screen fresh each frame from the icon sprite as a base
  int iconX = tft.width()  / 2 - iconW / 2;
  int iconY = tft.height() / 2 - iconH / 2;

  // compositing sprite — same size as screen
  TFT_eSprite introSprite = TFT_eSprite(&tft);
  introSprite.setColorDepth(16);
  introSprite.createSprite(tft.width(), tft.height());

  // helper: redraw the base (black bg + apple icon) into introSprite
  auto drawBase = [&]() {
    introSprite.fillSprite(TFT_BLACK);
    iconSprite.pushToSprite(&introSprite, iconX, iconY, TFT_BLACK);
  };

  // draw a string into introSprite and push to screen
  auto showStr = [&](const char* s, int x, int y) {
    introSprite.setFreeFont(&helvetica_light12pt7b);
    introSprite.setTextDatum(MC_DATUM);
    introSprite.setTextColor(TFT_WHITE, TFT_BLACK);
    introSprite.drawString(s, x, y);
    introSprite.pushSprite(0, 0);
  };

  // type a string character by character, redrawing base each time
  auto typeText = [&](const char* s, int x, int y, int delayMs = 60) {
    char buf[64] = {0};
    for (int i = 0; s[i] != '\0'; i++) {
      buf[i] = s[i];
      buf[i+1] = '\0';
      drawBase();
      // redraw any persistent text (Welcome to) if needed
      introSprite.setFreeFont(&helvetica_light12pt7b);
      introSprite.setTextDatum(MC_DATUM);
      introSprite.setTextColor(TFT_WHITE, TFT_BLACK);
      showStr(buf, x, y);
      delay(delayMs);
    }
  };

  // erase a string by just redrawing the base + other persistent elements
  // caller is responsible for re-pushing whatever else should stay visible
  auto eraseStr = [&](int x, int y) {
    // just redraw base — the string is gone since we don't include it
    drawBase();
    introSprite.pushSprite(0, 0);
  };

  // type "Welcome to" persistently
  char welcomeBuf[64] = {0};
  const char* welcome = "Welcome to";
  for (int i = 0; welcome[i] != '\0'; i++) {
    welcomeBuf[i] = welcome[i];
    welcomeBuf[i+1] = '\0';
    drawBase();
    introSprite.setFreeFont(&helvetica_light12pt7b);
    introSprite.setTextDatum(MC_DATUM);
    introSprite.setTextColor(TFT_WHITE, TFT_BLACK);
    introSprite.drawString(welcomeBuf, cx, cy - fh * 2);
    introSprite.pushSprite(0, 0);
    delay(80);
  }

  for (int i = 0; oss[i] != nullptr; i++) {
    // split into words
    char buf[64];
    strncpy(buf, oss[i], sizeof(buf));
    int wordCount = 0;
    char* words[8];
    char* tok = strtok(buf, " ");
    while (tok && wordCount < 8) { words[wordCount++] = tok; tok = strtok(nullptr, " "); }

    int blockH = wordCount * fh;
    int startY = cy - blockH / 2;

    // type each word
    char wordBuf[64] = {0};
    for (int w = 0; w < wordCount; w++) {
      for (int c = 0; words[w][c] != '\0'; c++) {
        wordBuf[c] = words[w][c];
        wordBuf[c+1] = '\0';
        drawBase();
        introSprite.setFreeFont(&helvetica_light12pt7b);
        introSprite.setTextDatum(MC_DATUM);
        introSprite.setTextColor(TFT_WHITE, TFT_BLACK);
        introSprite.drawString(welcome, cx, cy - fh * 2);
        // redraw all completed words
        for (int pw = 0; pw < w; pw++)
          introSprite.drawString(words[pw], cx, startY + pw * fh);
        introSprite.drawString(wordBuf, cx, startY + w * fh);
        introSprite.pushSprite(0, 0);
        delay(60);
      }
      memset(wordBuf, 0, sizeof(wordBuf));
    }

    delay(800);

    // erase OS name by redrawing base + welcome only
    drawBase();
    introSprite.setFreeFont(&helvetica_light12pt7b);
    introSprite.setTextDatum(MC_DATUM);
    introSprite.setTextColor(TFT_WHITE, TFT_BLACK);
    introSprite.drawString(welcome, cx, cy - fh * 2);
    introSprite.pushSprite(0, 0);
    delay(50);
  }

  // erase welcome too
  drawBase();
  introSprite.pushSprite(0, 0);
  delay(200);

  introSprite.deleteSprite();
  if (!isSetup){
    // just to change things up, but don't want to show "corner hit"
    switchMessage(false);
  } else {
    createIconSprite(currentIcon);
  }
 
  setScrollFont();
  delay(500);
  setScrollFont();
}
// ------------------------------------------------------------------
// DRAW FRAME — composites everything into screenSprite, pushes once
// ------------------------------------------------------------------
void drawFrame() {
  screenSprite.fillSprite(TFT_BLACK);
  setScrollFontOnSprite(screenSprite);

  int period = totalTextH + SCROLL_GAP;

  for (int copy = 0; copy < 2; copy++) {
    int topY = (int)scrollY + copy * period;

    if (topY >= tft.height())   continue;
    if (topY + totalTextH <= 0) continue;

    for (int line = 0; line < numWrappedLines; line++) {
      int lineY = topY + line * lineH;
      if (lineY + lineH <= 0)            continue;
      if (lineY         >= tft.height()) break;

      screenSprite.drawString(wrappedLines[line], 0, lineY);
    }
  }

  // blit icon using black as transparency key
  iconSprite.pushToSprite(&screenSprite, (int)ix, (int)iy, TFT_BLACK);

  screenSprite.pushSprite(0, 0);
}

// ------------------------------------------------------------------
// SETUP
// ------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(2);
  tft.fillScreen(TFT_BLACK);
  randomSeed(esp_random());

  showIntro();

  screenSprite.setColorDepth(16);
  screenSprite.createSprite(tft.width(), tft.height());

  buildWrappedText(currentMsg);
  createIconSprite(currentIcon);
  setScrollFont();

  ix  = random(20, tft.width()  - iconW - 20);
  iy  = random(20, tft.height() - iconH - 20);
  ivx = -ICON_SPEED;
  ivy =  ICON_SPEED * 0.7f;

  lastIntro = millis();
}

// ------------------------------------------------------------------
// LOOP
// ------------------------------------------------------------------
void loop() {

  if (millis() - lastIntro > 60000) {
    tft.fillScreen(TFT_BLACK);
    showIntro();
    tft.fillScreen(TFT_BLACK);
    lastIntro = millis();
    scrollY = tft.height();
  }

  scrollY -= scrollSpeed;
  int period = totalTextH + SCROLL_GAP;
  if (scrollY + period < 0) scrollY += period;

  ix += ivx;
  iy += ivy;

  if (ix <= 0) {
    ix = 0; ivx = fabsf(ivx);
    perturbVelocity(ivx, ivy, 0.5f);
  } else if (ix + iconW >= tft.width()) {
    ix = tft.width() - iconW; ivx = -fabsf(ivx);
    perturbVelocity(ivx, ivy, 0.5f);
  }

  if (iy <= 0) {
    iy = 0; ivy = fabsf(ivy);
    perturbVelocity(ivx, ivy, 0.5f);
  } else if (iy + iconH >= tft.height()) {
    iy = tft.height() - iconH; ivy = -fabsf(ivy);
    perturbVelocity(ivx, ivy, 0.5f);
  }

  bool inLeft   = ix              < tft.width()  / 30;
  bool inRight  = (ix + iconW)    > tft.width()  - tft.width()  / 30;
  bool inTop    = iy              < tft.height() / 30;
  bool inBottom = (iy + iconH)    > tft.height() - tft.height() / 30;
  bool corner   = (inLeft || inRight) && (inTop || inBottom);

  if (corner && !hasChangedMsg) {
    hasChangedMsg = true;
    switchMessage();
  }
  if (!corner) hasChangedMsg = false;

  drawFrame();

  delay(20);
}