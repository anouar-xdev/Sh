#ifndef UNIFIED_DRIVER_H
#define UNIFIED_DRIVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include <regex.h>
#include <sys/uio.h>
#include <iostream>
#include <string>
#include <sys/utsname.h>
#include <sys/system_properties.h>
#include <sys/prctl.h>
#include <fstream>

using namespace std;
inline int 选择值 = 0;
inline int 隐藏 = 0;

struct AppConfig {
    int driverOption = 0;
    int captureMode = 0;
    bool settingsLoaded = false;
};

static AppConfig g_appConfig;
static std::string configPath = "/storage/emulated/0/VoidHaxConfig.txt";

static void SaveConfig() {
    std::ofstream file(configPath);
    if (!file.is_open()) {
      //  printf("[Config] Error: Could not save config to %s\n", configPath.c_str());
        return;
    }
    file << "DRIVER_OPTION=" << g_appConfig.driverOption << "\n";
    file << "CAPTURE_MODE=" << g_appConfig.captureMode << "\n";
    file.close();
   // printf("[Config] Saved to: %s\n", configPath.c_str());
}

static bool LoadConfig() {
    std::ifstream file(configPath);
    if (!file.is_open()) {
      //  printf("[Config] No config file found at: %s\n", configPath.c_str());
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        line.erase(0, line.find_first_not_of(" \t\n\r"));
        line.erase(line.find_last_not_of(" \t\n\r") + 1);

        size_t equalsPos = line.find('=');
        if (equalsPos == std::string::npos) continue;

        std::string key = line.substr(0, equalsPos);
        std::string value = line.substr(equalsPos + 1);

        value.erase(0, value.find_first_not_of(" \t\n\r"));
        value.erase(value.find_last_not_of(" \t\n\r") + 1);

        if (key == "DRIVER_OPTION") {
            g_appConfig.driverOption = std::stoi(value);
        } else if (key == "CAPTURE_MODE") {
            g_appConfig.captureMode = std::stoi(value);
        }
    }

    file.close();

    if (g_appConfig.driverOption < 1 || g_appConfig.driverOption > 5) {
//        printf("[Config] Invalid driver option, using default\n");
        g_appConfig.driverOption = 0;
        return false;
    }

    if (g_appConfig.captureMode < 1 || g_appConfig.captureMode > 2) {
      //  printf("[Config] Invalid capture mode, using default\n");
        g_appConfig.captureMode = 0;
        return false;
    }

    g_appConfig.settingsLoaded = true;
    printf("[Config] Loaded successfully!\n");
    printf("[Config] Driver Option: %d\n", g_appConfig.driverOption);
    printf("[Config] Capture Mode: %d\n", g_appConfig.captureMode);

    return true;
}

static void DisplayCurrentConfig() {


    if (g_appConfig.settingsLoaded) {
        const char* driverNames[] = {
            "Unknown",
            "RT Hook",
            "RT DEV",
            "DEV (Most Compatible)",
            "QX10~11.4",
            "KPM (Neo prctl)"
        };

        const char* captureNames[] = {
            "Unknown",
            "Hidden from recordings/screenshots",
            "Visible everywhere (normal)"
        };

        printf("  | Driver Option : %s\n", driverNames[g_appConfig.driverOption]);
        printf("  | Capture Mode  : %s\n", captureNames[g_appConfig.captureMode]);
    } else {
        
    }
    
}

#define NA_PRCTL_MAGIC      0x4E41
#define NA_CMD_READ_MEM     1
#define NA_CMD_WRITE_MEM    2
#define NA_CMD_GET_PID      5
#define NA_MAX_RW_SIZE      4096
#define NA_MAX_PKG_NAME     256

struct na_cmd {
    uint32_t op;
    uint32_t pid;
    uint64_t addr;
    uint32_t size;
    int32_t  result;
    int32_t  screen_w;
    int32_t  screen_h;
    char     pkg[NA_MAX_PKG_NAME];
    uint8_t  data[NA_MAX_RW_SIZE];
};

class c_driver {
private:
    int has_upper = 0;
    int has_lower = 0;
    int has_symbol = 0;
    int has_digit = 0;
    int fd = -1;
    pid_t pid = -1;

