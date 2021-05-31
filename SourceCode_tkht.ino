/*
 * ------- ĐỒ ÁN THIẾT KẾ HỆ THỐNG CƠ ĐIỆN TỬ -------
 * ------ Đề tài: Hệ thống khóa cửa thông minh ------
 * --------------------------------------------------
 * 
 * --- Sơ đồ kết nối mạch ---
 * ------ Bàn phím 4x4 ------
 * Bàn phím (1 -> 9) -> Mega D(37, 21)/2=0
 * 
 * ------ Màn hình oled -----
 * (Oled) GND -> (Mega) GND
 *        VCC           +5V
 *        SCL           SCL
 *        SDA           SDA
 *        128x64 điểm màu
 * 
 * ------ Cb vân tay --------
 * (Sensor) xanh dương    ->   (Mega) D10
 *          màu vàng - RX             D12
 *          xanh lá  - TX             D13
 *          màu cam                   D11
 * 
 * ------ Khóa điện từ ------
 * (Relay) High -> (Mega) D9
 *         VCC            D8
 *         GND            D7
 * 
 * --------- Chuông ---------
 * (Chuông) signal  ->  (Mega) D6
 *          GND     ->         D5
 *
 * ------ Camera ov7670 -----
 * 
 * 
 * ------ UART ESP32 -------- 
 * (Mega) D2  ->  TX
 *        D3      RX
 *        
 * ------ Nút mở khóa --------
 * (Nút) dây đỏ  -> (Mega) 38
 *       dây nâu           40
 *       
 *  
 * ------ Công tắc hành trình -------
 * (Công tắc) dây trắng  ->  (Mega) 39
 *            dây tím               41    
 * 
 * ---Code by Hama---
 */

#include <Keypad.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMono9pt7b.h>

#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

#define GDN  5
#define bell 6
#define lock 9
#define GND  7
#define VCC  8
#define DC   38
#define btnOpen 40
#define VDD  39
#define congTac 41
#define nutChuong 42
#define GNDnutChuong 43
#define keuMoCua 44


// --- Khai báo hàm lưu dữ liệu EEPROM ---
char read_EEPROM[5];
int i=0, t=0;

// --- Khai báo hàm chuyển dữ liệu ESP32 ---
//SoftwareSerial sentData(10, 11);


// --- Khai báo hàm cho cảm biến vân tay ---
SoftwareSerial finger(12, 13);
Adafruit_Fingerprint vanTay = Adafruit_Fingerprint(&finger);


// --- Khai báo hàm cho Bàn phím 4x4
const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {28, 26, 24, 22};
byte colPins[COLS] = {36, 34, 32, 30};
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 


// --- Khai báo hàm cho Màn hình Oled
Adafruit_SSD1306 displayOLED;
int Variable1;
int length_mk = 5;
int id_finger = 0;
int dataEmpty = 0;
int choNayTrong = 0;
String name_finger[5];
String confirm_finger[5];
String sent_pass;
char BanPhim;
char new_password[5];
char confirm_pass[5];
char create_password[5];
char password[5];
char read_pass[5];
char delete_pass[5];
boolean hienThi = true;
String readID;
String read_finger[5];
boolean choPhep = false;
String Chuoi;
int bienDem;
int yes;




void setup() {
  Serial.begin(115200);
  vanTay.begin(57600);
  

  // Setup display Oled
  delay(100);
  displayOLED.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  displayOLED.clearDisplay();
  displayOLED.setTextColor(WHITE);
  displayOLED.setRotation(0);
  displayOLED.setTextWrap(false);
  displayOLED.dim(0);                   //Độ sáng
  
  //Setup Chuông
  pinMode(bell, OUTPUT);

  //Setup Khóa
  pinMode(lock, OUTPUT);
  pinMode(VCC, OUTPUT);
  pinMode(GND, OUTPUT);
  pinMode(GDN, OUTPUT);
  digitalWrite(lock, HIGH);
  digitalWrite(VCC, HIGH);
  digitalWrite(GND, LOW);
  digitalWrite(GDN, LOW);

  //Setup nút chuông
  pinMode(DC, OUTPUT);
  pinMode(btnOpen, INPUT_PULLUP);
  digitalWrite(DC, LOW);

  //Setup công tắc hành trình
  pinMode(VDD, OUTPUT);
  digitalWrite(VDD, LOW);
  pinMode(congTac, INPUT_PULLUP);

  //Setup nút khách
  pinMode(nutChuong, INPUT_PULLUP);
  pinMode(GNDnutChuong, OUTPUT);
  digitalWrite(GNDnutChuong, LOW);

  //Setup tín hiệu từ ESP32
  pinMode(keuMoCua, INPUT_PULLUP);


}


