#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <pwd.h>
#include <time.h>
#include <dirent.h>
#include <glob.h>

#define MAX_LINE 2048
#define CONFIG_FILE "/.config/cfetch/config"
#define ASCII_DIR "/.config/cfetch/ascii/"
#define RESET "\033[0m"
#define BOLD "\033[1m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define PURPLE "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"

typedef struct {
    int show_os;
    int show_kernel;
    int show_uptime;
    int show_packages;
    int show_shell;
    int show_terminal;
    int show_wm;
    int show_cpu;
    int show_gpu;
    int show_memory;
    int show_disk;
    int show_resolution;
    int show_colors;
    char ascii_file[256];
    char color_scheme[64];
} config_t;

typedef struct {
    char username[256];
    char hostname[256];
    char distro[512];
    char kernel[256];
    char uptime[256];
    char shell[256];
    char terminal[256];
    char cpu[512];
    char gpu[512];
    char gpu_driver[256];
    char gpu_memory[128];
    char memory[256];
    char disk[256];
    char resolution[256];
    char wm[256];
    char compositor[256];
    char packages[256];
    int cpu_cores;
    char distro_id[64];
} system_info_t;

void create_default_config(const char* config_path) {
    FILE* config = fopen(config_path, "w");
    if (!config) return;
    
    fprintf(config, "# CFETCH Configuration File\n");
    fprintf(config, "# Set to 1 to show, 0 to hide\n\n");
    fprintf(config, "show_os=1\n");
    fprintf(config, "show_kernel=1\n");
    fprintf(config, "show_uptime=1\n");
    fprintf(config, "show_packages=1\n");
    fprintf(config, "show_shell=1\n");
    fprintf(config, "show_terminal=1\n");
    fprintf(config, "show_wm=1\n");
    fprintf(config, "show_cpu=1\n");
    fprintf(config, "show_gpu=1\n");
    fprintf(config, "show_memory=1\n");
    fprintf(config, "show_disk=1\n");
    fprintf(config, "show_resolution=1\n");
    fprintf(config, "show_colors=1\n\n");
    fprintf(config, "# ASCII file (auto for distro detection, or specify file)\n");
    fprintf(config, "ascii_file=auto\n\n");
    fprintf(config, "# Color scheme (blue, red, green, purple, cyan, yellow)\n");
    fprintf(config, "color_scheme=blue\n");
    
    fclose(config);
}

void load_config(config_t* config) {
    char* home = getenv("HOME");
    char config_path[512];
    char config_dir[512];
    
    snprintf(config_dir, sizeof(config_dir), "%s/.config/cfetch", home);
    snprintf(config_path, sizeof(config_path), "%s%s", home, CONFIG_FILE);
    
    mkdir(config_dir, 0755);
    
    config->show_os = 1;
    config->show_kernel = 1;
    config->show_uptime = 1;
    config->show_packages = 1;
    config->show_shell = 1;
    config->show_terminal = 1;
    config->show_wm = 1;
    config->show_cpu = 1;
    config->show_gpu = 1;
    config->show_memory = 1;
    config->show_disk = 1;
    config->show_resolution = 1;
    config->show_colors = 1;
    strcpy(config->ascii_file, "auto");
    strcpy(config->color_scheme, "blue");
    
    FILE* file = fopen(config_path, "r");
    if (!file) {
        create_default_config(config_path);
        return;
    }
    
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        
        char key[256], value[256];
        if (sscanf(line, "%255[^=]=%255s", key, value) == 2) {
            if (strcmp(key, "show_os") == 0) config->show_os = atoi(value);
            else if (strcmp(key, "show_kernel") == 0) config->show_kernel = atoi(value);
            else if (strcmp(key, "show_uptime") == 0) config->show_uptime = atoi(value);
            else if (strcmp(key, "show_packages") == 0) config->show_packages = atoi(value);
            else if (strcmp(key, "show_shell") == 0) config->show_shell = atoi(value);
            else if (strcmp(key, "show_terminal") == 0) config->show_terminal = atoi(value);
            else if (strcmp(key, "show_wm") == 0) config->show_wm = atoi(value);
            else if (strcmp(key, "show_cpu") == 0) config->show_cpu = atoi(value);
            else if (strcmp(key, "show_gpu") == 0) config->show_gpu = atoi(value);
            else if (strcmp(key, "show_memory") == 0) config->show_memory = atoi(value);
            else if (strcmp(key, "show_disk") == 0) config->show_disk = atoi(value);
            else if (strcmp(key, "show_resolution") == 0) config->show_resolution = atoi(value);
            else if (strcmp(key, "show_colors") == 0) config->show_colors = atoi(value);
            else if (strcmp(key, "ascii_file") == 0) strcpy(config->ascii_file, value);
            else if (strcmp(key, "color_scheme") == 0) strcpy(config->color_scheme, value);
        }
    }
    fclose(file);
}

