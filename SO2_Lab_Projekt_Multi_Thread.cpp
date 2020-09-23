#include<iostream>
#include<windows.h>
#include<vector>
#include<iterator>
#include<sstream>
#include<time.h>
#include<math.h>

#define NUMBER_OF_THREADS 3
#define FIRST_THREAD 0
#define SECOND_THREAD 1
#define THIRD_THREAD 2

std::string string_outputPath = "Output_Multi_Thread.txt";
std::string string_filePath = "Numbers.txt";

HANDLE hSemaphore[3];
int threadQueue[NUMBER_OF_THREADS] = {FIRST_THREAD, SECOND_THREAD, THIRD_THREAD};
int int_thQue = 0;
double double_PCFreq = 0.0;
__int64 CounterStart = 0;
constexpr size_t BUFSIZE = 500000;

bool bool_writeLineToFile(HANDLE hFile);
bool bool_generateRandomNumbersAndSaveToFile(HANDLE hFile, int howManyRandomNumbers);
DWORD WINAPI threadFunction_countAverage(LPVOID lpParam);
DWORD WINAPI threadFunction_countMin(LPVOID lpParam);
DWORD WINAPI threadFunction_countMax(LPVOID lpParam);
void void_startTimeCounter();
double double_getCounter();

typedef struct DataToThread{
    std::vector<int> dataFromFile;
} MYDATA, *PMYDATA;

