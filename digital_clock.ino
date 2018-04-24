#include "LCD12864RSPI.h"
#include <dht11.h>
#define DHT11PIN 4
dht11 DHT11;
#define AR_SIZE( a ) sizeof( a ) / sizeof( a[0] )
#include <stdio.h>
#include <string.h>
#include <DS1302.h>

unsigned char show0[]="`C";
unsigned char show1[]="%";
unsigned char show2[]="    ";
unsigned char show3[]="   ";
unsigned char show4[]="fixed time:";
unsigned char show5[]="min";
unsigned char show6[]="nothing";

int pin13 = 2;
int pin12 = 12;
int pin11 = 11; 

uint8_t CE_PIN   = 6;
uint8_t IO_PIN   = 5;
uint8_t SCLK_PIN = 7;

int w = 0;//记录定时设定分钟数
int time = 0;
int button = 13;//13管脚为控制按钮
boolean onoff=LOW;//记录按钮状态
boolean timefix=LOW;//计时器开关，LOW表示未定时，HIGH表示定时
unsigned long buttonHoldTime=0;//按钮按下持续时间
int buttonStateOld;//按钮上一时刻状态
int buttonState;//按钮本时刻状态

char buf1[50];
char buf2[50];

char day[10];
String comdata = "";
int numdata[7] ={0}, j = 0, mark = 0;
DS1302 rtc(CE_PIN, IO_PIN, SCLK_PIN);

void ting()//控制蜂鸣器发声
{
  int checkstate = digitalRead(button);
  for(int i=0;i<80;i++)//输出一个频率的声音
  {
    digitalWrite(10,HIGH);//发声音
    delay(1);//延时1ms
    digitalWrite(10,LOW);//不发声音
    delay(1);//延时1ms
    }
    for (int i=0;i<100;i++)//输出另一个频率的声音，这里的100与前面的80一样，用来控制频率，可以自己调节
    {
      digitalWrite(10,HIGH);//发声音
      delay(2);
      digitalWrite(10,LOW);//不发声音
      delay(2);
      }
  }
  
void print_time()
{
    /* 从 DS1302 获取当前时间 */
    Time t = rtc.time();
    /* 将星期从数字转换为名称 */
    memset(day, 0, sizeof(day));
    switch (t.day)
    {
    case 1: strcpy(day, "Sunday"); break;
    case 2: strcpy(day, "Monday"); break;
    case 3: strcpy(day, "Tuesday"); break;
    case 4: strcpy(day, "Wednesday"); break;
    case 5: strcpy(day, "Thursday"); break;
    case 6: strcpy(day, "Friday"); break;
    case 7: strcpy(day, "Saturday"); break;
    }
    /* 将日期代码格式化凑成buf等待输出 */
    snprintf(buf1, sizeof(buf1), "%s %02d:%02d:%02d", day, t.hr, t.min, t.sec);
    snprintf(buf2, sizeof(buf2), "%04d-%02d-%02d", t.yr, t.mon, t.date);
    /* 输出日期到串口 */
    Serial.println(buf1);
    Serial.println(buf2);
}

//  ⊙℃％：
unsigned char str2[][2]={
  0x20,0x20,0xA1,0xD1,0xa1,0xe6,0xa3,0xa5,0xA3,0xBA};

unsigned  char msg1[]={
  0xCA, 0xAA,
  0xB6, 0xC8
};                    //湿度

unsigned char msg2[]={
  0xCE, 0xC2,
  0xB6, 0xC8
};                    //温度

void s_wsd(int r,int c)//在12864的X，Y坐标点显示Dht11的温度和湿度
{
  char tws[11];
  DHT11.read(DHT11PIN);
  snprintf(tws, sizeof(tws), "%2d%.2s  %2d%.2s",(DHT11.temperature-2),str2[2],DHT11.humidity,str2[3]);
  LCDA.DisplayString(r,c,(unsigned char *)tws,sizeof(tws));
}

