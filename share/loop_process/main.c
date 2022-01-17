#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "./html.c"

int main(int argc, char *argv[])
{
    // Tracker for arguments
    int i = 1;

    // The final file we are creating - html formatted
    FILE *fp = html_init();

    while (i < argc)
    {
        int rc = fork();
        if (rc == 0)
        {
            exit(0);
        }
        i++;
    }

    html_close(fp);

    return 0;
}
