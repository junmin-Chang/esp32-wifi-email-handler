#include <WiFi.h>
#include <LiquidCrystal.h>   
#include <ESP_Mail_Client.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

#define Echo 33
#define Trig 32
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
#define AUTHOR_EMAIL "" //  임시로 송신용 이메일 계정
#define AUTHOR_PASSWORD ""
#define RECIPIENT_EMAIL "chjm219@gmail.com" // 수신자 이메일

const int RS = 13, EN = 12, D4 = 14, D5 = 27, D6 = 26, D7 = 25;  
 
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "asia.pool.ntp.org", 9*60*60, 60000);
LiquidCrystal lcd(RS,EN,D4,D5,D6,D7); 
SMTPSession smtp;

const char *ssid     = ""; // 와이파이 이름
const char *password = ""; // password
char Time[ ] = "TIME:00:00:00";
char Date[ ] = "DATE:00/00/2000";
byte last_second, second_, minute_, hour_, day_, month_;
int year_;
unsigned int duration;
float distance;
 
void smtpCallback(SMTP_Status status); // 콜백함수


void setup() {
  pinMode(Echo, INPUT);
  pinMode(Trig, OUTPUT);
  Serial.begin(9600);
  lcd.begin(20, 4);                 
  WiFi.begin(ssid, password); 
  while ( WiFi.status() != WL_CONNECTED ) {
    delay(500);
    Serial.print(".");
  }
  timeClient.begin();
  Serial.println("연결 됨.");
}
 
 
void loop() {
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);
  duration = pulseIn(Echo, HIGH);
  distance = (1.0) * duration * 170 / 1000;

  Serial.println(String(distance));
  timeClient.update();
  unsigned long unix_epoch = timeClient.getEpochTime();    // NTP server의 유닉스 시간
  second_ = second(unix_epoch);
  if (last_second != second_) {
 
 
    minute_ = minute(unix_epoch);
    hour_   = hour(unix_epoch);
    day_    = day(unix_epoch);
    month_  = month(unix_epoch);
    year_   = year(unix_epoch);
 
 
 
    Time[12] = second_ % 10 + 48;
    Time[11] = second_ / 10 + 48;
    Time[9]  = minute_ % 10 + 48;
    Time[8]  = minute_ / 10 + 48;
    Time[6]  = hour_ % 10 + 48;
    Time[5]  = hour_ / 10 + 48;
 
 
 
    Date[5]  = day_ / 10 + 48;
    Date[6]  = day_ % 10 + 48;
    Date[8]  = month_ / 10 + 48;
    Date[9]  = month_ % 10 + 48;
    Date[13] = (year_ / 10) % 10 + 48;
    Date[14] = year_ % 10 % 10 + 48;



    lcd.setCursor(0, 0);
    lcd.print(Time);
    lcd.setCursor(0, 1);
    lcd.print(Date);

    
    // 거리가 5cm 미만일 때 전송
    if (distance < 50) {
       smtp.debug(1);

      smtp.callback(smtpCallback);
    
      ESP_Mail_Session session;
    
      session.server.host_name = SMTP_HOST;
      session.server.port = SMTP_PORT;
      session.login.email = AUTHOR_EMAIL;
      session.login.password = AUTHOR_PASSWORD;
      session.login.user_domain = "";
    
      SMTP_Message message;
    
      message.sender.name = "Junmin";
      message.sender.email = AUTHOR_EMAIL;
      message.subject = "거리가 너무 가깝습니다!!";
      message.addRecipient("Professor", RECIPIENT_EMAIL);
    
      String txtMessage = "현재 거리는\n" + String(distance) + "mm 입니다\n" + Time;
      message.text.content = txtMessage.c_str();
      message.text.charSet = "us-ascii";
      message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
    
      if (!smtp.connect(&session))
        return;
    
      if (!MailClient.sendMail(&smtp, &message))
        Serial.println("Error sending Email, " + smtp.errorReason());
      }    
   }
  delay(500);
} 
 

void smtpCallback(SMTP_Status status) {
  Serial.println(status.info());

  if (status.success()) {
    Serial.println("------------");
    ESP_MAIL_PRINTF("메세지 전송 success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("메세지 전송 failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++){   
        SMTP_Result result = smtp.sendingResult.getItem(i);
        time_t ts = (time_t)result.timestamp;
        localtime_r(&ts, &dt);
    
        ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
        ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
        ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients);
        ESP_MAIL_PRINTF("Subject: %s\n", result.subject);
    }
    Serial.println("----------------\n");
  }
    
}
