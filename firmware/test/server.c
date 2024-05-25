#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/socket.h>
#include <linux/net.h>
#include <linux/inet.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ice Marek ");
MODULE_DESCRIPTION("TCP Server");

static int port = 2005;
static char *server_ip = "10.0.0.2";
static struct socket *server_socket = NULL;

static int server_thread(void *data) {
    struct socket *client_socket = NULL;
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);
    char *server_message = "This is the server's message.";
    int ret;
    struct kvec iov;
    struct msghdr msg;
    bool message_received = false;

    // Accept incoming connection
    printk(KERN_INFO "Waiting for client connection...\n");

    while (true) {
        ret = kernel_accept(server_socket, &client_socket, O_NONBLOCK);
        if (ret == -EAGAIN) {
            msleep(100); // Sleep for a while before retrying
            continue;
        } else if (ret < 0) {
            printk(KERN_ERR "Error accepting connection: %d\n", ret);
            return ret;
        }
        break;
    }

    printk(KERN_INFO "Client connected\n");

    // Initialize the msghdr structure
    memset(&msg, 0, sizeof(msg));
    msg.msg_name = &client_addr;
    msg.msg_namelen = client_addr_len;

    // Receive client's message
    char *client_message = kmalloc(2000, GFP_KERNEL);
    if (!client_message) {
        printk(KERN_ERR "Failed to allocate memory for client message\n");
        return -ENOMEM;
    }

    while (!message_received) {
        iov.iov_base = client_message;
        iov.iov_len = 2000;
        ret = kernel_recvmsg(client_socket, &msg, &iov, 1, 2000, MSG_WAITALL);
        if (ret < 0) {
            printk(KERN_ERR "Error receiving message from client: %d\n", ret);
            kfree(client_message);
            return ret;
        }
        printk(KERN_INFO "Message from client: %s\n", client_message);
        message_received = true;
    }

    kfree(client_message);

    // Close the client socket
    sock_release(client_socket);

    return 0;
}

static int server_init(void) {
    struct sockaddr_in server_addr;
    int error;

    // Create socket
    error = sock_create_kern(&init_net, AF_INET, SOCK_STREAM, IPPROTO_TCP, &server_socket);
    if (error < 0) {
        printk(KERN_ERR "Error while creating socket: %d\n", error);
        return error;
    }
    printk(KERN_INFO "Socket created successfully\n");

    // Set server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = in_aton(server_ip);
    server_addr.sin_port = htons(port);

    // Bind socket
    error = kernel_bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (error < 0) {
        printk(KERN_ERR "Couldn't bind to the port: %d\n", error);
        sock_release(server_socket);
        return error;
    }
    printk(KERN_INFO "Done with binding\n");

    // Listen for clients
    error = kernel_listen(server_socket, 1);
    if (error < 0) {
        printk(KERN_ERR "Error while listening: %d\n", error);
        sock_release(server_socket);
        return error;
    }
    printk(KERN_INFO "Listening for incoming connections...\n");

    // Start server thread
    kthread_run(server_thread, NULL, "server_thread");

    return 0;
}

static void server_exit(void) {
    if (server_socket != NULL) {
        sock_release(server_socket);
        printk(KERN_INFO "Server socket released\n");
    }
}

module_init(server_init);
module_exit(server_exit);
