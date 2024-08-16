@ECHO OFF
SET mypath=%~dp0
echo -- PATH: %mypath:~0,-1%

SET cpython_path=%mypath%\..\external\cpython\
SET install_path=%mypath%\..\python\

if exist %mypath%\..\python\python.exe (
     echo -- exist python
) else (
     echo -- START::Python build 

          call %cpython_path%\PCBuild\clean.bat 
          call %cpython_path%\PCBuild\build.bat -e -p x64

     echo -- END::Python build 

     echo -- START::LAYOUT    
           %cpython_path%\PCBuild\amd64\python.exe %cpython_path%PC\layout\main.py -b %cpython_path%\PCBuild\amd64  -s %cpython_path% -t %cpython_path%\temp\  --include-underpth --include-stable --include-pip --include-dev --include-launchers  --include-venv --include-symbols --copy %install_path%
          %cpython_path%\PCBuild\amd64\python.exe -m pip install --upgrade pip

     echo -- END::LAYOUT 
     echo -- START::PIP INSTALL 
          %install_path%\python.exe -m pip install mediapipe
          %install_path%\python.exe -m pip  install PyGLM==2.5.7
     echo -- END::PIP INSTALL 
)