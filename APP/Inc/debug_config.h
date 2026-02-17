#ifndef DEBUG_CONFIG_H
#define DEBUG_CONFIG_H

/* ========================================== */
/* Debug output backend: nanoprintf + UART3   */
/* ========================================== */

/* dbg_printf: defined in nanoprintf_impl.c */
extern void dbg_printf(const char *fmt, ...);

/* ========================================== */
/* Global debug switch (0=all off, 1=on)      */
/* ========================================== */
#define DEBUG_ENABLE 1

/* ========================================== */
/* Test select: 0=production, 1=mode_switch,  */
/*   2=motion_detect, 3=power_manager,        */
/*   4=net_diag                               */
/* ========================================== */
#define TEST_SELECT 4

/* ========================================== */
/* Module-level switches                      */
/* ========================================== */
#define DEBUG_NET 1
#define DEBUG_VISION 1
#define DEBUG_ETH 1
#define DEBUG_JPEG 1
#define DEBUG_DCMI 1

/* ========================================== */
/* Debug macros                               */
/* ========================================== */
#if DEBUG_ENABLE

#define DBG_INFO(fmt, ...) dbg_printf("[INFO] " fmt "\r\n", ##__VA_ARGS__)
#define DBG_WARN(fmt, ...) dbg_printf("[WARN] " fmt "\r\n", ##__VA_ARGS__)
#define DBG_ERROR(fmt, ...) dbg_printf("[ERROR] " fmt "\r\n", ##__VA_ARGS__)

#if DEBUG_NET
#define DBG_NET(fmt, ...) dbg_printf("[NET] " fmt "\r\n", ##__VA_ARGS__)
#else
#define DBG_NET(fmt, ...) ((void)0)
#endif

#if DEBUG_VISION
#define DBG_VISION(fmt, ...) dbg_printf("[VIS] " fmt "\r\n", ##__VA_ARGS__)
#else
#define DBG_VISION(fmt, ...) ((void)0)
#endif

#if DEBUG_ETH
#define DBG_ETH(fmt, ...) dbg_printf("[ETH] " fmt "\r\n", ##__VA_ARGS__)
#else
#define DBG_ETH(fmt, ...) ((void)0)
#endif

#if DEBUG_JPEG
#define DBG_JPEG(fmt, ...) dbg_printf("[JPG] " fmt "\r\n", ##__VA_ARGS__)
#else
#define DBG_JPEG(fmt, ...) ((void)0)
#endif

#if DEBUG_DCMI
#define DBG_DCMI(fmt, ...) dbg_printf("[DCMI] " fmt "\r\n", ##__VA_ARGS__)
#else
#define DBG_DCMI(fmt, ...) ((void)0)
#endif

#else
#define DBG_INFO(fmt, ...) ((void)0)
#define DBG_WARN(fmt, ...) ((void)0)
#define DBG_ERROR(fmt, ...) ((void)0)
#define DBG_NET(fmt, ...) ((void)0)
#define DBG_VISION(fmt, ...) ((void)0)
#define DBG_ETH(fmt, ...) ((void)0)
#define DBG_JPEG(fmt, ...) ((void)0)
#define DBG_DCMI(fmt, ...) ((void)0)
#endif

#endif /* DEBUG_CONFIG_H */