const char* get_color(const char* scheme) {
    if (strcmp(scheme, "red") == 0) return RED;
    else if (strcmp(scheme, "green") == 0) return GREEN;
    else if (strcmp(scheme, "yellow") == 0) return YELLOW;
    else if (strcmp(scheme, "purple") == 0) return PURPLE;
    else if (strcmp(scheme, "cyan") == 0) return CYAN;
    else if (strcmp(scheme, "white") == 0) return WHITE;
    return BLUE;
}

void create_ascii_files(void) {
    char* home = getenv("HOME");
    char ascii_dir[512];
    snprintf(ascii_dir, sizeof(ascii_dir), "%s%s", home, ASCII_DIR);
    
    mkdir(ascii_dir, 0755);
    
    char file_path[512];
    FILE* file;
    
    snprintf(file_path, sizeof(file_path), "%subuntu", ascii_dir);
    file = fopen(file_path, "w");
    if (file) {
        fprintf(file, "         _\n");
        fprintf(file, "     ---(_)\n");
        fprintf(file, " _/  ---  \\\n");
        fprintf(file, "(_) |   |\n");
        fprintf(file, "  \\  --- _/\n");
        fprintf(file, "     ---(_)\n");
        fclose(file);
    }
    
    snprintf(file_path, sizeof(file_path), "%sdebian", ascii_dir);
    file = fopen(file_path, "w");
    if (file) {
        fprintf(file, "       _,met$$$$$gg.\n");
        fprintf(file, "    ,g$$$$$$$$$$$$$$$P.\n");
        fprintf(file, "  ,g$$P\"     \"\"\"Y$$.\".\n");
        fprintf(file, " ,$$P'              `$$$.\n");
        fprintf(file, "',$$P       ,ggs.     `$$b:\n");
        fprintf(file, "`d$$'     ,$P\"'   .    $$$\n");
        fprintf(file, " $$P      d$'     ,    $$P\n");
        fprintf(file, " $$:      $$.   -    ,d$$'\n");
        fprintf(file, " $$;      Y$b._   _,d$P'\n");
        fprintf(file, " Y$$.    `.`\"Y$$$$P\"'\n");
        fprintf(file, " `$$b      \"-.__\n");
        fprintf(file, "  `Y$$\n");
        fprintf(file, "   `Y$$.\n");
        fprintf(file, "     `$$b.\n");
        fprintf(file, "       `Y$$b.\n");
        fprintf(file, "          `\"Y$b._\n");
        fprintf(file, "              `\"\"\"\n");
        fclose(file);
    }
    
    snprintf(file_path, sizeof(file_path), "%sarch", ascii_dir);
    file = fopen(file_path, "w");
    if (file) {
        fprintf(file, "                   -`\n");
        fprintf(file, "                  .o+`\n");
        fprintf(file, "                 `ooo/\n");
        fprintf(file, "                `+oooo:\n");
        fprintf(file, "               `+oooooo:\n");
        fprintf(file, "               -+oooooo+:\n");
        fprintf(file, "             `/:-:++oooo+:\n");
        fprintf(file, "            `/++++/+++++++:\n");
        fprintf(file, "           `/++++++++++++++:\n");
        fprintf(file, "          `/+++ooooooooooooo/`\n");
        fprintf(file, "         ./ooosssso++osssssso+oss/\n");
        fprintf(file, "        .oossssso-````/ossssss+oss+\n");
        fprintf(file, "       -osssssso.      :ssssssso.\n");
        fprintf(file, "      :osssssss/        osssso+++.\n");
        fprintf(file, "     /ossssssss/        +ssssooo/-\n");
        fprintf(file, "   `/ossssso+/:-        -:/+osssso+-\n");
        fprintf(file, "  `+sso+:-`                 `.-/+oso:\n");
        fprintf(file, " `++:.                           `-/+/\n");
        fprintf(file, " .`                                 `/\n");
        fclose(file);
    }
    
    snprintf(file_path, sizeof(file_path), "%sfedora", ascii_dir);
    file = fopen(file_path, "w");
    if (file) {
        fprintf(file, "        ______\n");
        fprintf(file, "       /  ____)\n");
        fprintf(file, "      /  /\n");
        fprintf(file, "     /  /\n");
        fprintf(file, "    /  /  _____\n");
        fprintf(file, "   /  /  /____/\n");
        fprintf(file, "  /  /______\n");
        fprintf(file, " /       ___)\n");
        fprintf(file, "/_________/\n");
        fclose(file);
    }
    
    snprintf(file_path, sizeof(file_path), "%smanjaro", ascii_dir);
    file = fopen(file_path, "w");
    if (file) {
        fprintf(file, "██████████████████  ████████\n");
        fprintf(file, "██████████████████  ████████\n");
        fprintf(file, "██████████████████  ████████\n");
        fprintf(file, "██████████████████  ████████\n");
        fprintf(file, "████████            ████████\n");
        fprintf(file, "████████  ████████  ████████\n");
        fprintf(file, "████████  ████████  ████████\n");
        fprintf(file, "████████  ████████  ████████\n");
        fprintf(file, "████████  ████████  ████████\n");
        fprintf(file, "████████  ████████  ████████\n");
        fprintf(file, "████████  ████████  ████████\n");
        fprintf(file, "████████  ████████  ████████\n");
        fprintf(file, "████████  ████████  ████████\n");
        fprintf(file, "████████  ████████  ████████\n");
        fclose(file);
    }
    
    snprintf(file_path, sizeof(file_path), "%skali", ascii_dir);
    file = fopen(file_path, "w");
    if (file) {
        fprintf(file, "      ..,;:ccc,.\n");
        fprintf(file, "     ......''';lxO.\n");
        fprintf(file, "....''''..........,:ld;\n");
        fprintf(file, "           .';;;:::;,,.x,\n");
        fprintf(file, "      ..'''.            0Xxoc:,.  ...\n");
        fprintf(file, "  ....                ,ONkc;,;cokOdc',.\n");
        fprintf(file, " .                   OMo           ':ddo.\n");
        fprintf(file, "                    dMc               :OO;\n");
        fprintf(file, "                    0M.                 .:o.\n");
        fprintf(file, "                    ;Wd\n");
        fprintf(file, "                     ;XO,\n");
        fprintf(file, "                       ,d0Odlc;,..\n");
        fprintf(file, "                           ..',;:cdOOd::,.\n");
        fprintf(file, "                                    .:d;.':;\n");
        fprintf(file, "                                       'd,  .'\n");
        fprintf(file, "                                         ;l   ..\n");
        fprintf(file, "                                          .o\n");
        fprintf(file, "                                            c\n");
        fprintf(file, "                                            .'\n");
        fprintf(file, "                                             .\n");
        fclose(file);
    }
    
    snprintf(file_path, sizeof(file_path), "%slinux", ascii_dir);
    file = fopen(file_path, "w");
    if (file) {
        fprintf(file, "                 ###\n");
        fprintf(file, "               #######\n");
        fprintf(file, "             ###########\n");
        fprintf(file, "            #############\n");
        fprintf(file, "           ###############\n");
        fprintf(file, "          #################\n");
        fprintf(file, "         ###################\n");
        fprintf(file, "        #####################\n");
        fprintf(file, "        #####################\n");
        fprintf(file, "        #####################\n");
        fprintf(file, "         ###################\n");
        fprintf(file, "          #################\n");
        fprintf(file, "           ###############\n");
        fprintf(file, "            #############\n");
        fprintf(file, "             ###########\n");
        fprintf(file, "               #######\n");
        fprintf(file, "                 ###\n");
        fclose(file);
    }
}
char **print_ascii_art(const char* ascii_file, const char* color) {
    char* home = getenv("HOME");
    char file_path[512];

    snprintf(file_path, sizeof(file_path), "%s%s%s", home, ASCII_DIR, ascii_file);
    FILE* file = fopen(file_path, "r");
    if (!file) {
        snprintf(file_path, sizeof(file_path), "%s%slinux", home, ASCII_DIR);
        file = fopen(file_path, "r");
    }

    if (!file) {
        printf("%sNo ASCII art found!%s\n", color, RESET);
        return NULL;
    }

    char **lines = malloc(sizeof(char *) * MAX_LINE);
    if (!lines) {
        perror("malloc error");
        fclose(file);
        return NULL;
    }
    
    char line[MAX_LINE];
    int count = 0;

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';

        if (line[0] == '\0') {
            break;
        }
        lines[count] = strdup(line);
        if (!lines[count]) {
            perror("strdup error");
            for (int i = 0; i < count; ++i) {
                free(lines[i]);
            }
            free(lines);
            fclose(file);
            return NULL;
        }

        count++;
        if (count >= MAX_LINE) break;
    }
    
    fclose(file);
    lines[count] = NULL;
    
    return lines;
}

