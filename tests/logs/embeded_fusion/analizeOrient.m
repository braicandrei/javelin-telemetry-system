clear; close all;

fileName = 'datalog_20250421_085253.csv';
Fs = 225;
data = importfileOrient(fileName);

figure;

plot(data.roll);
hold on
plot(data.pitch);
plot(data.yaw);
grid on
grid minor
title('Orientation Estimate');
legend('Roll', 'Pitch', 'Yaw');
ylabel('Degrees');
