@echo off
start cmd /k mdbroker.exe config.json
start cmd /k mdworker.exe config.json
REM start cmd /k mdworker.exe config.json
REM start cmd /k mdclient.exe echo config.json