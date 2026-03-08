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

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

/*==========================================================================*/
SM_DEFINE_MODULE("bsp_spt")

/*==========================================================================*/
/* ===Thread event queue. */
/* Event queue size. */
#define PTR_QUEUE_SIZE 512
/* Event queue size... */
typedef struct {
    SpThreadEvt *buffer[PTR_QUEUE_SIZE];
    int end;
    int head;
    int tail;
    int nUsed;
    int nFree;
    int nMin;

#ifdef _WIN32
    CRITICAL_SECTION lock;
    HANDLE           sem_handle;
#else
    SDL_mutex *lock;
    SDL_sem   *sem_handle;
#endif
} PtrQueue;

/* Thread queue instance. */
static PtrQueue lspThreadQueue;

/*==========================================================================*/
static void _q_init(void) {
    lspThreadQueue.end   = PTR_QUEUE_SIZE - 1;
    lspThreadQueue.head  = 0;
    lspThreadQueue.tail  = 0;
    lspThreadQueue.nUsed = 0;
    lspThreadQueue.nFree = PTR_QUEUE_SIZE;
    lspThreadQueue.nMin  = PTR_QUEUE_SIZE;

    /* OS thread configurations... */
#ifdef _WIN32
    InitializeCriticalSection(&lspThreadQueue.lock);
    lspThreadQueue.sem_handle =
                            CreateSemaphore(NULL, 0, PTR_QUEUE_SIZE, NULL);
#else
    lspThreadQueue.lock       = SDL_CreateMutex();
    lspThreadQueue.sem_handle = SDL_CreateSemaphore(0);
#endif
}
/*..........................................................................*/
static bool _q_push(SpThreadEvt* ptr) {
    bool ret = false;
#ifdef _WIN32
    EnterCriticalSection(&lspThreadQueue.lock);
#else
    SDL_LockMutex(lspThreadQueue.lock);
#endif

    if (lspThreadQueue.nFree > 0) {
        lspThreadQueue.buffer[lspThreadQueue.head] = ptr;
        /* Wrap around. */
        lspThreadQueue.head == 0 ?
                    lspThreadQueue.head = lspThreadQueue.end :
                    --lspThreadQueue.head;
        ++lspThreadQueue.nUsed;
        --lspThreadQueue.nFree;
        if (lspThreadQueue.nMin > lspThreadQueue.nFree) {
            lspThreadQueue.nMin = lspThreadQueue.nFree;
        }
        ret = true;
    }

#ifdef _WIN32
    LeaveCriticalSection(&lspThreadQueue.lock);
#else
    SDL_UnlockMutex(lspThreadQueue.lock);
#endif

#ifdef _WIN32
    if (ret) ReleaseSemaphore(lspThreadQueue.sem_handle, 1, NULL);
#else
    if (ret) SDL_SemPost(lspThreadQueue.sem_handle);
#endif
    else     free(ptr);

    return ret;
}
/*..........................................................................*/
static SpThreadEvt *_q_pop(void) {
    SpThreadEvt *ptr = NULL;

#ifdef _WIN32
    EnterCriticalSection(&lspThreadQueue.lock);
#else
    SDL_LockMutex(lspThreadQueue.lock);
#endif

    if (lspThreadQueue.nUsed > 0) {
        ptr = lspThreadQueue.buffer[lspThreadQueue.tail];
        lspThreadQueue.tail == 0 ?
                    lspThreadQueue.tail = lspThreadQueue.end :
                    --lspThreadQueue.tail;
        --lspThreadQueue.nUsed;
        ++lspThreadQueue.nFree;
    }

#ifdef _WIN32
    LeaveCriticalSection(&lspThreadQueue.lock);
#else
    SDL_UnlockMutex(lspThreadQueue.lock);
#endif

    return ptr;
}

