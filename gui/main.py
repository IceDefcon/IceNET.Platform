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
# SEPARATORS
################################################################################################################################################

# Horizontal separator
vertical1 = ttk.Separator(root, orient='vertical')
vertical1.grid(row=0, column=3, rowspan=8, sticky='nsew')
vertical2 = ttk.Separator(root, orient='vertical')
vertical2.grid(row=0, column=7, rowspan=8, sticky='nsew')

horizontal1 = ttk.Separator(root, orient='horizontal')
horizontal1.grid(row=3, column=0, columnspan=3, sticky='nsew')
horizontal2 = ttk.Separator(root, orient='horizontal')
horizontal2.grid(row=3, column=3, columnspan=3, sticky='nsew')
horizontal3 = ttk.Separator(root, orient='horizontal')
horizontal3.grid(row=3, column=6, columnspan=93, sticky='nsew')

################################################################################################################################################
# TCP
################################################################################################################################################
tcp_manager = TcpManager(root)

################################################################################################################################################
# UART
################################################################################################################################################
# uart_manager = UartManager(root)

# Start the Tkinter main loop
root.mainloop()
