cd Debug
start ServerPrototype.exe
timeout /t 4
cd "../WPFClient/bin/Debug"
start WPFClient.exe
timeout /t 13
start WPFClient.exe -port 8083