#
# Author: Ice.Marek
# IceNET Technology 2024
#
import tkinter as tk
import socket
import time

def log_message(message):
    log_display.config(state=tk.NORMAL)
    log_display.insert(tk.END, message + "\n")
    log_display.config(state=tk.DISABLED)
    log_display.see(tk.END)  # Scroll to the end of the Text widget

def kill_application():
    # Create a TCP socket
    tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    # Connect to the server
    server_address = ip_data.get()
    port = int(port_data.get())
    tcp_socket.connect((server_address, port))
    log_message("[iceNET] Killing Linux Application")

    data = bytes([0xDE, 0xAD])
    tcp_socket.sendall(data)

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

        address = int(device_address.get(), 16)  # Convert hex address to integer
        register = int(device_register.get(), 16)  # Convert hex register to integer
        
        if write_var.get():
            data = bytes([0x01, address, register]) + bytes.fromhex(register_data.get())
        else:
            data = bytes([0x00, address, register, 0x00])

        # Send data over TCP
        tcp_socket.sendall(data)
        log_message(f"[iceNET] Client TX :: {data.hex()}")  # Log the hexadecimal representation of the data
        
        # Receive feedback data from the server
        while True:
            feedback_data = tcp_socket.recv(4)
            if feedback_data == b'\xFF':
                time.sleep(0.1)
                continue  # Read again if data is 0xFF
            else:
                break  # Exit loop if data is not 0xFF

        #
        # Dont use decode function when non ASCII hex is send over TCP
        #
        # feedback_text = feedback_data.decode()
        # log_message(f"[iceNET] Client RX :: {feedback_text}")
        log_message(f"[iceNET] Client RX :: {feedback_data.hex()}")  # Log the hexadecimal representation of the received data

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

def toggle_write_data_entry():
    if write_var.get():
        register_data.config(state=tk.NORMAL)
    else:
        register_data.config(state=tk.DISABLED)

# Create the main application window
root = tk.Tk()
root.title("TCP Client")

# Set the window size
root.geometry("850x500")

quit_button = tk.Button(root, text="QUIT", command=quit_application, width=12)
quit_button.grid(row=0, column=0, pady=5, padx=5, sticky='w')

send_button = tk.Button(root, text="OFFLOAD", command=send_data, width=14)
send_button.grid(row=0, column=1, pady=5, padx=5, sticky='w')

kill_button = tk.Button(root, text="KILL APP", command=kill_application, width=14)
kill_button.grid(row=0, column=2, pady=5, padx=5, sticky='w')

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

device_label = tk.Label(root, text="Device Address")
device_label.grid(row=3, column=0, pady=5, padx=5, sticky='e')
device_address = tk.Entry(root, width=16)
device_address.grid(row=3, column=1, pady=5, padx=5, sticky='w')
device_address.insert(0, "69")

device_register_label = tk.Label(root, text="Register Address")
device_register_label.grid(row=4, column=0, pady=5, padx=5, sticky='e')
device_register = tk.Entry(root, width=16)
device_register.grid(row=4, column=1, pady=5, padx=5, sticky='w')
device_register.insert(0, "00")

register_data_label = tk.Label(root, text="Write Data")
register_data_label.grid(row=5, column=0, pady=5, padx=5, sticky='e')
register_data = tk.Entry(root, width=16)
register_data.grid(row=5, column=1, pady=5, padx=5, sticky='w')
register_data.insert(0, "00")
register_data.config(state=tk.DISABLED) # Initialize "Write Data" entry as disabled

# Add a tick box (Checkbutton)
write_var = tk.BooleanVar()
write_box = tk.Checkbutton(root, text="Write", variable=write_var, command=toggle_write_data_entry)
write_box.grid(row=5, column=2, pady=5, padx=5, sticky='w')

log_display = tk.Text(root, width=100, height=12, state=tk.DISABLED)
log_display.grid(row=7, column=0, columnspan=100, pady=5, padx=5, sticky='w')

# Start the GUI event loop
root.mainloop()
