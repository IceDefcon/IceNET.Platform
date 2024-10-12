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

class TcpManager:
    def __init__(self, root, log_function):
        self.root = root
        self.log_function = log_function

        self.ip_label = tk.Label(self.root, text="Server IP Address")
        self.ip_label.grid(row=1, column=0, pady=5, padx=5, sticky='e')