void get_nvidia_gpu_info(system_info_t* info) {
    FILE* nvidia_smi = NULL;
    char command[] = "nvidia-smi --query-gpu=name,driver_version,memory.total --format=csv,noheader,nounits 2>/dev/null";
    nvidia_smi = popen(command, "r");
    
    if (nvidia_smi) {
        char line[MAX_LINE];
        if (fgets(line, sizeof(line), nvidia_smi)) {
            char* gpu_name = strtok(line, ",");
            char* driver_ver = strtok(NULL, ",");
            char* gpu_mem = strtok(NULL, ",");
            
            if (gpu_name) {
                while (*gpu_name == ' ') gpu_name++;
                strcpy(info->gpu, gpu_name);
            }
            
            if (driver_ver) {
                while (*driver_ver == ' ') driver_ver++;
                snprintf(info->gpu_driver, sizeof(info->gpu_driver), "NVIDIA %s", driver_ver);
            }
            
            if (gpu_mem) {
                while (*gpu_mem == ' ') gpu_mem++;
                char* newline = strchr(gpu_mem, '\n');
                if (newline) *newline = '\0';
                snprintf(info->gpu_memory, sizeof(info->gpu_memory), "%s MB", gpu_mem);
            }
        }
        pclose(nvidia_smi);
        return;
    }
}

