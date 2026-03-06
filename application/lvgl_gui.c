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
#include "qpc.h"
#include "bsp.h"
#include "application.h"

/*==========================================================================*/
SM_DEFINE_MODULE("lvgl_gui")

/*==========================================================================*/
/* ===hack_nerd_font_14. */
LV_FONT_DECLARE(hack_nerd_font_regular);

/*==========================================================================*/
/* LVGL main screen creating handler. */
static void GUI_createMainScreen(void);

/*==========================================================================*/
/* ...GUI main screen's LVGL objects. */
/* Toolbox at first panel... */
static lv_obj_t *GUI_Btn_cnnct      = NULL;
static lv_obj_t *GUI_Led_status     = NULL;
static lv_obj_t *GUI_DdBox_comList  = NULL;
static lv_obj_t *GUI_Btn_comRefresh = NULL;
static lv_obj_t *GUI_DdBox_baudrate = NULL;
static lv_obj_t *GUI_DdBox_dataBits = NULL;
static lv_obj_t *GUI_DdBox_stopBits = NULL;
static lv_obj_t *GUI_DdBox_parity   = NULL;
static lv_obj_t *GUI_DdBox_flowCtrl = NULL;
/*..........................................................................*/
/* ...JSON file directory path. */
#define MAX_CFG_PATH_DEPTH 3
static char *GUI_jsonCfgDirOptions[MAX_CFG_PATH_DEPTH] = {
    "./cfg",
    "../cfg",
    "../../cfg"
};
static char *currCfgPath = NULL;  /* Store found path of cfg dir. */
/*..........................................................................*/
/* ...JSON file status display. */
#define GUI_MAX_LENGTH_JSON_FILE_NAME 128
static char GUI_currLoadedJsonFileName[GUI_MAX_LENGTH_JSON_FILE_NAME];
static lv_obj_t *GUI_Label_currJsonFile = NULL;
/*..........................................................................*/
/* ...JSON toolbox. */
/* JSON file dropdown box. */
static lv_obj_t *GUI_DdBox_jsonFiles = NULL;
/* JSON file dropdown box list refresh. */
static lv_obj_t *GUI_Btn_jsonFileRefresh = NULL;
/* Load current selected JSON file. */
static lv_obj_t *GUI_Btn_jsonLoad = NULL;