void loop() {

  if (digitalRead(nutChuong) == LOW){
     Serial.println("CoKhach");
     ChuongKeu(100);     
     displayOLED.clearDisplay();
     displayOLED.setCursor(0,20);
     displayOLED.print("Xin chao! Vui long doi");
     displayOLED.display();
     delay(3000);
     displayOLED.clearDisplay();
  }

  if(digitalRead(keuMoCua) == LOW){
     ChuongKeu(100);
     displayOLED.clearDisplay();
     digitalWrite(lock, HIGH);
     displayOLED.setCursor(0,20);
     displayOLED.print("Xin chao! Moi vao ...");
     displayOLED.display();
     delay(3000);
     digitalWrite(lock, LOW);
     displayOLED.clearDisplay();
  }

  manHinhChinh:

  BanPhim = customKeypad.getKey();
  digitalWrite(bell, LOW);
  digitalWrite(lock, LOW);
  
  if(digitalRead(congTac)==HIGH){               // Khi cửa công tắc hành tự động bật mà ko theo thuật toán

    yes = 0;
    bienDem=0;
    while(1){
        for(int k=0; k<3; k++){
            Canh_Bao();
        }
        
        sent_DATA("1");
          

        if(digitalRead(congTac)==LOW){
          displayOLED.clearDisplay();
          sent_DATA("0");
          break;
        }
    }
  } else {
    reset_sentData();
  }
  
  if (digitalRead(btnOpen) == LOW){           // Khi nhấn nút mở cửa
     
     bienDem=0;
     ChuongKeu(100);
     digitalWrite(lock, HIGH);
     while(1){
        Open_Lock();
        delay(3000);

        sent_DATA("1");
        
        if(digitalRead(congTac)==LOW){
            displayOLED.clearDisplay();
            sent_DATA("0");
            break;
        } else {
            digitalWrite(lock, LOW);
        }
     }
  }


if (BanPhim=='D'){
  ChuongKeu(100);
  displayOLED.clearDisplay();
  i=0;

  while(1) {
    BanPhim = customKeypad.getKey();
    Start_Systems();

    if (BanPhim && BanPhim!='#' && BanPhim!='D' && BanPhim!='A' && BanPhim!='C' && BanPhim!='B'){
      ChuongKeu(100);
      OLED_Line2((String)BanPhim);
      read_pass[i++] = BanPhim;
    }
    
    if (BanPhim=='*' || digitalRead(congTac)==HIGH){
        i=0;
        ChuongKeu(100);
        Return();
        break;
    }
    
    if (i==length_mk){
      yes=0;
      bienDem=0;
      delay(200);
      for (int j=0; j<length_mk; j++)
        new_password[j] = EEPROM.read(j);
      i = 0;
      if(!(strncmp(read_pass, new_password, length_mk))){
        displayOLED.clearDisplay();
        digitalWrite(lock, HIGH);
        
        while(1){
          BanPhim = customKeypad.getKey();
          OLED_Line2("      Chinh Xac");
          OLED_Line3("  --- OPEN LOCK ---");
          
          sent_DATA("1");
          
          delay(3000);

          if (BanPhim=='*' || digitalRead(congTac)==LOW){
            ChuongKeu(100);
            reset_sentData();
            Return();
            break;
          } else {
            digitalWrite(lock, LOW);
          }
        }
      } else {
        displayOLED.clearDisplay();
        OLED_Line2("   Khong chinh xac");
        delay(2000);
        displayOLED.clearDisplay();
        }
      }
    
  }
}



  if (BanPhim == 'A'){              // Options Đổi mật khẩu
    i=0;
    displayOLED.clearDisplay();
    ChuongKeu(100);

    nhapLai:

    while(i<length_mk){
      BanPhim = customKeypad.getKey();
      Enter_OldPass();
      
      if (BanPhim && BanPhim!='A' && BanPhim!='B' && BanPhim!='C' && BanPhim!='D' && BanPhim!='#'){
        ChuongKeu(100);
        OLED_Line2((String)BanPhim);        // Nhập mật khẩu cũ
        create_password[i++] = BanPhim; 
      }
  
      if (BanPhim == '*' || digitalRead(congTac)==HIGH){          // Thoát options A
        ChuongKeu(100);
        Return();
        break;
      }
    }

    if (i==length_mk){                    // True OldPassword

      for (int j=0; j<length_mk; j++)     // Đọc bộ nhớ EEPROM
         password[j] = EEPROM.read(j);
            
      if(!(strncmp(create_password, password, length_mk))){     // Đúng OldPass thì next nhập mật khẩu mới
        displayOLED.clearDisplay();
        OLED_Line3("Nhap mat khau moi");
        i=0;

        while(i<length_mk){
          BanPhim = customKeypad.getKey();
          Set_Password();

          if (BanPhim=='*' && BanPhim=='A' && BanPhim=='B' && BanPhim=='C' && BanPhim=='D' && BanPhim=='#'){
            i=0;
            displayOLED.clearDisplay();
            goto nhapLai;
          }

          if (digitalRead(congTac)==HIGH) {
            ChuongKeu(100);
            Return();
            break;
          }
    
          if (BanPhim){
            ChuongKeu(100);
            OLED_Line2((String)BanPhim);
            new_password[i++] = BanPhim;      // Lưu mật khẩu mới (TẠM)
          }
        }
      
        if (i==length_mk){
            displayOLED.clearDisplay();
            i=0;
            while(1){
              BanPhim = customKeypad.getKey();
              Confirm_password();
    
              if (BanPhim && BanPhim!='A' && BanPhim!='B' && BanPhim!='C' && BanPhim!='D' && BanPhim!='#'){
                ChuongKeu(100);
                OLED_Line2((String)BanPhim);
                confirm_pass[i++] = BanPhim;      // Lưu xác nhận mật khẩu mới (TẠM)
              }
    
              if (i==length_mk){

                bienDem=0;
                
                if(!(strncmp(confirm_pass, new_password, length_mk))){    // So sánh mật khẩu mới và xác nhận
                  displayOLED.clearDisplay();
                  OLED_Line2(" Thanh cong");
                  for(int j=0; j<length_mk; j++)
                      EEPROM.write(j, (int)new_password[j]);             // Đúng, thì lưu vào EEPROM
                  delay(2000);
                  
                  sent_DATA("0");
                  
                  Return();
                  break;
                  
                } else {
                  displayOLED.clearDisplay();         // Sai, thì nhập lại xác nhận mật khẩu mới
                  OLED_Line3("Khong trung khop");
                  i=0;
                }
              }
              if (BanPhim == '*' || digitalRead(congTac)==HIGH){      // Bấm phím * để thoát change password
                ChuongKeu(100);
                Return();
                break;
              }
            }
          }  
      } else {
        displayOLED.clearDisplay();
        OLED_Line3("Khong chinh xac");      // Nếu mật khẩu cũ nhập sai thì về màn hình chính
        i=0;
        delay(1000);
        goto nhapLai;
      }
    } 
  }




  if (BanPhim=='C'){                // Options Vân Tay
    
    ChuongKeu(100);
    i=0;
    boolean totalID=true;
    id_finger=0;
    readID="";
    displayOLED.clearDisplay();
    while(1){
      BanPhim = customKeypad.getKey();
      XoaMatKhau();

      if (BanPhim && BanPhim!='*' && BanPhim!='#' && BanPhim!='D' && BanPhim!='A' && BanPhim!='B' && BanPhim!='C'){
          ChuongKeu(100);
          OLED_Line2((String)BanPhim);
          read_pass[i++] = BanPhim;
      }

      if (i==length_mk){
          for (int j=0; j<length_mk; j++)
              new_password[j] = EEPROM.read(j);
          i = 0;
          if(!(strncmp(read_pass, new_password, length_mk))){
              displayOLED.clearDisplay();
              OLED_Line2("      Chinh Xac!");
              delay(2000);
              displayOLED.clearDisplay();
              break;
          } else {
              displayOLED.clearDisplay();
              OLED_Line2("   Khong chinh xac");
              delay(2000);
              displayOLED.clearDisplay();
           }
       }
       if (BanPhim=='*' || digitalRead(congTac)==HIGH){
            ChuongKeu(100);
            Return();
            goto thoatC;
       }
    }
    
    veDay:
    i=0;
    totalID=true;
    id_finger=0;
    readID="";
    displayOLED.clearDisplay();
    
    while(1){
      BanPhim = customKeypad.getKey();
      OptionVanTay();
      
      if (totalID==true){
        for(int j=10; j<15; j++){
            confirm_finger[j-10] = EEPROM.read(j);
            if(confirm_finger[j-10]!="0"){
               id_finger++;
             }
          }
          totalID=false;
      }

      if (BanPhim=='1'){
        ChuongKeu(100);
        
        displayOLED.clearDisplay();
        OLED_Line2("Them id van tay");
        delay(1000);
        i=0;
        displayOLED.clearDisplay();

        if(id_finger >= 5){
           displayOLED.clearDisplay();
           OLED_Line1("        !!!");
           OLED_Line2("Qua gioi han van tay");
           OLED_Line3(" Khong the them nua");
           delay(2000);
           goto veDay;
        }

        while(1){
          BanPhim = customKeypad.getKey();
          ThemVanTay();
          dataEmpty = 0;

          for(int j=10; j<15; j++){
             read_finger[j-10] = EEPROM.read(j);
             if (read_finger[j-10] == "0"){
                dataEmpty++;
                choNayTrong = j-10;
                id_finger = dataEmpty;
              }
           }  

          while (BanPhim=='#' && readID!=0){
             ChuongKeu(100);
             i=0;
             displayOLED.clearDisplay();
             OLED_Line3("Kiem tra ID " + readID);
             delay(2000);

             for(int j=0; j<5; j++){
                if (read_finger[j]==readID ){
                   displayOLED.clearDisplay();
                   OLED_Line2("...Trung Ten ID...");
                   delay(2000);
                   choNayTrong = 0;
                   goto veDay;
                }
             }
                   name_finger[choNayTrong] = readID;
                   displayOLED.clearDisplay();
                   OLED_Line2("Cho chut ...Wait!");
                   delay(3000);
                   displayOLED.clearDisplay();

                   while(! getFingerprintEnroll());
                   EEPROM.write(choNayTrong+10, readID.toInt());             // Đúng, thì lưu vào EEPROM

                   displayOLED.clearDisplay();
                   choNayTrong = 0;
                   OLED_Line2("   Thanh cong  !!!");
                   delay(2000);

                   bienDem=0;
                   sent_DATA("0");
                   
                   Return();
                   goto veDay; 
             
          }

          if (BanPhim!='#' && BanPhim){
             ChuongKeu(100);
             OLED_Line2((String)BanPhim);
             confirm_pass[i++] = BanPhim;      // Lưu xác nhận mật khẩu mới (TẠM)
             
             readID += BanPhim;
          }

          if (BanPhim=='*' || digitalRead(congTac)==HIGH){
            ChuongKeu(100);
            choNayTrong = 0;
            Return();
            break;
          }
        }
      }

      if (BanPhim=='2'){
        ChuongKeu(100);
        displayOLED.clearDisplay();
        OLED_Line3("Xoa id van tay");
        delay(1000);
        i=0;
        displayOLED.clearDisplay();
        
        while(1){
          BanPhim = customKeypad.getKey();
          XoaVanTay();

          for (int j=10; j<15; j++){
             read_finger[j-10] = EEPROM.read(j);
          }

          if (BanPhim=='#' && readID!=0){
             i=0;
             displayOLED.clearDisplay();
             OLED_Line2("Dang tim ID " + readID);
             delay(2000);
             displayOLED.clearDisplay();        

          while(1){
             for(int j=0; j<5; j++){
                if(read_finger[j]==readID){
                    OLED_Line2("...Da tim thay...");
                    delay(2000);

                    displayOLED.clearDisplay();
                    deleteFingerprint(readID.toInt());
                    delay(2000);
                    
                    displayOLED.clearDisplay();
                    read_finger[j] = "0";
                    EEPROM.write(j+10, read_finger[j].toInt());
                    OLED_Line2("Da xoa ID " + readID);
                    delay(2000);
                    
                    bienDem=0;
                    sent_DATA("0");
                    goto veDay;
                  }
               }
                  OLED_Line2("Khong tim thay ID " + readID);
                  delay(1000);
                  displayOLED.clearDisplay();
                  goto veDay;
            }
                  
          }

          if (BanPhim && BanPhim!='#'){
             ChuongKeu(100);
             OLED_Line2((String)BanPhim);
             confirm_pass[i++] = BanPhim;      // Lưu xác nhận mật khẩu mới (TẠM)

             readID += BanPhim;
          }

          if (BanPhim=='*' || digitalRead(congTac)==HIGH){
            ChuongKeu(100);
            Return();
            break;
          }
        }
      }

      if (BanPhim=='*' || digitalRead(congTac)==HIGH){
        ChuongKeu(100);
        Return();
        break;
      }
    }
  }
  thoatC:


  
  if (BanPhim=='B'){                // Options xóa EEPROM mật khẩu
    veDay2:
    i=0;
    ChuongKeu(100);

    while(i<length_mk){
      thoat1:
      
      displayOLED.clearDisplay();
      BanPhim = customKeypad.getKey();
      Question_DeletePass();

      if (BanPhim=='*' || digitalRead(congTac)==HIGH){
        ChuongKeu(100);
        Return();
        break;
      }

      if (BanPhim=='1'){                      // Bấm phím 1 thì tự động xóa EEPROM
        ChuongKeu(100);
        displayOLED.clearDisplay();
        i=0;

        while(1){
           BanPhim = customKeypad.getKey();
           XoaMatKhau();

           for (int j=0; j<length_mk; j++)
              read_pass[j] = EEPROM.read(j);
           
           if(BanPhim){
              ChuongKeu(100);
              OLED_Line2((String)BanPhim);
              delete_pass[i++] = BanPhim;      // Lưu xác nhận mật khẩu mới (TẠM)
           }         

           if(i==length_mk){
              i=0;
              if(!(strncmp(read_pass, delete_pass, length_mk))){
                displayOLED.clearDisplay();
                OLED_Line2("Xac nhan xoa mat khau");
                delay(2000);
                i=0;
                break;
             } else {
                displayOLED.clearDisplay();
                OLED_Line2("  Khong chinh xac  ");
                OLED_Line3("   Hay thu lai...  ");
                delay(2000);
                i=0;
                displayOLED.clearDisplay();
                goto veDay2;
             }
           }

           if (BanPhim=='*'){
              ChuongKeu(100);
              Return();
              goto thoat1;
           }

           if (digitalRead(congTac)==HIGH){
              goto manHinhChinh;
           }
        }

        
        displayOLED.clearDisplay();
        OLED_Line2("Dang xoa mat khau");
        delay(1000);

        for (int j=0; j<length_mk; j++)
            EEPROM.write(j, 0);

        displayOLED.clearDisplay();
        OLED_Line2("Xoa thanh cong");
        bienDem=0;
        sent_DATA("0");
        delay(1000);
        
      }
  
      if (BanPhim=='2'){                // Bấm phím 2 thì tạo mới mật khẩu
        ChuongKeu(100);
        int a = EEPROM.read(1);
        
        if(a!=0){
           displayOLED.clearDisplay();
           OLED_Line2("     Ban phai xoa  ");
           OLED_Line3("  mat khau cu truoc");
           delay(3000);
           i=0;
           Return();
           break;
          }

          i=0;
          displayOLED.clearDisplay();
          while(i<length_mk){
            BanPhim = customKeypad.getKey();
            New_Password();
      
            if (BanPhim){
              ChuongKeu(100);
              OLED_Line2((String)BanPhim);
              new_password[i++] = BanPhim;
            }

            if (i==length_mk){
              i=0;
              displayOLED.clearDisplay();
              
              while(1){
                BanPhim = customKeypad.getKey();
                Confirm_password();
      
                if (BanPhim){
                  ChuongKeu(100);
                  OLED_Line2((String)BanPhim);
                  confirm_pass[i++] = BanPhim;      // Lưu xác nhận mật khẩu mới (TẠM)
                }

                if (BanPhim=='*' || digitalRead(congTac)==HIGH){
                  Return();
                  break;
                }
      
                if (i==length_mk){
                  if(!(strncmp(confirm_pass, new_password, length_mk))){    // So sánh mật khẩu mới và xác nhận
                    displayOLED.clearDisplay();
                    OLED_Line2(" Thanh cong");
                    for(int j=0; j<length_mk; j++)
                        EEPROM.write(j, new_password[j]);             // Đúng, thì lưu vào EEPROM
                    delay(2000);
                    bienDem=0;
                    sent_DATA("0");
                    Return();
                    break;
                    
                  } else {
                    displayOLED.clearDisplay();         // Sai, thì nhập lại xác nhận mật khẩu mới
                    OLED_Line3("Khong trung khop");
                    i=0;
                  }
                }
                if (BanPhim == '*' || digitalRead(congTac)==HIGH){      // Bấm phím * để thoát change password
                  ChuongKeu(100);
                  Return();
                  break;
                }
              }
              break;    // Về giao diện options B
            }

            if (BanPhim=='*' || digitalRead(congTac)==HIGH){
              ChuongKeu(100);
              Return();
              break;
            }
          }
      }
    }
  }


  if (hienThi==true){
    getFingerprintID();
  }
  
 

}

