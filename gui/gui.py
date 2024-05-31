import tkinter as tk
import socket

def log_message(message):
    feedback_display.config(state=tk.NORMAL)
    feedback_display.insert(tk.END, message + "\n")
    feedback_display.config(state=tk.DISABLED)
    feedback_display.see(tk.END)  # Scroll to the end of the Text widget

def send_data():
    tcp_socket = None
    try:
        # Create a TCP socket
        tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        
        # Connect to the server
        server_address = ip_data.get()
        port = int(port_data.get())
        tcp_socket.connect((server_address, port))
        log_message("[iceNET] Server connection established")

        # Get data from the entry field
        data = entry_data.get()
        
        # Check if the input data exceeds 8 ASCII characters
        if len(data) > 8:
            log_message("[iceNET] Error: Data length exceeds 8 ASCII characters")
            return
        
        # Send data over TCP
        tcp_socket.sendall(data.encode())
        log_message(f"[iceNET] Client TX :: {data}")
        
        # Receive feedback data from the server
        feedback_data = tcp_socket.recv(1024)
        feedback_text = feedback_data.decode()
        log_message(f"[iceNET] Client RX :: {feedback_text}")

    except Exception as e:
        log_message(f"[iceNET] Error sending/receiving data over TCP: {e}")
    
    finally:
        # Close the TCP connection
        if tcp_socket:
            tcp_socket.close()
            log_message("[iceNET] Server connection terminated")

def quit_application():
    log_message("[iceNET] Quitting application...")
    root.quit()
    root.destroy()

# Create the main application window
root = tk.Tk()
root.title("TCP Client")

# Set the window size
root.geometry("1000x500")

quit_button = tk.Button(root, text="QUIT", command=quit_application, width=16)
quit_button.grid(row=0, column=0, pady=5, padx=5, sticky='w')

ip_label = tk.Label(root, text="Server IP Address")
ip_label.grid(row=1, column=0, pady=5, padx=5, sticky='e')
ip_data = tk.Entry(root, width=16)
ip_data.grid(row=1, column=1, pady=5, padx=5, sticky='w')
ip_data.insert(0, "10.0.0.2")

port_label = tk.Label(root, text="Server Port")
port_label.grid(row=2, column=0, pady=5, padx=5, sticky='e')
port_data = tk.Entry(root, width=16)
port_data.grid(row=2, column=1, pady=5, padx=5, sticky='w')
port_data.insert(0, "2555")

device_label = tk.Label(root, text="I2C Device Address")
device_label.grid(row=3, column=0, pady=5, padx=5, sticky='e')
device_address = tk.Entry(root, width=16)
device_address.grid(row=3, column=1, pady=5, padx=5, sticky='w')
device_address.insert(0, "69")

device_register_label = tk.Label(root, text="I2C Register Address")
device_register_label.grid(row=4, column=0, pady=5, padx=5, sticky='e')
device_register = tk.Entry(root, width=16)
device_register.grid(row=4, column=1, pady=5, padx=5, sticky='w')
device_register.insert(0, "00")

register_data_label = tk.Label(root, text="I2C Register Data")
register_data_label.grid(row=5, column=0, pady=5, padx=5, sticky='e')
register_data = tk.Entry(root, width=16)
register_data.grid(row=5, column=1, pady=5, padx=5, sticky='w')
register_data.insert(0, "00")

command_button = tk.Button(root, text="Command", command=send_data, width=16)
command_button.grid(row=6, column=0, pady=5, padx=5, sticky='w')
entry_data = tk.Entry(root, width=16)
entry_data.grid(row=6, column=1, pady=5, padx=5, sticky='w')

feedback_display = tk.Text(root, width=100, height=12, state=tk.DISABLED)
feedback_display.grid(row=7, column=1, pady=5, padx=5, sticky='w')

# Start the GUI event loop
root.mainloop()
