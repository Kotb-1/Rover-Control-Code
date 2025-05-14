#include <Ps3Controller.h>
#include <WiFi.h>
#include <ESP32Servo.h>
#include <math.h>
// WIFI DATA
IPAddress local_IP(192,168,80,32);
IPAddress gateway(192,168,80,23);
IPAddress subnet(255,255,255,0);
const char* ssid="KOTB_MH";const char* password="12345678";
WiFiServer server_LV(32);WiFiServer server_cmd(33);
//Motor pin assigning
#define Logic_ML1 (27)
#define Logic_ML2 (26)
#define Logic_MR1 (17)
#define Logic_MR2 (5)
#define right (39)
#define right_far (36)
#define left (35)
#define left_far (32)
#define center (34)
#define PWML (25)
#define PWMR (16)
//Servo pin assign
#define servo_pin (22)
Servo servo_motor;
//Color sensor pin assign
#define S2 (0)
#define S3 (4)
#define OUT (2)
//Control data 
int sen1=0,sen2=0,sen3=0,sen4=0,sen5=0;//Extreme Left//middle//Extreme right
int joyx = 0,joyy = 0,t2tL = 14,t2tW = 21;
bool mode1 = false,mode2 = false,mode3 = false,mode4 = false;
// Controller
const int x_error = 10;
const int y_error = 10;const int neutheta = 90;

int ystart = 60;int lastx,lasty;
char red_out[6];char green_out[6];char blue_out[6];
char color_out[21];char irout[10];bool ismoving = false;
WiFiClient client_LV, client_cmd;
//The next lines are functions declared to help organize The functions
//PS3 Attach
void ps3_events(){
  if(Ps3.data.button.select){
    if( Ps3.data.button.square ){
      mode1=true;mode2=false;mode3=false;
      Stop();
      client_cmd.println("\n Line Follower Mode Sellected");
    }
    if( Ps3.data.button.triangle ){
      mode1=false;mode2=true;mode3=false;
      Stop();
      client_cmd.println("\n Color Mode Sellected");
    }
    if( Ps3.data.button.circle ){
      mode1=false;mode2=false;mode3=true;
      Stop();
      client_cmd.println("\n Rumbling Mode Sellected");
    } 
    if( Ps3.data.button.start ){
      mode1=false;mode2=false;mode3=false;
      client_cmd.println("\n No Mode Sellected");
      Stop();
      ESP.restart();
    }
  }
  if(mode3 || mode2){
      joyx = (Ps3.data.analog.stick.rx);
      joyy = (Ps3.data.analog.stick.ly)*-1;
      client_cmd.println(joyy);
  }

}
void Stop()
  {
    digitalWrite(Logic_ML1,LOW);
    digitalWrite(Logic_MR1,LOW);
    digitalWrite(Logic_ML2,LOW);
    digitalWrite(Logic_MR2,LOW);
    ledcWrite(PWMR, 0);
    ledcWrite(PWML, 0);
    servo_motor.write(90);
  }
//color
void Color(){
  int redFrequency = 0,greenFrequency = 0,blueFrequency = 0,Clear = 0; 
  //Set photodiodes to Red
  digitalWrite(S2,LOW);digitalWrite(S3,LOW);
  // Reading the output frequency
  redFrequency = pulseIn(OUT, LOW);
  //Set photodiodes to Green
  digitalWrite(S2,HIGH);digitalWrite(S3,HIGH);
  // Reading the output frequency
  greenFrequency = pulseIn(OUT, LOW);
  //Set photodiodes to blue
  digitalWrite(S2,LOW);digitalWrite(S3,HIGH);
  // Reading the output frequency
  blueFrequency = pulseIn(OUT, LOW);
  //formatting Strings
  sprintf(red_out,"%05d" , redFrequency);
  sprintf(green_out,"%05d" , greenFrequency);
  sprintf(blue_out,"%05d" , blueFrequency);
  // the next section increases delay will be removed after calibration
  digitalWrite(S2,HIGH);digitalWrite(S3,LOW);
  // Reading the output frequency
  Clear = pulseIn(OUT, LOW);
  //The color sensor will be calibrated IRL to spot colors
  //Printing the values
  sprintf(color_out, "%s,%s,%s" , red_out,green_out,blue_out);
  sprintf(color_out, "%s,%s,%s" , red_out,green_out,blue_out);
  client_LV.println(color_out);
  if(client_cmd){client_cmd.println(color_out);}
  }