void my_test()//显示温度、湿度值
{
  double temp=0;//定义中间变量
  char str[4]; //定义温度值存储数组，4位，其中3位为数字，1位为小数点
  DHT11.read(DHT11PIN);

  temp=DHT11.temperature-2;
  dtostrf(temp,4,2,str);//将获取的数值转化为字符型数组
  LCDA.DisplayString(1,0,(unsigned char *)str,sizeof(str)); //在第四行第三位显示温度值

  temp=0;
  temp=DHT11.humidity;//将湿度值 赋给中间变量temp
  dtostrf(temp,4,2,str);//将获取的数值转化为字符型数组
  LCDA.DisplayString(1,5,(unsigned char *)str,sizeof(str)); //在第四行第7位显示湿度值
  
  DHT11.read(DHT11PIN);
  float h = DHT11.temperature-2;
  
   if(50.0 > h && h > 30.0){
    pinMode(pin13,OUTPUT);
    digitalWrite(pin13,HIGH);
    }
  if(30.0 > h && h > 10.0 ){
    pinMode(pin12,OUTPUT);
    digitalWrite(pin12,HIGH);
    }
  if(10.0 > h && h > 0.0)
  {
    pinMode(pin11,OUTPUT);
    digitalWrite(pin11,HIGH);
    }
}

void clockdate()
{
  //以下是显示温度和湿度
  LCDA.DisplayString(0,0,msg2,sizeof(msg2));//在第一行0，0 显示汉字温度
  LCDA.DisplayString(0,5,msg1,sizeof(msg1));//在第一行0，3 显示汉字湿度 

  LCDA.DisplayString(2,0,buf1,sizeof(buf1));//在第三行显示星期几、时分秒
  LCDA.DisplayString(3,0,buf2,sizeof(buf2));//在第四行显示年月日
  
  LCDA.DisplayString(1,2,show0,sizeof(show0));//在温度值后加上·C的符号
  LCDA.DisplayString(1,7,show1,sizeof(show1));//在湿度值后加上%的符号
  //s_wsd(2,0);//在第三行显示具体的温度和湿度值
  }
  
void setup()
{

  Serial.begin(9600);
  rtc.write_protect(false);//设置时钟初始值
  rtc.halt(false);


  LCDA.Initialise(); // 屏幕初始化
  delay(100);
  pinMode(10,OUTPUT);//设置数字IO脚模式，OUTPUT为输出
  pinMode(button,INPUT);//设置引脚为输入模式
  Serial.println("DHT11 TEST PROGRAM ");
  Serial.print("LIBRARY VERSION: ");
  Serial.println(DHT11LIB_VERSION);
  Serial.println();

}

