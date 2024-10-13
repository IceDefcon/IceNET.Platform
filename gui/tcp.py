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
    def __init__(self, root, log_function, tcp_ip, tcp_port):
        self.root = root
        self.log_function = log_function
        self.tcp_ip = tcp_ip.get()
        self.tcp_port = int(tcp_port.get())
        # I2C
        self.device_label = tk.Label(self.root, text="I2C Device ID")
        self.device_label.grid(row=5, column=0, pady=5, padx=5, sticky='e')
        self.device_address = tk.Entry(self.root, width=16)
        self.device_address.grid(row=5, column=1, pady=5, padx=5, sticky='w')
        self.device_address.insert(0, "69")
        self.i2c_exe_button = tk.Button(self.root, text="EXE", command=lambda: self.tcp_execute(0))
        self.i2c_exe_button.grid(row=5, column=2, pady=5, padx=5, sticky='nsew')
        self.device_register_label = tk.Label(self.root, text="Register Address")
        self.device_register_label.grid(row=6, column=0, pady=5, padx=5, sticky='e')
        self.device_register = tk.Entry(self.root, width=16)
        self.device_register.grid(row=6, column=1, pady=5, padx=5, sticky='w')
        self.device_register.insert(0, "00")
        self.register_data_label = tk.Label(self.root, text="Write Data")
        self.register_data_label.grid(row=7, column=0, pady=5, padx=5, sticky='e')
        self.register_data = tk.Entry(self.root, width=16)
        self.register_data.grid(row=7, column=1, pady=5, padx=5, sticky='w')
        self.register_data.insert(0, "00")
        self.register_data.config(state=tk.DISABLED) # Initialize "Write Data" entry as disabled
        self.write_var = tk.BooleanVar() # Add a tick box (Checkbutton)
        self.write_box = tk.Checkbutton(self.root, text="Write", variable=self.write_var, command=self.toggle_write_data_entry)
        self.write_box.grid(row=7, column=2, pady=5, padx=5, sticky='w')
        # PWM
        self.pwm_speed_label = tk.Label(root, text="PWM Speed Hex")
        self.pwm_speed_label.grid(row=5, column=4, pady=5, padx=5, sticky='nsew')
        self.pwm_speed = tk.Entry(self.root, width=16)
        self.pwm_speed.grid(row=5, column=5, pady=5, padx=5, sticky='nsew')
        self.pwm_speed.insert(0, "00")
        ##################
        # tcp_execute    #
        ##################
        # 0 :: i2c       #
        # 1 :: Field box #
        # 2 :: Up        #
        # 3 :: Down      #
        # 4 :: 0%        #
        # 5 :: 50%       #
        # 6 :: 100%      #
        ##################
        self.pwm_exe_button = tk.Button(self.root, text="EXE", command=lambda: self.tcp_execute(1))
        self.pwm_exe_button.grid(row=5, column=6, pady=5, padx=5, sticky='nsew')
        self.pwm_up_button = tk.Button(self.root, text=" ▲ ", command=lambda: self.tcp_execute(2))
        self.pwm_up_button.grid(row=6, column=6, pady=5, padx=5, sticky='nsew')
        self.pwm_down_button = tk.Button(self.root, text=" ▼ ", command=lambda: self.tcp_execute(3))
        self.pwm_down_button.grid(row=7, column=6, pady=5, padx=5, sticky='nsew')

        self.pwm_stop_button = tk.Button(self.root, text="0%", command=lambda: self.tcp_execute(4))
        self.pwm_stop_button.grid(row=5, column=8, pady=5, padx=5, sticky='nsew')
        self.pwm_50_button = tk.Button(self.root, text="50%", command=lambda: self.tcp_execute(5))
        self.pwm_50_button.grid(row=6, column=8, pady=5, padx=5, sticky='nsew')
        self.pwm_100_button = tk.Button(self.root, text="100%", command=lambda: self.tcp_execute(6))
        self.pwm_100_button.grid(row=7, column=8, pady=5, padx=5, sticky='nsew')

    # Write Button
    def toggle_write_data_entry(self):
        if self.write_var.get():
            self.register_data.config(state=tk.NORMAL)
        else:
            self.register_data.config(state=tk.DISABLED)

    def i2c_assembly(self):
        header = 0x00 # 7 control bits + 1 R/W bit
        address = int(self.device_address.get(), 16)  # Convert hex address to integer
        register = int(self.device_register.get(), 16)  # Convert hex register to integer
        if self.write_var.get():
            data = bytes([header + 0x01, address, register]) + bytes.fromhex(self.register_data.get())
        else:
            data = bytes([header + 0x00, address, register, 0x00])
        return data

    def pwm_assembly(self):
        header = 0x02  # 7 control bits + 1 R/W bit
        pwm_speed_value = int(self.pwm_speed.get(), 16)  # Assuming input is in hexadecimal
        if pwm_speed_value > 0xFA:
            pwm_speed_value = 0xFA
            self.pwm_speed.delete(0, 'end')
            self.pwm_speed.insert(0, "FA")
        data = bytes([header, 0x00, 0x00, pwm_speed_value])
        return data

    def tcp_execute(self, header):
        tcp_socket = None
        try:
            # Retrieve IP and Port values from Entry widgets
            server_address = self.tcp_ip
            port = self.tcp_port

            # Create and connect the TCP socket
            tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

            tcp_socket.connect((server_address, port))
            self.log_function("[iceNET] Server connection established")

            # Determine data to send based on header
            if header == 0:
                data = self.i2c_assembly()
            elif header == 1:
                data = self.pwm_assembly()
            elif header == 2:  # UP
                current = int(self.pwm_speed.get(), 16) + 0x08
                current = min(current, 0xFA)
                data = bytes([0x02, 0x00, 0x00, current])
                self.pwm_speed.delete(0, 'end')
                self.pwm_speed.insert(0, f"{current:02X}")
            elif header == 3:  # DOWN
                current = int(self.pwm_speed.get(), 16) - 0x08
                current = max(current, 0x00)
                data = bytes([0x02, 0x00, 0x00, current])
                self.pwm_speed.delete(0, 'end')
                self.pwm_speed.insert(0, f"{current:02X}")
            elif header == 4:  # STOP (Speed 0%)
                data = bytes([0x02, 0x00, 0x00, 0x00])
                self.pwm_speed.delete(0, 'end')
                self.pwm_speed.insert(0, "00")
            elif header == 5:  # 50%
                data = bytes([0x02, 0x00, 0x00, 0x7D])
                self.pwm_speed.delete(0, 'end')
                self.pwm_speed.insert(0, "7D")
            elif header == 6:  # 100%
                data = bytes([0x02, 0x00, 0x00, 0xFA])
                self.pwm_speed.delete(0, 'end')
                self.pwm_speed.insert(0, "FA")
            else:
                self.log_function("[iceNET] Wrong Data Header")
                return

            # Send and log data
            tcp_socket.sendall(data)
            self.log_function(f"[iceNET] Client TX :: {data.hex()}")

            # Receive feedback
            while True:
                feedback_data = tcp_socket.recv(4)
                if feedback_data == b'\xFF':
                    time.sleep(0.1)
                    continue  # Retry if feedback is 0xFF
                else:
                    break  # Exit loop on valid feedback

            self.log_function(f"[iceNET] Client RX :: {feedback_data.hex()}")

        except Exception as e:
            self.log_function(f"[iceNET] Error sending/receiving data over TCP: {e}")

        finally:
            # Close the TCP connection
            if tcp_socket:
                tcp_socket.close()
                self.log_function("[iceNET] Server connection terminated")