    typedef struct _COPY_MEMORY {
        pid_t pid;
        uintptr_t addr;
        void* buffer;
        size_t size;
    } COPY_MEMORY, *PCOPY_MEMORY;

    typedef struct _MODULE_BASE {
        pid_t pid;
        char* name;
        uintptr_t base;
    } MODULE_BASE, *PMODULE_BASE;

    struct process {
        pid_t process_pid;
        char process_comm[15];
    };

    enum OPERATIONS {
        HOOK_OP_READ_MEM = 601,
        HOOK_OP_WRITE_MEM = 602,
        HOOK_OP_MODULE_BASE = 603,
        HOOK_OP_HIDE_PROCESS = 605,

        DEV_OP_INIT_KEY = 0x800,
        DEV_OP_READ_MEM = 0x801,
        DEV_OP_WRITE_MEM = 0x802,
        DEV_OP_MODULE_BASE = 0x803,
        DEV_OP_HIDE_PROCESS = 0x804,

        PROC_OP_INIT_KEY = 0xC00,
        PROC_OP_READ_MEM = 0xC01,
        PROC_OP_WRITE_MEM = 0xC02,
        PROC_OP_MODULE_BASE = 0xC03,
        PROC_OP_HIDE_PROCESS = 0xC04,
    };

    int symbol_file(const char *filename) {
        int length = strlen(filename);
        for (int i = 0; i < length; i++) {
            if (islower(filename[i])) {
                has_lower = 1;
            } else if (isupper(filename[i])) {
                has_upper = 1;
            } else if (ispunct(filename[i])) {
                has_symbol = 1;
            } else if (isdigit(filename[i])) {
                has_digit = 1;
            }
        }
        return has_lower && !has_upper && !has_symbol && !has_digit;
    }

