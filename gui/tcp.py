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
    def __init__(self, root):
        self.root = root
        # IP and Port
        self.tcp_ip_label = tk.Label(self.root, text="Server IP Address")
        self.tcp_ip_label.grid(row=1, column=0, pady=5, padx=5, sticky='e')
        self.tcp_ip = tk.Entry(self.root, width=14)
        self.tcp_ip.grid(row=1, column=1, pady=5, padx=5, sticky='w')
        self.tcp_ip.insert(0, "192.168.6.2")
        self.tcp_port_label = tk.Label(self.root, text="Server Port")
        self.tcp_port_label.grid(row=2, column=0, pady=5, padx=5, sticky='e')
        self.tcp_port = tk.Entry(self.root, width=14)
        self.tcp_port.grid(row=2, column=1, pady=5, padx=5, sticky='w')
        self.tcp_port.insert(0, "2555")
        self.tcp_socket = None
        # SEPARATORS
        self.vertical1 = ttk.Separator(self.root, orient='vertical')
        self.vertical1.grid(row=0, column=3, rowspan=8, sticky='nsew')
        self.vertical2 = ttk.Separator(self.root, orient='vertical')
        self.vertical2.grid(row=0, column=7, rowspan=8, sticky='nsew')
        self.horizontal1 = ttk.Separator(self.root, orient='horizontal')
        self.horizontal1.grid(row=3, column=0, columnspan=3, sticky='nsew')
        self.horizontal2 = ttk.Separator(self.root, orient='horizontal')
        self.horizontal2.grid(row=3, column=3, columnspan=3, sticky='nsew')
        self.horizontal3 = ttk.Separator(self.root, orient='horizontal')
        self.horizontal3.grid(row=3, column=6, columnspan=93, sticky='nsew')
        # Connection
        self.connect_button = tk.Button(self.root, text="CONNECT", command=self.connect_to_server)
        self.connect_button.grid(row=0, column=0, pady=5, padx=5, sticky='e')
        self.disconnect_button = tk.Button(self.root, text="DISCONNECT", command=self.disconnect_from_server)
        self.disconnect_button.grid(row=0, column=1, pady=5, padx=5, sticky='w')
        self.disconnect_button = tk.Button(self.root, text="KILL", command=self.kill_application)
        self.disconnect_button.grid(row=0, column=2, pady=5, padx=5, sticky='w')
        #################
        # tcp_execute   #
        #################
        # 0 :: I2C      #
        # 1 :: PWM box  #
        # 2 :: PWM Up   #
        # 3 :: PWM Down #
        # 4 :: PWM %    #
        # 5 :: PWM 50%  #
        # 6 :: PWM 100% #
        # 7 :: SPI      #
        #################
        # I2C
        self.i2c_device_label = tk.Label(self.root, text="I2C Device Address")
        self.i2c_device_label.grid(row=5, column=0, pady=5, padx=5, sticky='e')
        self.i2c_device_address = tk.Entry(self.root, width=14)
        self.i2c_device_address.grid(row=5, column=1, pady=5, padx=5, sticky='w')
        self.i2c_device_address.insert(0, "69")
        self.i2c_exe_button = tk.Button(self.root, text="EXE", command=lambda: self.tcp_execute(0))
        self.i2c_exe_button.grid(row=5, column=2, pady=5, padx=5, sticky='w')
        self.i2c_device_register_label = tk.Label(self.root, text="Register Address")
        self.i2c_device_register_label.grid(row=6, column=0, pady=5, padx=5, sticky='e')
        self.i2c_device_register = tk.Entry(self.root, width=14)
        self.i2c_device_register.grid(row=6, column=1, pady=5, padx=5, sticky='w')
        self.i2c_device_register.insert(0, "00")
        self.i2c_register_data_label = tk.Label(self.root, text="Write Data")
        self.i2c_register_data_label.grid(row=7, column=0, pady=5, padx=5, sticky='e')
        self.i2c_register_data = tk.Entry(self.root, width=14)
        self.i2c_register_data.grid(row=7, column=1, pady=5, padx=5, sticky='w')
        self.i2c_register_data.insert(0, "00")
        self.i2c_register_data.config(state=tk.DISABLED)
        self.i2c_write_var = tk.BooleanVar()
        self.i2c_write_box = tk.Checkbutton(self.root, text="Write", variable=self.i2c_write_var, command=self.i2c_toggle_write_data_entry)
        self.i2c_write_box.grid(row=7, column=2, pady=5, padx=5, sticky='w')
        # PWM
        self.pwm_speed_label = tk.Label(root, text="PWM Speed [Hex]")
        self.pwm_speed_label.grid(row=5, column=3, pady=5, padx=5, sticky='e')
        self.pwm_speed = tk.Entry(self.root, width=14)
        self.pwm_speed.grid(row=5, column=4, pady=5, padx=5, sticky='w')
        self.pwm_speed.insert(0, "00")
        self.pwm_exe_button = tk.Button(self.root, text="EXE", command=lambda: self.tcp_execute(1))
        self.pwm_exe_button.grid(row=5, column=5, pady=5, padx=5, sticky='nsew')
        self.pwm_up_button = tk.Button(self.root, text=" ▲ ", command=lambda: self.tcp_execute(2))
        self.pwm_up_button.grid(row=6, column=5, pady=5, padx=5, sticky='nsew')
        self.pwm_down_button = tk.Button(self.root, text=" ▼ ", command=lambda: self.tcp_execute(3))
        self.pwm_down_button.grid(row=7, column=5, pady=5, padx=5, sticky='nsew')
        self.pwm_stop_button = tk.Button(self.root, text="0%", command=lambda: self.tcp_execute(4))
        self.pwm_stop_button.grid(row=5, column=7, pady=5, padx=5, sticky='nsew')
        self.pwm_50_button = tk.Button(self.root, text="50%", command=lambda: self.tcp_execute(5))
        self.pwm_50_button.grid(row=6, column=7, pady=5, padx=5, sticky='nsew')
        self.pwm_100_button = tk.Button(self.root, text="100%", command=lambda: self.tcp_execute(6))
        self.pwm_100_button.grid(row=7, column=7, pady=5, padx=5, sticky='nsew')
        # SPI
        self.spi_device_label = tk.Label(self.root, text="SPI Register Address")
        self.spi_device_label.grid(row=0, column=3, pady=5, padx=5, sticky='e')
        self.spi_device_address = tk.Entry(self.root, width=14)
        self.spi_device_address.grid(row=0, column=4, pady=5, padx=5, sticky='w')
        self.spi_device_address.insert(0, "10")
        self.spi_exe_button = tk.Button(self.root, text="EXE", command=lambda: self.tcp_execute(7))
        self.spi_exe_button.grid(row=0, column=5, pady=5, padx=5, sticky='nsew')
        self.spi_device_register_label = tk.Label(self.root, text="Bytes to Read")
        self.spi_device_register_label.grid(row=1, column=3, pady=5, padx=5, sticky='e')
        self.spi_device_register = tk.Entry(self.root, width=14)
        self.spi_device_register.grid(row=1, column=4, pady=5, padx=5, sticky='w')
        self.spi_device_register.insert(0, "01")
        self.spi_register_data_label = tk.Label(self.root, text="Write Data")
        self.spi_register_data_label.grid(row=2, column=3, pady=5, padx=5, sticky='e')
        self.spi_register_data = tk.Entry(self.root, width=14)
        self.spi_register_data.grid(row=2, column=4, pady=5, padx=5, sticky='w')
        self.spi_register_data.insert(0, "00")
        self.spi_register_data.config(state=tk.DISABLED) # Initialize "Write Data" entry as disabled
        self.spi_write_var = tk.BooleanVar() # Add a tick box (Checkbutton)
        self.spi_write_box = tk.Checkbutton(self.root, text="Write", variable=self.spi_write_var, command=self.spi_toggle_write_data_entry)
        self.spi_write_box.grid(row=2, column=5, pady=5, padx=5, sticky='w')
        # CTRL
        self.assembly_label = tk.Label(self.root, text="Assembly")
        self.assembly_label.grid(row=0, column=8, pady=5, padx=5, sticky='e')
        self.assembly_button = tk.Button(self.root, text="EXE", command=self.assemblyData)
        self.assembly_button.grid(row=0, column=9, pady=5, padx=5, sticky='nsew')
        self.initialise_label = tk.Label(self.root, text="Initialise")
        self.initialise_label.grid(row=1, column=8, pady=5, padx=5, sticky='e')
        self.initialise_button = tk.Button(self.root, text="EXE", command=self.initPointers)
        self.initialise_button.grid(row=1, column=9, pady=5, padx=5, sticky='nsew')
        self.load_label = tk.Label(self.root, text="Load")
        self.load_label.grid(row=2, column=8, pady=5, padx=5, sticky='e')
        self.load_button = tk.Button(self.root, text="EXE", command=self.loadRam)
        self.load_button.grid(row=2, column=9, pady=5, padx=5, sticky='nsew')

        # Console
        self.tcp_display = tk.Text(self.root, width=150, height=12, state=tk.DISABLED)
        self.tcp_display.grid(row=8, column=0, columnspan=100, pady=5, padx=5, sticky='w')

    # Connect
    def connect_to_server(self):
        try:
            # Retrieve IP and Port
            server_address = self.tcp_ip.get()
            port = int(self.tcp_port.get())
            # Create and connect the TCP socket
            self.tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.tcp_socket.settimeout(1.0)
            self.tcp_socket.connect((server_address, port))
            self.tcp_console("[iceNET] Server connection established")

        except Exception as e:
            self.tcp_console(f"[iceNET] Server is Down: {e}")
            self.tcp_socket = None

    # Disconnect
    def disconnect_from_server(self):
        try:
            self.tcp_socket.close()
            self.tcp_console("[iceNET] Connection terminated")
            self.tcp_socket = None
        except Exception as e:
            self.tcp_console(f"[iceNET] Server is Down: {e}")
            self.tcp_socket = None

    # Debug Kill
    def kill_application(self):
        try:
            data = bytes([0xDE, 0xAD, 0xC0, 0xDE, 0x00, 0x00, 0x00, 0x00])
            self.tcp_socket.sendall(data)
            self.tcp_console("[iceNET] Kill Linux Application")
        except Exception as e:
            self.tcp_console(f"[iceNET] Server is Down: {e}")
            self.tcp_socket = None

    def assemblyData(self):
        try:
            data = bytes([0x45, 0x5E, 0xC0, 0xDE, 0x00, 0x00, 0x00, 0x00])
            self.tcp_socket.sendall(data)
            self.tcp_console("[iceNET] Assembly Data")
        except Exception as e:
            self.tcp_console(f"[iceNET] Server is Down: {e}")
            self.tcp_socket = None

    def initPointers(self):
        try:
            data = bytes([0x14, 0x17, 0xC0, 0xDE, 0x00, 0x00, 0x00, 0x00])
            self.tcp_socket.sendall(data)
            self.tcp_console("[iceNET] Initialise sector pointers")
        except Exception as e:
            self.tcp_console(f"[iceNET] Server is Down: {e}")
            self.tcp_socket = None

    def loadRam(self):
        try:
            data = bytes([0x5E, 0xDD, 0xC0, 0xDE, 0x00, 0x00, 0x00, 0x00])
            self.tcp_socket.sendall(data)
            self.tcp_console("[iceNET] Send data to RAM")
        except Exception as e:
            self.tcp_console(f"[iceNET] Server is Down: {e}")
            self.tcp_socket = None

    # Write i2c Button
    def i2c_toggle_write_data_entry(self):
        if self.i2c_write_var.get():
            self.i2c_register_data.config(state=tk.NORMAL)
        else:
            self.i2c_register_data.config(state=tk.DISABLED)

    # Write spi Button
    def spi_toggle_write_data_entry(self):
        if self.spi_write_var.get():
            self.spi_register_data.config(state=tk.NORMAL)
        else:
            self.spi_register_data.config(state=tk.DISABLED)

    def i2c_assembly(self):
        header = 0x00 # 0000 0000 Read
        address = int(self.i2c_device_address.get(), 16)  # Convert hex address to integer
        register = int(self.i2c_device_register.get(), 16)  # Convert hex register to integer
        if self.i2c_write_var.get():
            # 0000 0000 :: Read
            data = bytes([header + 0x01, address, register]) + bytes.fromhex(self.i2c_register_data.get()) + bytes([0x00, 0x00, 0x00, 0x00])
        else:
            # 0000 0001 :: Write
            data = bytes([header + 0x00, address, register, 0x00, 0x00, 0x00, 0x00, 0x00])
        return data

    def pwm_set(self, value):
        header = 0x02 # 0000 0010
        data = bytes([header, 0x00, 0x00, value, 0x00, 0x00, 0x00, 0x00])
        self.pwm_speed.delete(0, 'end')
        self.pwm_speed.insert(0, f"{value:02X}")
        return data

    def pwm_getset(self, value):
        header = 0x02 # 0000 0010
        current = int(self.pwm_speed.get(), 16) + value
        current = min(current, 0xFA)
        data = bytes([header, 0x00, 0x00, current, 0x00, 0x00, 0x00, 0x00])
        self.pwm_speed.delete(0, 'end')
        self.pwm_speed.insert(0, f"{current:02X}")
        return data

    def spi_assembly(self):
        header = 0x04 # 0000 0100
        data = bytes([header, 0x00, 0x00]) + bytes.fromhex(self.spi_register_data.get()) + bytes([0x00, 0x00, 0x00, 0x00])
        return data

    def tcp_execute(self, comand):
        try:
            if comand == 0:
                data = self.i2c_assembly()
            elif comand == 1:
                data = self.pwm_getset(0x00)
            elif comand == 2:  # UP
                data = self.pwm_getset(0x08)
            elif comand == 3:  # DOWN
                data = self.pwm_getset(-0x08)
            elif comand == 4:
                data = self.pwm_set(0x00) # STOP (Speed 0%)
            elif comand == 5:
                data = self.pwm_set(0x7D) # 50%
            elif comand == 6:
                data = self.pwm_set(0xFA) # 100%
            elif comand == 7:
                data = self.spi_assembly()
            else:
                header = 0x05
                data = bytes([header, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00])
                self.tcp_console("[iceNET] Unknown comand")
                return

            # Send and log data
            self.tcp_socket.sendall(data)
            self.tcp_console(f"[iceNET] Client TX :: {data.hex()}")

            # Receive feedback
            while True:
                feedback_data = self.tcp_socket.recv(8)

                # Check for invalid feedback: either 0xFF or 0x0000000000000000
                if feedback_data == b'\xFF' or feedback_data == b'\x00\x00\x00\x00\x00\x00\x00\x00':
                    # time.sleep(0.1)
                    continue  # Retry if feedback is invalid
                else:
                    break  # Exit loop on valid feedback

            self.tcp_console(f"[iceNET] Client RX :: {feedback_data.hex()}")

        except Exception as e:
            self.tcp_console(f"[iceNET] Error sending/receiving data over TCP: {e}")
            self.tcp_socket = None

        finally:
            if self.tcp_socket != None:
                self.tcp_console("[iceNET] Transfer complete")

    def tcp_console(self, message):
        self.tcp_display.config(state=tk.NORMAL)
        self.tcp_display.insert(tk.END, message + "\n")
        self.tcp_display.config(state=tk.DISABLED)
        self.tcp_display.see(tk.END)
