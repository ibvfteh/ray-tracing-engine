@echo off
for %%i in (*.vert *.frag *.rchit *.rgen *.rmiss) do glslangValidator.exe -V "%%~i" -o "%%~i.spv"