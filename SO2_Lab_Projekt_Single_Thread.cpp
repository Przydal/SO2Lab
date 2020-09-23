#include<iostream>
#include<windows.h>
#include<vector>
#include<iterator>
#include<sstream>
#include<time.h>
#include<math.h>

std::string outputPath = "Output_Single_Thread.txt";
std::string filePath = "Numbers.txt";

double PCFreq = 0.0;
__int64 CounterStart = 0;
constexpr size_t BUFSIZE = 500000;

bool bool_writeLineToFile(HANDLE hFile);
bool bool_generateRandomNumbersAndSaveToFile(HANDLE hFile, int howManyRandomNumbers);
float float_countAverage(std::vector<int> vector_int_numbersFromFile);
int int_countMin(std::vector<int> vector_int_numbersFromFile);
int int_countMax(std::vector<int> vector_int_numbersFromFile);
void void_startTimeCounter();
double double_getCounter();

int main(int argc, char** argv){
//Process1 - maximum numbers 100,000, minimum cycles 1, minimum break 1s
if(argc == 4 && atoi(argv[1])>0 && atoi(argv[1])<100001 && atoi(argv[2])>0 && atoi(argv[3])>0){
    std::cout << "[Process 1]: Process #1 starts" << std::endl;
    HANDLE hOutputFileHeader = CreateFile(outputPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
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
        std::string path = "SO2_Lab_Projekt_Single_Thread.exe 1337 32167 Process2Call "+std::to_string(i);
        /*Process2 created and Thread of Process2 is waiting to allow it to continue.
        No additional cost - Process2 is not checking all the time if the file is ready to access.*/
        CreateProcessA(NULL, &path[0], NULL, NULL, TRUE, CREATE_SUSPENDED, NULL, NULL, &si, &pi);
        std::cout << "[Process 2]: Process #2 created and waiting" << std::endl;
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
    float float_avg = 0;
    int int_min = 0, int_max = 0;
    double double_singleThreadTime= 0.00;
    std::cout << "[Process 2]: Resumed and accessing the file" << std::endl;
    std::string filePath = "Numbers.txt";
    HANDLE hFile2 = CreateFile(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
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

    //Variant 1 - all C++ programs by default are single threaded - no additional implementation required.
    void_startTimeCounter();
    float_avg = float_countAverage(b);
    int_min = int_countMin(b);
    int_max = int_countMax(b);
    double_singleThreadTime = double_getCounter();
    std::cout << "[Process 2]: [Variant A]: Time: " << double_singleThreadTime << "ms" << std::endl;

    HANDLE hOutputFile = CreateFile(outputPath.c_str(), FILE_APPEND_DATA, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if(hOutputFile == INVALID_HANDLE_VALUE){
        std::cout << "[Process 2]: [Variant A]: Unable to access the Output file!" << std::endl;
        return 3;
    }
    std::string strText = std::to_string(int_cycle)+".\t| AVG: "+std::to_string(float_avg)+"\t| MIN: "+std::to_string(int_min)+"\t| MAX: "+std::to_string(int_max)+"\t| SINGLE THREAD TIME: "+std::to_string(double_singleThreadTime);
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
void void_startTimeCounter(){
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
