itload('../transmit-data.it')
x = data;
f = fopen('transmit-data.dat', 'wb');
fwrite(f, [real(x.') ;imag(x.')], 'float');
fclose(f);
x = read_complex_binary('transmit-data.dat');
disp(max(abs(x - data)));
