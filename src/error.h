#ifndef ERROR_H
#define ERROR_H

enum {
    SUCCESS,
    ERR_ALLOC,
    ERR_OPENING_FILE,
    ERR_SDL_INIT,
    ERR_SDL_CREATE_WIN_RENDER,
};

void err_handle(int);

#endif // !ERROR_H