int main(int argc, char** argv){
//Process1 - maximum numbers 100,000, minimum cycles 1, minimum break 1s
if(argc == 4 && atoi(argv[1])>0 && atoi(argv[1])<100001 && atoi(argv[2])>0 && atoi(argv[3])>0){
    std::cout << "[Process 1]: Process #1 starts" << std::endl;
    HANDLE hOutputFileHeader = CreateFile(string_outputPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hOutputFileHeader == INVALID_HANDLE_VALUE){
        std::cout << "[Process 1]: Unable to create output file and generate headers!" << std::endl;
        return 2;
    }
    std::string argmnt1 = argv[1];
    std::string strText = "[Total numbers: "+argmnt1+"][Replies: "+argv[3]+"][Pause: "+argv[2]+" sec]";
    WriteFile(hOutputFileHeader, strText.c_str(), strText.size(), nullptr, nullptr);
    bool_writeLineToFile(hOutputFileHeader);
    CloseHandle(hOutputFileHeader);
    for(int i = 1 ; i<=atoi(argv[3]); i++){
        STARTUPINFOA si = {sizeof(si)};
        PROCESS_INFORMATION pi;
        std::string path = "SO2_Lab_Projekt_Multi_Thread.exe 1337 32167 Process2Call "+std::to_string(i);
        /*Process2 created and Thread of Process2 is waiting to allow it to continue.
        No additional cost - Process2 is not checking all the time if the file is ready to access.*/
        CreateProcessA(NULL, &path[0], NULL, NULL, TRUE, CREATE_SUSPENDED, NULL, NULL, &si, &pi);
        std::cout << "[Process 2]: Process #2 created and waiting" << std::endl;
        HANDLE hFile = CreateFile(string_filePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if(hFile == INVALID_HANDLE_VALUE){
            std::cout << "[Process 1]: Unable to create file!" << std::endl;
            return 2;
        }
        else{
            std::cout << "[Process 1]: File created successfully" << std::endl;
            }
        if(bool_generateRandomNumbersAndSaveToFile(hFile, atoi(argv[1])) == 1){
            std::cout << "[Process 1]: Numbers generated and saved to file successfully" << std::endl;
        };
        CloseHandle(hFile);
        //Process2 resumed when handle of File is closed after creating a file and generating/saving random numbers to it by Process1.
        ResumeThread(pi.hThread);
        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        std::cout << "[Process 1]: Process #2 exit code: " << exitCode << std::endl;
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        Sleep(atoi(argv[2])*1000);
        }
        std::cout << "[Process 1]: Process #1 termination" << std::endl;
        return 0;
}
//Process2
else if(argc == 5 && atoi(argv[1])==1337 && atoi(argv[2])==32167 && strcmp(argv[3], "Process2Call") == 0){
    int int_cycle = atoi(argv[4]);
    DWORD DWORD_avg = 0;
    DWORD DWORD_min = 0, DWORD_max = 0;
    double double_multiThreadTime = 0.00;
    std::cout << "[Process 2]: Resumed and accessing the file" << std::endl;
    HANDLE hFile2 = CreateFile(string_filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile2 == INVALID_HANDLE_VALUE){
        std::cout << "[Process 2]: Unable to access the file!" << std::endl;
        return 3;
    }
    else{
        std::cout << "[Process 2]: File opened successfully" << std::endl;
    }
    char char_buffer[BUFSIZE];
    DWORD DWORD_BytesToRead = BUFSIZE -1;
    DWORD DWORD_BytesRead = 0;
    ReadFile(hFile2,(void*)char_buffer, DWORD_BytesToRead, &DWORD_BytesRead, NULL);
    if(DWORD_BytesRead > 0){
        //clearing the read buffer
        char_buffer[DWORD_BytesRead] = '\0';
    }
    CloseHandle(hFile2);
    std::istringstream buf(char_buffer);
    std::vector<int> b{std::istream_iterator<int>(buf),{}};
    std::cout << "[Process 2]: Data from the file loaded" << std::endl;

    //Variant 2 - 3 threads created, each thread to count Average / Min / Max from the file generated by Process1
    void_startTimeCounter();
    PMYDATA PMYDATA_DataArray[NUMBER_OF_THREADS];
    DWORD   DWORD_ThreadIdArray[NUMBER_OF_THREADS];
    HANDLE  hThreadArray[NUMBER_OF_THREADS];
    for(int i=0 ; i < NUMBER_OF_THREADS; i++){
        hSemaphore[i] = CreateSemaphore(NULL, 0, 1, NULL);
        if(hSemaphore[i]==INVALID_HANDLE_VALUE){
            std::cout << "[Process 2]: [Variant B]: Unable to Create Semaphore: " << i << std::endl;
            return 5;
        }
    }

    for(int i=0; i<NUMBER_OF_THREADS; i++){
        // Allocate memory for thread data.
        PMYDATA_DataArray[i] = (PMYDATA) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(MYDATA));
        if( PMYDATA_DataArray[i] == NULL ){
            std::cout << "[Process 2]: [Variant B]: Unable to allocate memory" << std::endl;
            return 6;
        }
        PMYDATA_DataArray[i]->dataFromFile = b;
        if(i==0){
            hThreadArray[i] = CreateThread(NULL, 0, threadFunction_countAverage, PMYDATA_DataArray[i], 0, &DWORD_ThreadIdArray[i]);
        }else if(i==1){
            hThreadArray[i] = CreateThread(NULL, 0, threadFunction_countMin, PMYDATA_DataArray[i], 0, &DWORD_ThreadIdArray[i]);
        }else if(i==2){
            hThreadArray[i] = CreateThread(NULL, 0, threadFunction_countMax, PMYDATA_DataArray[i], 0, &DWORD_ThreadIdArray[i]);
        }
        if(hThreadArray[i] == NULL){
            std::cout << "[Process2]: [Variant B]: Unable to assign data" << std::endl;
            return 4;
        }
    }
    ReleaseSemaphore(hSemaphore[threadQueue[int_thQue++]], 1, 0);
    WaitForMultipleObjects(NUMBER_OF_THREADS, hThreadArray, TRUE, INFINITE);
    GetExitCodeThread(hThreadArray[0], &DWORD_avg);
    GetExitCodeThread(hThreadArray[1], &DWORD_min);
    GetExitCodeThread(hThreadArray[2], &DWORD_max);

    //HeapFreeMemory
    for(int i=0; i<NUMBER_OF_THREADS; i++){
        CloseHandle(hThreadArray[i]);
        CloseHandle(hSemaphore[i]);
        if(PMYDATA_DataArray[i] != NULL){
            HeapFree(GetProcessHeap(), 0, PMYDATA_DataArray[i]);
            PMYDATA_DataArray[i] = NULL;
        }
    }
    double_multiThreadTime = double_getCounter();
    std::cout << "[Process 2]: [Variant B]: Time: " << double_multiThreadTime << "ms" << std::endl;
    HANDLE hOutputFile = CreateFile(string_outputPath.c_str(), FILE_APPEND_DATA, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    std::string strText = std::to_string(int_cycle)+".\t| AVG: "+std::to_string(DWORD_avg)+"\t| MIN: "+std::to_string(DWORD_min)+"\t| MAX: "+std::to_string(DWORD_max)+"\t| MULTI THREAD TIME: "+std::to_string(double_multiThreadTime);
    WriteFile(hOutputFile, strText.c_str(), strText.size(), nullptr, nullptr);
    bool_writeLineToFile(hOutputFile);
    CloseHandle(hOutputFile);

    std::cout << "[Process 2]: Process #2 termination" << std::endl;
    return 0;
}else{
    std::cout << "[Process 1]: Process #1 starts" << std::endl;
    std::cout << "[Process 1]: Invalid input parameters!" << std::endl;
    std::cout << "[Process 1]: Process #1 termination" << std::endl;
    return 1;
    }
}

bool bool_writeLineToFile(HANDLE hFile){
    std::string newLine = "\r\n";
    return WriteFile(hFile, newLine.c_str(), newLine.size(), nullptr, nullptr);
}
bool bool_generateRandomNumbersAndSaveToFile(HANDLE hFile, int int_howManyRandomNumbers){
    if(int_howManyRandomNumbers>0){
        time_t t;
        srand((unsigned) time(&t));
        std::string strText = "";
        for(int i=0; i<int_howManyRandomNumbers; i++){
        strText = std::to_string(rand()% 100+1);
        WriteFile(hFile, strText.c_str(), strText.size(), nullptr, nullptr);
        bool_writeLineToFile(hFile);
        }
    }else{
        return 0;
    }
    return 1;
}
DWORD WINAPI threadFunction_countAverage(LPVOID lpParam){
    PMYDATA pDataArray;
    pDataArray = (PMYDATA)lpParam;
    float float_average = 0.00;
    for(int i: pDataArray->dataFromFile){
        float_average+=i;
    }
    float_average/=pDataArray->dataFromFile.size();
    WaitForSingleObject(hSemaphore[0], INFINITE);
    std::cout << "[Process 2]: [Variant B]: [Thread 1]: Average from input is: " << float_average << std::endl;
    ReleaseSemaphore(hSemaphore[threadQueue[int_thQue++]], 1, 0);
    return float_average;
}
DWORD WINAPI threadFunction_countMin(LPVOID lpParam){
    PMYDATA pDataArray;
    pDataArray = (PMYDATA)lpParam;
    float float_minimum = pDataArray->dataFromFile[0];
    for(int i: pDataArray->dataFromFile){
        if(i<float_minimum){
            float_minimum = i;
        }
    }
    WaitForSingleObject(hSemaphore[1],INFINITE);
    std::cout << "[Process 2]: [Variant B]: [Thread 2]: Minimum value from input is " << float_minimum << std::endl;
    ReleaseSemaphore(hSemaphore[threadQueue[int_thQue++]], 1, 0);
    return float_minimum;
}
DWORD WINAPI threadFunction_countMax(LPVOID lpParam){
    PMYDATA pDataArray;
    pDataArray = (PMYDATA)lpParam;
    float float_maximum = pDataArray->dataFromFile[0];
    for(int i: pDataArray->dataFromFile){
        if(i>float_maximum){
            float_maximum = i;
        }
    }
    WaitForSingleObject(hSemaphore[2], INFINITE);
    std::cout << "[Process 2]: [Variant B]: [Thread 3]: Maximum value from input is " << float_maximum << std::endl;
    ReleaseSemaphore(hSemaphore[threadQueue[int_thQue++]], 1, 0);
    return float_maximum;
}
void void_startTimeCounter(){
    LARGE_INTEGER li;
    if(!QueryPerformanceFrequency(&li))
    std::cout << "QueryPerformanceFrequency failed!\n" << std::endl;

    double_PCFreq = double(li.QuadPart)/1000.0;

    QueryPerformanceCounter(&li);
    CounterStart = li.QuadPart;
}
double double_getCounter(){
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return double(li.QuadPart-CounterStart)/double_PCFreq;
}
