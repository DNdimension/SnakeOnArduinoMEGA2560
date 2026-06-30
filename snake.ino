/* 
 * Adafruit_HT1632.h, Adafruit_HT1632.cpp was edited to check whether a pixel is set
 * In Adafruit_HT1632.h, added in the public is
 * bool getPixel(uint16_t i) const;
 *
 * In Adafruit_HT1632.cpp, added is
 * bool Adafruit_HT1632::getPixel(uint16_t i) const { return ledmatrix[i / 8] & (1 << (i & 7)); }
*/
#include <Adafruit_HT1632.h>
#include <RingBufCPP.h>
#define HT_DATA 24
#define HT_WR 25
#define HT_CS 26
#define SW_PIN 50
#define X_PIN A0
#define Y_PIN A1
#define RAND_PIN A2

const uint8_t LEFTBOUND=0,RIGHTBOUND=31,UPBOUND=7,DOWNBOUND=0,DNT=144,DEADZONE=36,WINNUM=7;
const uint8_t OVERpix[] PROGMEM = {201,241,202,242,203,243,204,244,205,245,206,246,209,214,217,222,225,230,233,238,177,169,162,154,146,139,148,156,165,173,182,113,89,73,114,90,74,115,91,75,116,92,76,117,93,77,118,94,78,81,97,105,25,49,26,50,27,51,28,52,29,53,9,17,33,41,38,46,14,13,20};
const uint8_t WINpix[] PROGMEM = {241,210,214,218,222,226,230,234,238,242,246,203,211,205,213,220,228,236,140,148,156,164,172,180,139,179,141,181,73,78,81,86,89,94,97,102,105,110,113,118,106,99,91,84,77};
Adafruit_HT1632 matrix = Adafruit_HT1632(HT_DATA, HT_WR, HT_CS);
RingBufCPP<uint8_t,WINNUM> mybuf;
uint8_t food,sx=0,sy=0,SFace=0;
int16_t rx,ry,dnt=0;
template <size_t N>
void showInfo(const uint8_t (&arr)[N]) {
  dnt+=4096;
  for(size_t i=0;i<N;i++) matrix.setPixel(pgm_read_byte(&arr[i]));
}
uint8_t ptoi(uint8_t px,uint8_t py) { //px-[0,31],py-[0,7]
  py=max(min(UPBOUND,py),DOWNBOUND)<<3; //py*=3
  if(px<8) return 192+py+max(px,LEFTBOUND); // 8*8*3+py+max(px,LEFTBOUND)
  if(px<16) return 120+py+px; // 8*8*2+py+(px-8)
  if(px<24) return 48+py+px; // 8*8+py+(px-16)
  return py+(min(px,RIGHTBOUND)-24); // py+(min(px,RIGHTBOUND)-24)
}
inline void initGame() {
  uint8_t tmp;
  while(mybuf.pull(&tmp)) {};
  mybuf.add(ptoi(0,0));
  SFace=sx=sy=0;
  matrix.clearScreen();
  matrix.setPixel(ptoi(0, 0));
  initFood();
}
inline void initFood() {
  do { food=random(256); } while(matrix.getPixel(food));
  matrix.setPixel(food);
}
inline void moveSnake() {
  dnt=DNT;
  uint8_t tmp;
  rx=analogRead(X_PIN)-512,ry=analogRead(Y_PIN)-512;
  if(rx>DEADZONE&&rx>abs(ry)&&SFace!=2) SFace=0;
  else if(rx<-DEADZONE&&rx<-abs(ry)&&SFace!=0) SFace=2;
  else if(ry>DEADZONE&&SFace!=3) SFace=1;
  else if(ry<-DEADZONE&&SFace!=1) SFace=3;
  if(SFace==0) sx++;
  else if(SFace==2) sx--;
  else if(SFace==1) sy++;
  else sy--;
  tmp=ptoi(sx,sy);
  if(tmp==food) {
    if(mybuf.numElements()+1==WINNUM) showInfo(WINpix);
    else initFood();
  }
  else {
    mybuf.pull(&tmp);
    matrix.clrPixel(tmp);
    tmp=ptoi(sx,sy);
    if(sx>RIGHTBOUND||sy>UPBOUND||matrix.getPixel(tmp)) showInfo(OVERpix);
    matrix.setPixel(tmp);
  }
  mybuf.add(tmp);
}

void setup() {
  food=255; 
  randomSeed(analogRead(RAND_PIN));
  matrix.begin(ADA_HT1632_COMMON_8NMOS);  //8 行模式（不行就改成 ADA_HT1632_COMMON_16NMOS）
  initGame();
}

void loop() {
  if(!dnt) moveSnake();
  else if(dnt==DNT+1) initGame(),dnt--;
  else dnt--;
  
  matrix.writeScreen();
} 