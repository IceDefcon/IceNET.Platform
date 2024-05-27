/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */

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
#include <linux/sched.h>

#include "networkStack.h"

static int port = 2555;
static char *server_ip = "10.0.0.2";
static struct socket *server_socket = NULL;
static struct task_struct *server_kthread;

static int server_kthread_function(void *data) 
{
    while (!kthread_should_stop()) 
    {
        struct socket *client_socket = NULL;
        struct sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);
        char *response_message = "message received";
        int ret;
        struct kvec iov;
        struct msghdr msg;
        bool message_received = false;

        // Accept incoming connection
        printk(KERN_INFO "[CTRL][NET] Waiting for client connection...\n");

        while (!kthread_should_stop()) 
        {
            ret = kernel_accept(server_socket, &client_socket, O_NONBLOCK);
            if (ret == -EAGAIN) 
            {
                msleep(100); // Sleep for a while before retrying
                continue;
            } 
            else if (ret < 0) 
            {
                printk(KERN_ERR "[CTRL][NET] Error accepting connection: %d\n", ret);
                return ret;
            }
            break;
        }

        if (kthread_should_stop()) 
        {
            if (client_socket) 
            {
                printk(KERN_INFO "[CTRL][NET] TCP server_kthread terminated\n");
                printk(KERN_INFO "[CTRL][NET] Release client socket\n");
                sock_release(client_socket);
            }
            break;
        }

        printk(KERN_INFO "[CTRL][NET] Client connected\n");

        // Initialize the msghdr structure
        memset(&msg, 0, sizeof(msg));
        msg.msg_name = &client_addr;
        msg.msg_namelen = client_addr_len;

        // Receive client's message
        char *client_message = kmalloc(2000, GFP_KERNEL);
        if (!client_message) 
        {
            printk(KERN_ERR "[CTRL][NET] Failed to allocate memory for client message\n");
            sock_release(client_socket);
            return -ENOMEM;
        }
        else
        {
            printk(KERN_ERR "[CTRL][NET] Memory allocated for client message\n");
        }

        while (!message_received && !kthread_should_stop()) 
        {
            iov.iov_base = client_message;
            iov.iov_len = 2000;
            ret = kernel_recvmsg(client_socket, &msg, &iov, 1, 2000, MSG_WAITALL);
            if (ret < 0) 
            {
                printk(KERN_ERR "[CTRL][NET] Error receiving message from client: %d\n", ret);
                kfree(client_message);
                sock_release(client_socket);
                return ret;
            }
            printk(KERN_INFO "[CTRL][NET] TODO :: Message from client: %s\n", client_message);
            message_received = true;
        }

#if 1 /* TODO :: Need synchronisation */
        // Prepare to send response
        memset(&msg, 0, sizeof(msg));
        iov.iov_base = response_message;
        iov.iov_len = strlen(response_message);
        msg.msg_name = &client_addr;
        msg.msg_namelen = client_addr_len;

        // Send response message to client
        ret = kernel_sendmsg(client_socket, &msg, &iov, 1, strlen(response_message));
        if (ret < 0) 
        {
            printk(KERN_ERR "[CTRL][NET] Error sending message to client: %d\n", ret);
        } 
        else 
        {
            printk(KERN_INFO "[CTRL][NET] Sent response to client: %s\n", response_message);
        }
#endif

        kfree(client_message);

        // Close the client socket
        sock_release(client_socket);
    }
    
    printk(KERN_INFO "[CTRL][NET] Stopping server_kthread\n");
    return 0;
}

int tcpServerInit(void) 
{
    struct sockaddr_in server_addr;
    int error;

    // Create socket
    error = sock_create_kern(&init_net, AF_INET, SOCK_STREAM, IPPROTO_TCP, &server_socket);
    if (error < 0) 
    {
        printk(KERN_ERR "[INIT][NET] Error while creating socket: %d\n", error);
        return error;
    }
    else
    {
        printk(KERN_INFO "[INIT][NET] Socket created successfully\n");
    }

    // Set server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = in_aton(server_ip);
    server_addr.sin_port = htons(port);

    // Bind socket
    error = kernel_bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (error < 0) 
    {
        printk(KERN_ERR "[INIT][NET] Couldn't bind to the port: %d\n", error);
        sock_release(server_socket);
        return error;
    }
    else
    {
        printk(KERN_INFO "[INIT][NET] Port %d binded successfully\n", port);
    }

    // Listen for clients
    error = kernel_listen(server_socket, 1);
    if (error < 0) 
    {
        printk(KERN_ERR "[INIT][NET] Error while listening: %d\n", error);
        sock_release(server_socket);
        return error;
    }
    else
    {
        printk(KERN_INFO "[INIT][NET] Listening for incoming connections...\n");
    }

    // Create the kernel thread
    server_kthread = kthread_create(server_kthread_function, NULL, "server_kthread");
    if (IS_ERR(server_kthread)) 
    {
        printk(KERN_INFO "[INIT][NET] Failed to create server_kthread\n");
        return PTR_ERR(server_kthread);
    }

    // Start the thread
    wake_up_process(server_kthread);
    printk(KERN_INFO "[INIT][NET] TCP server_kthread created and started\n");

    return 0;
}

void tcpServerDestroy(void) 
{
    if (server_socket != NULL) 
    {
        sock_release(server_socket);
        printk(KERN_INFO "[DESTROY][NET] Server socket released\n");
    }

    if (server_kthread) 
    {
        // Stop the thread
        kthread_stop(server_kthread);
        printk(KERN_INFO "[DESTROY][NET] server_kthread stopped\n");
    }
}
