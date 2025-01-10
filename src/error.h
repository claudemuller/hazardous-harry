#ifndef ERROR_H
#define ERROR_H

enum {
    SUCCESS,
    ERR_ALLOC,
    ERR_OPENING_FILE,
    ERR_SDL_INIT,
    ERR_SDL_CREATE_WIN_RENDER,
};

extern char err_additional[256];
extern const char *err_messages[];

void err_handle(const int);
int err_fatal(const int, const char *);

#endif // !ERROR_H
