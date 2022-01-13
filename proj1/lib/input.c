
// Request a rotation (r/l) from stdin
char *request_rot(char *src)
{

    printf("[%s] Rotate the Image by 90 degrees? (r/l/enter = none): ", src);
    char rot;
    scanf("%c", &rot);

    // If the character is NOT the escape character, then remove a character (the escape character from the buffer)
    if (rot != '\n')
    {
        getchar();
    }

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

    // Convert the integer into a string
    char *rot_char = (char *)malloc(sizeof(char) * 3);
    sprintf(rot_char, "%d", rotation);

    return rot_char;
}

// Request a caption from stdin
char *request_caption(char *src)
{
    char *cap = (char *)malloc(sizeof(char) * 20);
    printf("[%s] Provide a caption for the image: ", src);
    scanf("%20[^\n]%*c", cap);

    return cap;
}