void get_amd_gpu_info(system_info_t* info) {
    FILE* rocm_smi = NULL;
    char command[] = "rocm-smi --showproductname --csv 2>/dev/null | tail -n +2";
    rocm_smi = popen(command, "r");
    
    if (rocm_smi) {
        char line[MAX_LINE];
        if (fgets(line, sizeof(line), rocm_smi)) {
            char* gpu_name = strtok(line, ",");
            if (gpu_name) {
                strcpy(info->gpu, gpu_name);
                strcpy(info->gpu_driver, "AMDGPU");
            }
        }
        pclose(rocm_smi);
        return;
    }
    
    FILE* lspci_amd = NULL;
    char command2[] = "lspci | grep -i 'vga.*amd\\|vga.*ati\\|display.*amd'";
    lspci_amd = popen(command2, "r");
    
    if (lspci_amd) {
        char line[MAX_LINE];
        if (fgets(line, sizeof(line), lspci_amd)) {
            char* colon = strchr(line, ':');
            if (colon) {
                colon += 2;
                char* newline = strchr(colon, '\n');
                if (newline) *newline = '\0';
                strcpy(info->gpu, colon);
                strcpy(info->gpu_driver, "AMDGPU");
            }
        }
        pclose(lspci_amd);
        return;
    }
}

