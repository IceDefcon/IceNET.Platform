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

class UartManager:
    def __init__(self, root, log_function):
        self.root = root
        self.serial_port = None
        self.log_message = log_function  # Store the log function

        # Create and place the dropdown menu
        self.uart_combobox = ttk.Combobox(root, values=self.get_uart_ports())
        self.uart_combobox.grid(row=9, column=1, pady=5, padx=5, sticky='nsew')

        # Create and place the CONNECT button
        uart_open = tk.Button(root, text="OPEN", command=self.serial_connect)
        uart_open.grid(row=9, column=0, pady=5, padx=5, sticky='nsew')

        # Create and place the CLOSE button
        uart_close = tk.Button(root, text="CLOSE", command=self.serial_disconnect)
        uart_close.grid(row=9, column=2, pady=5, padx=5, sticky='nsew')

        # Create and place the UART display
        self.uart_display = tk.Text(root, width=150, height=12, state=tk.DISABLED)
        self.uart_display.grid(row=10, column=0, columnspan=100, pady=5, padx=5, sticky='w')

    def get_uart_ports(self):
        """Get a list of available UART ports."""
        ports = serial.tools.list_ports.comports()
        return [port.device for port in ports]

    def serial_connect(self):
        """Connect to the selected UART port."""
        selected_port = self.uart_combobox.get()

        try:
            self.serial_port = serial.Serial(selected_port, baudrate=921600, timeout=1)  # Adjust as needed
            threading.Thread(target=self.read_uart, daemon=True).start()  # Start reading in a new thread
            self.log_message(f"Connected to {selected_port}")
        except Exception as e:
            self.log_message(f"Error connecting to {selected_port}: {e}")

    def serial_disconnect(self):
        """Disconnect from the UART port."""
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()
            self.log_message(f"Disconnected from {self.serial_port.name}")
            self.serial_port = None  # Reset the variable

    def read_uart(self):
        """Continuously read data from the UART port and display it."""
        if self.serial_port is None:
            return  # Exit if not connected

        try:
            while True:
                if self.serial_port.in_waiting > 0:
                    data = self.serial_port.read(self.serial_port.in_waiting).decode('utf-8', errors='ignore')  # Read available data
                    self.log_uart_data(data)  # Call to update the display
        except Exception as e:
            self.log_message(f"Error reading from UART: {e}")

    def log_uart_data(self, data):
        """Log the UART data to the display."""
        self.uart_display.config(state=tk.NORMAL)
        self.uart_display.insert(tk.END, data)  # Insert new data at the end
        self.uart_display.config(state=tk.DISABLED)
        self.uart_display.see(tk.END)  # Scroll to the end
