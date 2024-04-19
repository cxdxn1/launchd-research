//
//  fakeDaemon.h
//  Omega
//
//  Created by Rayyan Khan

#ifndef fakeDaemon_h
#define fakeDaemon_h

#define LAUNCHD "/sbin/launchd.real"
#define HAXX "/sbin/haxx"

char *real_argv[];

int check_boot_argument(char *envp[]);

#endif /* fakeDaemon_h */