void get_intel_gpu_info(system_info_t* info) {
    FILE* intel_gpu = NULL;
    char command[] = "lspci | grep -i 'vga.*intel\\|display.*intel'";
    intel_gpu = popen(command, "r");
    
    if (intel_gpu) {
        char line[MAX_LINE];
        if (fgets(line, sizeof(line), intel_gpu)) {
            char* colon = strchr(line, ':');
            if (colon) {
                colon += 2;
                char* newline = strchr(colon, '\n');
                if (newline) *newline = '\0';
                strcpy(info->gpu, colon);
                strcpy(info->gpu_driver, "Intel");
            }
        }
        pclose(intel_gpu);
        return;
    }
}

void get_gpu_info(system_info_t* info) {
    strcpy(info->gpu, "Unknown");
    strcpy(info->gpu_driver, "Unknown");
    strcpy(info->gpu_memory, "Unknown");
    
    get_nvidia_gpu_info(info);
    
    if (strcmp(info->gpu, "Unknown") == 0) {
        get_amd_gpu_info(info);
    }
    
    if (strcmp(info->gpu, "Unknown") == 0) {
        get_intel_gpu_info(info);
    }
    
    if (strcmp(info->gpu, "Unknown") == 0) {
        FILE* lspci = NULL;
        char command[] = "lspci | grep -i 'vga\\|3d\\|display'";
        lspci = popen(command, "r");
        
        if (lspci) {
            char line[MAX_LINE];
            if (fgets(line, sizeof(line), lspci)) {
                char* colon = strchr(line, ':');
                if (colon) {
                    colon += 2;
                    char* newline = strchr(colon, '\n');
                    if (newline) *newline = '\0';
                    strcpy(info->gpu, colon);
                }
            }
            pclose(lspci);
        }
    }
}

void get_user_info(system_info_t* info) {
    struct passwd* pw = getpwuid(getuid());
    
    if (pw) {
        strcpy(info->username, pw->pw_name);
    } else {
        strcpy(info->username, "unknown");
    }
    
    if (gethostname(info->hostname, sizeof(info->hostname)) != 0) {
        strcpy(info->hostname, "unknown");
    }
}

void get_distro_info(system_info_t* info) {
    FILE* file = fopen("/etc/os-release", "r");
    char line[MAX_LINE];
    strcpy(info->distro, "Unknown Linux");
    strcpy(info->distro_id, "linux");
    
    if (!file) {
        file = fopen("/usr/lib/os-release", "r");
    }
    
    if (!file) {
        return;
    }
    
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "PRETTY_NAME=", 12) == 0) {
            char* start = strchr(line, '"');
            if (start) {
                start++;
                char* end = strrchr(start, '"');
                if (end) {
                    *end = '\0';
                    strcpy(info->distro, start);
                }
            }
        } else if (strncmp(line, "ID=", 3) == 0) {
            char* start = line + 3;
            char* end = strchr(start, '\n');
            if (end) *end = '\0';
            
            if (start[0] == '"') {
                start++;
                end = strchr(start, '"');
                if (end) *end = '\0';
            }
            
            strcpy(info->distro_id, start);
        }
    }
    fclose(file);
}

void get_kernel_info(system_info_t* info) {
    struct utsname buffer;
    
    if (uname(&buffer) == 0) {
        snprintf(info->kernel, sizeof(info->kernel), "%s %s", buffer.sysname, buffer.release);
    } else {
        strcpy(info->kernel, "Unknown");
    }
}

void get_uptime_info(system_info_t* info) {
    struct sysinfo si;
    
    if (sysinfo(&si) == 0) {
        long uptime_seconds = si.uptime;
        long days = uptime_seconds / 86400;
        long hours = (uptime_seconds % 86400) / 3600;
        long minutes = (uptime_seconds % 3600) / 60;
        
        if (days > 0) {
            snprintf(info->uptime, sizeof(info->uptime), "%ld days, %ld hours, %ld mins", days, hours, minutes);
        } else if (hours > 0) {
            snprintf(info->uptime, sizeof(info->uptime), "%ld hours, %ld mins", hours, minutes);
        } else {
            snprintf(info->uptime, sizeof(info->uptime), "%ld mins", minutes);
        }
    } else {
        strcpy(info->uptime, "Unknown");
    }
}

