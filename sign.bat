@echo off
echo GitHub仓库克隆工具 - 数字签名脚本
echo ==================================

REM 检查PowerShell是否存在
where powershell >nul 2>&1
if %errorlevel% neq 0 (
    echo 错误: 找不到PowerShell
    pause
    exit /b 1
)

REM 使用PowerShell执行签名脚本
powershell -ExecutionPolicy Bypass -File "%~dp0sign.ps1"
