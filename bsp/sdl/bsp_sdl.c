/*****************************************************************************
 * Copyright (C) 2026 Sunny Matato
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar.
 * See http://www.wtfpl.net/ for more details.
 ****************************************************************************/
/*==========================================================================*/
/* ...Redefine main() on some platforms so that it is called by SDL. */
#define SDL_MAIN_HANDLED

/*==========================================================================*/
#include "qpc.h"
#include "bsp.h"
#include "application.h"

/*==========================================================================*/
/* ...SDL. */
#define SDL_WINDOW_MAX_WIDTH  2048
#define SDL_WINDOW_MAX_HEIGHT 1266
static SDL_Window *l_window   = NULL;
static SDL_Renderer *l_renderer = NULL;
static int l_running  = 1;

/*==========================================================================*/
/* ...QF_run Thread. */
static SDL_Thread *l_qp_worker = NULL;
static volatile bool l_gui_ready = false;

/*==========================================================================*/
/* ...Render mutex: guards LVGL + SDL render calls shared between the main
 * loop and the window event watcher (which runs on the OS event thread). */
static SDL_mutex *l_render_mutex = NULL;

/*==========================================================================*/
static void SDL_WndProc(SDL_Event *e);

/*==========================================================================*/
/* ...Window event watcher (runs on the OS event thread).
 * On Windows, dragging/resizing triggers a modal loop that blocks
 * SDL_WaitEventTimeout in the main thread. This watcher fires inside that
 * modal loop, drains the event queue, advances LVGL, and renders — keeping
 * the display live while the window is being moved or resized. */
static int SDL_wdEvtWatcher(void *data, SDL_Event *e) {
    (void)data;
    if (e->type != SDL_WINDOWEVENT) return 0;
    if (e->window.event != SDL_WINDOWEVENT_MOVED
            && e->window.event != SDL_WINDOWEVENT_RESIZED
            && e->window.event != SDL_WINDOWEVENT_SIZE_CHANGED) return 0;
    if (!l_render_mutex || !l_renderer || !l_gui_ready) return 0;

    SDL_LockMutex(l_render_mutex);

    SDL_Event pending;
    while (SDL_PollEvent(&pending)) {
        SDL_WndProc(&pending);
    }

    uint32_t now = SDL_GetTicks();
    static uint32_t s_last = 0;
    if (s_last == 0) s_last = now;
    uint32_t diff = now - s_last;
    if (diff > 0) {
        BSP_lvgl_task(diff);
        s_last = now;
    }

    SDL_SetRenderDrawColor(l_renderer, 0, 0, 0, 255);
    SDL_RenderClear(l_renderer);
    BSP_lvgl_render();
    SDL_RenderPresent(l_renderer);

    SDL_UnlockMutex(l_render_mutex);
    return 0;
}

/*==========================================================================*/
/* ...QF_run thread. */
extern int main_gui(void);  /* QF_run main */
static int SDLCALL appThread(void *par);  /* QF_run thread loop. */
/*..........................................................................*/
/* ...QF_run thread creating trigger. */
static void SDL_createAppThread(void) {
    l_qp_worker = SDL_CreateThread(appThread, "QP_Worker", NULL);
    l_gui_ready = true;
}
/*..........................................................................*/
static int SDLCALL appThread(void *par) {
    (void)par;
    return main_gui();
}

