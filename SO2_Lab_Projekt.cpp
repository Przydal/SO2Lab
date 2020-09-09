/**
 * @file ProgramLab8
 * @author Jakub Przydalski
 * @date 28/06/2020
 * @brief Systemy operacyjne II, program na Lab 8
 */

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

std::string outputPath = "Output.txt";

HANDLE hSemaphore[3];
int threadQueue[NUMBER_OF_THREADS] = {FIRST_THREAD, SECOND_THREAD, THIRD_THREAD};
int thQue = 0;
double PCFreq = 0.0;
__int64 CounterStart = 0;

bool bool_writeLineToFile(HANDLE hFile);
bool bool_generateRandomNumbersAndSaveToFile(HANDLE hFile, int howManyRandomNumbers);
float float_countAverage(std::vector<int> vector_int_numbersFromFile);
int int_countMin(std::vector<int> vector_int_numbersFromFile);
int int_countMax(std::vector<int> vector_int_numbersFromFile);
DWORD WINAPI threadFunction_countAverage(LPVOID lpParam);
DWORD WINAPI threadFunction_countMin(LPVOID lpParam);
DWORD WINAPI threadFunction_countMax(LPVOID lpParam);
void void_startCounter();
double double_getCounter();

typedef struct DataToThread{
    std::vector<int> dataFromFile;
} MYDATA, *PMYDATA;

