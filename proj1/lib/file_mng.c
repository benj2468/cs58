
char *THUMBNAIM_PREFIX = "t_";
char *FINAL_PREFIX = "f_";

enum PREFIX
{
    THUMB = 't',
    FINAL = 'f'
};

char *fmt_file(char *src, enum PREFIX pref)
{
    char *src_2 = strdup(src);

    char *file = basename(src_2);

    char *dest = (char *)malloc(sizeof(src) + 2 * sizeof(char));
    sprintf(dest, "./%c_%s", pref, file);

    return dest;
}