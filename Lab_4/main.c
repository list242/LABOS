#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
       
#define USER_SHIFT 6
#define GROUP_SHIFT 3
#define OTHER_SHIFT 0

#define READ_SHIFT 2
#define WRITE_SHIFT 1
#define EXECUTE_SHIFT 0
     
mode_t adjustPermission(char operation, int shift, mode_t currentMode) {
    if (operation == '+') currentMode = currentMode | (1 << shift);  
    else currentMode = currentMode & ~(1 << shift); 
    return currentMode;
}
     
mode_t calculateNewMode(mode_t currentMode, const char* permissions) {
    mode_t newMode = strtol(permissions, NULL, 8);     
    if (newMode || (strcmp(permissions, "000") == 0)
    || (strcmp(permissions, "00") == 0)
    || (strcmp(permissions, "0") == 0)) return newMode;                      
    char operation = ' ';
    int groupShift = -1; 
    int permissionShift = -1;
    for(size_t i = 0; i < strlen(permissions); ++i) {   
        switch (permissions[i]) {
            case 'u':
                groupShift = USER_SHIFT;
                break;
            case 'g':
                groupShift = GROUP_SHIFT;
                break;
            case 'o':
                groupShift = OTHER_SHIFT;
                break;
            case '+':
            case '-':
                operation = permissions[i];
                break;
            case 'r':
                permissionShift = READ_SHIFT;
                break;
            case 'w':
                permissionShift = WRITE_SHIFT;
                break;
            case 'x':
                permissionShift = EXECUTE_SHIFT;
                break;
            case ',':
                groupShift = -1;
                permissionShift = -1;
                break;
        }
        if (permissionShift == -1 || operation == ' ') continue;
        if (groupShift == -1) {
            currentMode = adjustPermission(operation, USER_SHIFT + permissionShift, currentMode);
            currentMode = adjustPermission(operation, GROUP_SHIFT + permissionShift, currentMode);
            currentMode = adjustPermission(operation, OTHER_SHIFT + permissionShift, currentMode);
        }
        else currentMode = adjustPermission(operation, groupShift + permissionShift, currentMode);     
    }
    return currentMode;    
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "chmod: missing arguments");
        exit(1);
    }
    char* permissions = argv[1];   
    struct stat fileStat; 
    char* filePath;
    for (int i = 2; i < argc; ++i) {     
        filePath = argv[i];
        if (stat(filePath, &fileStat) == -1) {
            perror("file not found");
            exit(1);
        }
        if (chmod(filePath, calculateNewMode(fileStat.st_mode, permissions)) != 0)
            fprintf(stderr, "unable to change permissions");
    }
    return 0;
}
