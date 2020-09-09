/**
 * @file ProgramLab1
 * @author Jakub Przydalski
 * @date 28/06/2020
 * @brief Systemy operacyjne II, program na Lab 1
 */

#include<iostream>

int main(int argc, char** argv){
std:: cout << "Number of arguments " << argc << std::endl;

for(int i = 0; i < argc; i++){
    std::cout<< argv[i] << std::endl;
}

return 0;
}

