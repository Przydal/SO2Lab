/**
 * @file ProgramPoLab2
 * @author Jakub Przydalski
 * @date 26/06/2020
 * @brief Systemy operacyjne II, program po Lab 2.
 */

#include<iostream>
#include<windows.h>

bool bool_writeLineToFile(HANDLE hFile);
bool bool_openFileInNotepad(std::string name);

int main(int argc, char** argv){
std::string strText = "";
std::string filePath = "OutputFile.txt";
HANDLE hFile = CreateFile(filePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

if(hFile == INVALID_HANDLE_VALUE){
    std::cout << "Unable to create file " << std::endl;
    return 2;
}
else{
    std::cout << "File created successfully" << std::endl;
}

for(int i=1; i<argc; i++){
strText = argv[i];
WriteFile(hFile, strText.c_str(), strText.size(), nullptr, nullptr);
bool_writeLineToFile(hFile);
}
CloseHandle(hFile);
std::string notepadStatus = getenv("NOTEPAD");
if(notepadStatus.compare("TRUE")==0){
    bool_openFileInNotepad(filePath.c_str());
}
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
