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

#define RPIPE 0
#define WPIPE 1

#define PIPE_ELEMENT_SIZE 8

#define BATCH_SIZE 100

int file_exists(char *src)
{
    struct stat sb;
    return (stat(src, &sb) != -1);
}

void runner(char *src, FILE *fp)
{

    // Ask the user if they want to rotate
    char *rot_char = request_rot();
    char *thumb = fmt_file(src, THUMB);
    char *final = fmt_file(src, FINAL);

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
}

int main(int argc, char *argv[])
{
    // Tracker for arguments
    int i;
    // Option to display or not - display doesn't work if ssh on Thayer Machines
    int display;
    // The number of images we actually have
    int argc_true;

    // pipe to keep track of files that are ready
    int main_pipe[2];

    // Helpful tracker for making sure we've processed all of the images
    int sum = 0;

    // Setup our pipe
    if (pipe(main_pipe) == -1)
    {
        exit(201);
    }

    if (argc > 1 && strcmp(argv[1], "-d") == 0)
    {
        i = 2;
        display = 1;
        argc_true = argc - 2;
    }
    else
    {
        i = 1;
        display = 0;
        argc_true = argc - 1;
    }

    // The final file we are creating - html formatted
    FILE *fp = html_init();

    // First we loop over all of them and start the thumbnail conversions
    while (i < argc)
    {
        // If we have completed a batch, wait 5 seconds to let the program finish converting the previous batch.
        if (i > 0 && i % BATCH_SIZE == 0)
        {
            sleep(5);
        }

        // Current argument - expected to be a path to a file
        char *src = argv[i];
        int status;

        // Sanity check that is is in fact a file
        if (file_exists(src) == 0)
        {
            perror("Invalid File Name");
            exit(EXIT_FAILURE);
        }

        // New pointers to our thumbnail and final file names
        char *thumb = fmt_file(src, THUMB);

        int initial_fork = fork();
        if (initial_fork == 0)
        {

            /* Child process closes up read side of pipe */
            close(main_pipe[RPIPE]);

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
            // We've finished creating our thumbnail, so lets push this image number to the pipe!
            char str[PIPE_ELEMENT_SIZE];
            sprintf(str, "%d", i);
            write(main_pipe[WPIPE], str, PIPE_ELEMENT_SIZE);
            exit(0);
        }
        i++;
    }

    int expected_sum = (argc_true * (argc_true + 1) / 2);
    while (sum < expected_sum)
    {
        /* Parent process closes up write side of pipe */
        close(main_pipe[WPIPE]);

        char ready[PIPE_ELEMENT_SIZE];
        // Read in an image that is ready from the pipe!
        int nbytes = read(main_pipe[RPIPE], &ready, PIPE_ELEMENT_SIZE);

        int ready_int = atoi(ready);
        char *src = argv[ready_int];

        // Run the user-based requests for this image
        runner(src, fp);

        // Increase our sum
        sum = sum + ready_int;
    }

    // Cleanup code
    html_close(fp);

    return 0;
}