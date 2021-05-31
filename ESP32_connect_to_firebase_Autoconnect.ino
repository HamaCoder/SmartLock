#include <WiFi.h>
#include <FirebaseESP32.h>
#include <WebServer.h>
#include <time.h>
#include <AutoConnect.h>

#define FIREBASE_HOST "https://smart-lock-fa9e5-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_AUTH "5Bey4ujisHvfUwkkfamP8OZ6VUl1diXpAXSkjDpY"
FirebaseData firebaseData;

#define moCua 26

String Chuoi = "0!0@0#0$0%0";
String mk;
int matKhau, bienDem;
String vt1, vt2, vt3, vt4, vt5, statuslock;
int vantay1, vantay2, vantay3, vantay4, vantay5, trangthaikhoa;
byte ngat1, ngat2, ngat3, ngat4, ngat5, ngat6;

static const char AUX_TIMEZONE[] PROGMEM = R"(
{
  "title": "TimeZone",
  "uri": "/timezone",
  "menu": true,
  "element": [
    {
      "name": "caption",
      "type": "ACText",
      "value": "Sets the time zone to get the current local time.",
      "style": "font-family:Arial;font-weight:bold;text-align:center;margin-bottom:10px;color:DarkSlateBlue"
    },
    {
      "name": "timezone",
      "type": "ACSelect",
      "label": "Select TZ name",
      "option": [],
      "selected": 10
    },
    {
      "name": "newline",
      "type": "ACElement",
      "value": "<br>"
    },
    {
      "name": "start",
      "type": "ACSubmit",
      "value": "OK",
      "uri": "/start"
    }
  ]
}
)";

typedef struct {
  const char* zone;
  const char* ntpServer;
  int8_t      tzoff;
} Timezone_t;

static const Timezone_t TZ[] = {
  { "Europe/London", "europe.pool.ntp.org", 0 },
  { "Europe/Berlin", "europe.pool.ntp.org", 1 },
  { "Europe/Helsinki", "europe.pool.ntp.org", 2 },
  { "Europe/Moscow", "europe.pool.ntp.org", 3 },
  { "Asia/Dubai", "asia.pool.ntp.org", 4 },
  { "Asia/Karachi", "asia.pool.ntp.org", 5 },
  { "Asia/CanTho", "asia.pool.ntp.org", 6 },
  { "Asia/Jakarta", "asia.pool.ntp.org", 7 },
  { "Asia/Manila", "asia.pool.ntp.org", 8 },
  { "Asia/Tokyo", "asia.pool.ntp.org", 9 },
  { "Australia/Brisbane", "oceania.pool.ntp.org", 10 },
  { "Pacific/Noumea", "oceania.pool.ntp.org", 11 },
  { "Pacific/Auckland", "oceania.pool.ntp.org", 12 },
  { "Atlantic/Azores", "europe.pool.ntp.org", -1 },
  { "America/Noronha", "south-america.pool.ntp.org", -2 },
  { "America/Araguaina", "south-america.pool.ntp.org", -3 },
  { "America/Blanc-Sablon", "north-america.pool.ntp.org", -4},
  { "America/New_York", "north-america.pool.ntp.org", -5 },
  { "America/Chicago", "north-america.pool.ntp.org", -6 },
  { "America/Denver", "north-america.pool.ntp.org", -7 },
  { "America/Los_Angeles", "north-america.pool.ntp.org", -8 },
  { "America/Anchorage", "north-america.pool.ntp.org", -9 },
  { "Pacific/Honolulu", "north-america.pool.ntp.org", -10 },
  { "Pacific/Samoa", "oceania.pool.ntp.org", -11 }
};


WebServer         Server;
AutoConnect       Portal(Server);
AutoConnectConfig Config;
AutoConnectAux    Timezone;

void rootPage() {
  String  content =
    "<html>"
    "<head>"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
    "<script type=\"text/javascript\">"
    "setTimeout(\"location.reload()\", 1000);"
    "</script>"
    "</head>"
    "<body>"
    "<h2 align=\"center\" style=\"color:blue;margin:20px;\">Welcome to Smart Lock CET. Please connect to your wifi in (Configure new AP) Tab</h2>"
    "<h3 align=\"center\" style=\"color:gray;margin:10px;\">{{DateTime}}</h3>"
    "<p style=\"text-align:center;\">Click below button to return.</p>"
    "<p></p><p style=\"padding-top:15px;text-align:center\">" AUTOCONNECT_LINK(COG_24) "</p>"
    "</body>"
    "</html>";
  static const char *wd[7] = { "Sun","Mon","Tue","Wed","Thr","Fri","Sat" };
  struct tm *tm;
  time_t  t;
  char    dateTime[26];

  t = time(NULL);
  tm = localtime(&t);
  sprintf(dateTime, "%02d/%02d/%04d(%s) %02d:%02d:%02d.",
    tm->tm_mday + 1900, tm->tm_mon + 1, tm->tm_year,
    wd[tm->tm_wday],
    tm->tm_hour, tm->tm_min, tm->tm_sec);
  content.replace("{{DateTime}}", String(dateTime));
  Server.send(200, "text/html", content);
}

