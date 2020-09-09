/**
 * @file ProgramPoLab3
 * @author Jakub Przydalski
 * @date 26/06/2020
 * @brief Systemy operacyjne II, program po Lab 3.
 */

#include<iostream>
#include<windows.h>

bool bool_writeLineToFile(HANDLE hFile);
bool bool_openFileInNotepad(std::string name);

int main(int argc, char** argv){
std::string strText = "";
std::string filePath = "log.txt";
HANDLE hFile = CreateFile(filePath.c_str(), GENERIC_WRITE, 0, 1, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

if(hFile == INVALID_HANDLE_VALUE){
    std::cout << "Unable to create file " << std::endl;
    return 2;
}
else{
    std::cout << "File created successfully" << std::endl;
}
std::string userInput;


CloseHandle(hFile);
return 0;
}

bool bool_writeLineToFile(HANDLE hFile){
    std::string newLine = "\r\n";
    return WriteFile(hFile, newLine.c_str(), newLine.size(), nullptr, nullptr);
}
bool bool_openFileInNotepad(std::string name){
    bool processStatus = false;
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    std::string path = "notepad.exe "+name;
    processStatus = CreateProcessA(NULL, &path[0], NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
    if(processStatus){
        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
    return processStatus;
}
