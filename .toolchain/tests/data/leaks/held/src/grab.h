/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef GRAB_H
    #define GRAB_H

typedef struct note note_t;
typedef struct part part_t;

typedef struct ticket {
    unsigned id;
} ticket_t;

typedef struct text {
    unsigned long size;
    char *data;
} text_t;

char *grab(void);
char *seize(void);
note_t *note_new(void);
void note_delete(note_t *note);
note_t *part_new(int ok, part_t **out);
void part_delete(part_t *part);
ticket_t ticket_make(int ok);
void ticket_drop(ticket_t ticket);
void text_fill(text_t *text);
void text_delete(text_t *text);
void text_join(text_t *one, text_t *two);
int note_hand(note_t *(*make)(int ok), int ok);
note_t *pool_refuse(int ok);
int kinds(const char *mode);

#endif
