
char *THUMBNAIM_PREFIX = "t_";
char *FINAL_PREFIX = "f_";

enum PREFIX
{
    THUMB = 't',
    FINAL = 'f'
};

char *fmt_file_partial(char *src, enum PREFIX pref)
{
    char *src_2 = strdup(src);

    char *file = basename(src_2);

    char *dest = (char *)malloc(sizeof(src) + 2 * sizeof(char));
    sprintf(dest, "%c_%s", pref, file);

    return dest;
}

// Format a source file name with a given prefix
char *fmt_file(char *src, enum PREFIX pref)
{
    char *partial = fmt_file_partial(src, pref);

    char *dest = (char *)malloc(sizeof(partial) + 2 * sizeof(char));
    sprintf(dest, "./%s", partial);

    return dest;
}

int files_overlap(char *a, char *b)
{
    char *src_b = strdup(b);
    char *src_a = strdup(a);
    char *src_a_2 = strdup(a);

    char *dirname_b = dirname(src_b);
    char *filename = basename(src_a);
    char *dirname_a = dirname(src_a_2);

    if (strcmp(a, b) == 0)
    {
        return 1;
    }
    else if (strcmp(dirname_a, dirname_b) == 0)
    {
        return strcmp(filename, fmt_file_partial(b, THUMB)) == 0 || strcmp(filename, fmt_file_partial(b, FINAL)) == 0;
    }
}