//---- end loop ----




void XoaMatKhau(){
  displayOLED.setTextSize(1);
  displayOLED.setCursor(0,0);
  displayOLED.print("Nhap mat khau...");
  displayOLED.setCursor(0,20);
  displayOLED.print("Bam <- de tro ve");
  displayOLED.display();
}


void ThemVanTay(){
  displayOLED.setTextSize(1);
  displayOLED.setCursor(0,0);
  displayOLED.print("Nhap id van tay moi");

  displayOLED.setCursor(0,20);
  displayOLED.print("Tong ID ");

  displayOLED.setCursor(50,20);
  displayOLED.print(id_finger);

  displayOLED.setCursor(70,20);
  displayOLED.print("(ID < 111)");
  
  displayOLED.display();
}


void XoaVanTay(){
  displayOLED.setTextSize(1);
  displayOLED.setCursor(0,0);
  displayOLED.print("Nhap id muon xoa");
  displayOLED.display();
}


void OptionVanTay(){
  displayOLED.setTextSize(1);
  displayOLED.setCursor(0,0);
  displayOLED.print("Thiet lap van tay");

  displayOLED.setCursor(0,10);
  displayOLED.print("Bam 1:Them id van tay");

  displayOLED.setCursor(0,20);
  displayOLED.print("Bam 2:Xoa id van tay");
  displayOLED.display();
  
}


