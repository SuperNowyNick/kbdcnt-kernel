#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "../module/kbdcnt.h"

int main(int argc, char *argv[])
{
        if (argc <= 1) {
                printf("Usage: kbdcntret -options\n\t-r - reset counter\n\t-c - get key press counter\n\t-t - get timestamp of last counter reset\n");
                return 0;
        }
        int file_desc = open("/dev/kbdcnt", 0);
        if (file_desc < 0) {
                printf("Error. Cannot open device file\n");
                return 1;
        }
        int ret = 0;
        for (int i = 1; i < argc; i++) {
                if (!strcmp(argv[i], "-r")) {
                        ret = ioctl(file_desc, IOCTL_RST, NULL);
                        if (ret < 0) {
                                printf("Error. Ioctl get count returned %d\n",
                                               ret);
                                return 1;
                        }
                }
                if (!strcmp(argv[i], "-c")) {
                        unsigned long long count = 0;
                        ret = ioctl(file_desc, IOCTL_GET_CNT, &count);
                        if (ret < 0) {
                                printf("Error. Ioctl get count returned %d\n",
                                                ret);
                                return 1;
                        }
                        printf("Key press count: %llu\n", count);
                }
                if (!strcmp(argv[i], "-t")) {
                        signed long long timens = 0;
                        ret = ioctl(file_desc, IOCTL_GET_TIME, &timens);
                        if (ret < 0) {
                                printf("Error. Ioctl get time returned %d\n",
                                                ret);
                        return 1;
                }
                time_t time = timens / 1000000000;
                struct tm *tm_info = localtime(&time);
                char buffer[26];
                strftime(buffer, 26, "%F %T", tm_info);
                printf("Last reset time: %s\n", buffer);
                }
        }
        return 0;
}