void get_shell_info(system_info_t* info) {
    char* shell_path = getenv("SHELL");
    
    if (shell_path) {
        char* shell_basename = strrchr(shell_path, '/');
        if (shell_basename) {
            strcpy(info->shell, shell_basename + 1);
        } else {
            strcpy(info->shell, shell_path);
        }
    } else {
        strcpy(info->shell, "Unknown");
    }
}

void get_terminal_info(system_info_t* info) {
    char* term_program = getenv("TERM_PROGRAM");
    char* term = getenv("TERM");
    
    if (term_program) {
        strcpy(info->terminal, term_program);
    } else if (term) {
        strcpy(info->terminal, term);
    } else {
        strcpy(info->terminal, "Unknown");
    }
}

void get_wm_compositor_info(system_info_t* info) {
    strcpy(info->wm, "Unknown");
    strcpy(info->compositor, "Unknown");
    
    char* wayland_display = getenv("WAYLAND_DISPLAY");
    char* xdg_session_type = getenv("XDG_SESSION_TYPE");
    
    if (wayland_display || (xdg_session_type && strcmp(xdg_session_type, "wayland") == 0)) {
        strcpy(info->compositor, "Wayland");
        
        FILE* ps_cmd = NULL;
        char command[] = "ps aux | grep -E '(sway|wayfire|river|hyprland|mutter|kwin)' | grep -v grep";
        ps_cmd = popen(command, "r");
        
        if (ps_cmd) {
            char line[MAX_LINE];
            if (fgets(line, sizeof(line), ps_cmd)) {
                if (strstr(line, "sway")) strcpy(info->wm, "Sway");
                else if (strstr(line, "wayfire")) strcpy(info->wm, "Wayfire");
                else if (strstr(line, "river")) strcpy(info->wm, "River");
                else if (strstr(line, "hyprland")) strcpy(info->wm, "Hyprland");
                else if (strstr(line, "mutter")) strcpy(info->wm, "Mutter");
                else if (strstr(line, "kwin")) strcpy(info->wm, "KWin");
            }
            pclose(ps_cmd);
        }
    } else {
        strcpy(info->compositor, "X11");
        
        char* xdg_desktop = getenv("XDG_CURRENT_DESKTOP");
        if (xdg_desktop) {
            strcpy(info->wm, xdg_desktop);
        }
    }
}

void get_cpu_info(system_info_t* info) {
    FILE* file = fopen("/proc/cpuinfo", "r");
    char line[MAX_LINE];
    strcpy(info->cpu, "Unknown");
    info->cpu_cores = 0;
    
    if (!file) return;
    
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "model name", 10) == 0) {
            char* colon = strchr(line, ':');
            if (colon) {
                colon += 2;
                char* newline = strchr(colon, '\n');
                if (newline) *newline = '\0';
                
                char* at_sign = strstr(colon, " @");
                if (at_sign) *at_sign = '\0';
                
                strcpy(info->cpu, colon);
            }
        }
        if (strncmp(line, "processor", 9) == 0) {
            info->cpu_cores++;
        }
    }
    fclose(file);
    
    if (info->cpu_cores > 0) {
        char temp[512];
        snprintf(temp, sizeof(temp), "%s (%d cores)", info->cpu, info->cpu_cores);
        strcpy(info->cpu, temp);
    }
}

void get_memory_info(system_info_t* info) {
    struct sysinfo si;
    
    if (sysinfo(&si) == 0) {
        unsigned long total_mb = si.totalram / 1024 / 1024;
        unsigned long used_mb = (si.totalram - si.freeram) / 1024 / 1024;
        
        snprintf(info->memory, sizeof(info->memory), "%lu MB / %lu MB", used_mb, total_mb);
    } else {
        strcpy(info->memory, "Unknown");
    }
}

