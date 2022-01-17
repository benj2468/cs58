#include <stdio.h>


// Initialize the HTML file with the necessary headers
FILE *html_init()
{
    FILE *fp;
    fp = fopen("./index.html", "w");

    char html[] = "<html><title>Programming Project 1</title>"
                  "<h1>Benjamin's Submission</h1>"
                  "Please click on a thumbnail to view a medium - size image";
    fputs(html, fp);

    return fp;
}


// Finishing touches on HTML file
int html_close(FILE *fp)
{
    fputs("</html>", fp);

    return fclose(fp);
}
