#define SYMB_GRADUS 0x0A
#define SYMB_E      0x0B
#define SYMB_P      0x0C
#define SYMB_H      0x0D
#define SYMB_SPACE  0x0E
#define SYMB_t      0x0F
#define SYMB_MINUS  0x10
#define SYMB_C      0x11
#define SYMB_D      0x12
#define SYMB_Y      0x13
  
class TM1637{
  public:
    void print(float temp);

  private:
    int8_t font_tm1637[20] = {
      0x3F, //0x00 0
      0x06, //0x01 1
      0x5B, //0x02 2
      0x4F, //0x03 3
      0x66, //0x04 4
      0x6D, //0x05 5
      0x7D, //0x06 6
      0x07, //0x07 7
      0x7F, //0x08 8
      0x6F, //0x09 9
      0x63, //0x0A °
      0x79, //0x0B E
      0x73, //0x0C P
      0x76, //0x0D H
      0x00, //0x0E space
      0x78, //0x0F t
      0x40, //0x10 -
      0x39, //0x11 C
      0x77, //0x12 A
      0x00  //0x13 space
    };

    void _clearDisplay();
    void _writeByte(byte data);
    void _start();
    void _stop();
};

/**
 * Displays temperature on the display
 */
void TM1637::print(float temp){
  int8_t seg[6] = {SYMB_SPACE, SYMB_SPACE, SYMB_SPACE, SYMB_SPACE, SYMB_SPACE, SYMB_SPACE};
  int32_t tmp = round(temp);
  if(tmp > 125 or tmp < -50){ // if the data is wrong
    seg[3] = SYMB_MINUS;
    seg[4] = SYMB_MINUS;
    seg[5] = SYMB_GRADUS;
    seg[0] = SYMB_C;
    seg[1] = SYMB_SPACE;
    seg[2] = SYMB_SPACE;
  }
  else{ // if the data is correct
    uint8_t th = floor(abs(tmp) / 10);
    uint8_t tl = abs(tmp) % 10;
    if(th == 0) th = SYMB_SPACE;
    seg[3] = tmp < 0 ? SYMB_MINUS : tmp > 9 ? th : SYMB_SPACE;
    seg[4] = tmp < 0 ? tmp < -9 ? th : tl : tl;
    seg[5] = tmp < 0 ? tmp < -9 ? tl : SYMB_GRADUS : SYMB_GRADUS;
    seg[0] = tmp < 0 ? tmp < -9 ? SYMB_GRADUS : SYMB_C : SYMB_C;
    seg[1] = SYMB_SPACE;
    seg[2] = SYMB_SPACE;
  }

  _start();
  _writeByte(0x40);
  _stop();
  _start();
  _writeByte(0xC0);

  for(int8_t i=0; i<6; i++){
    _writeByte(font_tm1637[seg[i]]);
  }
  _stop();
  _start();
  _writeByte(0x8F /*+ _brightness*/);
  _stop();
}

/**
 * Sends a byte to display
 */
void TM1637::_writeByte(byte data){
  for(uint8_t i=0; i<8; i++){
    digitalWrite(TM1637_CLK, LOW);
    delayMicroseconds(50);
    if(data & 0x01) digitalWrite(TM1637_DIO, HIGH);
    else digitalWrite(TM1637_DIO, LOW);
    data >>= 1;
    delayMicroseconds(50);
    digitalWrite(TM1637_CLK, HIGH);
    delayMicroseconds(50);
  }
  digitalWrite(TM1637_CLK, LOW);
  delayMicroseconds(50);
  digitalWrite(TM1637_DIO, HIGH);
  delayMicroseconds(50);
  digitalWrite(TM1637_CLK, HIGH);
  pinMode(TM1637_DIO, INPUT);
  delayMicroseconds(50);
  uint8_t ack = digitalRead(TM1637_DIO);
  if(ack == 0){
     pinMode(TM1637_DIO, OUTPUT);
     digitalWrite(TM1637_DIO, LOW);
  }
  delayMicroseconds(50);
  pinMode(TM1637_DIO, OUTPUT);
  delayMicroseconds(50);
}

/**
 * Sends start command to display
 */
void TM1637::_start(){
  digitalWrite(TM1637_CLK, HIGH);
  delayMicroseconds(50);
  digitalWrite(TM1637_DIO, HIGH);
  delayMicroseconds(50);
  digitalWrite(TM1637_DIO, LOW);
  delayMicroseconds(50);
  digitalWrite(TM1637_CLK, LOW);
  delayMicroseconds(50);
}

/**
 * Sends stop command to display
 */
void TM1637::_stop(void){
  digitalWrite(TM1637_CLK, LOW);
  delayMicroseconds(50);
  digitalWrite(TM1637_DIO, LOW);
  delayMicroseconds(50);
  digitalWrite(TM1637_CLK, HIGH);
  delayMicroseconds(50);
  digitalWrite(TM1637_DIO, HIGH);
  delayMicroseconds(50);
}