void get_disk_info(system_info_t* info) {
    struct statvfs stat;
    
    if (statvfs("/", &stat) == 0) {
        unsigned long long total_gb = (stat.f_blocks * stat.f_frsize) / (1024 * 1024 * 1024);
        unsigned long long used_gb = ((stat.f_blocks - stat.f_bavail) * stat.f_frsize) / (1024 * 1024 * 1024);
        
        snprintf(info->disk, sizeof(info->disk), "%llu GB / %llu GB", used_gb, total_gb);
    } else {
        strcpy(info->disk, "Unknown");
    }
}

void get_resolution_info(system_info_t* info) {
    strcpy(info->resolution, "Unknown");
    
    char* wayland_display = getenv("WAYLAND_DISPLAY");
    if (wayland_display) {
        FILE* sway_cmd = NULL;
        char command[] = "swaymsg -t get_outputs 2>/dev/null | grep -o '\"current_mode\":{\"width\":[0-9]*,\"height\":[0-9]*'";
        sway_cmd = popen(command, "r");
        
        if (sway_cmd) {
            char line[256];
            if (fgets(line, sizeof(line), sway_cmd)) {
                int width, height;
                if (sscanf(line, "\"current_mode\":{\"width\":%d,\"height\":%d", &width, &height) == 2) {
                    snprintf(info->resolution, sizeof(info->resolution), "%dx%d", width, height);
                }
            }
            pclose(sway_cmd);
        }
    } else {
        FILE* xrandr = NULL;
        char command[] = "xrandr 2>/dev/null | grep ' connected' | head -1";
        xrandr = popen(command, "r");
        
        if (xrandr) {
            char line[MAX_LINE];
            if (fgets(line, sizeof(line), xrandr)) {
                char* res_start = strstr(line, " ");
                if (res_start) {
                    res_start = strstr(res_start + 1, " ");
                    if (res_start) {
                        res_start++;
                        char* space = strchr(res_start, ' ');
                        if (space) {
                            *space = '\0';
                            strcpy(info->resolution, res_start);
                        }
                    }
                }
            }
            pclose(xrandr);
        }
    }
}

void get_packages_info(system_info_t* info) {
    int package_count = 0;
    char manager[64] = "Unknown";
    
    FILE* dpkg = NULL;
    char command1[] = "dpkg -l 2>/dev/null | grep '^ii' | wc -l";
    dpkg = popen(command1, "r");
    
    if (dpkg && fscanf(dpkg, "%d", &package_count) == 1 && package_count > 0) {
        strcpy(manager, "dpkg");
        pclose(dpkg);
    } else {
        if (dpkg) pclose(dpkg);
        
        FILE* pacman = NULL;
        char command2[] = "pacman -Q 2>/dev/null | wc -l";
        pacman = popen(command2, "r");
        
        if (pacman && fscanf(pacman, "%d", &package_count) == 1 && package_count > 0) {
            strcpy(manager, "pacman");
            pclose(pacman);
        } else {
            if (pacman) pclose(pacman);
            
            FILE* rpm = NULL;
            char command3[] = "rpm -qa 2>/dev/null | wc -l";
            rpm = popen(command3, "r");
            
            if (rpm && fscanf(rpm, "%d", &package_count) == 1 && package_count > 0) {
                strcpy(manager, "rpm");
            }
            if (rpm) pclose(rpm);
        }
    }
    
    if (package_count > 0) {
        snprintf(info->packages, sizeof(info->packages), "%d (%s)", package_count, manager);
    } else {
        strcpy(info->packages, "Unknown");
    }
}

void free_string_array(char **arr) {
    if (!arr) return;
    int i = 0;
    while (arr[i]) {
        free(arr[i]);
        i++;
    }
    free(arr);
}