/*==========================================================================*/
void BSP_Post2LspThread(
    uint16_t sig_,
    const char *portName,
    int baudrate,
    int dataBits,
    SerialStopBits_t stopBits,
    SerialParity_t parity,
    SerialFlow_t flowControl)
{
    SpThreadEvt *evt = (SpThreadEvt *)malloc(sizeof(SpThreadEvt));
    SM_ENSURE(evt != (SpThreadEvt *)0);

    evt->sig = sig_;
    if (portName != (char *)0) {
        strncpy(evt->spConfig.portName, portName, 64);
    }
    evt->spConfig.baudrate    = baudrate;
    evt->spConfig.dataBits    = dataBits;
    evt->spConfig.stopBits    = stopBits;
    evt->spConfig.parity      = parity;
    evt->spConfig.flowControl = flowControl;

    _q_push(evt);
}

/*==========================================================================*/
/* ===Thread handler. */
static bool SpTrd_isRwWorking(void);
static void SpTrd_setRwWorking(void);
static void SpTrd_resetRwWorking(void);
/*..........................................................................*/
/* ...Thread SM. */
typedef enum {
    SP_idle,
    SP_connected
} SP_State;
/*..........................................................................*/
typedef struct {
    SP_State state;
    bool isRwWorking;
    struct sp_port *currentPort;
} SP_Task;

/* Instance. */
static SP_Task SP_Task_inst;

/*==========================================================================*/
/* ===Reading task. */
/* Receive buffer length. */
#define SERIAL_RX_BUF_SIZE      2048
/* Packet params. */
#define SERIAL_RX_PACKET_LENGTH 1024 /* Unit: bytes. */
#define SERIAL_RX_PACKET_IDLE   5    /* Unit: ms. */
/*..........................................................................*/
/* Pacage struct. */
typedef struct {
    /* Receiving buffer. */
    uint8_t rxBuf[SERIAL_RX_BUF_SIZE];
    size_t rxCnt;
    /* Time gap, for packet idle. */
#ifdef _WIN32
    DWORD latestRxTimeMark;
#else
    Uint32 latestRxTimeMark;
#endif
} ReceivePackage;

/* Instance. */
static ReceivePackage recvPackage = {
    .rxBuf[0] = 0,
    .rxCnt    = 0,
    .latestRxTimeMark = 0
};

/*==========================================================================*/
/**
 * @brief Indicates that the libserialport has receive enough bytes and need
 *        to package up to the AO_SpMngr
 *
 * @param pkg
 *
 * @return true Pack it up
 * @return false Not yet
 *
 * @sa SERIAL_RX_PACKET_LENGTH
 */
static bool SerialPortThread_rxTask_packetCntOverflow(ReceivePackage *pkg);
/*..........................................................................*/
/**
 * @brief
 * @param pkg
 * @return true
 * @return false
 */
static bool SerialPortThread_rxTask_packetTimeout(ReceivePackage *pkg);
/*..........................................................................*/
/**
 * @brief Reset the receive byte count
 *
 * @param[in] pkg Serial port package instance
 *
 * @sa ReceivePackage
 */
static void SerialPortThread_rxTask_packReset(ReceivePackage *pkg);
/*..........................................................................*/
/**
 * @brief Pack up the data and post to AO_SpMngr
 *
 * @param[in] pkg Serial port package instance
 *
 * @sa ReceivePackage
 */
static void SerialPortThread_rxTask_packUp(ReceivePackage *pkg);
/*..........................................................................*/
/**
 * @brief Nonblocking read data from serial buffer to local, and process
 *        package inside; When the hardware error occurs, handles it in
 *        place;
 *
 * @param[in] pkg Serial port package instance
 *
 * @sa SerialPortThread_rxTask_packetCntOverflow
 * @sa SerialPortThread_rxTask_packetTimeout
 */
static void SerialPortThread_rxTask(ReceivePackage *pkg);

/*==========================================================================*/
/**
 * @brief
 */
static void SP_Task_ctor(void) {
    SP_Task *me = &SP_Task_inst;
    me->state = SP_idle;
    me->isRwWorking = false;
    me->currentPort = NULL;
}
/*..........................................................................*/
/**
 * @brief Scan and return detected ports.
 *
 * @return char *
 */
