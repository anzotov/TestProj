#include "udp_handler.h"

#include <string.h>

struct udp_pcb* udp_create_socket(const u16_t port, udp_recv_fn recv, void *recv_arg)
{
    // создание сокета
    struct udp_pcb* upcb = udp_new();
    // проверяем, что не инициализировали сокет еще
    if (upcb == NULL)
    {
        return upcb;
    }

    err_t err = udp_bind(upcb, IP_ANY_TYPE, port);
    if (ERR_OK != err)
    {
        udp_remove(upcb);
        return NULL;
    }

    // регистрируем колбэк на прием пакета
    udp_recv(upcb, recv, recv_arg);
    return upcb;
}

err_t udp_send_msg(struct udp_pcb* upcb, const char* data, const ip_addr_t* dst_ip, u16_t dst_port)
{
    err_t err = ERR_ABRT;
    // если сокет не создался, то на выход с ошибкой
    if (upcb == NULL)
    {
        return err;
    }
    // аллоцируем память под буфер с данными
    struct pbuf* p = pbuf_alloc(PBUF_TRANSPORT, strlen(data)+1, PBUF_RAM);
    if (p == NULL)
    {
        return err;
    }
    do
    {
        // кладём данные в аллоцированный буфер
        err = pbuf_take(p, data, strlen(data)+1);
        if (ERR_OK != err)
        {
            break;
        }

        // отсылаем пакет
        err = udp_sendto(upcb, p, dst_ip, dst_port);
    } while(0);
    // очищаем аллоцированную память
    pbuf_free(p);
    return err;
}
