//varianta windows
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_VALUE 1000
#define SHARED_MEM_NAME "Local\\SharedCounter"
#define SEMAPHORE_NAME "Local\\CounterSemaphore"

int main() {
    srand((unsigned int)time(NULL) ^ GetCurrentProcessId());

    // Creare/obținere semafor inter-proces
    HANDLE hSem = CreateSemaphoreA(
        NULL, 1, 1, SEMAPHORE_NAME
    );
    if (hSem == NULL) {
        printf("CreateSemaphore failed (%lu)\n", GetLastError());
        return 1;
    }

    // Creare/obținere memorie partajată
    HANDLE hMapFile = CreateFileMappingA(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        sizeof(int),
        SHARED_MEM_NAME
    );
    if (hMapFile == NULL) {
        printf("CreateFileMapping failed (%lu)\n", GetLastError());
        return 1;
    }

    int* shared = (int*)MapViewOfFile(
        hMapFile,
        FILE_MAP_ALL_ACCESS,
        0, 0, sizeof(int)
    );
    if (shared == NULL) {
        printf("MapViewOfFile failed (%lu)\n", GetLastError());
        return 1;
    }

    // Inițializează memoria doar dacă valoarea este 0
    if (*shared == 0) *shared = 0;

    // Loop principal
    while (1) {
        WaitForSingleObject(hSem, INFINITE);

        int val = *shared;
        if (val >= MAX_VALUE) {
            ReleaseSemaphore(hSem, 1, NULL);
            break;
        }

        printf("[PID %lu] citeste: %d\n", GetCurrentProcessId(), val);

        // Aruncăm banul (0 sau 1). Cât timp 1, incrementăm
        while ((rand() % 2 == 1) && val < MAX_VALUE) {
            val++;
            *shared = val;
            printf("[PID %lu] scrie: %d\n",
                   GetCurrentProcessId(), val);
            Sleep(20);
        }

        ReleaseSemaphore(hSem, 1, NULL);
        Sleep(30);
    }

    UnmapViewOfFile(shared);
    CloseHandle(hMapFile);
    CloseHandle(hSem);

    return 0;
}