    char *driver_path() {
        const char *dev_path = "/dev";
        DIR *dir = opendir(dev_path);
        if (dir == NULL){
            printf("无法打开/dev目录\n");
            return NULL;
        }

        char *files[] = { "wanbai", "CheckMe", "Ckanri", "lanran","video188"};
        struct dirent *entry;
        char *file_path = NULL;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            size_t path_length = strlen(dev_path) + strlen(entry->d_name) + 2;
            file_path = (char *)malloc(path_length);
            snprintf(file_path, path_length, "%s/%s", dev_path, entry->d_name);
            for (int i = 0; i < 5; i++) {
                if (strcmp(entry->d_name, files[i]) == 0) {
                    printf("驱动文件：%s\n", file_path);
                    closedir(dir);
                    return file_path;
                }
            }

            struct stat file_info;
            if (stat(file_path, &file_info) < 0) {
                free(file_path);
                file_path = NULL;
                continue;
            }

            if (strstr(entry->d_name, "gpiochip") != NULL) {
                free(file_path);
                file_path = NULL;
                continue;
            }

            if ((S_ISCHR(file_info.st_mode) || S_ISBLK(file_info.st_mode))
                && strchr(entry->d_name, '_') == NULL && strchr(entry->d_name, '-') == NULL && strchr(entry->d_name, ':') == NULL) {
                if (strcmp(entry->d_name, "stdin") == 0 || strcmp(entry->d_name, "stdout") == 0
                    || strcmp(entry->d_name, "stderr") == 0) {
                    free(file_path);
                    file_path = NULL;
                    continue;
                }

                size_t file_name_length = strlen(entry->d_name);
                time_t current_time;
                time(&current_time);
                int current_year = localtime(&current_time)->tm_year + 1900;
                int file_year = localtime(&file_info.st_ctime)->tm_year + 1900;
                if (file_year <= 1980) {
                    free(file_path);
                    file_path = NULL;
                    continue;
                }

                time_t atime = file_info.st_atime;
                time_t ctime = file_info.st_ctime;
                if ((atime == ctime)) {
                    if ((file_info.st_mode & S_IFMT) == 8192 && file_info.st_size == 0
                        && file_info.st_gid == 0 && file_info.st_uid == 0 && file_name_length <= 9) {
                        printf("驱动文件：%s\n", file_path);
                        closedir(dir);
                        return file_path;
                    }
                }
            }
            free(file_path);
            file_path = NULL;
        }
        closedir(dir);
        return NULL;
    }

    char *gtqwq() {
        const char *dev_path = "/dev";
        DIR *dir = opendir(dev_path);
        if (dir == NULL){
            printf("无法打开/dev目录\n");
            return NULL;
        }

        struct dirent *entry;
        char file_path[256];
        while ((entry = readdir(dir)) != NULL) {
            if (strstr(entry->d_name,"std") != NULL || strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || strstr(entry->d_name, "gpiochip") != NULL) {
                continue;
            }

            if(strchr(entry->d_name, '_') != NULL && strchr(entry->d_name, '-') != NULL && strchr(entry->d_name, ':') != NULL) {
                continue;
            }

            sprintf(file_path, "%s/%s", dev_path, entry->d_name);

            struct stat file_info;
            if (stat(file_path, &file_info) < 0)
                continue;

            if((localtime(&file_info.st_ctime)->tm_year + 1900) <= 1980)
                continue;

            if(strlen(entry->d_name) > 7 || strlen(entry->d_name) < 5)
                continue;

            if(file_info.st_gid != 0 || file_info.st_uid != 0)
                continue;

            if (S_ISCHR(file_info.st_mode) || S_ISBLK(file_info.st_mode)){
                if(file_info.st_gid == 0 && file_info.st_uid == 0){
                    printf("%s\n",file_path);
                    char *devpath = (char *)malloc(32);
                    strcpy(devpath,file_path);
                    closedir(dir);
                    return devpath;
                }
            }
        }
        closedir(dir);
        return NULL;
    }

    char *execCom(const char *shell) {
        FILE *fp = popen(shell, "r");
        if (fp == NULL) {
            perror("popen failed");
            return NULL;
        }
        char buffer[256];
        char *result = (char *)malloc(1000);
        result[0] = '\0';
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            strcat(result, buffer);
        }
        pclose(fp);
        return result;
    }

    int findFirstMatchingPath(const char *path, regex_t *regex, char *result) {
        DIR *dir;
        struct dirent *entry;

        if ((dir = opendir(path)) != NULL) {
            while ((entry = readdir(dir)) != NULL) {
                char fullpath[1024];
                snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
                if (entry->d_type == DT_LNK) {
                    char linkpath[1024];
                    ssize_t len = readlink(fullpath, linkpath, sizeof(linkpath) - 1);
                    if (len != -1) {
                        linkpath[len] = '\0';
                        if (regexec(regex, linkpath, 0, NULL, 0) == 0) {
                            strcpy(result, fullpath);
                            closedir(dir);
                            return 1;
                        }
                    } else {
                        perror("readlink");
                    }
                }
            }
            closedir(dir);
        } else {
            perror("Unable to open directory");
        }
        return 0;
    }

    void createDriverNode(char *path, int major_number, int minor_number) {
        std::string command = "mknod " + std::string(path) + " c " + std::to_string(major_number) + " " + std::to_string(minor_number);
        system(command.c_str());
        printf("\n");
    }

    void removeDeviceNode(char *path) {
        if (unlink(path) == 0) {
        } else {
        }
    }

    bool probe_kpm() {
        na_cmd cmd;
        prctl(0xDEAD, 0, 0, 0, 0);
        memset(&cmd, 0, sizeof(cmd));
        errno = 0;
        int r = prctl(NA_PRCTL_MAGIC, NA_CMD_GET_PID, (unsigned long)&cmd, 0, 0);
        return (r == 0);
    }

    bool kpm_read(uintptr_t addr, void *buffer, size_t size) {
        if (size > NA_MAX_RW_SIZE) return false;
        na_cmd cmd;
        memset(&cmd, 0, sizeof(cmd));
        cmd.pid  = (uint32_t)this->pid;
        cmd.addr = (uint64_t)addr;
        cmd.size = (uint32_t)size;
        if (prctl(NA_PRCTL_MAGIC, NA_CMD_READ_MEM, (unsigned long)&cmd, 0, 0) != 0) return false;
        if (cmd.result != 0) return false;
        memcpy(buffer, cmd.data, size);
        return true;
    }

    bool kpm_write(uintptr_t addr, void *buffer, size_t size) {
        if (size > NA_MAX_RW_SIZE) return false;
        na_cmd cmd;
        memset(&cmd, 0, sizeof(cmd));
        cmd.pid  = (uint32_t)this->pid;
        cmd.addr = (uint64_t)addr;
        cmd.size = (uint32_t)size;
        memcpy(cmd.data, buffer, size);
        if (prctl(NA_PRCTL_MAGIC, NA_CMD_WRITE_MEM, (unsigned long)&cmd, 0, 0) != 0) return false;
        return (cmd.result == 0);
    }

    uintptr_t kpm_module_base(const char *name) {
        char maps_path[64];
        snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", this->pid);
        FILE *f = fopen(maps_path, "r");
        if (!f) return 0;
        char line[512];
        uintptr_t base = 0;
        while (fgets(line, sizeof(line), f)) {
            if (strstr(line, name)) {
                base = (uintptr_t)strtoull(line, nullptr, 16);
                break;
            }
        }
        fclose(f);
        return base;
    }

    int getDriverSelection() {
        DisplayCurrentConfig();

        if (g_appConfig.settingsLoaded && g_appConfig.driverOption >= 1 && g_appConfig.driverOption <= 5) {
            printf("[Config] Using driver option from config: %d\n", g_appConfig.driverOption);
            return g_appConfig.driverOption;
        }

        printf("\033[1;36m[+] 1. RT Hook\033[0m\n");
        printf("\033[1;36m[+] 2. RT DEV\033[0m\n");
        printf("\033[1;36m[+] 3. DEV (Most Compatible)\033[0m\n");
        printf("\033[1;36m[+] 4. QX10~11.4\033[0m\n");
        printf("\033[1;36m[+] 5. KPM (Neo prctl)\033[0m\n");
        printf("\033[1;33m[?] Select an option (1-5): \033[0m");

        int selection;
        scanf("%d", &selection);

        if (selection < 1 || selection > 5) {
            printf("\033[41m\033[37m[FATAL] Invalid driver option selected! Exiting...\033[0m\n");
            exit(1);
        }

        g_appConfig.driverOption = selection;
        g_appConfig.settingsLoaded = true;
        SaveConfig();

        return selection;
    }