static char *spGetPortLstString(void) {
    struct sp_port **ports_list;
    char *failureRet;

    enum sp_return ret = sp_list_ports(&ports_list);
    if (ret != SP_OK) {
        failureRet = strdup("ERROR");
        SM_ENSURE(failureRet != (char *)0);
        return failureRet;
    }

    size_t total_len = 0;
    int port_count = 0;
    for (int i = 0; ports_list[i] != NULL; i++) {
        char *name = sp_get_port_name(ports_list[i]);
        total_len += strlen(name);
        total_len += 1;
        ++port_count;
    }

    if (port_count == 0) {
        sp_free_port_list(ports_list);
        failureRet = strdup("NULL");
        SM_ENSURE(failureRet != (char *)0);
        return failureRet;
    }

    char *dynamic_buf = malloc(total_len + 1);
    if (dynamic_buf == (char *)0) {
        sp_free_port_list(ports_list);
        /* Memory allocation failure is not allowed. */
        SM_ERROR("Punishment.");
    }

    dynamic_buf[0] = '\0';
    for (int i = 0; ports_list[i] != NULL; i++) {
        char *name = sp_get_port_name(ports_list[i]);
        strcat(dynamic_buf, name);
        strcat(dynamic_buf, "\n");
    }
    dynamic_buf[total_len - 1] = '\0';

    sp_free_port_list(ports_list);

    return dynamic_buf;
}

/*==========================================================================*/
/* ===State handlers. */
/* Idle. */
static SP_State SP_Task_idle(SP_Task *me, SpThreadEvt *e) {
    switch (e->sig) {

        case SP_TRD_UPDATE_SERIAL_PORTS_SIG: {
            char *ports_str = spGetPortLstString();
            SM_ENSURE(ports_str != (char *)0);

            SpMngrEvt *evt = Q_NEW(
                                SpMngrEvt, SP_MNGR_GET_UPDATED_COM_INFO_SIG);
            evt->pld.portLst = ports_str;
            QACTIVE_POST(AO_SpMngr, (QEvt *)evt, (void *)0);

            return me->state;  /* Handled. */
        }

        case SP_TRD_OPEN_PORT_SIG: {
            /* Get handler of port... */
            enum sp_return ret;
            ret = sp_get_port_by_name(e->spConfig.portName, &me->currentPort);
            if (ret != SP_OK) {
                SpMngrEvt *evt = Q_NEW(SpMngrEvt, SP_MNGR_OPEN_COM_FAIL_SIG);
                QACTIVE_POST(AO_SpMngr, (QEvt *)evt, (void *)0);

                return me->state;  /* Handled. */
            }

            /* Get handler of port success. */

            /* Open port. */
            ret = sp_open(me->currentPort, SP_MODE_READ_WRITE);
            if (ret != SP_OK) {
                /* Open port fail. */
                sp_free_port(me->currentPort);

                SpMngrEvt *evt = Q_NEW(SpMngrEvt, SP_MNGR_OPEN_COM_FAIL_SIG);
                QACTIVE_POST(AO_SpMngr, (QEvt *)evt, (void *)0);

                return me->state;  /* Handled. */
            }

            /* Serial port opened... */

            /* Pass configurations... */
            sp_set_baudrate(me->currentPort, e->spConfig.baudrate);
            sp_set_bits(    me->currentPort, e->spConfig.dataBits);
            sp_set_stopbits(me->currentPort, e->spConfig.stopBits);
            sp_set_parity(
                            me->currentPort,
                            (enum sp_parity)(e->spConfig.parity));
            sp_set_flowcontrol(
                            me->currentPort,
                            (enum sp_flowcontrol)(e->spConfig.flowControl));

            /* Clear the buffer of the comport. */
            sp_flush(me->currentPort, SP_BUF_BOTH);

            /* Inform AO_SpMngr... */
            SpMngrEvt *evt = Q_NEW(SpMngrEvt, SP_MNGR_OPEN_COM_SUCCESS_SIG);
            QACTIVE_POST(AO_SpMngr, (QEvt *)evt, (void *)0);

            /* Thread turn blocking mode. */
            SpTrd_setRwWorking();

            return SP_connected;  /* Tran. */
        }

        default: {
            return me->state;  /* Ignored. */
        }
    }
}
/*..........................................................................*/
/* Connected. */
static SP_State SP_Task_connected(SP_Task *me, SpThreadEvt *e) {
    switch (e->sig) {

        case SP_TRD_CLOSE_PORT_SIG: {
            /* Destroy the opened port. */
            sp_close(me->currentPort);
            sp_free_port(me->currentPort);
            me->currentPort = NULL;

            /* Close success. */
            SpMngrEvt *evt = Q_NEW(SpMngrEvt, SP_MNGR_CLOSE_COM_SUCCESS_SIG);
            QACTIVE_POST(AO_SpMngr, (QEvt *)evt, (void *)0);

            /* Thread turn non-blocking mode. */
            SpTrd_resetRwWorking();

            return SP_idle;  /* Tran. */
        }

        case SP_TRD_PORT_ERR_SHUT_DOWN_SIG: {
            SpMngrEvt *evt = Q_NEW(SpMngrEvt, SP_MNGR_PORT_ERR_SHUT_DOWN_SIG);
            QACTIVE_POST(AO_SpMngr, (QEvt *)evt, (void *)0);

            /* Self-trigger. */
            BSP_Post2LspThread(
                            SP_TRD_UPDATE_SERIAL_PORTS_SIG,
                            (char *)0, 0, 0, 0, 0, 0);

            /* Thread turn non-blocking mode. */
            SpTrd_resetRwWorking();

            return SP_idle;  /* Tran. */
        }

        default: {
            return me->state; /* Ignored. */
        }
    }
}