void ChuongKeu(int ring){
  digitalWrite(bell, HIGH);
  delay(ring);
  digitalWrite(bell, LOW);
}


void Question_DeletePass(){
  displayOLED.setTextSize(1);
  displayOLED.setCursor(0,0);
  displayOLED.print("Tuy chon he thong");

  displayOLED.setCursor(0,10);
  displayOLED.print("Bam 1: Xoa mat khau");

  displayOLED.setCursor(0,20);
  displayOLED.print("Bam 2: Tao mat khau");
  
  displayOLED.display();
}


void Confirm_password(){
  displayOLED.setTextSize(1);
  displayOLED.setCursor(0,0);
  displayOLED.print("Xac nhan mat khau");
  displayOLED.display();
  
}


void OLED_Line3(String rum){
  displayOLED.setTextSize(1);
  displayOLED.setCursor(0,20);
  displayOLED.print(rum);
  displayOLED.display();
}


void OLED_Line1(String rum){
  displayOLED.setTextSize(1);
  displayOLED.setCursor(7*i,0);
  displayOLED.print(rum);
  displayOLED.display();
}


void OLED_Line2(String rum){
  displayOLED.setTextSize(1);
  displayOLED.setCursor(7*i,10);
  displayOLED.print(rum);
  displayOLED.display();
}


