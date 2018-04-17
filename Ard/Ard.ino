/*    Author : S.Chandrakumar, A.R. Gokul Nath
 *  Firmware for controlling the P&P machine
 */

// vartiable in line 9-18 denotes pulse and direction pin for respective x,y,z and rotation axis

#define xp 11
#define yp 10
#define xd 2
#define yd 3
#define zp 9
#define zd 4
#define rp 6
#define rd 5

#define en 13 //commonn enable pin for all 4 axis

#define but 8 // end button for x axis
#define buty 7 // end button for y axis
#define butz 12 // end button for z axis

// lines 26- 29 are required for internal processing
#define selx 1
#define sely 2
#define selz 3
#define selr 4

// direction pin for x,y,z axis
// x requires bd,fd
// y requries right and left
// z requires up and down
#define right LOW
#define left HIGH
#define fd LOW
#define bd HIGH
#define up HIGH
#define down LOW

/* The spec of our machines
 *  ----------------------------------------------------
 *  axis  | distance in mm | Step Config.(Motor type)
 *    x   |     22300      |    3200(NEMA 23)
 *    y   |     53600      |    3200(NEMA 23)
 *    z   |      120       |    HALF(NEMA 17)
 *    r   |      360       |    FULL(HALLOW SHAFT STEPPER)
 *  -----------------------------------------------------
 *  inference for line 52-55 : max_x_pos, max_y_pos, max_z_pos, max_r_pos : maximum length in mm for respective axis
 *  inference for line 58-61 : max_x_pul, max_x_pul, max_x_pul, max_x_pul : maximum pulses required for end to end movement
 */
#define max_x_pos 22300
#define max_y_pos 53600
#define max_z_pos 120
#define max_r_pos 360

#define max_x_pul 17780
#define max_y_pul 43005
#define max_z_pul 248
#define max_r_pul 3200

//PCB zero and (work_zero_x (mm), work_zero_y (mm)) should coinside
#define work_zero_x 5500
#define work_zero_y 25500

//placement head should be moved to (cam_x,cam_y,cam_z) in order to have proper focus of the component
#define cam_x 7050
#define cam_y 18850
#define cam_z 5

//cx,cy,cz denotes current x,y,z
unsigned long long  cx, cy, cz;
unsigned long long xdelay = 500, ydelay = 1800;// delay for proper movement

unsigned long long currentMicros = 0;
unsigned long long previousMicros = 0;
float acc;

unsigned long long angle;

//enable state for the machine
bool enable;

