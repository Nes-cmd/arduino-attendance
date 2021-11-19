#include <LiquidCrystal.h>
#include <SPI.h>
#include <SD.h>
File myFile;
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
String menu[] = {
  "  MENU",
  "Create Att",
  "Take Att",
  "Analysis",
  "Clear All",
};
int pres,total,pos = 1,menuSize = 5, lineLength = 45,oldPos;
String fileName = "ATT.txt";
void setup() {
  lcd.begin(16,4);
  Serial.begin(115200);
  lcd.print("Initializing SD card...");
  if (!SD.begin(8)) {
    lcd.print("Initialization failed!");
    while (1);
  }
  lcd.print("Initialization done.");
  delay(2000);
  totalAtt();
  printF();
  menuUpdate(pos);
}

void loop() {
  String id = "";
  while(Serial.available() > 0){
    id = Serial.readString();
    id = id.substring(0,5);
  }
  
  int button = analogRead(A0);
  if(button > 1010){
    oldPos = pos;
    lcd.clear();
    lcd.print("Mode :");
    lcd.print(menu[pos-1]);
    if(pos == 2){
      createAtt();
    }
    else if(pos == 4){
      lcd.clear();
      lcd.print("Total Attendance");
      lcd.setCursor(0,1);
      lcd.print("created");
      lcd.setCursor(0,2);
      lcd.print(total);
    }
    else if(pos == 5){
      clearAll();
    }
    delay(240);
  }
  else if(button > 500 && button < 520){
    pres = 1;
    menuUpdate(possitioner(pres));
    delay(240);
  }
  else if(button > 665 && button < 685){
    pres = 2;
    menuUpdate(possitioner(pres));
    delay(240);
  }
  
  if(oldPos == 3 && id.length() >= 5){
    lcd.clear();
    lcd.print(menu[oldPos-1]);
    lcd.print(" Mode:");
    takeAtt(id);
  }
}
void takeAtt(String id){
  int loc = findLocation(id);
  String x = "";
  lcd.setCursor(0,1);
  if(loc != -1){
    x = readLine(loc);
    if(x != ""){
      String y = x.substring(9,13);
      String att = x.substring(5,9);
      lcd.setCursor(0,1);
      lcd.print(x.substring(14));
      lcd.setCursor(0,2);
      if(y.toInt() < total){
        updateLine(loc,att.toInt()+1);
        lcd.print("Atend = ");
        lcd.print(((att.toInt()+1.0)/total)*100);
        lcd.print("%");
      }
      else{
        lcd.print("Already Atended");
        lcd.setCursor(0,3);
        lcd.print(((att.toInt()+0.0)/total)*100);
        lcd.print("%");
      }
    }
  }
  else{
    lcd.print("Unregistered id");
  }
}
void createAtt(){
  total++;
  myFile = SD.open("ATTTOT.txt", O_WRITE);
  myFile.print(total);
  
  myFile.close();
  lcd.clear();
  lcd.print("Attendance created");
  lcd.setCursor(0,3);
  lcd.print("Total = ");
  lcd.print(total);
}
void totalAtt(){
  myFile = SD.open("ATTTOT.txt");
  String x = myFile.readString();
  total = x.toInt();
  myFile.close();
}
String readLine(int loc){
  myFile = SD.open(fileName);
  Serial.println(loc);
  myFile.seek(loc*lineLength);
  String x = myFile.readStringUntil('|');
  myFile.close();
  return x;
}
void updateLine(int loc, int att){
  myFile = SD.open(fileName, O_WRITE);
  myFile.seek(loc*lineLength+6);
  myFile.print(att);
  myFile.seek(loc*lineLength+10);
  myFile.print(total);
  myFile.close();
}
int findLocation(String id){
  myFile = SD.open(fileName);
  int line = myFile.size()/lineLength;
  int mid, left = 0,right = line;
  mid = (left+right)/2;
  while(left <= right){
    myFile.seek(mid*lineLength);
    String data = myFile.readStringUntil(' ');
    if(data.toInt() == id.toInt()){
      myFile.close();
      return mid;
    }
    else if(id.toInt() < data.toInt()){
      right = mid-1;
    }
    else{
      left = mid+1;
    }
    mid = (left+right)/2;
  }
  myFile.close();
  return -1;
}
int possitioner(int pres){
  if(pres == 2){
    pos++;
    if( pos > menuSize ){
      pos = menuSize;
    }
  }
  else if(pres == 1){
    pos--;
    if(pos < 1){
      pos = 1;
    }
  }
  return pos;
}
void printF(){
    myFile = SD.open(fileName);
    while (myFile.available()) {
      Serial.write(myFile.read());}
   myFile.close();
}
void menuUpdate(int current){
  lcd.clear();
  int init = 0,last = 4;
  if(current > 4){
    init = current - 4;
    last = current;
  }
  int cursorpos = 0;
  for(int i = init;i < last;i++){
    if(current == (i+1)){
      lcd.setCursor(2,cursorpos);
      lcd.print(">");
      lcd.print(menu[i]);
    }
    else{
      lcd.setCursor(3,cursorpos);
      lcd.print(menu[i]);
    }
    cursorpos++;
  }
}
void clearAll(){
  myFile = SD.open(fileName, O_WRITE);
  int s = myFile.size();
  s = s/lineLength;
  for(int i = 0;i<s;i++){
    myFile.seek( i*lineLength + 6);
    myFile.print("0   0   ");
  }
  myFile.close();
  total = -1;
  createAtt();
  lcd.setCursor(0,1);
  lcd.print("All data cleared");
}
