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
#include <linux/inet.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ice Marek");
MODULE_DESCRIPTION("TCP communication");
MODULE_VERSION("1.0");

static struct socket *sock;
static struct sockaddr_in server_addr, client_addr;

#define PORT 12345
// Define your destination IP address here
#define DEST_IP "10.0.0.1"

// Declare the kernel thread
static struct task_struct *tcpStateMachine;

// // Kernel thread function
// static int tcp_state_machine(void *data) {
//     while (!kthread_should_stop()) {
//         printk(KERN_INFO "TCP State Machine: Running\n");
//         msleep(1000); // Sleep for 1000 milliseconds (1 second)
//     }
//     return 0;
// }

static int tcp_state_machine(void *data) {
    int ret;
    struct sockaddr_in saddr;

    // Create a TCP socket
    ret = sock_create_kern(AF_INET, SOCK_STREAM, IPPROTO_TCP, &sock);
    if (ret) {
        printk(KERN_ERR "Failed to create socket\n");
        return ret;
    }

    // Set up the socket address structure
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(1234); // Example port number
    if (!in4_pton(DEST_IP, -1, (u8 *)&saddr.sin_addr.s_addr, '\0', NULL)) {
        printk(KERN_ERR "Failed to convert IP address\n");
        sock_release(sock);
        return -EINVAL;
    }

    // Connect the socket
    ret = kernel_connect(sock, (struct sockaddr *)&saddr, sizeof(saddr), 0);
    if (ret) {
        printk(KERN_ERR "Failed to connect socket\n");
        sock_release(sock);
        return ret;
    }

    while (!kthread_should_stop()) {
        printk(KERN_INFO "TCP State Machine: Running\n");
        msleep(1000); // Sleep for 1000 milliseconds (1 second)
    }

    // Release the socket when the thread stops
    sock_release(sock);

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
