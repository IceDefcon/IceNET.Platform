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
root.title("TCP Client")
root.update_idletasks() # Adjust the window size depending on the widgets used

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
tcp_manager = TcpManager(root)

################################################################################################################################################
# UART
################################################################################################################################################
uart_manager = UartManager(root)

# Start the Tkinter main loop
root.mainloop()
