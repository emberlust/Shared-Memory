#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <random>
#include <time.h>

#define BUF_SIZE sizeof(int)
//numele obiectuului
TCHAR szName[] = TEXT("countermem");
#define MAX_SEM_COUNT 2
//numele semaforului
TCHAR semName[] = TEXT("sem");

int _tmain()
{
   HANDLE hMapFile;
   LPCTSTR pBuf;


   //accesare memorie
   hMapFile = OpenFileMapping(
       FILE_MAP_ALL_ACCESS, 
       FALSE,               
       szName);             

   //daca nu se poate accesa, se creeaza obiectul mapat
   if (hMapFile == NULL)
   {
      hMapFile = CreateFileMapping(
          INVALID_HANDLE_VALUE, 
          NULL,                 
          PAGE_READWRITE,       
          0,                    
          BUF_SIZE,             
          szName);              

      if (hMapFile == NULL)
      {
         _tprintf(TEXT("Could not create file mapping object (%d).\n"),
                  GetLastError());
         return 1;
      }
   }

   pBuf = (LPTSTR)MapViewOfFile(hMapFile,            
                                FILE_MAP_ALL_ACCESS, 
                                0,
                                0,
                                BUF_SIZE);

   if (pBuf == NULL)
   {
      _tprintf(TEXT("Could not map view of file (%d).\n"),
               GetLastError());

      CloseHandle(hMapFile);

      return 1;
   }

   //accesare semafor
   HANDLE ghSemaphore;
   ghSemaphore = OpenSemaphore(
       SEMAPHORE_ALL_ACCESS,
       FALSE,
       semName);

   //daca nu exista, se creeaza un nou semafor
   if (ghSemaphore == NULL)
   {
      ghSemaphore = CreateSemaphore(
          NULL,          
          MAX_SEM_COUNT, 
          MAX_SEM_COUNT, 
          semName);

      if (ghSemaphore == NULL)
      {
         printf("CreateSemaphore error: %d\n", GetLastError());
         return 1;
      }
   }

   DWORD dwWaitResult;
   int counter = 0;

   srand(time(NULL));

   //secventa de incrementare a numarului
   while (counter < 1000)
   {
      dwWaitResult = WaitForSingleObject(
          ghSemaphore, 
          0L);

      switch (dwWaitResult)
      {
      case WAIT_OBJECT_0:

         counter = *(int *)pBuf;
         //procesul 'da cu banul' pentru a decide daca scrie sau nu
         if(rand() % 2 + 1 == 2)
         {
            counter++;
            CopyMemory((PVOID)pBuf, (const void *)(&counter), sizeof(int));
            printf("%d\n", counter);
         }
         Sleep(200);

         if (!ReleaseSemaphore(
                 ghSemaphore, 
                 1,           
                 NULL))       
         {
            printf("ReleaseSemaphore error: %d\n", GetLastError());
         }
         break;

      case WAIT_TIMEOUT:
         break;
      }
   }

   UnmapViewOfFile(pBuf);

   CloseHandle(hMapFile);
   CloseHandle(ghSemaphore);

   return 0;
}