//get the corresponding pul pin for the motor
unsigned long long getPul(unsigned long long mot)
{
  switch ((int)mot)
  {
  case selx:
    return xp;
  case sely:
    return yp;
  case selz:
    return zp;
  case selr:
    return rp;
  }
}
//set direction for the motor axis
void setDirforMot(unsigned long long mot, bool dir)
{
  switch ((int)mot)
  {
  case selx:
    if (dir == false)
      digitalWrite(xd, LOW);
    else
      digitalWrite(xd, HIGH);
    break;
  case sely:
    if (dir == false)
      digitalWrite(yd, LOW);
    else
      digitalWrite(yd, HIGH);
    break;
  case selz:
    if (dir == false)
      digitalWrite(zd, LOW);
    else
      digitalWrite(zd, HIGH);
    break;
  case selr:
    if (dir == false)
      digitalWrite(rd, LOW);
    else
      digitalWrite(rd, HIGH);
    break;
  }
}
//convert the corresponding postion to required pulse
unsigned long long cnvtPosToPulse(unsigned long long pos, unsigned long long cx, unsigned long long mot, bool *dir)
{
  unsigned long long max_pul = -1, max_pos = -1;
  switch ((int)mot)
  {
  case selx:
    max_pul = max_x_pul;
    max_pos = max_x_pos;
    break;
  case sely:
    max_pul = max_y_pul;
    max_pos = max_y_pos;
    break;
  case selz:
    max_pul = max_z_pul;
    max_pos = max_z_pos;
    break;
  case selr:
    max_pul = max_r_pul;
    max_pos = max_r_pos;
    break;
  }
  if (max_pul != -1 && max_pos != -1)
  {
    float posPerPul = (float)max_pos / (float)max_pul;
    unsigned long long reqPul = pos / posPerPul;
    if (reqPul > cx)
    {
      reqPul -= cx;
      if (mot == selx)*dir = bd;
      else if (mot == sely)*dir = right;
      else if (mot == selz)*dir = up;
      else if (mot == selr)*dir = right;
      return reqPul;
    }
    else
    {
      reqPul = cx - reqPul;
      if (mot == selx)*dir = fd;
      else if (mot == sely)*dir = left;
      else if (mot == selz)*dir = down;
      else if (mot == selr)*dir = left;
      return reqPul;
    }
  }
}
//set acceleration for the motor
float setAcc(float a)
{
  a = 100;
  a /= 100;
  return a;
}
//set acceleration for z axis only
float setAccz(float a)
{
  a = 100;
  a /= 100;
  return a;
}
//initialize the motors
void it()
{
  acc = setAcc(100);
  moveMot(selx, 0);
  moveMot(sely, 0);
  moveMot(selx, 4100);
  cz = -1;
  moveMot(sely, 47800);
  moveMot(selz, 0);
  acc=setAccz(100);
  moveMot(selz, 45);
}
//move motor to the corresponding position
void moveMot(unsigned long long mot, unsigned long long pos)
{
  //  digitalWrite(en,LOW);
  unsigned long long delay_Micros = 2000;

  if (pos == 0 && (cx == -1 || cy == -1 || cz == -1))
  {
    float a = 2000, b = 333;
    unsigned long long currentMicros = 0, previousMicros = 0;
    if (mot == selx)
    {
      setDirforMot(selx, fd);
      unsigned long long s = 0;
      while (digitalRead(but))
      {
        if (a <= b) a = b;
        else a = a - acc;
        unsigned long long delay_Micros = a;
        currentMicros = micros();
        if (currentMicros - previousMicros >= delay_Micros)
        {
          previousMicros = currentMicros;
          digitalWrite(getPul(mot), HIGH);
          delayMicroseconds(a);
          digitalWrite(getPul(mot), LOW);
          s++;
        }
      }
      cx = 0;
      //            Serial.println(s);
    }
    else if (mot == sely)
    {
      setDirforMot(sely, left);
      unsigned long long s = 0;
      while (digitalRead(buty))
      {
        if (a <= b) a = b;
        else a = a - acc;
        unsigned long long delay_Micros = a;
        currentMicros = micros();
        if (currentMicros - previousMicros >= delay_Micros)
        {
          previousMicros = currentMicros;
          digitalWrite(getPul(mot), HIGH);
          delayMicroseconds(a);
          digitalWrite(getPul(mot), LOW);
          s++;
        }
      }
      cy = 0;
      //            Serial.println(s);
    }
    else if (mot == selz)
    {
      //      Serial.printlnln("Init z");
      //      float a=2000,b=333;
      //      /unsigned long long currentMicros=0,previousMicros=0;
      setDirforMot(selz, down);
      unsigned long long s = 0;
      while (digitalRead(butz))
      {
        if (a <= b) a = b;
        else a = a - acc;
        unsigned long long delay_Micros = a;
        currentMicros = micros();
        if (currentMicros - previousMicros >= delay_Micros)
        {
          previousMicros = currentMicros;
          digitalWrite(getPul(mot), HIGH);
          delayMicroseconds(a);
          digitalWrite(getPul(mot), LOW);
          s++;
        }
      }
      cz = 1;
      //      Serial.printlnln(s);
    }
  }
  else
  {
    switch ((int)mot)
    {
    case selx:
      {
        //        Serial.printlnln("Moving motor x");
        bool dir;
        unsigned long long reqiredPulse = cnvtPosToPulse(pos, cx, selx, &dir);
        //        Serial.println(" and the requried pulse is ");
        //        Serial.println(reqiredPulse);
        unsigned long long rP = reqiredPulse;
        float a = 2000, b = 333;
        acc = setAcc(30);
        unsigned long long currentMicros = 0, previousMicros = 0;
        setDirforMot(selx, dir);
        unsigned long long s = 0;
        while (reqiredPulse > 0)
        {
          if(!digitalRead(but) && dir==fd)
          {
            cx=0;
            Serial.println("x button pressed");
            return;
          }
          if (a <= b) a = b;
          else a = a - acc;
          unsigned long long delay_Micros = a;
          currentMicros = micros();
          if (currentMicros - previousMicros >= delay_Micros)
          {
            previousMicros = currentMicros;
            digitalWrite(getPul(mot), HIGH);
            delayMicroseconds(a);
            digitalWrite(getPul(mot), LOW);
            s++;
            reqiredPulse--;
          }
        }
        if (dir == bd)
          cx += rP;
        else
          cx -= rP;
        //        Serial.printlnln(s);
        break;
      }
    case sely:
      {
        //        Serial.printlnln("Moving motor y");
        bool dir;
        unsigned long long reqiredPulse = cnvtPosToPulse(pos, cy, sely, &dir);
        //        Serial.println(" and the requried pulse is ");
        //        Serial.println(reqiredPulse);
        unsigned long long rP = reqiredPulse;
        float a = 2000, b = 333;
        acc = setAcc(30);
        unsigned long long currentMicros = 0, previousMicros = 0;
        setDirforMot(sely, dir);
        unsigned long long s = 0;
        while (reqiredPulse > 0)
        {
          if(!digitalRead(buty)&& dir==left)
          {
            cy=0;
            Serial.println("y button pressed");
            return;
          }
          if (a <= b) a = b;
          else a = a - acc;
          unsigned long long delay_Micros = a;
          currentMicros = micros();
          if (currentMicros - previousMicros >= delay_Micros)
          {
            previousMicros = currentMicros;
            digitalWrite(getPul(mot), HIGH);
            delayMicroseconds(a);
            digitalWrite(getPul(mot), LOW);
            s++;
            reqiredPulse--;
          }
        }
        if (dir == right)
          cy += rP;
        else
          cy -= rP;
        //        Serial.printlnln(cy);
        break;
      }
    case selz:
      {
        //        Serial.printlnln("Moving motor z");
        bool dir;
        unsigned long long reqiredPulse = cnvtPosToPulse(pos, cz, selz, &dir);
        //        Serial.println(" and the requried pulse is ");
        //        Serial.println(reqiredPulse);
        unsigned long long rP = reqiredPulse;
        float a = 2000, b = 333;
        acc = setAccz(30);
        unsigned long long currentMicros = 0, previousMicros = 0;
        setDirforMot(selz, dir);
        unsigned long long s = 0;
        while (reqiredPulse > 0)
        {
          if (a <= b) a = b;
          else a = a - acc;
          unsigned long long delay_Micros = a;
          currentMicros = micros();
          if (currentMicros - previousMicros >= delay_Micros)
          {
            previousMicros = currentMicros;
            digitalWrite(getPul(mot), HIGH);
            delayMicroseconds(a);
            digitalWrite(getPul(mot), LOW);
            s++;
            reqiredPulse--;
          }
        }
        if (dir == up)
          cz += rP;
        else
          cz -= rP;
        //        Serial.printlnln(cz);
        break;
      }
    case selr:
      {
        //        Serial.printlnln("Moving motor rot");
        bool dir;
        unsigned long long reqiredPulse = cnvtPosToPulse(pos, angle, selr, &dir);
        //        Serial.println(" and the requried pulse is ");
        //        Serial.println(reqiredPulse);
        unsigned long long rP = reqiredPulse;
        float a = 2000, b = 333;
        acc = setAccz(30);
        unsigned long long currentMicros = 0, previousMicros = 0;
        setDirforMot(selr, dir);
        unsigned long long s = 0;
        while (reqiredPulse > 0)
        {
          if (a <= b) a = b;
          else a = a - acc;
          unsigned long long delay_Micros = a;
          currentMicros = micros();
          if (currentMicros - previousMicros >= delay_Micros)
          {
            previousMicros = currentMicros;
            digitalWrite(getPul(mot), HIGH);
            delayMicroseconds(a);
            digitalWrite(getPul(mot), LOW);
            s++;
            reqiredPulse--;
          }
        }
        if (dir == right)
          angle =0;
        else
          angle =0;
        //        Serial.printlnln(angle);
        break;
      }
    }
  }
  //  digitalWrite(en,HIGH);
}


