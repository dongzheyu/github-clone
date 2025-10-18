# GitHub仓库克隆工具 - 数字签名脚本
Write-Host "GitHub仓库克隆工具 - 数字签名脚本" -ForegroundColor Green
Write-Host "==================================" -ForegroundColor Green

# 检查signtool是否存在
try {
    $signtoolPath = Get-Command "signtool" -ErrorAction Stop
    Write-Host "找到signtool: $($signtoolPath.Source)" -ForegroundColor Green
} catch {
    Write-Host "错误: 找不到signtool，请确保Windows SDK已安装且signtool已添加到系统PATH环境变量中" -ForegroundColor Red
    pause
    exit 1
}

# 检查证书文件是否存在
$certPath = "resources\Gihtub-Clone.pfx"
if (-not (Test-Path $certPath)) {
    Write-Host "错误: 找不到签名证书 $certPath" -ForegroundColor Red
    pause
    exit 1
}

# 时间戳URL
$timestampUrl = "http://timestamp.digicert.com"

# 可执行文件路径
$win32Exe = "cmake-build-debug-mingw\github_clone_win32.exe"
$qtExe = "cmake-build-debug-mingw\github_clone_qt.exe"

# 签名Win32版本
if (Test-Path $win32Exe) {
    Write-Host "正在签名Win32版本..." -ForegroundColor Yellow
    $arguments = "sign /f `"$certPath`" /p `"DZY@013520`" /fd SHA256 /t $timestampUrl `"$win32Exe`""
    $process = Start-Process -FilePath "signtool" -ArgumentList $arguments -NoNewWindow -Wait -PassThru
    
    if ($process.ExitCode -eq 0) {
        Write-Host "Win32版本签名成功" -ForegroundColor Green
    } else {
        Write-Host "Win32版本签名失败，退出代码: $($process.ExitCode)" -ForegroundColor Red
        pause
        exit $process.ExitCode
    }
} else {
    Write-Host "警告: 找不到Win32可执行文件 $win32Exe" -ForegroundColor Yellow
}

# 签名Qt版本
if (Test-Path $qtExe) {
    Write-Host "正在签名Qt版本..." -ForegroundColor Yellow
    $arguments = "sign /f `"$certPath`" /p `"DZY@013520`" /fd SHA256 /t $timestampUrl `"$qtExe`""
    $process = Start-Process -FilePath "signtool" -ArgumentList $arguments -NoNewWindow -Wait -PassThru
    
    if ($process.ExitCode -eq 0) {
        Write-Host "Qt版本签名成功" -ForegroundColor Green
    } else {
        Write-Host "Qt版本签名失败，退出代码: $($process.ExitCode)" -ForegroundColor Red
        pause
        exit $process.ExitCode
    }
} else {
    Write-Host "警告: 找不到Qt可执行文件 $qtExe" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "签名过程完成" -ForegroundColor Green
pause