/*==========================================================================*/
/* ===SM dispatcher. */
static void SP_TaskDispatch(SP_Task *me, SpThreadEvt *e) {
    switch (me->state) {
        case SP_idle: {
            me->state = SP_Task_idle(me, e);
            break;
        }
        case SP_connected: {
            me->state = SP_Task_connected(me, e);
            break;
        }
        default: {
            break;
        }
    }
}
/*..........................................................................*/
static void ThreadEvt_dispatch(void) {
    SpThreadEvt *evt = _q_pop();
    SM_ENSURE(evt != (SpThreadEvt *)0);

    /* Event dispatch. */
    SP_TaskDispatch(&SP_Task_inst, evt);

    free(evt); /* GC. */
}

/*==========================================================================*/
static bool SpTrd_isRwWorking(void) {
    return SP_Task_inst.isRwWorking;
}
/*..........................................................................*/
static void SpTrd_setRwWorking(void) {
    SP_Task_inst.isRwWorking = true;
    SerialPortThread_rxTask_packReset(&recvPackage);
}
/*..........................................................................*/
static void SpTrd_resetRwWorking(void) {
    SP_Task_inst.isRwWorking = false;
    SerialPortThread_rxTask_packReset(&recvPackage);
}

/*==========================================================================*/
/* ===SerialPort thread entrance. */
#ifdef _WIN32
DWORD WINAPI serialThread(LPVOID lpParam) {
    (void)lpParam;

    /* SpThread priority. */
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
#else
int SDLCALL serialThread(void *lpParam) {
    (void)lpParam;

    /* SpThread priority. */
    SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH);
#endif

    /* Super loop.... */
    for (;;) {
        if (SpTrd_isRwWorking()) {
            /*..............................................................*/
            /* ...Serial port is connected. */

            /* Bytes reading and package posting... */
            SerialPortThread_rxTask(&recvPackage);

            /* Thread SM event dispatching... */
            while (
#ifdef _WIN32
                WaitForSingleObject(lspThreadQueue.sem_handle, 0) ==
                                                                WAIT_OBJECT_0
#else
                SDL_SemTryWait(lspThreadQueue.sem_handle) == 0
#endif
            ) {
                ThreadEvt_dispatch();
            }
        } else {
            /*..............................................................*/
            /* ...Serial port is disconnected; block until next command. */

            /* Waiting for events coming... */
#ifdef _WIN32
            WaitForSingleObject(lspThreadQueue.sem_handle, INFINITE);
#else
            SDL_SemWait(lspThreadQueue.sem_handle);
#endif

            /* Evt processing... */
            ThreadEvt_dispatch();
        }
    }

    return 0;
}

