#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <stdbool.h>
#include <process.h>

void clearConsole()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

HANDLE openSerialPort(const char *port)
{
    HANDLE hSerial = CreateFile(port, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hSerial == INVALID_HANDLE_VALUE)
    {
        printf("Erro ao abrir a porta serial!\n");
    }
    else
    {
        DCB dcbSerialParams = {0};
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

        if (!GetCommState(hSerial, &dcbSerialParams))
        {
            printf("Erro ao obter configurações da porta serial!\n");
        }
        else
        {
            dcbSerialParams.BaudRate = CBR_9600;
            dcbSerialParams.ByteSize = 8;
            dcbSerialParams.StopBits = ONESTOPBIT;
            dcbSerialParams.Parity = NOPARITY;

            if (!SetCommState(hSerial, &dcbSerialParams))
            {
                printf("Erro ao ajustar configurações da porta serial!\n");
            }
        }
    }

    return hSerial;
}

void closeSerialPort(HANDLE hSerial)
{
    CloseHandle(hSerial);
}

void sendData(HANDLE hSerial, const char *data)
{
    DWORD bytesWritten;
    WriteFile(hSerial, data, strlen(data), &bytesWritten, NULL);
}

void receiveData(HANDLE hSerial, char *buffer, int bufSize)
{
    DWORD bytesRead;
    if (ReadFile(hSerial, buffer, bufSize - 1, &bytesRead, NULL))
    {
        buffer[bytesRead] = '\0';
    }
    else
    {
        buffer[0] = '\0';
    }
}

bool isDataAvailable(HANDLE hSerial)
{
    COMSTAT comStat;
    DWORD errors;

    if (ClearCommError(hSerial, &errors, &comStat))
    {
        return (comStat.cbInQue > 0);
    }

    return false;
}

void initialScreen()
{
    clearConsole();
    printf("Sistema de Comunicacao PC-Arduino via Serial\n");
    printf("Integrantes: Cristian, Eduardo Nunes, Vitor Borges e Wanderson Gama\n");
    printf("Deseja avancar? Sim [s]  Nao [n]!\n");
}

volatile char response = '\0';
volatile bool captureInput = true;
unsigned __stdcall inputThread()
{
    while (captureInput)
    {
        char temp;
        if (scanf(" %c", &temp) == 1)
        {
            response = temp;
            break;
        }
        Sleep(100);
    }
    return 0;
}

void communicationScreen()
{
    HANDLE hSerial = openSerialPort("COM9");
    bool isPcArduinoMode = false;
    bool isCommunicationChosen = false;
    while (1)
    {

        captureInput = true;
        _beginthreadex(NULL, 0, inputThread, NULL, 0, NULL);
        while (!isCommunicationChosen)
        {
            clearConsole();
            printf("Qual comunicacao voce deseja utilizar? PC-Arduino [p] Arduino-PC [a]\n");
            printf("Digite sua escolha ou aguarde uma mensagem do Arduino:\n");

            if (response != '\0')
            {
                if (response == 'p' || response == 'P' || response == 'a' || response == 'A')
                {
                    isCommunicationChosen = true;
                    isPcArduinoMode = (response == 'p' || response == 'P');

                    char myWord[] = {'p', '\0'};
                    myWord[0] = response;
                    sendData(hSerial, myWord);
                }
                else
                {
                    printf("Comunicacao nao identificada, tente novamente!\n");
                    response = '\0';
                }
            }

            if (isDataAvailable(hSerial))
            {
                char receivedChoice[2];
                receiveData(hSerial, receivedChoice, 2);
                response = receivedChoice[0];
                if (response == 'p' || response == 'P' || response == 'a' || response == 'A')
                {
                    isCommunicationChosen = true;
                    isPcArduinoMode = (response == 'p' || response == 'P');
                }
            }
            Sleep(1000);
        }

        captureInput = false;
        Sleep(1000);

        if (isPcArduinoMode)
        {
            printf("Comunicacao PC-Arduino identificada!\n");
        }
        else
        {
            printf("Comunicacao Arduino-PC identificada, Aguarde a mensagem!\n");
        }

        if (isPcArduinoMode)
        {
            char message[17];
            fflush(stdout);
            printf("Escreva: ");
            scanf(" %16[^\n]", message);
            sendData(hSerial, message);
        }
        else
        {
            while (1)
            {
                if (isDataAvailable(hSerial))
                {
                    char receivedMessage[10];
                    receiveData(hSerial, receivedMessage, 11);
                    printf("Mensagem recebida do Arduino:\n%s\n", receivedMessage);
                    break;
                }
                Sleep(100);
            }
        }

        Sleep(5000);
        clearConsole();
        isPcArduinoMode = false;
        isCommunicationChosen = false;
        response = '\0';
    }

    closeSerialPort(hSerial);
}

int main()
{
    initialScreen();
    char answer;
    scanf(" %c", &answer);

    if (answer == 's' || answer == 'S')
    {
        communicationScreen();
    }
    else
    {
        printf("Saindo do programa...\n");
    }

    return 0;
}
