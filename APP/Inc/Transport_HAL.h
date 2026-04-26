#ifndef TRANSPORT_HAL_H
#define TRANSPORT_HAL_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 通信通道抽象 — 可插拔后端 (ETH / RS485 / ...)
 *
 * 每个后端实现 Transport_Ops_t 虚函数表，上层通过统一接口调用。
 * 同一时刻可注册多个通道，由业务层选择使用哪个。
 */

/** 通道类型标识 */
typedef enum {
  TRANSPORT_ETH = 0,
  TRANSPORT_RS485,
  TRANSPORT_MAX
} TransportType_t;

/** 发送/接收结果 */
typedef enum {
  TRANS_OK = 0,
  TRANS_ERR_INIT,
  TRANS_ERR_SEND,
  TRANS_ERR_RECV,
  TRANS_ERR_TIMEOUT,
  TRANS_ERR_NO_IMPL
} TransResult_t;

/** 虚函数表 — 每个后端填充一组函数指针 */
typedef struct {
  TransportType_t type;
  int8_t       (*init)(void);
  TransResult_t (*send)(const uint8_t *data, uint32_t len, uint32_t id);
  TransResult_t (*recv)(uint8_t *buf, uint32_t buf_size, uint32_t *out_len,
                        uint32_t timeout_ms);
  void         (*deinit)(void);
} Transport_Ops_t;

/** 注册一个通道后端 */
void Transport_Register(const Transport_Ops_t *ops);

/** 获取指定类型的通道 (未注册返回 NULL) */
const Transport_Ops_t *Transport_Get(TransportType_t type);

#endif /* TRANSPORT_HAL_H */
