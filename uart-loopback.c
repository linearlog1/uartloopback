/*
 *
 * Copyright © 2017 Advantch. All rights reserved
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <termios.h>
#include <sys/ioctl.h>


void print_usage(char *prg)
{
    fprintf(stderr, "\nUsage: %s [options] tty\n\n", prg);
    fprintf(stderr, "Options: -b <baud>  [9600|19200|38400|115200]\n");
    fprintf(stderr, "Options: -c <count> [1 ~ 127]\n");
    fprintf(stderr, "Options: -s stress test mode\n");
    fprintf(stderr, "         -h         (show this help)\n");
    fprintf(stderr, "\nExample:\n");
    fprintf(stderr, "uart-loopback /dev/ttyS1\n");
    fprintf(stderr, "\n");
    exit(1);
}

int main(int argc, char **argv)
{
    fd_set readfs;    /* file descriptor set */
    struct timeval Timeout;
    int fd,ret;
    char *tty;
    char rx[16];
    int btr;
    int brt_opt = B115200;
    int opt,count=100;
    char i;
	struct termios Opt;

    while ((opt = getopt(argc, argv, "b:c:s?")) != -1) {
        switch (opt) {
        case 'b':
            btr = atoi(optarg);
            switch (btr) {
                case 9600:
                    brt_opt = B9600;
                    break;
                case 19200:
                    brt_opt = B9600;
                    break;
                case 38400:
                    brt_opt = B9600;
                    break;
                case 115200:
                    brt_opt = B9600;
                    break;
                default:
                    printf("Err : unsupport baud rate!\n");
                    print_usage(argv[0]);
                    exit(1);
                    break;
            }
            break;

        case 'c':
            count = atoi(optarg);
            if ((count < 1) || (count > 127)) {
                printf("Err : count number not correct!\n");
                print_usage(argv[0]);
                exit(1);
            }
            break;
        case 's':
            count = 0xffff;
            break;
        case 'h':
        default:
            print_usage(argv[0]);
            break;
        }
    }

    if (argc - optind != 1)
        print_usage(argv[0]);

    tty = argv[optind];

    printf("baud=%d\n",btr);
    printf("count=%d\n",count);

    printf("uart port used: %s\n",tty);
    
    if ((fd = open (tty, O_RDWR | O_NOCTTY | O_NDELAY)) < 0) {
        perror(tty);
        exit(1);
    }

    tcflush(fd, TCIFLUSH);
    tcgetattr(fd, &Opt);

    cfsetispeed(&Opt, brt_opt);
	cfsetospeed(&Opt, brt_opt);

	Opt.c_cflag &= ~CSIZE;
    Opt.c_cflag |= CS8;  /* 8 bits */
    Opt.c_cflag &= ~CSTOPB; /* 1 stop bit */
    Opt.c_cflag &= ~PARENB; /* no parity */
    Opt.c_cflag &= ~PARODD;
    Opt.c_cflag &= ~CRTSCTS; /* HW flow control off */
    Opt.c_lflag =0; /* RAW input */
    Opt.c_iflag = 0;            /* SW flow control off, no parity checks etc */
    Opt.c_oflag &= ~OPOST; /* RAW output */
    Opt.c_cc[VTIME]=10; /* 1 sec */
    Opt.c_cc[VMIN]=16;
    Opt.c_cflag |= (CLOCAL | CREAD);

    if (tcsetattr(fd, TCSANOW, &Opt) != 0) {
        printf("set baud rate error!\n");
        exit(1);
    }

    for (i = 0; i < count; i++) {

		printf("Tx : %d\n", i);

        ret = write(fd, &i, 1);

		usleep(1000);

        if (ret != 1) {
            printf("write error! ret = 0x%x\n", ret);
            exit(1);
        }

        FD_SET(fd, &readfs);  /* set testing source */

        /* set timeout value within input loop */
        Timeout.tv_usec = 0;  /* milliseconds */
        Timeout.tv_sec  = 10;  /* seconds */

        ret = select(fd+1, &readfs, NULL, NULL, &Timeout);

        if (ret == 0) {
            printf("read timeout error!\n");
            exit(1);
        } else {
        	ret = read(fd, &rx, 1);
        }

        if (ret != 1) {
            printf("read error!\n");
            exit(1);
        }

        if (i != rx[0]) {
            printf("read data error: wrote 0x%x read 0x%x \n", i, rx[0]);
            exit(1);
        } else {
			printf("Rx : %d OK!\n", rx[0]);
		}

    }

    printf("\n");

    close(fd);

    return 0;
}
