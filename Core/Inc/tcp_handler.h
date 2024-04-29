#ifndef INC_TCP_HANDLER_H_
#define INC_TCP_HANDLER_H_

#include "ip4_addr.h"
#include "err.h"
#include "tcp.h"

struct tcp_pcb* tcp_create_socket(const u16_t port, tcp_accept_fn accept); ///< функция создания сокета
err_t tcp_send_msg(struct tcp_pcb* tpcb, const char* data); ///< функция отправки сообщения

#endif
