#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

int selectionButton = 9;
int confirmationButton = 8;
int maxWordLength = 5;

bool isStarting = true;
bool isPcArduinoMode = false;
bool isDebugMode = false;
bool isExternalChoice = false;

char alphabet[] = "aeiou";
int currentLetterIndex = 0;
String message = "";
unsigned long lastButtonPressTime = 0;
unsigned long lastTime = 0;

void setup()
{
    lcd.begin(16, 2);
    pinMode(selectionButton, INPUT);
    pinMode(confirmationButton, INPUT);
    Serial.begin(9600);
    lcd.setCursor(0, 0);
    lcd.print("Escolha:");
    lcd.setCursor(0, 1);
    lcd.print("( ) PC - ( ) Ino");
}

void loop()
{
    if (Serial.available() > 0)
    {
        String receivedChoice = Serial.readString();
        char firstCharacter = receivedChoice[0];

        switch (firstCharacter)
        {
            case 'p':
                isStarting = true;
                isPcArduinoMode = true;
                isExternalChoice = true;
                break;
                
            case 'a':
                isStarting = true;
                isPcArduinoMode = false;
                isExternalChoice = true;
                break;
        }
    }

    if (isDebugMode)
    {
        restartSystem();
        return;
    }

    if (isStarting)
    {
        updateDisplayMode();

        if (digitalRead(confirmationButton) || isExternalChoice)
        {
            lcd.clear();

            if (isPcArduinoMode)
            {
                if (!isExternalChoice)
                {
                    Serial.print('p');
                }

                lcd.setCursor(0, 0);
                lcd.print("Aguardando...");

                while (1)
                {
                    if (Serial.available())
                    {
                        message = Serial.readString();
                        lcd.setCursor(0, 0);
                        lcd.print("Msg recebida:");
                        lcd.setCursor(0, 1);
                        lcd.print(message);
                        delay(5000);
                        restartSystem();
                        break;
                    }
                }
            }
            else
            {
                if (!isExternalChoice)
                {
                    Serial.print('a');
                }
                
                lcd.setCursor(0, 0);
                lcd.print("Escreva:");
                isStarting = false;
                lastButtonPressTime = millis();
            }
        }
    }
    else
    {
        if (!isPcArduinoMode)
        {
            if (digitalRead(selectionButton))
            {
                currentLetterIndex = (currentLetterIndex + 1) % 5;
                lastButtonPressTime = millis();
                delay(100);
            }

            if (millis() - lastButtonPressTime > 2000)
            {
                message += alphabet[currentLetterIndex];
                lastButtonPressTime = millis();
            }

            if ((digitalRead(confirmationButton) && message.length() >= 5) || (message.length() >= maxWordLength))
            {
                sendMessage(message);
                restartSystem();
            }
            else
            {
                lcd.setCursor(0, 1);
                lcd.print(message + alphabet[currentLetterIndex]);
            }
        }
    }

    delay(500);
}

void updateDisplayMode()
{
    if (!isExternalChoice)
    {
        if (digitalRead(selectionButton))
        {
            isPcArduinoMode = !isPcArduinoMode;
        }
        
        lcd.setCursor(0, 1);
        lcd.print(isPcArduinoMode ? "(X) PC - ( ) Ino" : "( ) PC - (X) Ino");
    }
}

void sendMessage(String message)
{
    Serial.print(message);
    lastTime = millis();
    isDebugMode = true;
}

void restartSystem()
{
    if (millis() - lastTime > 5000)
    {
        Serial.flush();
        currentLetterIndex = 0;
        message = "";
        isStarting = true;
        isDebugMode = false;
        isPcArduinoMode = false;
        isExternalChoice = false;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Escolha:");
        lcd.setCursor(0, 1);
        lcd.print("( ) PC - ( ) Ino");
    }
}