void New_Password(){
  displayOLED.setTextSize(1);
  displayOLED.setCursor(0,0);
  displayOLED.print("Nhap mat khau (5 so)");
  displayOLED.display();

}


void Return(){
  i=0;
  hienThi = true;
  displayOLED.clearDisplay();
}


void Set_Password(){
  hienThi = false;
  displayOLED.setTextSize(1);
  displayOLED.setCursor(0,0);
  displayOLED.print("Mat khau moi");
  displayOLED.display();
}


void Enter_OldPass(){
  hienThi = false;
  displayOLED.setTextSize(1);
  displayOLED.setCursor(0,0);
  displayOLED.print("Nhap mat khau cu");
  displayOLED.display();
}


void Start_Systems(){
  
////  displayOLED.setFont(&FreeMono9pt7b);
//  displayOLED.setTextSize(1);
  displayOLED.setCursor(0, 0);
  displayOLED.print("Nhap mat khau");
  displayOLED.setCursor(0,20);
  displayOLED.print("Bam <- de thoat");
  displayOLED.display();
}


void Open_Lock(){
 
  displayOLED.clearDisplay();
  OLED_Line2("  MO CUA - OPEN LOCK");
  displayOLED.display();

}


void Canh_Bao(){
  ChuongKeu(300);
  displayOLED.clearDisplay();
  displayOLED.setCursor(0,0);
  displayOLED.print("       CANH BAO");
  displayOLED.setCursor(0,10);
  displayOLED.print("       Nguy hiem");
  displayOLED.setCursor(0,20);
  displayOLED.print("  Mo cua khong hop le");
  displayOLED.display();
  delay(300);
}


