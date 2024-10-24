# Author: Ice.Marek
# IceNET Technology 2024

import tkinter as tk
from tkinter import ttk

import serial
import serial.tools.list_ports
import threading

class UartManager:
    def __init__(self, root):
        self.root = root
        self.serial_port = None

        self.uart_open = tk.Button(self.root, text="OPEN", command=self.serial_connect)
        self.uart_open.grid(row=9, column=0, pady=5, padx=5, sticky='nsew')
        self.uart_close = tk.Button(self.root, text="CLOSE", command=self.serial_disconnect)
        self.uart_close.grid(row=9, column=1, pady=5, padx=5, sticky='nsew')
        # Initialize the UART Combobox with available ports
        self.uart_combobox = ttk.Combobox(self.root, values=self.get_uart_ports(), width=14)
        self.uart_combobox.grid(row=9, column=2, pady=5, padx=5, sticky='nsew')
        self.refresh_button = tk.Button(self.root, text="REFRESH", command=self.refresh_uart_ports)
        self.refresh_button.grid(row=9, column=3, pady=5, padx=5, sticky='nsew')

        # Set the default value for the combobox if available
        available_ports = self.get_uart_ports()
        if '/dev/ttyUSB0' in available_ports:
            self.uart_combobox.set('/dev/ttyUSB0')  # Set the default value
        elif available_ports:
            self.uart_combobox.set(available_ports[0])  # Set to first available port
        else:
            self.uart_combobox.set('')  # No ports available

        # Create a message entry box
        self.message_entry = tk.Entry(self.root, width=50)
        self.message_entry.grid(row=11, column=0, columnspan=100, pady=5, padx=5, sticky='nsew')

        # Bind the Enter key to send the message
        self.message_entry.bind('<Return>', self.send_message)  # Bind Enter key

        self.uart_display = tk.Text(self.root, width=150, height=12, state=tk.DISABLED)
        self.uart_display.grid(row=10, column=0, columnspan=100, pady=5, padx=5, sticky='w')

    def get_uart_ports(self):
        ports = serial.tools.list_ports.comports()
        return [port.device for port in ports]

    def serial_connect(self):
        selected_port = self.uart_combobox.get()
        try:
            self.serial_port = serial.Serial(selected_port, baudrate=921600, timeout=1)
            threading.Thread(target=self.read_uart, daemon=True).start()
            self.uart_console(f"Connected to {selected_port}\r\n")
        except Exception as e:
            self.uart_console(f"Error connecting to {selected_port}: {e}\r\n")

    def serial_disconnect(self):
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()
            self.uart_console(f"Disconnected from {self.serial_port.name}\r\n")
            self.serial_port = None

    def read_uart(self):
        if self.serial_port is None:
            return

        try:
            while True:
                if self.serial_port.in_waiting > 0:
                    data = self.serial_port.read(self.serial_port.in_waiting).decode('utf-8', errors='ignore')
                    self.uart_console(data)
        except Exception as e:
            self.uart_console(f"Error reading from UART: {e}\r\n")

    def refresh_uart_ports(self):
        self.uart_combobox['values'] = self.get_uart_ports()
        available_ports = self.get_uart_ports()
        if '/dev/ttyUSB0' in available_ports:
            self.uart_combobox.set('/dev/ttyUSB0')  # Set the default value
        elif available_ports:
            self.uart_combobox.set(available_ports[0])  # Set to first available port
        else:
            self.uart_combobox.set('')  # No ports available

    def send_message(self, event=None):  # Allow for an event parameter for binding
        if self.serial_port and self.serial_port.is_open:
            message = self.message_entry.get()
            if message:
                self.serial_port.write(message.encode('utf-8'))  # Send message over UART
                self.uart_console(f"Sent: {message}\r\n")
                self.message_entry.delete(0, tk.END)  # Clear the entry box after sending
            else:
                self.uart_console("Message cannot be empty.\r\n")
        else:
            self.uart_console("No UART connection established.\r\n")

    def uart_console(self, data):
        self.uart_display.config(state=tk.NORMAL)
        self.uart_display.insert(tk.END, data)
        self.uart_display.config(state=tk.DISABLED)
        self.uart_display.see(tk.END)  # Scroll to the end
