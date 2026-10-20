/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

void test_hop_threads(host_t *h)
{
    h->cfg.threads = 4;
    if (host_open(h))
        return;
    test_load_wasm(h, "game:mover@0.1.0", "mover");
    test_load_game_reader(h);
    host_run_frames_unsaid(h->w, 50);
    host_close(h);
}