uint8_t getFingerprintEnroll() {        // Nạp vân tay

  int p = -1;
   
  while (p != FINGERPRINT_OK) {
    p = vanTay.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      displayOLED.clearDisplay();
      OLED_Line1("Quet thanh cong");
      break;
    case FINGERPRINT_NOFINGER:
      displayOLED.clearDisplay();
      OLED_Line1(" Quet van tay moi ...");
      OLED_Line3("Dat tay len may quet");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      displayOLED.clearDisplay();
      OLED_Line1("Mat ket noi. Thu lai");
      break;
    case FINGERPRINT_IMAGEFAIL:
      displayOLED.clearDisplay();
      OLED_Line1("Van tay ivailid");
      break;
    default:
      displayOLED.clearDisplay();
      OLED_Line1("Unknown error");
      break;
    }
  }

  delay(1000);
  displayOLED.clearDisplay();
  // OK success!

  p = vanTay.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      displayOLED.clearDisplay();
      OLED_Line2("...wait...");
      break;
    case FINGERPRINT_IMAGEMESS:
      displayOLED.clearDisplay();
      OLED_Line2("Van tay trung lap");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      displayOLED.clearDisplay();
      OLED_Line2("Mat ket noi. Thu lai");
      return p;
    default:
      displayOLED.clearDisplay();
      OLED_Line2("Unknown error");
      return p;
  }

  delay(1000);
  displayOLED.clearDisplay();

  
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = vanTay.getImage();
  }
 
  p = -1;
  OLED_Line1("Quet van tay lan 2");
  delay(1000);
  while (p != FINGERPRINT_OK) {
    p = vanTay.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      displayOLED.clearDisplay();
      OLED_Line2("Quet thanh cong");
      break;
    case FINGERPRINT_NOFINGER:
      displayOLED.clearDisplay();
      OLED_Line2("Tiep tuc giu tren may");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      displayOLED.clearDisplay();
      OLED_Line2("Mat ket noi. Thu lai");
      break;
    default:
      displayOLED.clearDisplay();
      OLED_Line2("Unknown error");
      break;
    }
  }

  // OK success!
  delay(1000);
  displayOLED.clearDisplay();

  
  p = vanTay.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      displayOLED.clearDisplay();
      OLED_Line3("...wait...");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      displayOLED.clearDisplay();
      OLED_Line3("Mat ket noi. Thu lai");
      return p;
    default:
      displayOLED.clearDisplay();
      OLED_Line3("Unknown error");
      return p;
  }

  // OK converted!
  delay(1000);
  displayOLED.clearDisplay();
  

  p = vanTay.createModel();
  if (p == FINGERPRINT_OK) {
    OLED_Line1("Trung khop ...");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    OLED_Line1("Mat ket noi");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    OLED_Line1("Khong trung khop");
    return p;
  } else {
    OLED_Line1("Unknown error");
    return p;
  }

  delay(1000);
  displayOLED.clearDisplay();
  
  p = vanTay.storeModel(readID.toInt());
  if (p == FINGERPRINT_OK) {
    OLED_Line2("Luu thanh cong!");
    sent_DATA("0");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    OLED_Line2("Mat ket noi");
    return p;
  } else {
    OLED_Line2("Unknown error");
    return p;
  }
  delay(1000);

  return true;
}


