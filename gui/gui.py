import tkinter as tk
import socket

# Global variable to hold the socket object
tcp_socket = None

def send_data():
    global tcp_socket
    try:
        # Connect to the server if not already connected
        if not tcp_socket:
            # Create a TCP socket
            tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            # Connect to the server
            server_address = ip_data.get()  # Get IP address from entry widget
            port = int(port_data.get())  # Get port from entry widget and convert to integer
            tcp_socket.connect((server_address, port))
            print("[iceNET] Connected to TCP server")

        # Get data from the entry field
        data = entry_data.get()
        
        # Check if the input data exceeds 8 ASCII characters
        if len(data) > 8:
            print("[iceNET] Error: Data length exceeds 8 ASCII characters")
            return
        
        # Send data over TCP
        tcp_socket.sendall(data.encode())  # Assuming sending ASCII characters directly
        print("[iceNET] Sent data over TCP:", data)
        
        # Receive feedback data from the server
        feedback_data = tcp_socket.recv(1024)  # Adjust the buffer size as per your requirement
        feedback_text = feedback_data.decode()  # Decode the received data
        print("[iceNET] Received feedback data:", feedback_text)

        # Display feedback data in the Text widget
        feedback_display.config(state=tk.NORMAL)
        feedback_display.delete(1.0, tk.END)
        feedback_display.insert(tk.END, feedback_text)
        feedback_display.config(state=tk.DISABLED)

    except Exception as e:
        print("[iceNET] Error sending/receiving data over TCP:", e)
    
    finally:
        # Close the TCP connection
        if tcp_socket:
            tcp_socket.close()
            print("[iceNET] Disconnected from TCP server")
            tcp_socket = None

# Create the main application window
root = tk.Tk()
root.title("TCP Client")

# Set the window size to a fixed size of 750x300 to accommodate the feedback display
root.geometry("750x300")

ip_label = tk.Label(root, text="Server IP Address:")
ip_label.grid(row=0, column=0, pady=5, padx=5, sticky='e')
ip_data = tk.Entry(root, width=16)
ip_data.grid(row=0, column=1, pady=5, padx=5, sticky='w')
ip_data.insert(0, "10.0.0.2")  # Set default IP address

port_label = tk.Label(root, text="Server Port:")
port_label.grid(row=1, column=0, pady=5, padx=5, sticky='e')
port_data = tk.Entry(root, width=16)
port_data.grid(row=1, column=1, pady=5, padx=5, sticky='w')
port_data.insert(0, "2555")  # Set default port

send_button = tk.Button(root, text="SEND", command=send_data, width=16)
send_button.grid(row=2, column=0, pady=5, padx=5, sticky='w')
entry_data = tk.Entry(root, width=16)
entry_data.grid(row=2, column=1, pady=5, padx=5, sticky='w')

# Create a Text widget to display feedback from the server
feedback_label = tk.Label(root, text="Server Feedback:")
feedback_label.grid(row=3, column=0, pady=5, padx=5, sticky='nw')
feedback_display = tk.Text(root, width=64, height=4, state=tk.DISABLED)
feedback_display.grid(row=3, column=1, pady=5, padx=5, sticky='w')

# Start the GUI event loop
root.mainloop()
