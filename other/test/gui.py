import tkinter as tk
import os

def send_command_to_cpp():
    with open('/tmp/my_pipe', 'w') as pipe:
        command = "execute_task"
        pipe.write(command)  # Send command to C++ application

root = tk.Tk()
root.title("Tkinter to C++ Communication")

send_button = tk.Button(root, text="Send Command to C++", command=send_command_to_cpp)
send_button.pack(pady=20)

root.mainloop()