uint8_t deleteFingerprint(uint8_t id_finger) {    // Xóa vân tay
  uint8_t p = -1;

  p = vanTay.deleteModel(readID.toInt());

  if (p == FINGERPRINT_OK) {
    OLED_Line2("Xoa thanh cong!");
    sent_DATA("0");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    OLED_Line2("Mat ket noi");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    OLED_Line2("Khong the xoa (!ID)");
    return p;
  } else {
    OLED_Line2("Unknown error: 0x"); 
    //OLED_Line3(p, HEX);
    return p;
  }
  
}


uint8_t getFingerprintID() {          // Quét vân tay
  
  uint8_t p = vanTay.getImage();
  displayOLED.clearDisplay();
  
  switch (p) {
    
    case FINGERPRINT_OK:
    
        OLED_Line1("Quet thanh cong");
  
        p = vanTay.image2Tz();
        switch (p) {
          case FINGERPRINT_OK:
            OLED_Line2("...wait...");
            break;
          case FINGERPRINT_PACKETRECIEVEERR:
            OLED_Line2("Mat ket noi");
            return p;
          default:
            OLED_Line2("Unknown error");
            return p;
        }

        p = vanTay.fingerSearch();
        if (p == FINGERPRINT_OK) {
          OLED_Line3("Phu hop!");
        } else if (p == FINGERPRINT_NOTFOUND) {
          OLED_Line3("Khong thay ID phu hop");
          return p;
        } else {
          OLED_Line3("Unknown error");
          return p;
        }
      
        delay(1000);
        displayOLED.clearDisplay();
      
        displayOLED.setCursor(0,0);
        displayOLED.print("Hi! ID #"); 
        displayOLED.setCursor(50,0);
        displayOLED.print(vanTay.fingerID);
        displayOLED.display();
      
        digitalWrite(lock, HIGH);
        bienDem=0;
        yes=0;

        while(1){
          BanPhim = customKeypad.getKey();
          sent_DATA("1");

          if (BanPhim=='*'){
              ChuongKeu(100);
              bienDem=0;
              break;
          }
      
          delay(3000);
          if (digitalRead(congTac)==LOW){
              ChuongKeu(100);
              reset_sentData();
              break;
          } else {
            digitalWrite(lock, LOW);
          }
        }
        
        displayOLED.clearDisplay();
        choPhep = true;
        return vanTay.fingerID;
      
      break;
    case FINGERPRINT_NOFINGER:
      OLED_Line2("Quet Van Tay");
      return p;
    default:
      OLED_Line1("Unknown error");
      return p;
  }
}


