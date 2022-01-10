#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <sys/wait.h> // because man waitpid() said we'd need this

char *THUMBNAIM_PREFIX = "thumb_";
char *FINAL_PREFIX = "final_";

char *fmt_file(char *src, char *add)
{
    char *src_2 = strdup(src);

    char *file = basename(src_2);

    char *dest = (char *)malloc(sizeof(src) + 2 * sizeof(char));
    sprintf(dest, "./%s%s", add, file);

    return dest;
}

int main(int argc, char *argv[])
{
    int i = 1;

    FILE *fp;
    fp = fopen("./index.html", "w");

    char html[] = "<html><title>Programming Project 1</title>"
                  "<h1>Benjamin's Submission</h1>"
                  "Please click on a thumbnail to view a medium - size image";

    fputs(html, fp);

    while (i < argc)
    {
        char *src = argv[i];
        int status;

        int rc = fork();
        if (rc == 0)
        {
            // First resize the image
            execlp("convert", "convert", "-resize", "10\%", src, fmt_file(src, THUMBNAIM_PREFIX), NULL);
        }

        waitpid(rc, &status, 0);
        // printf("back from waitpid. rc = %d and status = %d\n", rc, status);

        // // rc = fork();
        // // if (rc == 0)
        // // {
        // //     // Next display the image
        // //     rc = execlp("display", src, NULL);
        // // }

        // Ask the user if they want to rotate
        char rot;
        printf("Rotate the Image by 90 degrees? (r/l/enter = none): ");
        scanf("%c", &rot);
        getchar();

        char right = 'r';
        char left = 'l';

        int rotation;
        if ((int)rot == (int)right)
        {
            rotation = 90;
        }
        else if ((int)rot == (int)left)
        {
            rotation = -90;
        }
        else
        {
            rotation = 0;
        }

        rot = 0;

        char *rot_char = (char *)malloc(sizeof(char) * 3);
        sprintf(rot_char, "%d", rotation);

        char *thumb = fmt_file(src, THUMBNAIM_PREFIX);
        char *final = fmt_file(src, FINAL_PREFIX);

        int rotate_fork = fork();
        if (rotate_fork == 0)
        {
            execlp("convert", "convert", "-rotate", rot_char, thumb, thumb, NULL);
        }
        waitpid(rotate_fork, &status, 0);

        // Ask the user for a caption
        char *cap;
        printf("Provide a caption for the image: ");
        scanf("%20s", cap);

        int final_fork = fork();
        if (final_fork == 0)
        {
            execlp("convert", "convert", "-resize", "25\%", "-rotate", rot_char, src, final, NULL);
        }
        waitpid(final_fork, &status, 0);

        char html_addition[100];

        sprintf(html_addition, "<h2>%s</h2><a href=\"%s\"><img src=\"%s\" border=\"1\" /></a> ", cap, thumb, final);

        fputs(html_addition, fp);

        i++;
    };

    fputs("</html>", fp);
    fclose(fp);

    return 0;
}