int main(int argc, char** argv){
if(argc == 4 && atoi(argv[1])>0 && atoi(argv[1])<100001 && atoi(argv[2])>0 && atoi(argv[3])>0){
    std::cout << "[Process 1]: Process #1 starts" << std::endl;
    HANDLE hOutputFileHeader = CreateFile(outputPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hOutputFileHeader == INVALID_HANDLE_VALUE){
        std::cout << "[Process 1]: Unable to create file!" << std::endl;
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
    std::string path = "SO2_Lab_Projekt.exe 1337 32167 Process2Call "+std::to_string(i);
    CreateProcessA(NULL, &path[0], NULL, NULL, TRUE, CREATE_SUSPENDED, NULL, NULL, &si, &pi);
    std::cout << "[Process 2]: Process #2 created and waiting" << std::endl;
    std::string filePath = "Numbers.txt";
    HANDLE hFile = CreateFile(filePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
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
    ResumeThread(pi.hThread);
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    Sleep(atoi(argv[2])*1000);
    }
    std::cout << "[Process 1]: Process #1 termination" << std::endl;
    return 0;
}
else if(argc == 5 && atoi(argv[1])==1337 && atoi(argv[2])==32167 && strcmp(argv[3], "Process2Call") == 0){
    int int_cycle = atoi(argv[4]);
    float float_avg = 0;
    int int_min = 0, int_max = 0;
    double double_singleThreadTime= 0.00, double_multiThreadTime = 0.00;
    std::cout << "[Process 2]: Accessing the file" << std::endl;
    std::string filePath = "Numbers.txt";
    HANDLE hFile2 = CreateFile(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile2 == INVALID_HANDLE_VALUE){
    std::cout << "[Process 2]: Unable to access the file!" << std::endl;
    return 3;
    }
    else{
    std::cout << "[Process 2]: File opened successfully" << std::endl;
    }
    constexpr size_t BUFSIZE = 500000;
    char buffer[BUFSIZE];
    DWORD dwBytesToRead = BUFSIZE -1;
    DWORD dwBytesRead = 0;
    ReadFile(hFile2,(void*)buffer, dwBytesToRead, &dwBytesRead, NULL);
    if(dwBytesRead > 0){
        //clearing the read buffer
        buffer[dwBytesRead] = '\0';
    }
    CloseHandle(hFile2);
    std::istringstream buf(buffer);
    std::vector<int> b{std::istream_iterator<int>(buf),{}};
    std::cout << "[Process 2]: Data from the file loaded" << std::endl;

    //Variant 1
    void_startCounter();
    float_avg = float_countAverage(b);
    int_min = int_countMin(b);
    int_max = int_countMax(b);
    double_singleThreadTime = double_getCounter();
    std::cout << "[Process 2]: [Variant A]: Time: " << double_singleThreadTime << "ms" << std::endl;

    //Variant 2
    void_startCounter();
    PMYDATA pDataArray[NUMBER_OF_THREADS];
    DWORD   dwThreadIdArray[NUMBER_OF_THREADS];
    HANDLE  hThreadArray[NUMBER_OF_THREADS];
    for(int i=0 ; i < NUMBER_OF_THREADS; i++){
        hSemaphore[i] = CreateSemaphore(NULL, 0, 1, NULL);
        if(hSemaphore[i]==INVALID_HANDLE_VALUE){
            std::cout << "[Process 2]: [Variant B]: Unable to Create Semaphore: " << i << std::endl;
            return 5;
        }
    }

    for( int i=0; i<NUMBER_OF_THREADS; i++ ){
        // Allocate memory for thread data.
        pDataArray[i] = (PMYDATA) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(MYDATA));
        if( pDataArray[i] == NULL ){
            std::cout << "[Process 2]: [Variant B]: Unable to allocate memory" << std::endl;
            return 1;
        }
        pDataArray[i]->dataFromFile = b;
        if(i==0){
            hThreadArray[i] = CreateThread(NULL, 0, threadFunction_countAverage, pDataArray[i], 0, &dwThreadIdArray[i]);
        }else if(i==1){
            hThreadArray[i] = CreateThread(NULL, 0, threadFunction_countMin, pDataArray[i], 0, &dwThreadIdArray[i]);
        }else if(i==2){
            hThreadArray[i] = CreateThread(NULL, 0, threadFunction_countMax, pDataArray[i], 0, &dwThreadIdArray[i]);
        }
        if(hThreadArray[i] == NULL){
            std::cout << "[Process2]: [Variant B]: Unable to assign data" << std::endl;
            return 4;
        }
    }
    ReleaseSemaphore(hSemaphore[threadQueue[thQue++]], 1, 0);
    WaitForMultipleObjects(NUMBER_OF_THREADS, hThreadArray, TRUE, INFINITE);

    for(int i=0; i<NUMBER_OF_THREADS; i++){
        CloseHandle(hThreadArray[i]);
        CloseHandle(hSemaphore[i]);
        if(pDataArray[i] != NULL){
            HeapFree(GetProcessHeap(), 0, pDataArray[i]);
            pDataArray[i] = NULL;
        }
    }
    double_multiThreadTime = double_getCounter();
    std::cout << "[Process 2]: [Variant B]: Time: " << double_multiThreadTime << "ms" << std::endl;
    HANDLE hOutputFile = CreateFile(outputPath.c_str(), FILE_APPEND_DATA, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    std::string strText = std::to_string(int_cycle)+".\t| AVG: "+std::to_string(float_avg)+"\t| MIN: "+std::to_string(int_min)+"\t| MAX: "+std::to_string(int_max)+"\t| SINGLE THREAD TIME: "+std::to_string(double_singleThreadTime)+"\t| MULTI THREAD TIME: "+std::to_string(double_multiThreadTime)+"\t|";
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
float float_countAverage(std::vector<int> vector_int_numbersFromFile){
    float float_average = 0.00;
    for(int n: vector_int_numbersFromFile){
        float_average+=n;
    }
    float_average=float_average/vector_int_numbersFromFile.size();
    std::cout << "[Process 2]: [Variant A]: Average from input is: " << float_average << std::endl;
    return float_average;
}
int int_countMin(std::vector<int> vector_int_numbersFromFile){
    int int_minimum = 101;
    for(int n: vector_int_numbersFromFile){
        if(n<int_minimum){
            int_minimum = n;
        }
    }
    std::cout << "[Process 2]: [Variant A]: Minimum value from input is " << int_minimum << std::endl;
    return int_minimum;
}
int int_countMax(std::vector<int> vector_int_numbersFromFile){
    int int_maximum = 0;
    for(int n: vector_int_numbersFromFile){
        if(n>int_maximum){
            int_maximum = n;
        }
    }
    std::cout << "[Process 2]: [Variant A]: Maximum value from input is " << int_maximum << std::endl;
    return int_maximum;
}
DWORD WINAPI threadFunction_countAverage(LPVOID lpParam){
    PMYDATA pDataArray;
    pDataArray = (PMYDATA)lpParam;
    float average = 0.00;
    for(int i: pDataArray->dataFromFile){
        average+=i;
    }
    average/=pDataArray->dataFromFile.size();
    WaitForSingleObject(hSemaphore[0], INFINITE);
    std::cout << "[Process 2]: [Variant B]: [Thread 1]: Average from input is: " << average << std::endl;
    ReleaseSemaphore(hSemaphore[threadQueue[thQue++]], 1, 0);
    return 0;
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
    ReleaseSemaphore(hSemaphore[threadQueue[thQue++]], 1, 0);
    return 0;
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
    ReleaseSemaphore(hSemaphore[threadQueue[thQue++]], 1, 0);
    return 0;
}
void void_startCounter(){
    LARGE_INTEGER li;
    if(!QueryPerformanceFrequency(&li))
    std::cout << "QueryPerformanceFrequency failed!\n" << std::endl;

    PCFreq = double(li.QuadPart)/1000.0;

    QueryPerformanceCounter(&li);
    CounterStart = li.QuadPart;
}
double double_getCounter(){
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return double(li.QuadPart-CounterStart)/PCFreq;
}
