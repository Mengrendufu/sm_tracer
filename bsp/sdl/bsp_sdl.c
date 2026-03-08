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
#define SDL_MAIN_HANDLED

/*==========================================================================*/
#include "qpc.h"
#include "bsp.h"
#include "application.h"

#ifdef _WIN32
#include "SDL_syswm.h"
#include <timeapi.h>   /* timeBeginPeriod / timeEndPeriod */
#endif

/*==========================================================================*/
static SDL_Window   *l_window   = NULL;
static SDL_Renderer *l_renderer = NULL;
static int           l_running  = 1;

static SDL_Thread *l_qp_worker = NULL;

#define RENDER_INTERVAL_MS 16

/*==========================================================================*/
static void SDL_WndProc(SDL_Event *e);
static bool do_render_frame(void);

#ifdef _WIN32
#define MODAL_TIMER_ID 1
#define MODAL_TIMER_MS 16

static WNDPROC l_orig_wndproc = NULL;

static LRESULT CALLBACK win32_subclass_proc(HWND hwnd, UINT msg,
                                            WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_NCLBUTTONDOWN:
        case WM_ENTERSIZEMOVE:
            SetTimer(hwnd, MODAL_TIMER_ID, MODAL_TIMER_MS, NULL);
            break;
        case WM_NCLBUTTONUP:
        case WM_EXITSIZEMOVE:
            KillTimer(hwnd, MODAL_TIMER_ID);
            break;
        case WM_SIZING:
        case WM_MOVING: {
            /* Do NOT call BSP_lvgl_resize / BSP_msgbuf_resize here.
             * The RECT from WM_SIZING is the outer window frame (includes
             * title bar + borders), not the SDL client area.  Passing those
             * inflated dimensions corrupts the panel layout and hides the
             * scrollbar.  SDL_WINDOWEVENT_SIZE_CHANGED fires after the resize
             * completes and carries the correct client dimensions — that
             * handler owns the resize.  We only need to keep rendering so the
             * window looks live during the drag. */
            do_render_frame();
            break;
        }
        case WM_TIMER:
            if (wp == MODAL_TIMER_ID) {
                SDL_Event e;
                while (SDL_PollEvent(&e)) {
                    SDL_WndProc(&e);
                }
                do_render_frame();
            }
            break;
        default: break;
    }
    return CallWindowProcW(l_orig_wndproc, hwnd, msg, wp, lp);
}

static void win32_install_subclass(SDL_Window *window) {
    SDL_SysWMinfo wmi;
    SDL_VERSION(&wmi.version);
    if (!SDL_GetWindowWMInfo(window, &wmi)) return;
    if (wmi.subsystem != SDL_SYSWM_WINDOWS) return;
    HWND hwnd = wmi.info.win.window;
    l_orig_wndproc = (WNDPROC)(LONG_PTR)GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)win32_subclass_proc);
}
#endif /* _WIN32 */

/*==========================================================================*/
static bool do_render_frame(void) {
    static uint32_t last_render = 0;
    static uint32_t last_tick   = 0;

    /* Diagnostics: count rendered vs skipped frames per second. */
    static uint32_t diag_last     = 0;
    static uint32_t diag_rendered = 0;
    static uint32_t diag_skipped  = 0;
    static uint64_t diag_lvgl_us  = 0;

    uint32_t now = SDL_GetTicks();

    if (last_tick == 0) {
        last_tick   = now;
        last_render = now;
        diag_last   = now;
    }

    if (diag_last == 0) diag_last = now;
    if (now - diag_last >= 1000) {
        fprintf(stderr, "[RENDER] rendered=%u skipped=%u lvgl_us=%llu per sec\n",
                diag_rendered, diag_skipped, (unsigned long long)diag_lvgl_us);
        diag_rendered = 0;
        diag_skipped  = 0;
        diag_lvgl_us  = 0;
        diag_last     = now;
    }

    if (now - last_render < RENDER_INTERVAL_MS) return false;

    uint32_t diff = now - last_tick;
    if (diff > 0) {
        uint64_t t0 = SDL_GetPerformanceCounter();
        BSP_lvgl_task(diff);
        uint64_t freq = SDL_GetPerformanceFrequency();
        diag_lvgl_us += (SDL_GetPerformanceCounter() - t0) * 1000000ULL / freq;
        last_tick = now;
    }
    last_render = now;

    bool lvgl_dirty   = BSP_lvgl_is_dirty();
    bool msgbuf_dirty = BSP_msgbuf_is_dirty();

    if (!lvgl_dirty && !msgbuf_dirty) {
        diag_skipped++;
        return false;
    }

    SDL_SetRenderDrawColor(l_renderer, 0, 0, 0, 255);
    SDL_RenderClear(l_renderer);
    BSP_lvgl_render();
    BSP_msgbuf_render();
    SDL_RenderPresent(l_renderer);
    diag_rendered++;
    return true;
}

/*==========================================================================*/
extern int main_gui(void);
static int SDLCALL appThread(void *par) {
    (void)par;
    return main_gui();
}

