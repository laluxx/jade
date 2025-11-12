#ifndef ELINE_H
#define ELINE_H

#include <stddef.h>
#include <stdbool.h>
#include "keymap.h"
#include "killring.h"

typedef struct {
    size_t mark;
    size_t end;
    bool active;
} Region;

void yank(Line *line);
void kill_line(Line *line);

typedef struct Line {
    const char *prompt;
    char *buffer;
    size_t len;
    size_t point;
    size_t cap;
    Region region;
    KeyMap keymap;
    int arg;
    KeySequence last_key; // TODO Option to print it
    KillRing kr;
} Line;


extern bool mark_word_navigation;
extern bool show_digit_argument;
extern bool mark_yank;
extern bool electric_pair_mode;
extern bool electric_pair_mode_brackets;
extern bool show_last_key;


void line_init(Line *line);
void line_free(Line *line);
bool should_insert_pair();
char get_closing_pair(char c);
void insert(Line *line, char c);
bool should_delete_pair(Line *line);
void delete_backward_char(Line *line);
void delete_char(Line *line);
void backward_char(Line *line);
void forward_char(Line *line);
void move_beginning_of_line(Line *line);
void move_end_of_line(Line *line);
void set_mark(Line *line);
void kill_region(Line *line);
void clear_line(Line *line);
bool line_read(Line *line, const char *prompt);
void line_refresh(Line *line, const char *prompt);

bool isWordChar(char c);
bool isPunctuationChar(char c);
size_t beginning_of_word(Line *line, size_t pos);
size_t end_of_word(Line *line, size_t pos);
void forward_word(Line *line);
void backward_word(Line *line);

void digit_argument(Line *line);
void open_line(Line *line);
void keyboard_quit(Line *line);

void kill_word(Line *line);

#endif // ELINE_H
