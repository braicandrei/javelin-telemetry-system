clear; close all;

fileName = 'datalog_2025417_224827.csv';
Fs = 225;
data = importfile(fileName);

acc = table2array(data(:,1:3));
gyro = table2array(data(:,4:6));
mag = table2array(data(:,7:9));

accSI = acc*9.81;
gyroSI = gyro*(pi/180);
magSI = mag*100;

fuse = complementaryFilter('SampleRate',Fs);
q = fuse(accSI,gyroSI,magSI);

plot(eulerd( q, 'ZYX', 'frame'));
grid on
grid minor
title('Orientation Estimate');
legend('Z-rotation', 'Y-rotation', 'X-rotation');
ylabel('Degrees');