/*==========================================================================*/
static void SDL_WndProc(SDL_Event *e) {
    switch (e->type) {
        case SDL_QUIT: {
            l_running = 0;
            QF_stop();
            break;
        }

        case SDL_WINDOWEVENT: {
            switch (e->window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                case SDL_WINDOWEVENT_SIZE_CHANGED: {
                    BSP_lvgl_resize(e->window.data1, e->window.data2);
                    BSP_msgbuf_resize(e->window.data1, e->window.data2);
                    break;
                }
                default: break;
            }
            break;
        }

        case SDL_MOUSEMOTION: {
            BSP_lvgl_feed_mouse(
                e->motion.x, e->motion.y,
                (e->motion.state & SDL_BUTTON_LMASK));
            BSP_msgbuf_mouse_motion(
                e->motion.x, e->motion.y,
                (e->motion.state & SDL_BUTTON_LMASK));
            break;
        }
        case SDL_MOUSEBUTTONDOWN: {
            if (e->button.button == SDL_BUTTON_LEFT) {
                BSP_lvgl_feed_mouse(e->button.x, e->button.y, true);
                BSP_msgbuf_mouse_button(e->button.x, e->button.y, true);
            }
            break;
        }
        case SDL_MOUSEBUTTONUP: {
            if (e->button.button == SDL_BUTTON_LEFT) {
                BSP_lvgl_feed_mouse(e->button.x, e->button.y, false);
                BSP_msgbuf_mouse_button(e->button.x, e->button.y, false);
            }
            break;
        }

        case SDL_MOUSEWHEEL: {
            BSP_msgbuf_mouse_wheel(e->wheel.y);
            break;
        }

        case SDL_KEYDOWN: {
            if ((e->key.keysym.sym == SDLK_c)
                    && (e->key.keysym.mod & KMOD_CTRL)) {
                BSP_msgbuf_copy_selection();
            }
            break;
        }

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

        default: break;
    }
}

/*==========================================================================*/
int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) return -1;

#ifdef _WIN32
    timeBeginPeriod(1);
#endif

#ifndef _WIN32
    /* On Linux/macOS, prefer OpenGL for reliable driver support.
     * On Windows, let SDL pick D3D11 so PRESENTVSYNC uses the DXGI flip
     * model (genuinely blocking) instead of OpenGL wglSwapBuffers which
     * can busy-wait depending on the GPU driver. */
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
#endif

    l_window = SDL_CreateWindow(
                    "SM_Tracer",
                    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                    LVGL_WIND_WIDTH, LVGL_WIND_HEIGHT + MSGBUF_PANEL_HEIGHT,
#ifdef _WIN32
                    /* On Windows, do NOT force SDL_WINDOW_OPENGL — let SDL pick
                     * D3D11 so PRESENTVSYNC uses the DXGI flip model (genuinely
                     * blocking) instead of wglSwapBuffers which busy-waits on
                     * many GPU drivers and causes ~18-25% idle CPU. */
                    SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
#else
                    SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
#endif
                    );
    if (!l_window) return -1;

    l_renderer = SDL_CreateRenderer(l_window, -1,
                                    SDL_RENDERER_ACCELERATED |
                                    SDL_RENDERER_PRESENTVSYNC);
    if (!l_renderer) return -1;

    {
        SDL_RendererInfo info;
        if (SDL_GetRendererInfo(l_renderer, &info) == 0) {
            fprintf(stderr, "[RENDERER] name=%s flags=0x%x\n",
                    info.name,
                    (unsigned)info.flags);
        }
    }

    int win_w, win_h;
    SDL_GetWindowSize(l_window, &win_w, &win_h);

    BSP_lvgl_cfg_t lvgl_cfg = {
        .renderer      = (void *)l_renderer,
        .screen_width  = win_w,
        .screen_height = LVGL_WIND_HEIGHT
    };
    BSP_lvgl_init(&lvgl_cfg);
    BSP_msgbuf_init(l_renderer, LVGL_WIND_HEIGHT, win_w, MSGBUF_PANEL_HEIGHT);

    GUI_init();

#ifdef _WIN32
    win32_install_subclass(l_window);
#endif

    l_qp_worker = SDL_CreateThread(appThread, "QP_Worker", NULL);

    while (l_running) {
        uint32_t frame_start = SDL_GetTicks();

        SDL_Event msg;
        while (SDL_PollEvent(&msg)) {
            SDL_WndProc(&msg);
        }

        bool rendered = do_render_frame();

        if (!rendered) {
            uint32_t elapsed = SDL_GetTicks() - frame_start;
            if (elapsed < RENDER_INTERVAL_MS) {
                SDL_Delay(RENDER_INTERVAL_MS - elapsed);
            }
        }
    }

    if (l_qp_worker) SDL_WaitThread(l_qp_worker, NULL);

    SDL_DestroyRenderer(l_renderer);
    SDL_DestroyWindow(l_window);

#ifdef _WIN32
    timeEndPeriod(1);
#endif
    SDL_Quit();

    return 0;
}

/*==========================================================================*/
void BSP_deliverClipboard(const char *text) {
    if (text) SDL_SetClipboardText(text);
}
