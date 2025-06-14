clear; close all;

fileName = 'datalog_20250509_110929.csv';
Fs = 225;
data = importfile(fileName);

acc = table2array(data(:,1:3));
gyro = table2array(data(:,4:6));
mag = table2array(data(:,7:9));

accCor = acc;
accCor(:,1) = -acc(:,2);
accCor(:,2) = acc(:,1);
gyroCor = gyro;
gyroCor(:,1) = -gyro(:,2);
gyroCor(:,2) = gyro(:,1);
magCor = mag;
magCor(:,1) = mag(:,2);
magCor(:,2) = -mag(:,1);



accSI = accCor*9.81;
gyroSI = gyroCor*(pi/180);
magSI = magCor*100;

fuse = complementaryFilter('SampleRate',Fs);
q = fuse(accSI,gyroSI,magSI);

figure
subplot(3,1,1);
plot(accSI);
subplot(3,1,2);
plot(gyroSI);
subplot(3,1,3);
plot(magSI);


figure
plot(eulerd( q, 'ZYX', 'frame'));
grid on
grid minor
title('Orientation Estimate');
legend('Z-rotation', 'Y-rotation', 'X-rotation');
ylabel('Degrees');
