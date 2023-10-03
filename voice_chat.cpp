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
HWAVEOUT hwo;
UINT msg = WOM_DONE;
SOCKET client_socket;
int index = 0;

void CALLBACK waveOutProc(HWAVEOUT  hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    msg = uMsg;
    std::cout << "changed to " << uMsg << std::endl;
    index++;
}

void sound_recieve()
{
    LPSTR buffer = (LPSTR)calloc(BUFFER_SIZE * 2, sizeof(LPSTR));
    WAVEHDR       _audioHeader;
    HWAVEOUT      _audioOut     = 0;

    WAVEFORMATEX wfx;
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 2;
    wfx.nSamplesPerSec = 16000;
    wfx.nAvgBytesPerSec = 16000 * 2 * sizeof(short);
    wfx.nBlockAlign = 2 * sizeof(short);
    wfx.wBitsPerSample = 16;
    wfx.cbSize = 0; 

    _audioHeader.dwBufferLength  = BUFFER_SIZE * 2;
    _audioHeader.lpData          = (LPSTR)calloc(BUFFER_SIZE * 2, sizeof(LPSTR));
    _audioHeader.dwBytesRecorded = 0;
    _audioHeader.dwUser          = 0;
    _audioHeader.dwFlags         = 0;
    _audioHeader.dwLoops         = 0;
    _audioHeader.lpNext          = NULL;
    _audioHeader.reserved        = 0;
    MMRESULT result = waveOutOpen(&_audioOut, WAVE_MAPPER, &wfx, (DWORD_PTR)&waveOutProc, 0, CALLBACK_FUNCTION);
    if (result != MMSYSERR_NOERROR)
    {
        std::cerr << "Failed to open audio output device." << std::endl;
        closesocket(client_socket);
        WSACleanup();
        return ;
    }
    int lastindex = index;
    while (1)
    {
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE * 2, 0);
        std::cout << waveOutPrepareHeader(_audioOut, &_audioHeader, bytes_received) << " ";
        memcpy(_audioHeader.lpData, buffer, BUFFER_SIZE * 2);
        lastindex = index;
        MMRESULT res = waveOutWrite(_audioOut, &_audioHeader, bytes_received);
        if (res == 0)
        {
            while (lastindex == index)
                1 + 1; //TODO load next buffer
        }
    }
}

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

        std::thread recv_sounds(sound_recieve);
        recv_sounds.detach();
        

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