#include <iostream>
#include <Windows.h>
#include <fstream>

void CALLBACK WaveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    if (uMsg == WIM_DATA)
    {
        WAVEHDR* waveHdr = (WAVEHDR*)dwParam1;
        std::ofstream outputFile("captured_audio.raw", std::ios::binary | std::ios::app); // Open the file in binary append mode
        if (outputFile.is_open())
        {
            // Write the captured audio data to the file
            outputFile.write(waveHdr->lpData, waveHdr->dwBytesRecorded);
            outputFile.close();
        }
        waveInAddBuffer(hwi, waveHdr, sizeof(WAVEHDR));
    }
}


int main()
{
    WAVEFORMATEX wfx;
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 2;
    wfx.nSamplesPerSec = 44100;
    wfx.nAvgBytesPerSec = 44100 * 2 * sizeof(short);
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
        waveHdr.lpData = (LPSTR)malloc(8192 * sizeof(short));
        waveHdr.dwBufferLength = 8192 * sizeof(short);
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
