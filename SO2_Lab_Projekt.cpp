/**
 * @file ProgramPoLab5
 * @author Jakub Przydalski
 * @date 26/06/2020
 * @brief Systemy operacyjne II, program po Lab 5.
 */

#include<iostream>
#include<windows.h>
#include<vector>
#include<iterator>
#include<sstream>
#include<time.h>


bool bool_writeLineToFile(HANDLE hFile);
bool bool_generateRandomNumbersAndSaveToFile(HANDLE hFile, int howManyRandomNumbers);
float float_countAverage(std::vector<int> vector_int_numbersFromFile);
float float_countMin(std::vector<int> vector_int_numbersFromFile);
float float_countMax(std::vector<int> vector_int_numbersFromFile);

int main(int argc, char** argv){
if(argc == 2 && atoi(argv[1])>0 && atoi(argv[1]) <101){
    std::cout << "[Process 1]: Process #1 starts" << std::endl;
    STARTUPINFOA si = {sizeof(si)};
    PROCESS_INFORMATION pi;
    std::string path = "SO2_LAB_Projekt.exe 1337 32167 Process2Call";
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
    bool_generateRandomNumbersAndSaveToFile(hFile, atoi(argv[1]));
    CloseHandle(hFile);
    ResumeThread(pi.hThread);
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    std::cout << "[Process 1]: Process #1 termination" << std::endl;
    return 0;
}
else if(argc == 4 && atoi(argv[1])==1337 && atoi(argv[2])==32167 && strcmp(argv[3], "Process2Call") == 0){
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
    constexpr size_t BUFSIZE = 1024;
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
    std::cout << "[Process 2]: Data from file loaded" << std::endl;
    float float_average = float_countAverage(b);
    std::cout << "[Process 2]: Average from input is " << float_average <<" from " << b.size() << " numbers" << std::endl;
    float float_min = float_countMin(b);
    std::cout << "[Process 2]: Minimum value from input is " << float_min <<" from " << std::endl;
    float float_max = float_countMax(b);
    std::cout << "[Process 2]: Maximum value from input is " << float_max <<" from " << std::endl;
    std::cout << "[Process 2]: Process #2 termination" << std::endl;
    return 0;
}
else{
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

bool bool_generateRandomNumbersAndSaveToFile(HANDLE hFile, int howManyRandomNumbers){
    if(howManyRandomNumbers>0){
        time_t t;
        srand((unsigned) time(&t));
        std::string strText = "";
        for(int i=0; i<howManyRandomNumbers; i++){
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
    float average = 0.00;
    for(int n: vector_int_numbersFromFile){
        average+=n;
    }
    average=average/vector_int_numbersFromFile.size();
    return average;
}

float float_countMin(std::vector<int> vector_int_numbersFromFile){
    float float_minimum = 101.00;
    for(int n: vector_int_numbersFromFile){
        if(n<float_minimum){
            float_minimum = n;
        }
    }
    return float_minimum;
}

float float_countMax(std::vector<int> vector_int_numbersFromFile){
    float float_maximum = 0.00;
    for(int n: vector_int_numbersFromFile){
        if(n>float_maximum){
            float_maximum = n;
        }
    }
    return float_maximum;
}


