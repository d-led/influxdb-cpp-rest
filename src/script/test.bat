start influxd
ping 127.0.0.1 -n 2 > nul
%1\test -d yes
%1\test-shared -d yes
taskkill /F /IM influxd.exe /T
start influxd -config src\auth_test\influxdb.conf
ping 127.0.0.1 -n 2 > nul
%1\auth_test -d yes
taskkill /F /IM influxd.exe /T
