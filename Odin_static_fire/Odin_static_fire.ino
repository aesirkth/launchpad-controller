/*
 © ÆSIR - Student Rocket Assosiation
 Made by Patrick Oppel & Alexander Medén
 18/09/22
*/

#define RELAY_FIRE  A2 
#define RELAY_FILL  A3
#define RELAY_VENT  A4
#define FILL 7
#define ARM 2
#define FIRE 3

bool armed = false;
bool fire_safe = true;
bool fill_start = false;
bool vent = false;
unsigned long int start_fill;
unsigned long int start_arm;
unsigned long int start_fire;
unsigned long int fill_com;
unsigned long int arm_com;
unsigned long int fire_com;

void setup() {

  // put your setup code here, to run once:
  //Interrupts to react on the pulse signals from the remote
  attachInterrupt(digitalPinToInterrupt(FILL),fill,CHANGE);
  attachInterrupt(digitalPinToInterrupt(ARM),arm,CHANGE);
  attachInterrupt(digitalPinToInterrupt(FIRE),fire,CHANGE);
  
  Serial.begin(57600);
  pinMode(RELAY_FIRE, OUTPUT);
  pinMode(RELAY_FILL, OUTPUT);
  pinMode(RELAY_VENT, OUTPUT);
  pinMode(FILL,INPUT);
  pinMode(ARM,INPUT);
  pinMode(FIRE,INPUT);
  // Turn relays off from the start
  digitalWrite(RELAY_FIRE,HIGH);
  digitalWrite(RELAY_FILL,HIGH);
  digitalWrite(RELAY_VENT,HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  
}

void fill()
{
  if(digitalRead(FILL) == HIGH)
  {
    start_fill = micros(); 
  }
  else{   //End of PWM
    fill_com = (int)(micros()-start_fill);
    //start_fill = 0;
    //Serial.println(fill_com);
   
    if(fill_com >= 1050 && fill_com <= 1950)
    {
      digitalWrite(RELAY_FILL,HIGH);
      digitalWrite(RELAY_VENT,HIGH);
      if(fill_start)
      {
        //Serial.println("FILL OFF");
        fill_start = false;
      }
      if(vent)
      {
        //Serial.println("VENT OFF");
        fill_start = false;
      }
    }
    else if(fill_com > 1950)
    {
      digitalWrite(RELAY_FILL,HIGH);
      digitalWrite(RELAY_VENT,LOW);
      if (fill_start){
        fill_start = false;
      }
      if(vent==0){
        vent = true;
      }
      //Serial.println("FILL");
    }
    else if(fill_com < 1050){
      digitalWrite(RELAY_FILL,LOW);
      digitalWrite(RELAY_VENT,HIGH);
      if (fill_start==0){
        fill_start = true;
      }
      if (vent){
        vent = false;
      }
    }
  }
}

void arm()
{
  if(digitalRead(ARM) == HIGH)
  {
    start_arm = micros();  
  }
  else
  {
    if(digitalRead(ARM) == LOW)
    {
      arm_com = (int)(micros()-start_arm);
      start_arm = 0;
    }
    
    if(arm_com > 1900)
    {
      //Serial.println("ARM");
      armed = true;
    }
    else if(arm_com < 1900 && arm_com > 1100)
    {
      armed = false;
    }
    else if(arm_com < 1100)
    {
      armed = false;      
    }
  }
}

void fire()
{
  if(armed && fire_safe)
  {
    if(digitalRead(FIRE) == HIGH)
    {
      start_fire = micros();
    }
    else
    {
      
      if(digitalRead(FIRE) == LOW)
      {
        fire_com = (int)(micros()-start_fire);
        start_fire = 0;
      }
      if(fire_com < 1100)
      {
        //digitalWrite(RELAY_FILL,HIGH);
        Serial.println("wait");
      }
      else if(fire_com > 1900)
      {
        Serial.println("FIRE");
        digitalWrite(RELAY_FIRE,LOW);
      }
    }
  }
  else if(!fire_safe)
  {
    if(digitalRead(FIRE) == HIGH)
    {
      start_fire = micros();  
    }
    else
    {
      if(digitalRead(FIRE) == LOW)
      {
        fire_com = (int)(micros()-start_fire);
        start_fire = 0;
        //Serial.println(fire_com);
      }
      if(fire_com < 1100)
      {
        fire_safe = true;
        //Serial.println("fire off");
      }
      else if(fire_com > 1900)
      {
        //Serial.println("FIRE ACTIVE");
        //Serial.println("SWITCH OFF");
        fire_safe = false;
      }
      digitalWrite(RELAY_FIRE,HIGH);
    }
  }
  else if(!armed && fire_safe)
  {
    if(digitalRead(FIRE) == HIGH)
    {
      start_fire = micros();
    }
    else
    {
      if(digitalRead(FIRE) == LOW)
      {
        fire_com = (int)(micros()-start_fire);
        start_fire = 0;
        //Serial.println(fire_com);
      }
      if(fire_com < 1100)
      {
        fire_safe = true;
        //Serial.println("fire off");
      }
      else if(fire_com > 1900)
      {
        //Serial.println("FIRE ACTIVE");
        //Serial.println("SWITCH OFF");
        fire_safe = false;
      }
    }
    digitalWrite(RELAY_FIRE,HIGH);
  }
}