/*==========================================================================*/
/* ...Event callback of LVGL objects. */
/* GUI_Btn_cnnct. */
static void GUIEvtCb_Btn_cnnct(lv_event_t *e) {
    (void)e;
    GuiMngrEvt *evt = Q_NEW(GuiMngrEvt, GUI_CLICK_OPEN_CLOSE_BUTTON_SIG);
    QACTIVE_POST(AO_GuiMngr, (QEvt*)evt, (void *)0);
}
/*..........................................................................*/
/* GUI_DdBox_comList. */
static void GUIEvtCb_DdBox_comList(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        int mlSize = 64;
        char *comNameStr = malloc(mlSize);
        SM_ENSURE(comNameStr != (char *)0);
        lv_dropdown_get_selected_str(obj, comNameStr, mlSize);

        /* Post to AO_GuiMngr first */
        GuiMngrEvt *evt = Q_NEW(GuiMngrEvt, GUI_UPDATE_COM_SIG);
        evt->txt = comNameStr;
        QACTIVE_POST(AO_GuiMngr, (QEvt*)evt, (void *)0);
    }
}
/*..........................................................................*/
/* GUI_Btn_comRefresh. */
static void GUIEvtCb_Btn_comRefresh(lv_event_t *e) {
    (void)e;
    GuiMngrEvt *evt = Q_NEW(GuiMngrEvt, GUI_CLICK_REFRESH_BUTTON_SIG);
    QACTIVE_POST(AO_GuiMngr, (QEvt*)evt, (void *)0);
}
/*..........................................................................*/
/* GUI_DdBox_baudrate. */
static void GUIEvtCb_DdBox_baudrate(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        int mlSize = 32;
        char *baudrateStr = malloc(mlSize);
        SM_ENSURE(baudrateStr != (char *)0);
        lv_dropdown_get_selected_str(obj, baudrateStr, mlSize);

        /* Post to AO_GuiMngr first */
        GuiMngrEvt *evt = Q_NEW(GuiMngrEvt, GUI_UPDATE_BAUDRATE_SIG);
        evt->txt = baudrateStr;
        QACTIVE_POST(AO_GuiMngr, (QEvt*)evt, (void *)0);
    }
}
/*..........................................................................*/
/* GUI_DdBox_dataBits. */
static void GUIEvtCb_DdBox_dataBits(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        int mlSize = 32;
        char *dataBitsStr = malloc(mlSize);
        SM_ENSURE(dataBitsStr != (char *)0);
        lv_dropdown_get_selected_str(obj, dataBitsStr, mlSize);

        /* Post to AO_GuiMngr first */
        GuiMngrEvt *evt = Q_NEW(GuiMngrEvt, GUI_UPDATE_DATABITS_SIG);
        evt->txt = dataBitsStr;
        QACTIVE_POST(AO_GuiMngr, (QEvt*)evt, (void *)0);
    }
}
/*..........................................................................*/
/* GUI_DdBox_stopBits. */
static void GUIEvtCb_DdBoxStopBits(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        int mlSize = 32;
        char *stopBitsStr = malloc(mlSize);
        SM_ENSURE(stopBitsStr != (char *)0);
        lv_dropdown_get_selected_str(obj, stopBitsStr, mlSize);

        /* Post to AO_GuiMngr first */
        GuiMngrEvt *evt = Q_NEW(GuiMngrEvt, GUI_UPDATE_STOPBITS_SIG);
        evt->txt = stopBitsStr;
        QACTIVE_POST(AO_GuiMngr, (QEvt*)evt, (void *)0);
    }
}
/*..........................................................................*/
/* GUI_DdBox_parity. */
static void GUIEvtCb_DdBox_parity(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        int mlSize = 32;
        char *parityStr = malloc(mlSize);
        SM_ENSURE(parityStr != (char *)0);
        lv_dropdown_get_selected_str(obj, parityStr, mlSize);

        /* Post to AO_GuiMngr first */
        GuiMngrEvt *evt = Q_NEW(GuiMngrEvt, GUI_UPDATE_PARITY_SIG);
        evt->txt = parityStr;
        QACTIVE_POST(AO_GuiMngr, (QEvt*)evt, (void *)0);
    }
}
/*..........................................................................*/
/* GUI_DdBox_flowCtrl. */
static void GUIEvtCb_flowCtrl(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        int mlSize = 32;
        char *flowCtrlStr = malloc(mlSize);
        SM_ENSURE(flowCtrlStr != (char *)0);
        lv_dropdown_get_selected_str(obj, flowCtrlStr, mlSize);

        /* Post to AO_GuiMngr first */
        GuiMngrEvt *evt = Q_NEW(GuiMngrEvt, GUI_UPDATE_FLOWCONTROL_SIG);
        evt->txt = flowCtrlStr;
        QACTIVE_POST(AO_GuiMngr, (QEvt*)evt, (void *)0);
    }
}
/*..........................................................................*/
/* ...GUI_DdBox_jsonFiles. */
static void GUI_postJsonFilePath(void) {
    char currFileName[256];  /* A file name's length exceeds 256 is crazy. */
    char *selectedJsonFile = malloc(256);
    SM_ENSURE(selectedJsonFile != (char *)0);

    lv_dropdown_get_selected_str(GUI_DdBox_jsonFiles, currFileName, 256);

    if (currCfgPath) {
        snprintf(selectedJsonFile, 256, "%s/%s", currCfgPath, currFileName);
    } else {
        snprintf(selectedJsonFile, 256, "%s", currFileName);
    }

    /* Post to AO_GuiMngr first */
    GuiMngrEvt *evt = Q_NEW(
                        GuiMngrEvt, GUI_MNGR_UPDATE_SELECTED_JSON_FILE_SIG);
    evt->txt = selectedJsonFile;
    QACTIVE_POST(AO_GuiMngr, (QEvt*)evt, (void *)0);
}
static void GUIEvtCb_DdBox_jsonFiles(lv_event_t *e) {
    (void)e;
    GUI_postJsonFilePath(); /* Send the selected json file to AO_GuiMngr. */
}
/*..........................................................................*/
/* ...GUI_Btn_jsonFileRefresh. */
static bool GUI_isJsonFile(const char *fileName) {
    const char *dot = strrchr(fileName, '.');
    return (dot && strcmp(dot, ".json") == 0);
}
static void GUIEvtCb_Btn_jsonFileRefresh(lv_event_t *e) {
    (void)e;
    currCfgPath = (char *)0;
    /* ...Find the cfg dir. */
    for (int i = 0; i < MAX_CFG_PATH_DEPTH; ++i) {
        DIR *d;
        if ((d = opendir(GUI_jsonCfgDirOptions[i])) != NULL) {
            currCfgPath = GUI_jsonCfgDirOptions[i];
            closedir(d);
            break;
        }
    }
    if (currCfgPath) { /* Valid ? */
        /* List all the .json files */
        DIR *dirCfg = opendir(currCfgPath);
        SM_ENSURE(dirCfg != (DIR *)0);

        /* ...String of json files. */
        size_t capacity = 256;
        size_t used_len = 0;
        char *list_buf = (char *)malloc(capacity);
        SM_ENSURE(list_buf != (char *)0);
        list_buf[0] = '\0';

        char current_selection[128] = {0};
        lv_dropdown_get_selected_str(
            GUI_DdBox_jsonFiles,
            current_selection, sizeof(current_selection));
        int preserved_index = -1;

        /* ...Find .json files. */
        struct dirent *dir;
        int count = 0;
        while ((dir = readdir(dirCfg)) != NULL) {
            if (dir->d_name[0] == '.') continue;
            if (!GUI_isJsonFile(dir->d_name)) continue;

            if (strcmp(dir->d_name, current_selection) == 0) {
                preserved_index = count;
            }

            size_t name_len = strlen(dir->d_name);
            size_t needed = used_len + name_len + 2;
            if (needed >= capacity) {
                size_t new_cap = capacity * 2;
                if (new_cap < needed) new_cap = needed + 32;
                char *new_ptr = (char *)realloc(list_buf, new_cap);
                SM_ENSURE(new_ptr != (char *)0);
                list_buf = new_ptr;
                capacity = new_cap;
            }
            if (count > 0) {
                strcat(list_buf, "\n");
                ++used_len;
            }
            strcat(list_buf, dir->d_name);
            used_len += name_len;
            ++count;
        }
        closedir(dirCfg);
        if (count > 0) {
            lv_dropdown_set_options(GUI_DdBox_jsonFiles, list_buf);
            if (preserved_index >= 0) {
                lv_dropdown_set_selected(
                                        GUI_DdBox_jsonFiles, preserved_index);
            } else {
                lv_dropdown_set_selected(GUI_DdBox_jsonFiles, 0);
            }
        } else {  /* No json file found */
            lv_dropdown_set_options(GUI_DdBox_jsonFiles, "Empty");
        }
        GUI_postJsonFilePath();
        free(list_buf);
    }
}
/*..........................................................................*/
/* GUI_Btn_jsonLoad. */
static void GUIEvtCb_Btn_jsonLoad(lv_event_t *e) {
    (void)e;
    GuiMngrEvt *evt = Q_NEW(GuiMngrEvt, GUI_MNGR_LOAD_SELECTED_JSON_FILE_SIG);
    QACTIVE_POST(AO_GuiMngr, (QEvt*)evt, (void *)0);
}
/*..........................................................................*/
/* GUI_Btn_msgBoxClean. */
static void GUIEvtCb_Btn_msgBoxClean(lv_event_t *e) {
    (void)e;
    GUI_msgBoxClean();
}
/*..........................................................................*/
/* GUI_DdBox_dispBuf. */
// static void GUIEvtCb_DdBox_dispBuf(lv_event_t *e) {
//     lv_obj_t *obj = lv_event_get_target(e);
//     char slctBufSize[8];
//     int32_t newBufSize;
//     lv_dropdown_get_selected_str(obj, slctBufSize, 8);
//     if (strcmp(slctBufSize, "10K") == 0) {
//         newBufSize = 1024 * 10;
//     } else if (strcmp(slctBufSize, "20K") == 0) {
//         newBufSize = 1024 * 20;
//     } else if (strcmp(slctBufSize, "40K") == 0) {
//         newBufSize = 1024 * 40;
//     } else if (strcmp(slctBufSize, "80K") == 0) {
//         newBufSize = 1024 * 80;
//     } else if (strcmp(slctBufSize, "100K") == 0) {
//         newBufSize = 1024 * 100;
//     } else {
//         newBufSize = 1024 * 100; /* Default */
//     }
//     (void)newBufSize;
// }