/*==========================================================================*/
static void SDL_WndProc(SDL_Event *e) {
    switch (e->type) {
        /* Window closed */
        case SDL_QUIT: {
            l_running = 0;
            QF_stop();
            break;
        }

        /* SDL window events */
        case SDL_WINDOWEVENT: {
            switch (e->window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                {
                    if (l_gui_ready) {
                        int new_w = e->window.data1;
                        int new_h = e->window.data2;
                        BSP_lvgl_resize(new_w, new_h);
                    }
                    break;
                }
                default: {
                    break;
                }
            }
            break;
        }

        /* ...Mouse actions. */
        case SDL_MOUSEMOTION: {
            BSP_lvgl_feed_mouse(
                            e->motion.x, e->motion.y,
                            (e->motion.state & SDL_BUTTON_LMASK));
            break;
        }
        case SDL_MOUSEBUTTONDOWN: {
            if (e->button.button == SDL_BUTTON_LEFT) {
                BSP_lvgl_feed_mouse(e->button.x, e->button.y, true);
            }
            break;
        }
        case SDL_MOUSEBUTTONUP: {
            if (e->button.button == SDL_BUTTON_LEFT) {
                BSP_lvgl_feed_mouse(e->button.x, e->button.y, false);
            }
            break;
        }

        /* System Keyboard: "Ctrl + c". */
        case SDL_KEYDOWN: {
            if (
                (e->key.keysym.sym == SDLK_c)
                    && (e->key.keysym.mod & KMOD_CTRL)
            ) {
                /* Dummy... */
            }
            break;
        }

        /* LVGL... */
        case DISP_UPDATE_CLOSE_STATE_SIG: {
            GUI_updateSerialCnnState(DISPLAY_STATE_CLOSED);
            break;
        }
        case DISP_UPDATE_OPENING_STATE_SIG: {
            GUI_updateSerialCnnState(DISPLAY_STATE_OPENING);
            break;
        }
        case DISP_UPDATE_OPEN_STATE_SIG: {
            GUI_updateSerialCnnState(DISPLAY_STATE_OPENED);
            break;
        }
        case DISP_UPDATE_CLOSING_STATE_SIG: {
            GUI_updateSerialCnnState(DISPLAY_STATE_CLOSING);
            break;
        }

        case DISP_UPDATE_COM_DROPDOWN_BOX_INFO_SIG: {
            GUI_updateComLists((char *)e->user.data1);
            break;
        }

        case DISP_UPDATE_LOADED_JSON_FILE_SIG: {
            GUI_updateLoadedJsonFile((char *)e->user.data1);
            break;
        }

        case DISP_UPDATE_RECV_BOX_MSG_SIG: {
            GUI_updateRecvBoxMsg((char *)(e->user.data1));
            break;
        }

        default: {
            break;
        }
    }
}

/*==========================================================================*/
/* ===SDL main. */
int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    SDL_Event msg;

    /* SDL HardWare initialization */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return -1;

    l_render_mutex = SDL_CreateMutex();
    if (!l_render_mutex) return -1;

    /* SDL Window. */
    l_window = SDL_CreateWindow(
                    "SM_Tracer",
                    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                    LVGL_WIND_WIDTH, LVGL_WIND_HEIGHT,
                    SDL_WINDOW_SHOWN);

    SDL_SetWindowMaximumSize(
        l_window, SDL_WINDOW_MAX_WIDTH, SDL_WINDOW_MAX_HEIGHT);

    /* SDL Render */
    l_renderer = SDL_CreateRenderer(
                    l_window,
                    -1,
                    SDL_RENDERER_ACCELERATED);
    /* Make sure the render is initialized correctly */
    if (!l_renderer) return -1;

    /* LVGL... */
    BSP_lvgl_cfg_t lvgl_cfg = {
        .renderer = (void*)l_renderer,
        .screen_width  = LVGL_WIND_WIDTH,
        .screen_height = LVGL_WIND_HEIGHT
    };
    BSP_lvgl_init(&lvgl_cfg);
    GUI_init();

    SDL_AddEventWatch(SDL_wdEvtWatcher, NULL);

    /* Spwam app thread... */
    SDL_createAppThread();



    /* Super loop... */
    uint32_t last_tick = SDL_GetTicks();
    while (l_running) {
        SDL_LockMutex(l_render_mutex);

        uint32_t current_tick = SDL_GetTicks();
        uint32_t diff = current_tick - last_tick;
        if (diff > 0) {
            BSP_lvgl_task(diff);
            last_tick = current_tick;
        }

        if (SDL_WaitEventTimeout(&msg, 5)) SDL_WndProc(&msg);

        SDL_SetRenderDrawColor(l_renderer, 0, 0, 0, 255);
        SDL_RenderClear(l_renderer);
        BSP_lvgl_render();
        SDL_RenderPresent(l_renderer);

        SDL_UnlockMutex(l_render_mutex);
    }

    /* Spawn-thread clean ... */
    if (l_qp_worker) SDL_WaitThread(l_qp_worker, NULL);

    /* SDL thread clean ... */
    SDL_DestroyRenderer(l_renderer);
    SDL_DestroyWindow(l_window);
    SDL_DestroyMutex(l_render_mutex);
    SDL_Quit();

    return 0;
}

/*==========================================================================*/
void BSP_deliverClipboard(const char *text) {
    if (text) SDL_SetClipboardText(text);
}
