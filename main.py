import PySimpleGUI as sg

layout = [
    [sg.Text("Smoothing"), sg.Slider(range=(0,1), resolution=0.01, default_value=0.5)],
    [sg.Button("Start"), sg.Button("Stop"), sg.Button("Save")]
]

window = sg.Window("Hand Mouse", layout)

event, values = window.read()
print(event, values)
window.close()