void loop()
{
  //LCDA.CLEAR();//清屏
  //delay(100);

  //Serial.println("\n");

  buttonState=digitalRead(button);
  delay(20);
  if (buttonState == HIGH && buttonStateOld==LOW)//按钮发生变化
  {
    buttonHoldTime = millis();//用millis()时间函数来计时，该函数最常记录为9小时22分钟，由于本程序最多记录60分钟，所以可以使用
    buttonStateOld=buttonState;

    if(onoff==HIGH)//当进入定时设置状态时
    {
      if(w>59)
      {
            w=0;
            timefix=LOW;
        }
        else
        {
            w=w+1;
            timefix=HIGH;
          }
          LCDA.CLEAR();
          LCDA.DisplayString(0,0,w,sizeof(w));
      }
    }
    else if (buttonState == HIGH && buttonStateOld==HIGH)//判断按钮是否持续按下
    {
      if(millis()-buttonHoldTime>3000)//当按钮按下3秒钟进入定时状态
      {
        buttonStateOld=LOW;
        if(onoff=LOW)//进入
        {
          w=0;
          timefix=LOW;
          LCDA.CLEAR();//清屏
          LCDA.DisplayString(0,0,show4,sizeof(show4));//显示Fixed Time
          LCDA.DisplayString(1,0,w,sizeof(w));//显示定时时间
          LCDA.DisplayString(2,0,show5,sizeof(show5));//显示min

          onoff=HIGH;
          }
          else//退出
          {
            print_time();//显示温度湿度时间
            clockdate();
             my_test();
            buttonHoldTime=millis();
           onoff=LOW;
            }
        }
        else
        {
          buttonStateOld=buttonState;
          }
      }
      else
      {
        buttonStateOld=buttonState;
        }

        if (onoff==LOW)
        {
             print_time();
          clockdate();
          my_test();
          }
         if(timefix==HIGH && w>0 && onoff==LOW)
         {
          unsigned long delaytime=w*60000;//把分钟算成秒
          if((millis()-buttonHoldTime)>=delaytime)
          {
            do
            {
              buttonState=digitalRead(button);
              ting();
              }
              while(buttonState==LOW);//当按下按钮后，停止蜂鸣
              w=0;
            }
            else
            {
               print_time();
              clockdate();
              my_test();
              }

          }
  delay(1000);

  int chk = DHT11.read(DHT11PIN);
  
 DHT11.read(DHT11PIN);
  float h = DHT11.temperature-2;//读取DHT11的温度值
  
   if(50.0 > h > 30.0){//比较，如果DHT11的温度是在30到50度间
    pinMode(pin13,OUTPUT);//设置2号管脚为输出状态
    digitalWrite(pin13,HIGH);//设置2号管脚为高电平，则红灯亮，表示温度高，注意高温
    }
  if(30.0 > h > 10.0 ){//如果DHT11的温度是在10到30度之间，
    pinMode(pin12,OUTPUT);//设置12号管脚为输出状态
    digitalWrite(pin12,HIGH);//设置12号管脚为高电平，则橙色灯亮
    }
  if(10.0 > h > 0.0)//如果DHT11的温度是在0到10度之间
  {
    pinMode(pin11,OUTPUT);//设置11号管脚为输出状态
    digitalWrite(pin11,HIGH);//设置11号管脚为高电平，则绿灯亮，表示温度低，注意低温
    }
  Serial.print("Read sensor: ");
  switch (chk)
  {
  case DHTLIB_OK: 
    Serial.println("OK"); 
    break;
  case DHTLIB_ERROR_CHECKSUM: 
    Serial.println("Checksum error"); 
    break;
  case DHTLIB_ERROR_TIMEOUT: 
    Serial.println("Time out error"); 
    break;
  default: 
    Serial.println("Unknown error"); 
    break;
  }


  Serial.print("Humidity (%): ");//串口显示Humidity
  Serial.println((float)DHT11.humidity, 2);//串口显示湿度值
  Serial.print("Temperature (oC): ");//串口显示Temperature
  Serial.println((float)DHT11.temperature-2, 2);//串口显示温度值
  delay(2000);

   while (Serial.available() > 0)
    {
        comdata += char(Serial.read());
        delay(2);
        mark = 1;
    }
    /* 以逗号分隔分解comdata的字符串，分解结果变成转换成数字到numdata[]数组 */
    if(mark == 1)
    {
        Serial.print("You inputed : ");
        Serial.println(comdata);
        for(int i = 0; i < comdata.length() ; i++)
        {
            if(comdata[i] == ',' || comdata[i] == 0x10 || comdata[i] == 0x13)
            {
                j++;
            }
            else
            {
                numdata[j] = numdata[j] * 10 + (comdata[i] - '0');
            }
        }
        /* 将转换好的numdata凑成时间格式，写入DS1302 */
        Time t(numdata[0], numdata[1], numdata[2], numdata[3], numdata[4], numdata[5], numdata[6]);
        rtc.time(t);
        mark = 0;j=0;
        /* 清空 comdata 变量，以便等待下一次输入 */
        comdata = String("");
        /* 清空 numdata */
        for(int i = 0; i < 7 ; i++) numdata[i]=0;
    }
   
  //注：这个坐标值汉字和温度、湿度值正好上下对齐了。
 
}



