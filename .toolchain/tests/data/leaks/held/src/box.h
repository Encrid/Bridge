/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BOX_H
    #define BOX_H

typedef struct box box_t;

box_t *box_open(int size);
int box_close(box_t *box);
int box_count(void);
int box_number(int base, box_t *box);
char *box_format(const char *text, ...);

#endif
