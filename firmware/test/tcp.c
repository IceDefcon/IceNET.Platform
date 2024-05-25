#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/net.h>
#include <linux/in.h>
#include <linux/socket.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/kthread.h> // Include kthread related headers
#include <linux/delay.h>   // Include mdelay

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ice Marek");
MODULE_DESCRIPTION("TCP communication");
MODULE_VERSION("1.0");

static struct socket *sock;
static struct sockaddr_in server_addr, client_addr;

#define PORT 12345

// Declare the kernel thread
static struct task_struct *tcpStateMachine;


// Kernel thread function
static int tcp_state_machine(void *data) {
    int ret;

    while (!kthread_should_stop()) {
        printk(KERN_INFO "TCP State Machine: Running\n");

        // Accept incoming connections and receive data
        if (sock) 
        {
            struct socket *new_sock;
            struct sockaddr_in client_addr;
            int size = sizeof(client_addr);
            char buffer[1];

            // Accept connection
            ret = sock->ops->accept(sock, &new_sock, O_NONBLOCK);
            if (ret < 0) {
                printk(KERN_ALERT "Failed to accept connection\n");
                return ret;
            }

            // Receive data
            memset(buffer, 0, sizeof(buffer));
            ret = kernel_recvmsg(new_sock, &msg, NULL, 0, sizeof(buffer), 0);
            if (ret < 0) {
                printk(KERN_ALERT "Failed to receive data\n");
            } else {
                // Check received data
                if (buffer[0] == 0xAB) {
                    printk(KERN_INFO "Received 0xAB from client\n");
                } else {
                    printk(KERN_INFO "Received unexpected data from client\n");
                }
            }

            // Release socket
            sock_release(new_sock);
        }

        msleep(1000); // Sleep for 1000 milliseconds (1 second)
    }

    return 0;
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

    // Create the kernel thread
    tcpStateMachine = kthread_run(tcp_state_machine, NULL, "tcpStateMachine");
    if (IS_ERR(tcpStateMachine)) {
        printk(KERN_ALERT "Failed to create kernel thread\n");
        sock_release(sock);
        return PTR_ERR(tcpStateMachine);
    }

    printk(KERN_INFO "Kernel module listening on port %d\n", PORT);
    return 0;
}

static void __exit tcp_module_exit(void) {
    // Stop and clean up the kernel thread
    if (tcpStateMachine) {
        kthread_stop(tcpStateMachine);
    }
    sock_release(sock);
    printk(KERN_INFO "Kernel module unloaded\n");
}

module_init(tcp_module_init);
module_exit(tcp_module_exit);
