import os
import sys

(_, executable, bmp_port, svd, gdb) = sys.argv

LAUNCH = f'''{{
    "configurations": [
        {{
            "name": "Debug",
            "cwd": "${{workspaceRoot}}",
            "executable": "${{workspaceRoot}}/{executable}",
            "request": "launch",
            "type": "cortex-debug",
            "runToEntryPoint": "main",
            "servertype": "bmp",
            "BMPGDBSerialPort": "{bmp_port}",
            "svdPath": "${{workspaceRoot}}/{svd}",
            "gdbPath": "{gdb}",
            "objdumpPath": "arm-none-eabi-objdump",
            "preLaunchTask": "Build"
        }}
    ]
}}
'''

dirname = os.path.dirname(__file__)
with open(dirname + "/../.vscode/launch.json", 'w') as launch_json:
    launch_json.write(LAUNCH)