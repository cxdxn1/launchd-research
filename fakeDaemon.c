//
//  fakeDaemon.c
//  Omega
//
//  Created by Rayyan Khan
//  Mostly stolen from https://github.com/TheRealClarity/haxx and https://github.com/asdfugil/haxx

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <spawn.h>
#include <string.h>
#include <IOKit/IOKitLib.h>
#include "fakeDaemon.h"

static const char __unused fakelaunchd[] = "fakelaunchd";

int main(int argc, char *argv[], char *envp[]) {
    if (getpid() != 1) {
        fprintf(stderr, "fakeDaemon cannot be run directly.\n");
        exit(1);
    }
    
    // We redirect standard input, output, and error to /dev/console
    int fd_console = open("/dev/console", O_RDWR, 0);
    dup2(fd_console, 0);
    dup2(fd_console, 1);
    dup2(fd_console, 2);
    // Ladies and gentlemen, we are PID 1 ¯\_(ツ)_/¯
    for (uint8_t i = 0; i < 10; i++) {
        printf("============ WE ARE PID 1 ============\n");
    }
    
    close(fd_console);
    close(0);
    close(1);
    close(2);

    if (check_boot_argument(envp)) {
        pid_t pid = fork();
        if (pid == 0) {
            execve(HAXX, real_argv, envp);
            fprintf(stderr, "cannot execute %s: %s... bailing\n", HAXX, strerror(errno));
            exit(1);
        }
    } else {
        pid_t pid = fork();
        if (pid == 0) {
            execve(LAUNCHD, argv, envp);
            fprintf(stderr, "cannot execute %s: %s... spinning\n", LAUNCHD, strerror(errno));
            while (1) {
                sleep(__INT_MAX__);
            }
        }
    }
    
    // Time to gtfo
    exit(42);
}

int check_boot_argument(char *envp[]) {
    io_registry_entry_t entry = IORegistryEntryFromPath(MACH_PORT_NULL, "IODeviceTree:/options");
    if (entry == 0) {
        execve(LAUNCHD, real_argv, envp);
        fprintf(stderr, "cannot execute %s: %s... bailing\n", LAUNCHD, strerror(errno));
        exit(1);
    }

    CFStringRef cfNvramVar = IORegistryEntryCreateCFProperty(entry, CFSTR("boot-args"), kCFAllocatorDefault, 0);
    if (cfNvramVar == NULL) {
        execve(LAUNCHD, real_argv, envp);
        fprintf(stderr, "cannot proceed %s: %s... bailing\n", LAUNCHD, strerror(errno));
        exit(1);
    }

    const char *nvramVar = CFStringGetCStringPtr(cfNvramVar, kCFStringEncodingUTF8);

    if (nvramVar != NULL) {
        if (strstr(nvramVar, "no_untether") != NULL) {
            int fd = open("/var/log/no_untether.txt", O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
            if (fd != -1) {
                const char *message = "no_untether boot argument is set.\n";
                write(fd, message, strlen(message));
                close(fd);
            }
            return 1;
        }
    }

    return 0;
}