void print_system_info(system_info_t *info, config_t *config) {
    const char* color = get_color(config->color_scheme);
    
    char **ascii;
    if (strcmp(config->ascii_file, "auto") == 0) {
        ascii = print_ascii_art(info->distro_id, color);
    } else {
        ascii = print_ascii_art(config->ascii_file, color);
    }

    if (!ascii) {
        fprintf(stderr, "ASCII yüklenemedi!\n");
        return;
    }
    int sizeascii = 0;
    while (ascii[sizeascii] != NULL) sizeascii++;

    int max_ascii_width = 0;
    for (int i = 0; i < sizeascii; i++) {
        int len = (int)strlen(ascii[i]);
        if (len > max_ascii_width) max_ascii_width = len;
    }

    typedef struct {
      const char *label;
      const char *value;
      int enabled;
      int is_header;
    } info_line_t;

    char user_host[512];
    snprintf(user_host, sizeof(user_host), "%s@%s", info->username, info->hostname);

    info_line_t infos[] = {
        {"", user_host, 1, 1},
        {"OS:", info->distro, config->show_os, 0},
        {"Kernel:", info->kernel, config->show_kernel, 0},
        {"Uptime:", info->uptime, config->show_uptime, 0},
        {"Packages:", info->packages, config->show_packages, 0},
        {"Shell:", info->shell, config->show_shell, 0},
        {"Terminal:", info->terminal, config->show_terminal, 0},
        {"DE/WM:", (strcmp(info->compositor, "Unknown") != 0) ? info->wm : NULL, config->show_wm, 0},
        {"CPU:", info->cpu, config->show_cpu, 0},
        {"GPU:", (strcmp(info->gpu, "Unknown") != 0) ? info->gpu : NULL, config->show_gpu, 0},
        {"GPU Driver:", (strcmp(info->gpu_driver, "Unknown") != 0) ? info->gpu_driver : NULL, config->show_gpu, 0},
        {"GPU Memory:", (strcmp(info->gpu_memory, "Unknown") != 0) ? info->gpu_memory : NULL, config->show_gpu, 0},
        {"Memory:", info->memory, config->show_memory, 0},
        {"Disk:", info->disk, config->show_disk, 0},
        {"Resolution:", (strcmp(info->resolution, "Unknown") != 0) ? info->resolution : NULL, config->show_resolution, 0}
    };

    int info_count = sizeof(infos) / sizeof(infos[0]);


    int active_info_count = 0;
    for (int i = 0; i < info_count; i++) {
        if (infos[i].enabled && infos[i].value != NULL)
            active_info_count++;
    }

    int max_lines = sizeascii > active_info_count ? sizeascii : active_info_count;

    printf("\n"); 

    int info_index = 0;
    for (int i = 0; i < max_lines; i++) {
        const char *art_line = (i < sizeascii) ? ascii[i] : "";

        const char *label = NULL;
        const char *value = NULL;
        int is_header = 0;

        while (info_index < info_count) {
            if (infos[info_index].enabled && infos[info_index].value != NULL) {
                label = infos[info_index].label;
                value = infos[info_index].value;
                is_header = infos[info_index].is_header;
                info_index++;
                break;
            }
            info_index++;
        }


        printf("%s%-*s%s", color, max_ascii_width + 2, art_line, RESET);

        if (label && value) {
            if (is_header) {
                printf("| %s%s%s%s\n", color, BOLD, value, RESET);
            } else {
                printf("| %s%s%-10s%s %s\n", color, BOLD, label, RESET, value);
            }
        } else {
            printf("\n");
        }
    }

    if (config->show_colors) {
        printf("\n%s", color);
        printf("████████");
        printf("%s", RESET);
        printf("\n");
    }

    printf("\n");
    
    free_string_array(ascii);
}

void gather_all_info(system_info_t* info) {
    get_user_info(info);
    get_distro_info(info);
    get_kernel_info(info);
    get_uptime_info(info);
    get_shell_info(info);
    get_terminal_info(info);
    get_wm_compositor_info(info);
    get_cpu_info(info);
    get_gpu_info(info);
    get_memory_info(info);
    get_disk_info(info);
    get_resolution_info(info);
    get_packages_info(info);
}

int main(void) {
    system_info_t info;
    config_t config;
    
    printf("\033[2J\033[H");
    
    load_config(&config);
    create_ascii_files();
    gather_all_info(&info);
    
    const char* color = get_color(config.color_scheme);
    /*
    if (strcmp(config.ascii_file, "auto") == 0) {
        print_ascii_art(info.distro_id, color);
    } else {
        print_ascii_art(config.ascii_file, color);
	} */
    
    print_system_info(&info, &config);
    
    return 0;
}
