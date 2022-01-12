#include <stdio.h>
#include "./file_mng.c"

// Initialize the HTML file with the necessary headers
FILE *html_init()
{
    FILE *fp;
    fp = fopen("./index.html", "w");

    char html[] = "<html><title>Programming Project 1</title>"
                  "<h1>Benjamin's Submission</h1>"
                  "Please click on a thumbnail to view a medium - size image";
    fputs(html, fp);
    fflush(fp);

    return fp;
}

// Add a line of data from a given file to the HTML page
void html_add_line(FILE *fp, char *src, char *cap)
{
    char html_addition[100];

    char *thumb = fmt_file(src, THUMB);
    char *final = fmt_file(src, FINAL);

    sprintf(html_addition, "<h2>%s</h2><a href=\"%s\"><img src=\"%s\" border=\"1\" /></a>", cap, final, thumb);

    fputs(html_addition, fp);
    fflush(fp);
}

// Finishing touches on HTML file
int html_close(FILE *fp)
{
    fputs("</html>", fp);
    fflush(fp);

    return fclose(fp);
}