void startPage() {

  String  tz = Server.arg("timezone");

  for (uint8_t n = 0; n < sizeof(TZ) / sizeof(Timezone_t); n++) {
    String  tzName = String(TZ[n].zone);
    if (tz.equalsIgnoreCase(tzName)) {
      configTime(TZ[n].tzoff * 3600, 0, TZ[n].ntpServer);
      Serial.println("Time zone: " + tz);
      Serial.println("ntp server: " + String(TZ[n].ntpServer));
      break;
    }
  }

  Server.sendHeader("Location", String("http://") + Server.client().localIP().toString() + String("/"));
  Server.send(302, "text/plain", "");
  Server.client().flush();
  Server.client().stop();
}



void setup(){
  
  Serial.begin(115200);

  pinMode(moCua, OUTPUT);
  digitalWrite(moCua, HIGH);

  Config.autoReconnect = true;
  Config.hostName = "SMART LOCK - CET";
  Portal.config(Config);

  Timezone.load(AUX_TIMEZONE);

  AutoConnectSelect&  tz = Timezone["timezone"].as<AutoConnectSelect>();
  for (uint8_t n = 0; n < sizeof(TZ) / sizeof(Timezone_t); n++) {
    tz.add(String(TZ[n].zone));
  }

  Portal.join({ Timezone });

  Server.on("/", rootPage);
  Server.on("/start", startPage);  


  if (Portal.begin()) {
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
    Serial.println(WiFi.getHostname());
  }

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  Firebase.setString(firebaseData, "/H&RSL2645/IpCamera", WiFi.localIP().toString());
  
 }

int counter = 0;
int sent_status;
int Dem;
float counter2 = 0.5;




void loop(){

  Portal.handleClient();
  
  if(Serial.available() > 0){
    Chuoi = Serial.readString();
    if(Chuoi.length() >= 17){
      bienDem = 0;
      Serial.println(Chuoi);
    }

    if(Chuoi.length() < 10 && Chuoi.length() > 1){
      Firebase.setString(firebaseData, "H&RSL2645/Notification", "1");
      delay(3000);
    }
  }

  if(bienDem <= 1){
    for(int i=0; i < Chuoi.length(); i++){
       if(Chuoi.charAt(i) == '!'){
          ngat1 = i;
       } 
       if(Chuoi.charAt(i) == '@'){
          ngat2 = i;
       }
       if(Chuoi.charAt(i) == '#'){
          ngat3 = i;
       }
       if(Chuoi.charAt(i) == '$'){
          ngat4 = i;
       }
       if(Chuoi.charAt(i) == '%'){
          ngat5 = i;
       }
       if(Chuoi.charAt(i) == '^'){
          ngat6 = i;
       }
    }
  
    mk = Chuoi;
    vt1 = Chuoi;
    vt2 = Chuoi;
    vt3 = Chuoi;
    vt4 = Chuoi;
    vt5 = Chuoi;
    statuslock = Chuoi;
    
    mk.remove(ngat1);
    vt1.remove(0, ngat1 + 1);
    vt2.remove(0, ngat2 + 1);
    vt3.remove(0, ngat3 + 1);
    vt4.remove(0, ngat4 + 1);
    vt5.remove(0, ngat5 + 1);
    statuslock.remove(0, ngat6 + 1);
  
    matKhau = mk.toInt();
    vantay1 = vt1.toInt();
    vantay2 = vt2.toInt();
    vantay3 = vt3.toInt();
    vantay4 = vt4.toInt();
    vantay5 = vt5.toInt();
    trangthaikhoa = statuslock.toInt();

    if (matKhau == 0){
      mk = "null";
    } else {
      mk = String(matKhau);
    }
    
    if (vantay1 == 0){
      vt1 = "null";
    } else {
      vt1 = String(vantay1);
    }
    
    if (vantay2 == 0){
      vt2 = "null";
    } else {
      vt2 = String(vantay2);
    }
    
    if (vantay3 == 0){
      vt3 = "null";
    } else {
      vt3 = String(vantay3);
    }
    
    if (vantay4 == 0){
      vt4 = "null";
    } else {
      vt4 = String(vantay4);
    }
    
    if (vantay5 == 0){
      vt5 = "null";
    } else {
      vt5 = String(vantay5);
    }
    
    statuslock = String(trangthaikhoa);
    
    Firebase.setString(firebaseData, "/H&RSL2645/Pass", mk);
  
    Firebase.setString(firebaseData, "/H&RSL2645/VanTay1", vt1);
    
    Firebase.setString(firebaseData, "/H&RSL2645/VanTay2", vt2);
  
    Firebase.setString(firebaseData, "/H&RSL2645/VanTay3", vt3);
  
    Firebase.setString(firebaseData, "/H&RSL2645/VanTay4", vt4);
    
    Firebase.setString(firebaseData, "/H&RSL2645/VanTay5", vt5);
  
    Firebase.setString(firebaseData, "/H&RSL2645/Lock", statuslock);

  }

  bienDem++;
  if (bienDem == 100){
    bienDem = 2;
  }


  if(Firebase.get(firebaseData, "H&RSL2645/Khoa") == true){
     sent_status = firebaseData.intData();
  } else {
      Serial.println("");
  }

  if(sent_status==1){
    if(Dem < 1){
      Serial.println("mocuadi");
      digitalWrite(moCua, LOW);
      delay(3000);
    } else {
      digitalWrite(moCua, HIGH);
      Firebase.setInt(firebaseData, "H&RSL2645/Khoa", 0);
    }
    Dem++;
  } else {
    Dem=0;
  }

  
}
