#include "tcp_handler.h"

#include <string.h>

struct tcp_pcb* tcp_create_socket(const u16_t port, tcp_accept_fn accept)
{
    // создание сокета
    struct tcp_pcb* tpcb = tcp_new();
    // проверяем, что не инициализировали сокет еще
    if (tpcb == NULL)
    {
        return tpcb;
    }

    err_t err = tcp_bind(tpcb, IP_ANY_TYPE, port);
    if (ERR_OK != err)
    {
        tcp_close(tpcb);
        return NULL;
    }

    struct tcp_pcb* new_tpcb = tcp_listen(tpcb);
    if (new_tpcb == NULL)
    {
        tcp_close(tpcb);
    }
    tcp_accept(new_tpcb, accept);
    return new_tpcb;
}

err_t tcp_send_msg(struct tcp_pcb* tpcb, const char* data)
{
    err_t err = ERR_ABRT;
    // если сокет не создался, то на выход с ошибкой
    if (tpcb == NULL)
    {
        return err;
    }
    // отсылаем пакет
    err = tcp_write(tpcb, data, strlen(data)+1, TCP_WRITE_FLAG_COPY);
    if (ERR_OK == err)
    {
        err = tcp_output(tpcb);
    }
    return err;
}
