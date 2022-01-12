#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <sys/wait.h>
#include "./lib/input.c"
#include "./lib/html.c"

int main(int argc, char *argv[])
{
    int i;
    int display;

    if (argc > 1 && strcmp(argv[1], "-d") == 0)
    {
        i = 2;
        display = 1;
    }
    else
    {
        i = 1;
        display = 0;
    }

    FILE *fp = html_init();

    while (i < argc)
    {
        char *src = argv[i];
        int status;

        char *thumb = fmt_file(src, THUMB);
        char *final = fmt_file(src, FINAL);

        int rc = fork();
        if (rc == 0)
        {
            // First resize the image
            execlp("convert", "convert", "-resize", "10\%", src, thumb, NULL);
            exit(100);
        }
        waitpid(rc, &status, 0);

        if (display)
        {
            int display_fork = fork();
            if (display_fork == 0)
            {
                // Next display the image
                execlp("display", "display", src, NULL);
                exit(101);
            }
        }

        // Ask the user if they want to rotate
        char *rot_char = request_rot();
        int rotate_fork = fork();
        if (rotate_fork == 0)
        {
            execlp("convert", "convert", "-rotate", rot_char, thumb, thumb, NULL);
            exit(102);
        }

        int final_fork = fork();
        if (final_fork == 0)
        {
            execlp("convert", "convert", "-resize", "25\%", "-rotate", rot_char, src, final, NULL);
            exit(103);
        }

        // Ask the user for a caption
        char *cap = request_caption();

        html_add_line(fp, src, cap);

        // If we have more than 400 children (since each child has pid +1 from parent if available?) then wait a second for some children to die.
        // if (final_fork - getpid() > 400)
        // {
        //     sleep(1);
        // }

        i++;
    };

    html_close(fp);

    return 0;
}