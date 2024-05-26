#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h> // Include kthread header file
#include <linux/net.h>
#include <linux/tcp.h>
#include <net/tcp.h>
#include <linux/delay.h> // For msleep

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ice Marek");
MODULE_DESCRIPTION("TCP Server");

static DEFINE_MUTEX(accept_mutex); // Define a mutex

static struct task_struct *accept_thread; // Define the accept thread

static struct socket *server_socket = NULL;

// Function to handle client connection
static void handle_client_connection(struct socket *client_socket) 
{
    // Add your logic to handle the client connection here
    // For example, you can receive data from the client and send responses back
}

// Function executed by the kthread
static int accept_thread_func(void *data)
{
    struct socket *client_socket = NULL;
    int ret;

    while (!kthread_should_stop()) // Loop until the thread is stopped
    {
        /**
         * 
         * O_NONBLOCK :: should return immediately if there are no pending connections
         * true ::  indicating whether the socket is blocking or non-blocking
         * 
         */
        ret = server_socket->ops->accept(server_socket, client_socket, O_NONBLOCK, true); // Pass address of client_socket

        if (ret < 0) 
        {
            if (ret != -EAGAIN && ret != -EWOULDBLOCK) // Ignore non-blocking errors
            {
                printk(KERN_ERR "Failed to accept incoming connection\n");
                break; // Break the loop on any error other than non-blocking
            }
        } 
        else 
        {
            // Handle the accepted connection
            handle_client_connection(client_socket);
        }

        // msleep(10); /* Release 90% of CPU resources */ 
    }

    mutex_unlock(&accept_mutex); // Unlock the mutex before thread exit

    return 0;
}

static int __init tcp_server_init(void) 
{
    struct sockaddr_in addr;
    struct socket *client_socket = NULL;
    int ret;

    ret = sock_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, &server_socket);
    if (ret < 0) 
    {
        printk(KERN_ERR "Failed to create socket\n");
        return ret;
    }

    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    // addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_addr.s_addr = htonl(0x0A000002); // 10.0.0.2

    ret = server_socket->ops->bind(server_socket, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    if (ret < 0) 
    {
        printk(KERN_ERR "Failed to bind socket\n");

        if (server_socket) 
        {
            sock_release(server_socket);
            printk(KERN_INFO "TCP server socket released at binding problem\n");
        }

        return ret;
    }

    ret = server_socket->ops->listen(server_socket, 5);
    if (ret < 0) 
    {
        printk(KERN_ERR "Failed to listen on socket\n");
        return ret;
    }

    printk(KERN_INFO "TCP server initialized\n");

    mutex_init(&accept_mutex);
    
    // Create the kthread for accepting connections
    accept_thread = kthread_run(accept_thread_func, NULL, "accept_thread");
    if (IS_ERR(accept_thread)) 
    {
        printk(KERN_ERR "Failed to create accept thread\n");
        return PTR_ERR(accept_thread);
    }

    return 0;
}

static void __exit tcp_server_exit(void) 
{
    if (server_socket) 
    {
        sock_release(server_socket);
        printk(KERN_INFO "TCP server socket released\n");
    }

    // Stop and free the accept thread
    if (accept_thread) 
    {
        kthread_stop(accept_thread);

        // Lock the mutex to ensure the thread has completed before cleanup
        mutex_lock(&accept_mutex);
        mutex_unlock(&accept_mutex);
    }

    // Destroy the mutex
    mutex_destroy(&accept_mutex);
}

module_init(tcp_server_init);
module_exit(tcp_server_exit);
