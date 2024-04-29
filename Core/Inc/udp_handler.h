#ifndef INC_UDP_HANDLER_H_
#define INC_UDP_HANDLER_H_

#include "ip4_addr.h"
#include "err.h"
#include "udp.h"

struct udp_pcb* udp_create_socket(const u16_t port, udp_recv_fn recv, void *recv_arg); ///< функция создания сокета
err_t udp_send_msg(struct udp_pcb* upcb, const char* data, const ip_addr_t* dst_ip, u16_t dst_port); ///< функция отправки сообщения

#endif