/*==========================================================================*/
static bool SerialPortThread_rxTask_packetCntOverflow(ReceivePackage *pkg) {
    return (pkg->rxCnt >= SERIAL_RX_PACKET_LENGTH);
}
/*..........................................................................*/
static bool SerialPortThread_rxTask_packetTimeout(ReceivePackage *pkg) {
#ifdef _WIN32
    return ((GetTickCount() - pkg->latestRxTimeMark >= SERIAL_RX_PACKET_IDLE)
                && (pkg->rxCnt != 0));
#else
    return ((SDL_GetTicks() - pkg->latestRxTimeMark >= SERIAL_RX_PACKET_IDLE)
                && (pkg->rxCnt != 0));
#endif
}
/*..........................................................................*/
static void SerialPortThread_rxTask_packReset(ReceivePackage *pkg) {
    pkg->rxCnt = 0;
}
/*..........................................................................*/
static void SerialPortThread_rxTask_packUp(ReceivePackage *pkg) {
    /* Heap malloc. */
    uint8_t *pkgBuf = malloc(pkg->rxCnt);
    SM_ENSURE(pkgBuf != (uint8_t *)0);

    /* Copy: pkg->rxBuf --> pkgBuf. */
    memcpy(pkgBuf, pkg->rxBuf, pkg->rxCnt);

    /* Send the byte package to AO_SpMngr... */
    SpMngrEvt *evt = Q_NEW(SpMngrEvt, SP_MNGR_RECV_PACKET_SIG);
    evt->pld.packBuf      = pkgBuf;
    evt->pldSize.packSize = pkg->rxCnt;
    QACTIVE_POST(AO_SpMngr, (QEvt *)evt, (void *)0);

    /* Package buffer clean up. */
    SerialPortThread_rxTask_packReset(pkg);
}
/*..........................................................................*/
static void SerialPortThread_rxTask(ReceivePackage *pkg) {
    /* Non-blocking reading bytes... */
    int byteReadCnt = sp_nonblocking_read(
                                        SP_Task_inst.currentPort,
                                        pkg->rxBuf + pkg->rxCnt,
                                        SERIAL_RX_BUF_SIZE - pkg->rxCnt);

    if (byteReadCnt > 0) {
        /*..................................................................*/
        /* ...Bytes input. */

        /* Update time mark. */
#ifdef _WIN32
        pkg->latestRxTimeMark = GetTickCount();
#else
        pkg->latestRxTimeMark = SDL_GetTicks();
#endif

        /* Update received length. */
        pkg->rxCnt += byteReadCnt;

        if (SerialPortThread_rxTask_packetCntOverflow(pkg)) {
            SerialPortThread_rxTask_packUp(pkg);
        }
    } else if (byteReadCnt < 0) {
        /* Hardware fault: device lost... */
        sp_close(SP_Task_inst.currentPort);
        sp_free_port(SP_Task_inst.currentPort);
        SP_Task_inst.currentPort = NULL;

        /* Self-trigger. */
        BSP_Post2LspThread(
                        SP_TRD_PORT_ERR_SHUT_DOWN_SIG,
                        (char *)0, 0, 0, 0, 0, 0);
    } else {
        /*..................................................................*/
        /* ...Bytes reading idle. */

        /* 5ms package process. */
        if (SerialPortThread_rxTask_packetTimeout(pkg)) {
            SerialPortThread_rxTask_packUp(pkg);
        }
    }
}

/*==========================================================================*/
void BSP_Serial_Init(void) {
    _q_init();  /* Thread event queue init. */
    SP_Task_ctor();
#ifdef _WIN32
    CreateThread(NULL, 0, serialThread, NULL, 0, NULL);
#else
    SDL_CreateThread(serialThread, "SerialPort", NULL);
#endif
}
