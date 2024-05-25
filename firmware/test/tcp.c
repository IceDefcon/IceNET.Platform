#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/net.h>
#include <linux/in.h>
#include <linux/socket.h>
#include <linux/slab.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ice Marek");
MODULE_DESCRIPTION("TCP communication");
MODULE_VERSION("1.0");

static struct socket *sock;
static struct sockaddr_in server_addr, client_addr;
static char buffer[1024];

#define PORT 12345

static int tcp_receive_data(void) {
    struct msghdr msg;
    struct kvec iov;
    int len, ret;

    iov.iov_base = buffer;
    iov.iov_len = sizeof(buffer);

    memset(&msg, 0, sizeof(msg));
    msg.msg_flags = MSG_DONTWAIT;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = NULL;

    len = kernel_recvmsg(sock, &msg, &iov, 1, sizeof(buffer), msg.msg_flags);
    if (len < 0) {
        if (len == -EAGAIN || len == -EWOULDBLOCK) {
            return 0; // No data available
        }
        printk(KERN_ALERT "Error receiving data: %d\n", len);
        return len;
    } else if (len == 0) {
        return 0; // Connection closed
    } else {
        buffer[len] = '\0'; // Null-terminate the received data
        printk(KERN_INFO "Received data from client: %s\n", buffer);
        return len;
    }
}

static int __init tcp_module_init(void) {
    int ret;

    // Create a socket
    ret = sock_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, &sock);
    if (ret < 0) {
        printk(KERN_ALERT "Failed to create socket\n");
        return ret;
    }

    // Prepare the server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    ret = sock->ops->bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret < 0) {
        printk(KERN_ALERT "Failed to bind socket\n");
        sock_release(sock);
        return ret;
    }

    // Listen for incoming connections
    ret = sock->ops->listen(sock, 1);
    if (ret < 0) {
        printk(KERN_ALERT "Failed to listen on socket\n");
        sock_release(sock);
        return ret;
    }

    printk(KERN_INFO "Kernel module listening on port %d\n", PORT);

    // Accept connections and receive data
    while (!kthread_should_stop()) {
        struct socket *newsock;
        ret = sock_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, &newsock);
        if (ret < 0) {
            printk(KERN_ALERT "Failed to create new socket\n");
            continue;
        }

        ret = sock->ops->accept(sock, newsock, O_NONBLOCK);
        if (ret < 0) {
            if (ret != -EAGAIN && ret != -EWOULDBLOCK) {
                printk(KERN_ALERT "Error accepting connection: %d\n", ret);
            }
            sock_release(newsock);
            continue;
        }

        // Receive data
        tcp_receive_data();

        sock_release(newsock);
    }

    return 0;
}

static void __exit tcp_module_exit(void) {
    sock_release(sock);
    printk(KERN_INFO "Kernel module unloaded\n");
}

module_init(tcp_module_init);
module_exit(tcp_module_exit);