/*==========================================================================*/
static void GUI_createMainScreen(void) {
    /* ...Container root. */
    lv_obj_t *root = lv_obj_create(lv_scr_act());
    lv_obj_set_size(root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(root, 5, 0);
    lv_obj_set_style_pad_gap(root, 5, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_bg_opa(root, 0, 0);

    /* ...Com control panel. */
    lv_obj_t *root_comCtrlPanel = lv_obj_create(root);
    lv_obj_set_width(root_comCtrlPanel, LV_PCT(100));
    lv_obj_set_height(root_comCtrlPanel, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(root_comCtrlPanel, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        root_comCtrlPanel,
        LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(root_comCtrlPanel, 5, 0);
    lv_obj_set_style_pad_gap(root_comCtrlPanel, 10, 0);
    lv_obj_set_style_bg_color(
        root_comCtrlPanel, lv_palette_lighten(LV_PALETTE_BLUE_GREY, 5), 0);

    /* ...Com connecting button. */
    GUI_Btn_cnnct = lv_btn_create(root_comCtrlPanel);
    lv_obj_add_flag(GUI_Btn_cnnct, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_height(GUI_Btn_cnnct, 36);
    lv_obj_set_width(GUI_Btn_cnnct, 100);
    lv_obj_add_event_cb(
        GUI_Btn_cnnct, GUIEvtCb_Btn_cnnct, LV_EVENT_CLICKED, NULL);
    lv_obj_t *GUI_Btn_cnnct_label = lv_label_create(GUI_Btn_cnnct);
    lv_label_set_text(GUI_Btn_cnnct_label, "open");
    lv_obj_center(GUI_Btn_cnnct_label);

    /* ...Com connecting status led. */
    GUI_Led_status = lv_led_create(root_comCtrlPanel);
    lv_obj_set_size(GUI_Led_status, 20, 20);
    lv_led_set_color(GUI_Led_status, lv_palette_main(LV_PALETTE_GREEN));
    lv_led_off(GUI_Led_status);

    /* ...Com lists dropdown box. */
    GUI_DdBox_comList = lv_dropdown_create(root_comCtrlPanel);
    lv_dropdown_set_options(
        GUI_DdBox_comList, "NULL");  /* Default. */
    lv_obj_set_width(GUI_DdBox_comList, 100);
    lv_obj_add_event_cb(
        GUI_DdBox_comList, GUIEvtCb_DdBox_comList, LV_EVENT_ALL, NULL);

    /* ...Button refreshing the com lists dropdown box. */
    GUI_Btn_comRefresh = lv_btn_create(root_comCtrlPanel);
    lv_obj_set_size(GUI_Btn_comRefresh, 40, 40);
    lv_obj_set_style_radius(
        GUI_Btn_comRefresh, LV_RADIUS_CIRCLE, 0);  /* Round */
    lv_obj_set_style_pad_all(GUI_Btn_comRefresh, 0, 0);
    lv_obj_set_style_bg_color(
        GUI_Btn_comRefresh, lv_palette_main(LV_PALETTE_ORANGE), 0);
    lv_obj_add_event_cb(
        GUI_Btn_comRefresh, GUIEvtCb_Btn_comRefresh, LV_EVENT_CLICKED, NULL);
    lv_obj_t *GUI_Btn_comRefresh_lable = lv_label_create(GUI_Btn_comRefresh);
    lv_label_set_text(GUI_Btn_comRefresh_lable, LV_SYMBOL_REFRESH);
    lv_obj_center(GUI_Btn_comRefresh_lable);

    /* ...Baudrate dropdown box. */
    GUI_DdBox_baudrate = lv_dropdown_create(root_comCtrlPanel);
    lv_dropdown_set_options(
        GUI_DdBox_baudrate,
        "2400\n4800\n9600\n115200\n921600");
    lv_dropdown_set_selected(GUI_DdBox_baudrate, 3);
    lv_obj_set_width(GUI_DdBox_baudrate, 100);
    lv_obj_add_event_cb(
        GUI_DdBox_baudrate, GUIEvtCb_DdBox_baudrate, LV_EVENT_ALL, NULL);

    /* ...Databits dropdown box. */
    GUI_DdBox_dataBits = lv_dropdown_create(root_comCtrlPanel);
    lv_dropdown_set_options(GUI_DdBox_dataBits, "5\n6\n7\n8");
    lv_dropdown_set_selected(GUI_DdBox_dataBits, 3); /* Default 8 */
    lv_obj_set_width(GUI_DdBox_dataBits, 60);
    lv_obj_add_event_cb(
        GUI_DdBox_dataBits, GUIEvtCb_DdBox_dataBits, LV_EVENT_ALL, NULL);

    /* ...Stopbits dropdown box. */
    GUI_DdBox_stopBits = lv_dropdown_create(root_comCtrlPanel);
    lv_dropdown_set_options(GUI_DdBox_stopBits, "1\n1.5\n2");
    lv_dropdown_set_selected(GUI_DdBox_stopBits, 0); /* Default 1 */
    lv_obj_set_width(GUI_DdBox_stopBits, 70);
    lv_obj_add_event_cb(
        GUI_DdBox_stopBits, GUIEvtCb_DdBoxStopBits, LV_EVENT_ALL, NULL);

    /* ...Parity dropdown box. */
    GUI_DdBox_parity = lv_dropdown_create(root_comCtrlPanel);
    lv_dropdown_set_options(GUI_DdBox_parity, "None\nOdd\nEven\nMark\nSpace");
    lv_dropdown_set_selected(GUI_DdBox_parity, 0); /* Default None */
    lv_obj_set_width(GUI_DdBox_parity, 86);
    lv_obj_add_event_cb(
        GUI_DdBox_parity, GUIEvtCb_DdBox_parity, LV_EVENT_ALL, NULL);

    /* ...FlowControl dropdown box. */
    GUI_DdBox_flowCtrl = lv_dropdown_create(root_comCtrlPanel);
    lv_dropdown_set_options(
        GUI_DdBox_flowCtrl, "None\nXon/Xoff\nRTS/CTS\nDTR/DSR");
    lv_dropdown_set_selected(GUI_DdBox_flowCtrl, 0); /* Default None */
    lv_obj_set_width(GUI_DdBox_flowCtrl, 120);
    lv_obj_add_event_cb(
        GUI_DdBox_flowCtrl, GUIEvtCb_flowCtrl, LV_EVENT_ALL, NULL);

    /* ...Sub container of root container. */
    lv_obj_t *root_subContainer = lv_obj_create(root);
    lv_obj_set_width(root_subContainer, LV_PCT(100));
    lv_obj_set_height(root_subContainer, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(root_subContainer, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(root_subContainer, 0, 0);
    lv_obj_set_style_border_width(root_subContainer, 2, 0);
    lv_obj_set_style_pad_gap(root_subContainer, 5, 0);

    /* ...Message box control panel. */
    lv_obj_t *GUI_msgBoxCtrlPanel = lv_obj_create(root_subContainer);
    // lv_obj_set_width(GUI_msgBoxCtrlPanel, 160);
    lv_obj_set_width(GUI_msgBoxCtrlPanel, 80);
    lv_obj_set_height(GUI_msgBoxCtrlPanel, 52);
    lv_obj_set_flex_flow(GUI_msgBoxCtrlPanel, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        GUI_msgBoxCtrlPanel,
        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(GUI_msgBoxCtrlPanel, 5, 0);
    lv_obj_set_style_border_width(GUI_msgBoxCtrlPanel, 0, 0);
    lv_obj_set_style_pad_gap(GUI_msgBoxCtrlPanel, 5, 0);
    lv_obj_set_style_bg_color(
        GUI_msgBoxCtrlPanel, lv_palette_lighten(LV_PALETTE_BLUE_GREY, 5), 0);

    /* ...Button cleaning message box content. */
    lv_obj_t *GUI_Btn_msgBoxClean = lv_btn_create(GUI_msgBoxCtrlPanel);
    lv_obj_set_size(GUI_Btn_msgBoxClean, 40, 40);
    lv_obj_set_style_bg_color(
        GUI_Btn_msgBoxClean, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_set_style_text_align(GUI_Btn_msgBoxClean, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_add_event_cb(
        GUI_Btn_msgBoxClean,
        GUIEvtCb_Btn_msgBoxClean, LV_EVENT_CLICKED, NULL);
    lv_obj_t *GUI_Btn_msgBoxClean_label =
                                        lv_label_create(GUI_Btn_msgBoxClean);
    lv_label_set_text(GUI_Btn_msgBoxClean_label, LV_SYMBOL_TRASH);
    lv_obj_center(GUI_Btn_msgBoxClean_label);

    /* ...Msg display buffer setting dropdown box. */
    // lv_obj_t *GUI_DdBox_dispBuf = lv_dropdown_create(GUI_msgBoxCtrlPanel);
    // lv_dropdown_set_options(
    //     GUI_DdBox_dispBuf,
    //     "10K\n20K\n40K\n80K\n100K");
    // lv_dropdown_set_selected(GUI_DdBox_dispBuf, 4);
    // lv_obj_set_width(GUI_DdBox_dispBuf, 80);
    // lv_obj_set_height(GUI_DdBox_dispBuf, 40);
    // lv_obj_add_event_cb(
    //     GUI_DdBox_dispBuf,
    //     GUIEvtCb_DdBox_dispBuf, LV_EVENT_VALUE_CHANGED, NULL);

    /* ...JSON config panel. */
    lv_obj_t *GUI_jsonToolBoxPanel = lv_obj_create(root_subContainer);
    lv_obj_set_flex_grow(GUI_jsonToolBoxPanel, 1);
    lv_obj_set_height(GUI_jsonToolBoxPanel, 52);
    lv_obj_set_flex_flow(GUI_jsonToolBoxPanel, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        GUI_jsonToolBoxPanel,
        LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(GUI_jsonToolBoxPanel, 5, 0);
    lv_obj_set_style_border_width(GUI_jsonToolBoxPanel, 0, 0);
    lv_obj_set_style_pad_gap(GUI_jsonToolBoxPanel, 5, 0);
    lv_obj_set_style_bg_color(
        GUI_jsonToolBoxPanel, lv_palette_lighten(LV_PALETTE_BLUE_GREY, 5), 0);

    /* ...Protocal indication label. */
    lv_obj_t *prtcLable = lv_label_create(GUI_jsonToolBoxPanel);
    lv_label_set_text(prtcLable, "Protocol:");
    lv_obj_set_style_text_opa(prtcLable, LV_OPA_70, 0);

    /* ...JSON files dropdown box. */
    GUI_DdBox_jsonFiles = lv_dropdown_create(GUI_jsonToolBoxPanel);
    lv_dropdown_set_options(GUI_DdBox_jsonFiles, "Empty");
    lv_obj_set_width(GUI_DdBox_jsonFiles, 222);
    lv_obj_add_event_cb(
        GUI_DdBox_jsonFiles,
        GUIEvtCb_DdBox_jsonFiles, LV_EVENT_VALUE_CHANGED, NULL);

    /* ...Button refreshing JSON files at dir cfg/. */
    GUI_Btn_jsonFileRefresh = lv_btn_create(GUI_jsonToolBoxPanel);
    lv_obj_set_size(GUI_Btn_jsonFileRefresh, 36, 36);
    lv_obj_set_style_pad_all(GUI_Btn_jsonFileRefresh, 0, 0);
    lv_obj_set_style_radius(
        GUI_Btn_jsonFileRefresh, LV_RADIUS_CIRCLE, 0);  /* Round button. */
    lv_obj_set_style_bg_color(
        GUI_Btn_jsonFileRefresh, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_add_event_cb(
        GUI_Btn_jsonFileRefresh,
        GUIEvtCb_Btn_jsonFileRefresh, LV_EVENT_CLICKED, NULL);
    lv_obj_t *GUI_Btn_jsonFileRefresh_lable =
                                    lv_label_create(GUI_Btn_jsonFileRefresh);
    lv_label_set_text(GUI_Btn_jsonFileRefresh_lable, LV_SYMBOL_REFRESH);
    lv_obj_center(GUI_Btn_jsonFileRefresh_lable);

    /* ...Button loading the current selected JSON file. */
    GUI_Btn_jsonLoad = lv_btn_create(GUI_jsonToolBoxPanel);
    lv_obj_set_height(GUI_Btn_jsonLoad, 36);
    lv_obj_set_style_bg_color(
        GUI_Btn_jsonLoad, lv_palette_main(LV_PALETTE_TEAL), 0);
    lv_obj_add_event_cb(
        GUI_Btn_jsonLoad, GUIEvtCb_Btn_jsonLoad, LV_EVENT_CLICKED, NULL);
    lv_obj_t *GUI_Btn_jsonLoad_label = lv_label_create(GUI_Btn_jsonLoad);
    lv_label_set_text(GUI_Btn_jsonLoad_label, LV_SYMBOL_UPLOAD);
    lv_obj_center(GUI_Btn_jsonLoad_label);

    /* ...Label indicating the current config. */
    GUI_Label_currJsonFile = lv_label_create(GUI_jsonToolBoxPanel);
    snprintf(
        GUI_currLoadedJsonFileName, GUI_MAX_LENGTH_JSON_FILE_NAME,
        "%s loaded.", "Empty");
    lv_label_set_text(GUI_Label_currJsonFile, GUI_currLoadedJsonFileName);
    lv_obj_set_style_text_color(
        GUI_Label_currJsonFile, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_set_flex_grow(GUI_Label_currJsonFile, 1);
    lv_obj_set_style_text_align(
        GUI_Label_currJsonFile, LV_TEXT_ALIGN_RIGHT, 0);
}

/*==========================================================================*/
void GUI_init(void) {
    /* White background. */
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_white(), 0);
    /* LVGL tool menu. */
    GUI_createMainScreen();
}

/*==========================================================================*/
void GUI_updateSerialCnnState(uint8_t state) {
    lv_obj_t *GUI_Btn_cnnct_label = lv_obj_get_child(GUI_Btn_cnnct, 0);
    switch (state) {
        case DISPLAY_STATE_CLOSED: {
            /* widgets control */
            lv_obj_clear_state(GUI_Btn_cnnct, LV_STATE_DISABLED);
            lv_obj_add_state(GUI_Led_status, LV_STATE_DISABLED);
            lv_obj_clear_state(GUI_DdBox_comList, LV_STATE_DISABLED);
            lv_obj_clear_state(GUI_Btn_comRefresh, LV_STATE_DISABLED);
            lv_obj_clear_state(GUI_DdBox_baudrate, LV_STATE_DISABLED);
            lv_obj_clear_state(GUI_DdBox_dataBits, LV_STATE_DISABLED);
            lv_obj_clear_state(GUI_DdBox_stopBits, LV_STATE_DISABLED);
            lv_obj_clear_state(GUI_DdBox_parity, LV_STATE_DISABLED);
            lv_obj_clear_state(GUI_DdBox_flowCtrl, LV_STATE_DISABLED);

            /* force uncheck */
            lv_obj_clear_state(GUI_Btn_cnnct, LV_STATE_CHECKED);
            lv_obj_set_style_bg_color(
                GUI_Btn_cnnct,
                lv_palette_main(LV_PALETTE_BLUE), LV_STATE_DEFAULT);

            /* display open */
            lv_label_set_text(GUI_Btn_cnnct_label, "open");

            /* led grey */
            lv_led_set_color(
                GUI_Led_status,
                lv_palette_main(LV_PALETTE_GREY));

            /* led off */
            lv_led_off(GUI_Led_status);
            break;
        }

        case DISPLAY_STATE_OPENING: {
            /* widgets control */
            lv_obj_add_state(GUI_Btn_cnnct, LV_STATE_DISABLED);
            lv_obj_add_state(GUI_Led_status, LV_STATE_DISABLED);
            lv_obj_add_state(GUI_DdBox_comList, LV_STATE_DISABLED);
            lv_obj_add_state(GUI_Btn_comRefresh, LV_STATE_DISABLED);
            lv_obj_add_state(GUI_DdBox_baudrate, LV_STATE_DISABLED);
            lv_obj_add_state(GUI_DdBox_dataBits, LV_STATE_DISABLED);
            lv_obj_add_state(GUI_DdBox_stopBits, LV_STATE_DISABLED);
            lv_obj_add_state(GUI_DdBox_parity, LV_STATE_DISABLED);
            lv_obj_add_state(GUI_DdBox_flowCtrl, LV_STATE_DISABLED);

            /* yellow background */
            lv_obj_clear_state(GUI_Btn_cnnct, LV_STATE_CHECKED);
            lv_obj_set_style_bg_color(
                GUI_Btn_cnnct,
                lv_palette_main(LV_PALETTE_YELLOW), LV_STATE_DEFAULT);

            /* display opening */
            lv_label_set_text(GUI_Btn_cnnct_label, "opening");

            /* led grey */
            lv_led_set_color(
                GUI_Led_status,
                lv_palette_main(LV_PALETTE_YELLOW));

            lv_led_on(GUI_Led_status);
            break;
        }

        case DISPLAY_STATE_OPENED: {
            /* widgets control */
            lv_obj_clear_state(GUI_Btn_cnnct, LV_STATE_DISABLED);
            lv_obj_clear_state(GUI_Led_status, LV_STATE_DISABLED);
            lv_obj_add_state(GUI_DdBox_comList, LV_STATE_DISABLED);
            lv_obj_add_state(GUI_Btn_comRefresh, LV_STATE_DISABLED);
            lv_obj_add_state(GUI_DdBox_baudrate, LV_STATE_DISABLED);
            lv_obj_add_state(GUI_DdBox_dataBits, LV_STATE_DISABLED);
            lv_obj_add_state(GUI_DdBox_stopBits, LV_STATE_DISABLED);
            lv_obj_add_state(GUI_DdBox_parity, LV_STATE_DISABLED);
            lv_obj_add_state(GUI_DdBox_flowCtrl, LV_STATE_DISABLED);

            /* force checked, use default checked theme color */
            lv_obj_add_state(GUI_Btn_cnnct, LV_STATE_CHECKED);

            /* display close */
            lv_label_set_text(GUI_Btn_cnnct_label, "close");

            /* led grey */
            lv_led_set_color(
                GUI_Led_status,
                lv_palette_main(LV_PALETTE_GREEN));

            lv_led_on(GUI_Led_status);
            break;
        }

        case DISPLAY_STATE_CLOSING: {
            /* widgets control */
            lv_obj_add_state(GUI_Btn_cnnct, LV_STATE_DISABLED);
            lv_obj_add_state(GUI_Led_status, LV_STATE_DISABLED);
            lv_obj_add_state(GUI_DdBox_comList, LV_STATE_DISABLED);
            lv_obj_add_state(GUI_Btn_comRefresh, LV_STATE_DISABLED);
            lv_obj_add_state(GUI_DdBox_baudrate, LV_STATE_DISABLED);
            lv_obj_add_state(GUI_DdBox_dataBits, LV_STATE_DISABLED);
            lv_obj_add_state(GUI_DdBox_stopBits, LV_STATE_DISABLED);
            lv_obj_add_state(GUI_DdBox_parity, LV_STATE_DISABLED);
            lv_obj_add_state(GUI_DdBox_flowCtrl, LV_STATE_DISABLED);

            /* yellow background */
            lv_obj_clear_state(GUI_Btn_cnnct, LV_STATE_CHECKED);
            lv_obj_set_style_bg_color(
                GUI_Btn_cnnct,
                lv_palette_main(LV_PALETTE_YELLOW), LV_STATE_DEFAULT);

            /* display opening */
            lv_label_set_text(GUI_Btn_cnnct_label, "closing");

            /* led grey */
            lv_led_set_color(
                GUI_Led_status, lv_palette_main(LV_PALETTE_YELLOW));

            lv_led_on(GUI_Led_status);
            break;
        }

        default: {
            break;
        }
    }
}

/*==========================================================================*/
static int get_option_index(const char *options_str, const char *target) {
    if (!options_str || !target || *target == '\0') return -1;

    /* Duplicate */
    char *temp = strdup(options_str);
    SM_ENSURE(temp != (char *)0);

    int idx = 0;
    int result = -1;
    char *token = strtok(temp, "\n");
    while (token != NULL) {
        if (strcmp(token, target) == 0) {
            result = idx;  /* Found */
            break;
        }
        token = strtok(NULL, "\n");
        idx++;
    }
    free(temp);  /*! GC */
    return result;
}
/*..........................................................................*/
void GUI_updateComLists(char *comLists) {
    /* Current selected COM name */
    char current_selection[30] = {0};
    lv_dropdown_get_selected_str(
        GUI_DdBox_comList, current_selection, sizeof(current_selection));

    lv_dropdown_close(GUI_DdBox_comList);  /* Close */

    int target_idx = 0;  /* Default 0 */

    /* Update display data */
    lv_dropdown_set_options(GUI_DdBox_comList, comLists);

    /* Find last selected COM in new port list */
    int found_idx = get_option_index(comLists, current_selection);

    if (found_idx >= 0) {
        /* Still same... */
        target_idx = found_idx;
    } else {
        /* Disappear... */
        /* Updated selected com name */
        char *comNameStr = malloc(30);
        SM_ENSURE(comNameStr != (char *)0);

        /* Copy */
        lv_dropdown_get_selected_str(
                        GUI_DdBox_comList, comNameStr, sizeof(comNameStr));

        GuiMngrEvt *evt = Q_NEW(GuiMngrEvt, GUI_UPDATE_COM_SIG);
        evt->txt = comNameStr;
        QACTIVE_POST(AO_GuiMngr, (QEvt*)evt, (void *)0);
    }

    /* Refresh COM selection */
    lv_dropdown_set_selected(GUI_DdBox_comList, target_idx);
    /* Dirty marked */
    lv_obj_invalidate(lv_scr_act());

    free(comLists);  /*! FREE */
}

/*==========================================================================*/
void GUI_updateLoadedJsonFile(char *fileName) {
    snprintf(
        GUI_currLoadedJsonFileName, GUI_MAX_LENGTH_JSON_FILE_NAME,
        "%s loaded.", fileName);
    lv_label_set_text(GUI_Label_currJsonFile, GUI_currLoadedJsonFileName);
    free(fileName); /* GC */
}

/*==========================================================================*/
/**
 * @brief
 *
 * @param[in] s
 */
static void GUI_dispMsg(char *s) {
    BSP_msgbuf_append(s);
}
/*..........................................................................*/
void GUI_updateRecvBoxMsg(char *dispMsg) {
    GUI_dispMsg(dispMsg);
    /* Ownership transferred to BSP_msgbuf_append; do NOT free here. */
}
/*..........................................................................*/
void GUI_msgBoxClean(void) {
    BSP_msgbuf_clear();
}
