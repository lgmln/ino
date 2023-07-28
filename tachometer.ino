#include <LiquidCrystal.h>

// C++ code
//

const int ignPin = 4;     // вход с катушки
const int ref_time = 200; // период обновления
const int sz = 15;        // размер массива измерений
const int k = 30;         // оборотов на Герц
const char swPin = 3;     // пин переключателя экранов
const char vltPin= A6;    // пин вольтметра

long tm = 0,              // микросекунды
     _tm = 0;             // прежние микросекунды
bool ign = 0,             // сигнал с катушки
     _ign = 0;            // прежний сигнал с катушки
long per = 0;             // период сигнала
float freq = 0;           // частота сигнала
unsigned long rpm = 0;    // обороты в минуту
char n = -1;              // номер измерения
unsigned long freq1000;   // 1000-кратная частота
long tout = 1000000 / sz; // таймаут измерения
unsigned long _rpm=0;     // прежние обороты
long pers[sz];            // массив периодов
byte sym[8];              // матрица символа
char scrNum=0;            // номер экрана
char scrCnt=3;            // количество экранов
unsigned long btnTimer = 0; // таймер кнопки
bool btnState = 0;        // состояние кнопки
float vltK=1.1/1023*21/1.06; // коэф. вольтметра
int vlts[sz];              // массив напряжений
int vlt;                  // тек. напр в долях

LiquidCrystal lcd(15, 16, 19, 20, 21, 22);


void setup()
{
  pinMode(ignPin, INPUT);
  pinMode(swPin, INPUT_PULLUP);
  pinMode(vltPin, INPUT);
  analogReference(INTERNAL);
  
  byte f=B00000000;
  lcd.write(byte(0));  
  for (char b=0;b<6;b++)
  {
    for (char r=0; r<8; r++)
    {
      sym[r]=f;
    }
    lcd.createChar(byte(b), sym);

    f=(f>>1)|B00010000;
  };

  lcd.begin(16, 2);
  ign = _ign = digitalRead(ignPin);
  lcd.setCursor(0, 0);
  lcd.print("Ready to");
  lcd.setCursor(0, 1);
  lcd.print(" start");
  #ifdef DBG
  Serial.begin(115200);
  #endif
  for (int m=0; m<6;m++) lcd.write(byte(m));
  delay(1000);
}

inline void show_res()
{
  lcd.clear();
  switch (scrNum)
  {
    case 0:
    {
      //int vltInt=analogRead(vltPin);
      lcd.setCursor(0, 0);
      lcd.print(vltK*vlt, 1);
      lcd.print(" V");
      lcd.setCursor(0, 1);
      lcd.print(rpm);
      _rpm = rpm;
      break;
    }
    case 1:
    {
      int drpm = rpm - _rpm;
      int bar = constrain(10+drpm/4,0,80);
      for (char m=0;m<(bar/5);m++) lcd.write(byte(5));
      lcd.write(constrain(bar%5,0,4));
      bar-=40;
      lcd.setCursor(0, 1);
      for (char m=0;m<(bar/5);m++) lcd.write(byte(5));
      lcd.write(constrain(bar%5,0,4));
      break;
    }
    case 2:
    {
      lcd.setCursor(0, 0);
      lcd.print(freq,1);
      lcd.setCursor(0, 1);
      lcd.print(rpm);
      //lcd.print(' ');
      //int drpm = long(rpm) - _rpm;
      //lcd.print(drpm < 0 ? '-' : (drpm > 0 ? '+' : ' '));
      //lcd.print(abs(drpm));
      //_rpm = rpm;
      break;
    }
  }
  n = -1;
  //delay(ref_time);
  #ifdef DBG
  for (char m=0; m<sz; m++)
  {
        Serial.print(pers[m]);
        Serial.print(' ');
  }
  Serial.print('\n');
  #endif  
  delay(300);
}


void engStopMsg()
{
  int vltInt=analogRead(vltPin);
  lcd.clear();
  lcd.setCursor(0, 0);
//  lcd.print(tout);
  lcd.print(vltK*vltInt, 1);
  lcd.print(" V");
  lcd.setCursor(0, 1);
//  lcd.print(per);
  lcd.print("Stopped ");
  delay(300);
}

void loop()
{
  
  // обработка нажатия кнопки
  bool curBS=digitalRead(swPin);
  if (btnState != curBS) {
    if (millis() - btnTimer > 100)
    {
      btnState = !btnState;
      if (!btnState) scrNum=(++scrNum)%scrCnt;
    }
  }     
  else
    btnTimer=millis();

    
  // обработка сигнала зажигания
  ign = digitalRead(ignPin);
  tm = micros();
  per = tm - _tm;
  if ((per < 0) || (per > tout))
  {
    engStopMsg();
    n = -1;
    tm = micros();
    _tm = tm;
    goto aaa;
  }


  if (ign < _ign)
  {
    if (n < 0)
    {
      n = 0;
    }
    else
    {
      if (per)
      {

        if (n < sz)
        {
          pers[n] = per;
          vlts[n] = analogRead(vltPin);
          n++;
        }
        else
        {

          //delay(ref_time);
          //for (char m=0; m<sz; m++)
          //{
          //Serial.print(pers[m]);
          //Serial.print(' ');
          //}
          //Serial.print('\n');
          for (char h = (sz - 1); h > 0; h--)
          {
            for (char l = 0; l < h; l++)
            {
              if (pers[l] > pers[l + 1])
              {
                per = pers[l + 1];
                pers[l + 1] = pers[l];
                pers[l] = pers[l + 1];
              }
              if (vlts[l] > vlts[l + 1])
              {
                per = vlts[l + 1];
                vlts[l + 1] = vlts[l];
                vlts[l] = vlts[l + 1];
              }
            }
          }
          per = pers[(sz >> 1)];
          vlt = vlts[(sz >> 1)];
          freq1000 = 1000000000 / per;
          freq = 0.001 * freq1000;
          rpm = round(freq * k);
          show_res();
          tm = micros();
          n = -1;
        }
      }
    }
    _tm = tm;
  }

  aaa:
  _ign = ign;

}
