int main(void)
{
    const char *p[] = {MOD_PATH, TWO_PATH, THREE_PATH};

    return (int)(sizeof(p) / sizeof(p[0])) - 3;
}
