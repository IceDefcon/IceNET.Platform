# Author: Ice.Marek
# IceNET Technology 2024

import tkinter as tk
from tkinter import ttk
from tkinter import font

import socket
import time
import threading
import serial
import serial.tools.list_ports

from uart import UartManager
from tcp import TcpManager

################################################################################################################################################
# MAIN
################################################################################################################################################

def quit_application():
    tcp_console("[iceNET] Quitting application...")
    root.quit()
    root.destroy()

def kill_application():
    # Create a TCP socket
    tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_address = ip_data.get()
    port = int(port_data.get())
    tcp_socket.connect((server_address, port))
    tcp_console("[iceNET] Killing Linux Application")
    data = bytes([0xDE, 0xAD])
    tcp_socket.sendall(data)

def tcp_console(message):
    tcp_display.config(state=tk.NORMAL)
    tcp_display.insert(tk.END, message + "\n")
    tcp_display.config(state=tk.DISABLED)
    tcp_display.see(tk.END)

# Create the main application window
root = tk.Tk()
root.title("TCP Client")
# Adjust the window size depending on the widgets used
root.update_idletasks()

# Quit and Kill
quit_button = tk.Button(root, text="QUIT", command=quit_application, width=12)
quit_button.grid(row=0, column=0, pady=5, padx=5, sticky='w')
kill_button = tk.Button(root, text="KILL APP", command=kill_application, width=14)
kill_button.grid(row=0, column=1, pady=5, padx=5, sticky='w')

# IP and Port
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

################################################################################################################################################
# SEPARATORS
################################################################################################################################################

# Horizontal separator
separator1 = ttk.Separator(root, orient='horizontal')
separator1.grid(row=3, column=0, columnspan=100, sticky='ew')

# Vertical Separators
separator2 = ttk.Separator(root, orient='vertical')
separator2.grid(row=0, column=3, rowspan=8, sticky='ns')

# Vertical Separators
separator3 = ttk.Separator(root, orient='vertical')
separator3.grid(row=0, column=7, rowspan=8, sticky='ns')

################################################################################################################################################
# TCP
################################################################################################################################################



tcp_display = tk.Text(root, width=150, height=12, state=tk.DISABLED)
tcp_display.grid(row=8, column=0, columnspan=100, pady=5, padx=5, sticky='w')

################################################################################################################################################
# TCP
################################################################################################################################################
tcp_manager = TcpManager(root, tcp_console, ip_data, port_data)

################################################################################################################################################
# UART
################################################################################################################################################
uart_manager = UartManager(root, tcp_console)

# Start the Tkinter main loop
root.mainloop()