void sent_DATA(String trangthaikhoa){

  if(bienDem==0){
  int t=0;
  while (t==0){
        
    for (int j=0; j<length_mk; j++){
      int bienTam = EEPROM.read(j);
      if (bienTam == 0){
        bienTam = 0;
      } else {
      bienTam = bienTam-48;
      }
      sent_pass+=bienTam;
      read_finger[j] = EEPROM.read(j+10);
      }
      
      String vantay1 = read_finger[0];
      String vantay2 = read_finger[1];
      String vantay3 = read_finger[2];
      String vantay4 = read_finger[3];
      String vantay5 = read_finger[4];
      
      String guiDuLieu = sent_pass + "!" + vantay1 + "@" + vantay2 + "#" + vantay3 + "$" + vantay4 + "%" + vantay5 + "^" + trangthaikhoa;
      sent_pass = "";
      Serial.println(guiDuLieu);
      t=1;
    }
    bienDem=10;
  }

  if (Serial.available() > 0){
      Chuoi = Serial.readString();

      if (Chuoi.length() <= 1){
         ChuongKeu(100);
         displayOLED.clearDisplay();
         displayOLED.setCursor(0,20);
         displayOLED.print("Kiem tra internet...");
         displayOLED.display();
         delay(1000);
         displayOLED.clearDisplay();
      }

    
      if (Chuoi.length() > 4 && Chuoi.length() <=15) {

         ChuongKeu(100);
         displayOLED.clearDisplay();
         displayOLED.setCursor(0,20);
         displayOLED.print("..Da mo cua..");
         displayOLED.display();
         ChuongKeu(100);
         digitalWrite(lock, HIGH);
         delay(3000);
         digitalWrite(lock, LOW);
         displayOLED.clearDisplay();
      }
  
  }
  

}


void reset_sentData(){
  yes++;
    if (yes == 1){
      bienDem=0;
      sent_DATA("0");
    }

    if (yes == 100){
      yes = 2;
    }
}