void setup()
{
  //pulse dir setup
  Serial.begin(9600);
  pinMode(xp, OUTPUT);
  pinMode(xd, OUTPUT);
  pinMode(en, OUTPUT);
  pinMode(yp, OUTPUT);
  pinMode(yd, OUTPUT);
  pinMode(zp, OUTPUT);
  pinMode(zd, OUTPUT);
  pinMode(rp, OUTPUT);
  pinMode(rd, OUTPUT);
  pinMode(but, INPUT_PULLUP);
  pinMode(buty, INPUT_PULLUP);
  pinMode(butz, INPUT_PULLUP);
  digitalWrite(en, HIGH);
  cx = -1;
  cy = -1;
  cz = -1;
  angle = 0;
  acc = setAcc(70);
  Serial.println("Available");
  while (!Serial.available());
  {
    unsigned long long a = Serial.read();
    if (a == 'i') {
      enable=true;
      digitalWrite(en, LOW);
      it();
      enable=false;
      digitalWrite(en, HIGH);
      Serial.println("Init Ended");
    }
  }
}
// soft reset the arduino
void(* reset) (void) = 0;
void loop()
{
  byte p=1;
  if (Serial.available())
  {
    String a = Serial.readStringUntil(' ');
    char axis[1];
    a.toCharArray(axis, 1);
    //    Serial.println(a[0]);Serial.println(" Prunsigned long longing a[0]\n");

    /*
    -------------------------------------------------------------------------------------------
    | x <x_pos>   | used to move x_axis
    | y <y_pos>   | used to move y axis
    | z <z_pos>   | used to move z axis
    |     m       | used to reset z axis
    | a <angle>   | used to rotate placemetn head to specific angle
    |     i       | used to reinitialize machine
    |     e       | enable the motors
    |     d       | disable the motor
    |     r       | soft reset the motor
    |     0       | placement head to working place 0,0
    |     c       | goto camera positon
    */
    switch (a[0])
    {
    case 'x':
      {
        //  used to move x axis
        //  format : x <x_pos>
        String x1 = Serial.readStringUntil('\n');
        char x2[7];
        x1.toCharArray(x2, 7);
        unsigned long long x_pos = atoi(x2);
        //                  Serial.println(x_pos);Serial.println(" Prunsigned long longing x_pos\n");
        if(enable && x_pos>=0 && x_pos<=max_x_pos)
          moveMot(selx, x_pos);
        Serial.println("Ended");
        break;
      }
    case 'y':
      {
        //  used to move y axis
        //  format : y <y_pos>
        String x1 = Serial.readStringUntil('\n');
        char x2[7];
        x1.toCharArray(x2, 7);
        unsigned long long x_pos = atol(x2);
        //                  Serial.println(x_pos);Serial.println(" Prunsigned long longing y_pos\n");
        if(enable && x_pos>=0 && x_pos<=max_y_pos)
          moveMot(sely, x_pos);
        Serial.println("Ended");
        break;
      }
    case 'z':
      {
        //  used to move z axis
        //  format : z <z_pos>
        String x1 = Serial.readStringUntil('\n');
        char x2[5];
        x1.toCharArray(x2, 5);
        unsigned long long x_pos = atoi(x2);
        //        Serial.println(x_pos);Serial.println(" Prunsigned long longing z_pos\n");
        if(enable && x_pos>=0 && x_pos<=max_z_pos)
          moveMot(selz, x_pos);
        Serial.println("Ended");
        break;
      }
    case 'm':
      {
        //  reset only z axis
        moveMot(selx, 4100);
        cz = -1;
        moveMot(sely, 47800);
        moveMot(selz, 0);
        acc=setAccz(100);
        moveMot(selz, 45);
        Serial.println("Ended");
        break;
      }
    case 'a':
      {
        //  used to rotate the placement head to a specific angle
        //  format : a <angle>
        String x1 = Serial.readStringUntil('\n');
        char x2[5];
        x1.toCharArray(x2, 5);
        unsigned long long x_pos = atoi(x2);
        //        Serial.println(x_pos);Serial.println(" Printing r_pos\n");
        if(enable)
          moveMot(selr, x_pos);
        Serial.println("Ended");
        break;
      }
    case 'i':
      {
        //  reinitialize the machine
        //  format : i
        cx = -1;
        cy = -1;
        cz = -1;
        it();
        Serial.println("Ended");
        break;
      }
    case 'e':
      {
        //  used to enable the motor. Essentially this will be the first cmd to be provided to the motor
        //  format : e
        enable=true;
        digitalWrite(en, LOW);
        Serial.println("Ended");
        break;
      }
    case 'd':
      //  used to disable the motors. Provide this cmd at the task completion end. This would same power
      //  format : d
      enable=false;
      digitalWrite(en, HIGH);
      Serial.println("Ended");
      break;
    case 'r':
      //  used to soft reset the arduino
      //  format : r
      reset();
      break;
    case '0':
      // used to move the placement head to the zero of the working area
      // format : 0
      moveMot(selx,work_zero_x);
      moveMot(sely,work_zero_y);
      Serial.println("Ended");
      break;
    case 'c':
      //  used to move the placement head to the camera positon
      //  format : c
      moveMot(selx,cam_x);
      moveMot(sely,cam_y);
      moveMot(selz,cam_z);
      Serial.println("Ended");
      break;
    }
  }
}
