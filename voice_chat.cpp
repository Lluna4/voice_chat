#define _WINSOCK_DEPRECATED_NO_WARNINGS
#undef _WINSOCKAPI_
#define _WINSOCKAPI_
#include <iostream>
#include <Windows.h>
#include <fstream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "winmm.lib")

const std::string SERVER_IP = "127.0.0.1";
const int BUFFER_SIZE = 8192;

SOCKET client_socket;

void CALLBACK WaveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    if (uMsg == WIM_DATA)
    {
        WAVEHDR* waveHdr = (WAVEHDR*)dwParam1;
        int bytes_send = send(client_socket, waveHdr->lpData, waveHdr->dwBufferLength, 0);
        std::cout << waveHdr->dwBufferLength << std::endl;
        waveInAddBuffer(hwi, waveHdr, sizeof(WAVEHDR));
    }
}


int main()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }
	client_socket = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(SERVER_IP.c_str());
    server_address.sin_port = htons(5050);

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR)
    {
		std::cerr << "Connect failed: " << WSAGetLastError();
        closesocket(client_socket);
        WSACleanup();
    }

    WAVEFORMATEX wfx;
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 2;
    wfx.nSamplesPerSec = 16000;
    wfx.nAvgBytesPerSec = 16000 * 2 * sizeof(short);
    wfx.nBlockAlign = 2 * sizeof(short);
    wfx.wBitsPerSample = 16;
    wfx.cbSize = 0; 

	HWAVEIN hWaveIn;
    MMRESULT result = waveInOpen(&hWaveIn, WAVE_MAPPER, &wfx, (DWORD_PTR)WaveInProc, 0, CALLBACK_FUNCTION);
    if (result == MMSYSERR_NOERROR)
    {
        // Successfully opened the audio input device
        std::cout << "Audio input device opened successfully." << std::endl;
        
        WAVEHDR waveHdr;
        waveHdr.lpData = (LPSTR)malloc(BUFFER_SIZE * sizeof(short));
        waveHdr.dwBufferLength = BUFFER_SIZE * sizeof(short);
        waveHdr.dwFlags = 0;

        waveInPrepareHeader(hWaveIn, &waveHdr, sizeof(WAVEHDR));
        waveInAddBuffer(hWaveIn, &waveHdr, sizeof(WAVEHDR));

        waveInStart(hWaveIn);
        
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();

        // Stop audio capture
        waveInStop(hWaveIn);
        waveInReset(hWaveIn);

        // Clean up resources
        waveInUnprepareHeader(hWaveIn, &waveHdr, sizeof(WAVEHDR));
        free(waveHdr.lpData);
        waveInClose(hWaveIn);
    }
    else
    {
        // Handle the error
        std::cerr << "Failed to open audio input device." << std::endl;
    }
}

// Ejecutar programa: Ctrl + F5 o menú Depurar > Iniciar sin depurar
// Depurar programa: F5 o menú Depurar > Iniciar depuración

// Sugerencias para primeros pasos: 1. Use la ventana del Explorador de soluciones para agregar y administrar archivos
//   2. Use la ventana de Team Explorer para conectar con el control de código fuente
//   3. Use la ventana de salida para ver la salida de compilación y otros mensajes
//   4. Use la ventana Lista de errores para ver los errores
//   5. Vaya a Proyecto > Agregar nuevo elemento para crear nuevos archivos de código, o a Proyecto > Agregar elemento existente para agregar archivos de código existentes al proyecto
//   6. En el futuro, para volver a abrir este proyecto, vaya a Archivo > Abrir > Proyecto y seleccione el archivo .sln
