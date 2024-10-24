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

root = tk.Tk()
root.title("iceNET Platform")
root.update_idletasks() # Adjust the window size depending on the widgets used

################################################################################################################################################
# MAIN
################################################################################################################################################

# Create the Notebook (tab interface)
notebook = ttk.Notebook(root)
notebook.grid(row=0, column=0, sticky='nsew')

# Create frames for UART and TCP tabs
uart_tab = ttk.Frame(notebook)
tcp_tab = ttk.Frame(notebook)

# Add the tabs to the notebook
notebook.add(tcp_tab, text="TCP")
notebook.add(uart_tab, text="UART")

################################################################################################################################################
# TCP
################################################################################################################################################
tcp_manager = TcpManager(tcp_tab)

################################################################################################################################################
# UART
################################################################################################################################################
uart_manager = UartManager(uart_tab)

# Start the Tkinter main loop
root.mainloop()