//steering
void Steer_Drive_Controlled(int xmy,int ymy,bool iscontroler,bool isline){
  int x_error2,y_error2,x,y;
  if (iscontroler) {
    x = joyx;y = joyy;
    y_error2 = y_error;x_error2 = x_error;
  }
  else
  {
   x = xmy;y = ymy; 
   y_error2 = 0;x_error2 = 0;
  }
  if (abs(x)< x_error2 && abs(y)< y_error2) {Stop();return;}
  int PWM,PWM_in,PWM_out; float R,Theta,Theta_in,Theta_out,R_in,R_out;
  bool Left_turn = false, Right_turn= false;
  bool forward = y > y_error2;
  digitalWrite(Logic_ML1, forward ? HIGH : LOW );
  digitalWrite(Logic_ML2, forward ? LOW  : HIGH);
  digitalWrite(Logic_MR1, forward ? HIGH : LOW );
  digitalWrite(Logic_MR2, forward ? LOW  : HIGH);
  if(abs(y)> y_error2){
    y = map(abs(y),y_error2,128,ystart,128);
    PWM = map(abs(y), 0, 128, 0, 255);}
  else{PWM=0;}
  PWM_out = PWM;
  PWM_in = PWM;
  if (abs(x)>x_error2){
      Theta = map(abs(x),0,128,0,50);
      Left_turn = x<-x_error2;
      Right_turn = !Left_turn;
      R = t2tL/(tan(Theta*PI/180));
      R_in = R - 0.4*(t2tW/2);
      R_out = R + 0.4*(t2tW/2);
      PWM_in = constrain(PWM * (R_in / R), 0, 255);
      PWM_out = constrain(PWM * (R_out / R), 0, 255);
    }
  else{
    Right_turn = false;
    Left_turn = false;
    
  }
  if (!iscontroler && (Right_turn||Left_turn) && isline){
      PWM_out = PWM;
      PWM_in = PWM;
      digitalWrite(Logic_ML1, Right_turn ? HIGH : LOW );
      digitalWrite(Logic_ML2, Right_turn ? LOW  : HIGH);
      digitalWrite(Logic_MR1, Left_turn ? HIGH : LOW );
      digitalWrite(Logic_MR2, Left_turn ? LOW  : HIGH);
    }
  if(Right_turn){
      ledcWrite(PWML, PWM_out);
      ledcWrite(PWMR, PWM_in);
      servo_motor.write(neutheta+Theta);
    }
  else if(Left_turn){
      ledcWrite(PWMR, PWM_out);
      ledcWrite(PWML, PWM_in);
      servo_motor.write(neutheta-Theta);
    }
  else{
      ledcWrite(PWML, PWM_out);
      ledcWrite(PWMR, PWM_in);
      servo_motor.write(neutheta);
    }
}
void LF(){
  int xmy,ymy;
  bool isblack = !(sen3) || !(sen1) || !(sen2) || !(sen5) || !(sen4);
  int howmuch = (!(sen1))*-2 + (!(sen2))*-1 + (!(sen4))*1 + (!(sen5))*2;
  xmy = howmuch*42;
  sprintf(irout, "%d,%d,%d,%d,%d" , !(sen1),!(sen2),!(sen3),!(sen4),!(sen5));
  if(client_cmd){client_cmd.println(irout);}
  if (isblack){
    if(howmuch == 0){
      ymy=30;
      Steer_Drive_Controlled(xmy,ymy,false,true);
    }
    else{
      ymy = map(abs(howmuch),1,3,40,60);
      Steer_Drive_Controlled(xmy,ymy,false,true);
    }
    lastx = xmy;
    lasty = ymy;
    ismoving =true;
  } 
  else{
      Steer_Drive_Controlled(lastx,lasty,false,true);
  }
}
void setup() {
  // start communicating with the PS controller
    Ps3.attach(ps3_events);
    Ps3.begin("20:10:98:44:77:38");
  //WIFI BEGIN
    WiFi.config(local_IP,gateway,subnet);
    WiFi.begin(ssid,password);
    while(WiFi.status()!=WL_CONNECTED){
      delay(500);
      }
    server_LV.begin();
    server_cmd.begin();
  //Motor pins
    pinMode(Logic_ML1,OUTPUT);
    pinMode(Logic_ML2,OUTPUT);
    pinMode(Logic_MR1,OUTPUT);
    pinMode(Logic_MR2,OUTPUT);
    ledcAttach(PWML, 25000, 8);
    ledcAttach(PWMR, 25000, 8);
  //Servo pins
    servo_motor.attach(servo_pin);
    servo_motor.write(90);
  //Color sensor outputs
    pinMode(S2, OUTPUT);
    pinMode(S3, OUTPUT);
  //Color sensor Input frequency
    pinMode(OUT,INPUT);
// line follower
    pinMode(left_far,INPUT);  //extreme left sensor
    pinMode(left,INPUT);  //left
    pinMode(center,INPUT);  // middle sensor
    pinMode(right,INPUT);   // right
    pinMode(right_far,INPUT);  // extreme right sensor  
}
bool client_LV_conn,client_cmd_conn;
void loop() {
  //WiFi client
  if (!client_LV.connected()) {
    client_LV.stop();
    client_LV = server_LV.available();
  }
  if (!client_cmd.connected()) {
    client_cmd.stop();
    client_cmd = server_cmd.available();
    if (client_cmd) {
      client_cmd.println("CMD connected");
    }
  }
  //line follower
  sen1=!digitalRead(left_far);// Extreme Left
  sen2=!digitalRead(left);
  sen3=!digitalRead(center);// middle
  sen4=!digitalRead(right);
  sen5=!digitalRead(right_far);// Extreme right
  if(mode2 ){
    if(client_LV.connected()){Color();}
    Steer_Drive_Controlled(0,0,true,false);
    }
  else if (mode1) {
    LF();}    
  else if(mode3){
    Steer_Drive_Controlled(0,0,true,false);}
  else{}
  delay(20);
 } 
