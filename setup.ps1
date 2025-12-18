# ---------------------------------------------
# Install MSVC Build Tools (minimal workload)
# ---------------------------------------------

winget install -e --id Microsoft.VisualStudio.2022.BuildTools `
  --accept-package-agreements `
  --accept-source-agreements `
  --override "--quiet --wait --norestart \
    --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 \
    --add Microsoft.VisualStudio.Component.Windows10SDK.20348"

Write-Host "C++ toolchain installed."

# ---------------------------------------------
# Create Developer Command Prompt Shortcut
# ---------------------------------------------

$vsPath = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
$shortcutPath = "$env:USERPROFILE\Desktop\Developer Command Prompt for VS.lnk"

$WshShell = New-Object -ComObject WScript.Shell
$Shortcut = $WshShell.CreateShortcut($shortcutPath)

$Shortcut.TargetPath = "cmd.exe"
$Shortcut.Arguments = "/k `"$vsPath`""
$Shortcut.WorkingDirectory = "$env:USERPROFILE"
$Shortcut.IconLocation = "cmd.exe, 0"
$Shortcut.Save()

Write-Host "Developer Command Prompt shortcut created on Desktop."

# ---------------------------------------------
# Install Visual Studio Code
# ---------------------------------------------

Write-Host "Installing VS Code..."

winget install -e --id Microsoft.VisualStudioCode `
  --accept-package-agreements `
  --accept-source-agreements `
  --silent

Write-Host "VS Code installed."

# ---------------------------------------------
# Install C++ extensions for VS Code
# ---------------------------------------------

# Use full path to code CLI since it's not in PATH yet
$codeCmd = "$env:LOCALAPPDATA\Programs\Microsoft VS Code\bin\code.cmd"
if (-not (Test-Path $codeCmd)) {
    $codeCmd = "$env:ProgramFiles\Microsoft VS Code\bin\code.cmd"
}

if (-not (Test-Path $codeCmd)) {
    Write-Error "VS Code CLI not found! Exiting."
    exit 1
}

Write-Host "Installing VS Code C++ extensions..."
& $codeCmd --install-extension ms-vscode.cpptools
& $codeCmd --install-extension ms-vscode.cpptools-extension-pack
Write-Host "VS Code C++ extensions installed."

# ---------------------------------------------
# Create VS Code desktop shortcut
# ---------------------------------------------

Write-Host "Creating VS Code desktop shortcut..."

$codePath = "$env:USERPROFILE\AppData\Local\Programs\Microsoft VS Code\Code.exe"
$shortcutPath = "$env:USERPROFILE\Desktop\Visual Studio Code.lnk"

$Shortcut = $WshShell.CreateShortcut($shortcutPath)
$Shortcut.TargetPath = $codePath
$Shortcut.WorkingDirectory = "$env:USERPROFILE"
$Shortcut.IconLocation = "$codePath, 0"
$Shortcut.Save()

Write-Host "VS Code shortcut created."

# ---------------------------------------------
# Configure VS Code terminal to auto-load MSVC
# ---------------------------------------------

Write-Host "Configuring VS Code terminal for MSVC..."

$settingsPath = "$env:APPDATA\Code\User\settings.json"
$settingsDir = Split-Path $settingsPath
if (-not (Test-Path $settingsDir)) {
    New-Item -ItemType Directory -Path $settingsDir -Force
}

$vcvarsPath = "C:\\Program^ Files^ ^(x86^)\\Microsoft^ Visual^ Studio\\2022\\BuildTools\\VC\\auxiliary\\Build\\vcvars64.bat"

$settings = @"
{
    "terminal.integrated.profiles.windows": {
        "Developer Command Prompt": {
            "path": "C:\\Windows\\System32\\cmd.exe",
            "args": ["/k", "$vcvarsPath"],
            "icon": "terminal-cmd"
        }
    },
    "terminal.integrated.defaultProfile.windows": "Developer Command Prompt"
}
"@

$settings | Out-File -Encoding utf8 $settingsPath -Force

Write-Host "VS Code terminal configured for MSVC."
Write-Host "`nSetup completed successfully!"