public:
    c_driver() {
        LoadConfig();

        选择值 = getDriverSelection();

        if (选择值 == 1) {
            printf("\033[41m\033[37mRT loaded\033[0m\n");
            fd = 0;
        } else if (选择值 == 2) {
            char *device_name = driver_path();
            if (device_name == NULL) {
                printf("\033[41m\033[37m[FATAL] RT DEV driver not found! Exiting...\033[0m\n");
                exit(1);
            }
            fd = open(device_name, O_RDWR);
            if (fd == -1) {
                printf("\033[41m\033[37m[FATAL] Driver not loaded! Exiting...\033[0m\n");
                free(device_name);
                exit(1);
            }
            free(device_name);
        } else if (选择值 == 3) {
            char *device_name = gtqwq();
            if (device_name == NULL) {
                printf("\033[41m\033[37m[FATAL] DEV driver not found! Exiting...\033[0m\n");
                exit(1);
            }
            fd = open(device_name, O_RDWR);
            if (fd == -1) {
                printf("\033[41m\033[37m[FATAL] Driver not loaded! Exiting...\033[0m\n");
                free(device_name);
                exit(1);
            }
            free(device_name);
        } else if (选择值 == 4) {
            char *output = execCom("ls -l /proc/*/exe 2>/dev/null | grep -E \"/data/[a-z]{6} \\(deleted\\)\"");
            char filePath[256] = {0};
            char pid[56] = {0};
            if (output != NULL && strlen(output) > 0) {
                char *procStart = strstr(output, "/proc/");
                if (procStart == NULL) {
                    printf("\033[41m\033[37m[FATAL] QX driver not found! Exiting...\033[0m\n");
                    free(output);
                    exit(1);
                }
                char *pidStart = procStart + 6;
                char *pidEnd = strchr(pidStart, '/');
                if (pidEnd == NULL) {
                    printf("\033[41m\033[37m[FATAL] QX driver not found! Exiting...\033[0m\n");
                    free(output);
                    exit(1);
                }
                strncpy(pid, pidStart, pidEnd - pidStart);
                pid[pidEnd - pidStart] = '\0';
                char *arrowStart = strstr(output, "->");
                if (arrowStart == NULL) {
                    printf("\033[41m\033[37m[FATAL] QX driver not found! Exiting...\033[0m\n");
                    free(output);
                    exit(1);
                }
                char *start = arrowStart + 3;
                char *end = strchr(output, '(');
                if (end == NULL) {
                    printf("\033[41m\033[37m[FATAL] QX driver not found! Exiting...\033[0m\n");
                    free(output);
                    exit(1);
                }
                end = end - 1;
                strncpy(filePath, start, end - start + 1);
                filePath[end - start] = '\0';
                char *replacePtr = strstr(filePath, "data");
                if (replacePtr != NULL) {
                    memmove(replacePtr + 2, replacePtr + 3, strlen(replacePtr + 3) + 1);
                    memmove(replacePtr, "dev", strlen("dev"));
                }
                free(output);
            } else {
                printf("\033[41m\033[37m[FATAL] QX driver not found! Exiting...\033[0m\n");
                if (output) free(output);
                exit(1);
            }

            char fdPath[256];
            char pattern[100];
            snprintf(pattern, sizeof(pattern), ".*%s.*", filePath + 5);
            int major_number = 0;
            snprintf(fdPath, sizeof(fdPath), "/proc/%s/fd", pid);
            regex_t regex;
            if (regcomp(&regex, pattern, 0) != 0) {
                fprintf(stderr, "[FATAL] Failed to compile regex! Exiting...\n");
                exit(1);
            }
            char result[1024];
            if (findFirstMatchingPath(fdPath, &regex, result)) {
                char cmd[256];
                sprintf(cmd, "ls -AL -l  %s | grep -Eo '[0-9]{3},' | grep  -Eo '[0-9]{3}'", result);
                char *fdInfo = execCom(cmd);
                if (fdInfo == NULL || strlen(fdInfo) == 0) {
                    printf("\033[41m\033[37m[FATAL] Driver not loaded! Exiting...\033[0m\n");
                    regfree(&regex);
                    if (fdInfo) free(fdInfo);
                    exit(1);
                }
                fdInfo[strlen(fdInfo)-1] = '\0';
                major_number = atoi(fdInfo);
                free(fdInfo);
            } else {
                printf("\033[41m\033[37m[FATAL] Driver not loaded! Exiting...\033[0m\n");
                regfree(&regex);
                exit(1);
            }
            regfree(&regex);
            if (filePath[0] != '\0') {
                createDriverNode(filePath, major_number, 0);
                fd = open(filePath, O_RDWR);
                if (fd == -1) {
                    removeDeviceNode(filePath);
                    printf("\033[41m\033[37m[FATAL] Driver not loaded! Exiting...\033[0m\n");
                    exit(1);
                } else {
                    removeDeviceNode(filePath);
                }
            } else {
                printf("\033[41m\033[37m[FATAL] Driver not loaded! Exiting...\033[0m\n");
                exit(1);
            }
        } else if (选择值 == 5) {
            if (probe_kpm()) {
                printf("\033[42m\033[37mKPM (Neo) backend active\033[0m\n");
                fd = -1;
            } else {
                printf("\033[41m\033[37m[FATAL] KPM not available on this kernel! Exiting...\033[0m\n");
                exit(1);
            }
        } else {
            printf("\033[41m\033[37m[FATAL] Invalid selection! Exiting...\033[0m\n");
            exit(1);
        }
        system("clear");
    }

    ~c_driver() {
        if (fd > 0)
            close(fd);
    }

    void initialize(pid_t pid) {
        this->pid = pid;
    }

    bool mem_addr_virtophy(unsigned long vaddr) {
        int pageSize = getpagesize();
        unsigned long v_pageIndex = vaddr / pageSize;
        unsigned long pfn_item_offset = v_pageIndex * sizeof(uint64_t);
        uint64_t item = 0;
        char filename[32];
        snprintf(filename, sizeof(filename), "/proc/%d/pagemap", this->pid);
        int fd = open(filename, O_RDONLY);
        if (fd < 0) {
            return false;
        }

        if (lseek(fd, pfn_item_offset, SEEK_SET) < 0) {
            close(fd);
            return false;
        }

        if (read(fd, &item, sizeof(uint64_t)) != sizeof(uint64_t)) {
            close(fd);
            return false;
        }

        if (0 == (item & (((uint64_t) 1) << 63))) {
            close(fd);
            return false;
        }
        close(fd);
        return true;
    }

    bool init_key(char* key) {
        if (选择值 == 2 || 选择值 == 3 || 选择值 == 4) {
            char buf[0x100];
            strcpy(buf, key);
            int opcode = DEV_OP_INIT_KEY;
            if (ioctl(fd, opcode, buf) != 0) {
                return false;
            }
            return true;
        }
        return false;
    }

    bool read(uintptr_t addr, void *buffer, size_t size) {
        if (选择值 == 5) {
            return kpm_read(addr, buffer, size);
        } else {
            COPY_MEMORY cm;
            cm.pid = this->pid;
            cm.addr = addr;
            cm.buffer = buffer;
            cm.size = size;

            int opcode;
            if (选择值 == 1) {
                opcode = HOOK_OP_READ_MEM;
            } else {
                opcode = DEV_OP_READ_MEM;
            }

            if (ioctl(fd, opcode, &cm) != 0) {
                return false;
            }
            return true;
        }
    }

    bool write(uintptr_t addr, void *buffer, size_t size) {
        if (选择值 == 5) {
            return kpm_write(addr, buffer, size);
        } else {
            COPY_MEMORY cm;
            cm.pid = this->pid;
            cm.addr = addr;
            cm.buffer = buffer;
            cm.size = size;

            int opcode;
            if (选择值 == 1) {
                opcode = HOOK_OP_WRITE_MEM;
            } else {
                opcode = DEV_OP_WRITE_MEM;
            }

            if (ioctl(fd, opcode, &cm) != 0) {
                return false;
            }
            return true;
        }
    }

    template <typename T>
    T read(uintptr_t addr) {
        T res{};
        if (this->read(addr, &res, sizeof(T))) {
            return res;
        }
        return res;
    }

    template <typename T>
    bool write(uintptr_t addr, T value) {
        return this->write(addr, &value, sizeof(T));
    }

    uintptr_t getModuleBase(char* name) {
        if (选择值 == 5) {
            return kpm_module_base(name);
        } else if (选择值 == 1) {
            FILE* fp;
            char cmd[0x100] = "";
            uintptr_t ret = 0;
            snprintf(cmd, sizeof(cmd), "ls -l /proc/%d/map_files/ | grep '%s'", pid, name);
            fp = popen(cmd, "r");
            if (!fp) {
                std::cerr << "Failed to run command" << std::endl;
                return 0;
            }
            fscanf(fp, "%*s %*d %*s %*s %*d %*s %*s %lx-%*lx", &ret);
            pclose(fp);
            return ret;
        } else if (选择值 == 2 || 选择值 == 3 || 选择值 == 4) {
            MODULE_BASE mb;
            char buf[0x100];
            strcpy(buf, name);
            mb.pid = this->pid;
            mb.name = buf;
            int opcode = DEV_OP_MODULE_BASE;
            if (ioctl(fd, opcode, &mb) != 0) {
                return 0;
            }
            return mb.base;
        }
        return 0;
    }

    void hide_process() {
        if (选择值 == 1) {
            ioctl(fd, HOOK_OP_HIDE_PROCESS);
        } else if (选择值 == 2 || 选择值 == 3 || 选择值 == 4) {
            ioctl(fd, DEV_OP_HIDE_PROCESS);
        }
    }
};

inline c_driver *driver = new c_driver();
#endif