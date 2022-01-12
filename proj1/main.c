#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "./lib/input.c"
#include "./lib/html.c"

int main(int argc, char *argv[])
{
    // Tracker for arguments
    int i;
    // Option to display or not - display doesn't work if ssh on Thayer Machines
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

    // The final file we are creating - html formatted
    FILE *fp = html_init();

    // Look over each argument
    while (i < argc)
    {
        // Current argument - expected to be a path to a file
        char *src = argv[i];
        int status;

        // Sanity check that is is in fact a file
        struct stat sb;
        if (stat(src, &sb) == -1)
        {
            perror("Invalid File Name");
            exit(EXIT_FAILURE);
        }

        // New pointers to our thumbnail and final file names
        char *thumb = fmt_file(src, THUMB);
        char *final = fmt_file(src, FINAL);

        // FORK #1: Resize to the thumbnail
        int rc = fork();
        if (rc == 0)
        {
            execlp("convert", "convert", "-resize", "10\%", src, thumb, NULL);
            exit(100);
        }
        // Wait becuase we are going to rotate/display the thumbnail
        waitpid(rc, &status, 0);

        // If we have the display toggle on
        if (display)
        {
            // FORK #2: Display the thumbnail
            int display_fork = fork();
            if (display_fork == 0)
            {
                // Next display the image
                execlp("display", "display", thumb, NULL);
                exit(101);
            }
        }

        // Ask the user if they want to rotate
        char *rot_char = request_rot();

        // FORK #3: Rotate the thumbnail
        int rotate_fork = fork();
        if (rotate_fork == 0)
        {
            execlp("convert", "convert", "-rotate", rot_char, thumb, thumb, NULL);
            exit(102);
        }

        // FORK #4: Resize and Rotate the src into the final image
        int final_fork = fork();
        if (final_fork == 0)
        {
            execlp("convert", "convert", "-resize", "25\%", "-rotate", rot_char, src, final, NULL);
            exit(103);
        }

        // Ask the user for a caption
        char *cap = request_caption();

        // Add data from current file to HTML file
        html_add_line(fp, src, cap);

        // We need to check to make sure at this point that we don't have too many processes open - need a way to count children.

        i++;
    };

    html_close(fp);

    return 0;
}