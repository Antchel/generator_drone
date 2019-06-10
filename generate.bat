@echo off
chcp 65001

"tools\jstp" gen-c -in=ifc -out=bin DroneGen 1.0.0
"tools\jstp" gen-help -in=ifc -out=bin DroneGen 1.0.0