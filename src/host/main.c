/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "host.h"

static int say_usage(void)
{
    printf("usage: bridge-host <config.toml>\n"
        "       bridge-host --show-config <config.toml>\n");
    return (2);
}

static bool take_config(host_t *h, const char *name)
{
    bool taken = config_read(&h->config, name);

    taken = taken && !config_check(&h->config);
    if (!taken)
        return (false);
    taken = config_world(h);
    taken = config_backends(h) && taken;
    return (config_content(h) && taken);
}

int main(int argc, char **argv)
{
    static host_t h;
    bool show = argc == 3 && !strcmp(argv[1], "--show-config");
    char *named = argc == 2 || show ? argv[argc - 1] : nullptr;
    const char *name = nullptr;
    bool taken = false;

    setvbuf(stdout, nullptr, _IONBF, 0);
    if (!named || named[0] == '-')
        return (say_usage());
    name = config_go_beside(named);
    if (!name)
        printf("cannot go into '%s'\n", named);
    taken = name && take_config(&h, name);
    if (show)
        return (show_config(&h, taken));
    taken = taken && take_lock(name);
    return (taken ? run(&h) : 1);
}
