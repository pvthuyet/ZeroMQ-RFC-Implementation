@echo off
start cmd /k ppbroker.exe config.json
start cmd /k ppworker.exe 1 config.json
start cmd /k ppworker.exe 2 config.json
start cmd /k lpclient.exe config.json
start cmd /k lpclient.exe config.json
start cmd /k lpclient.exe config.json