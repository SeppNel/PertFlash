package com.seppnel.pertflash;

import android.content.Context;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.util.Log;

import com.hoho.android.usbserial.driver.UsbSerialDriver;
import com.hoho.android.usbserial.driver.UsbSerialPort;
import com.hoho.android.usbserial.driver.UsbSerialProber;
import com.hoho.android.usbserial.util.SerialInputOutputManager;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.List;
import android.os.Process;

public class Serial implements SerialInputOutputManager.Listener {
    private static final int READ_WAIT_MILLIS = 0;
    private static final int mThreadPriority = Process.THREAD_PRIORITY_URGENT_AUDIO;
    private static final float SERIAL_CLICK = -1.0f;
    private static final float AVG_UPPER_LIMIT = 1000.0f;
    private static final float AVG_LOWER_LIMIT = 1.0f;

    private UsbSerialPort port;
    private final MainActivity mainActivity;
    private byte[] readBuffer = new byte[4];

    private double sum = 0.0;
    private int runs = 0;


    private static final byte[] HEX_ARRAY = "0123456789ABCDEF".getBytes(StandardCharsets.US_ASCII);
    public static String bytesToHex(byte[] bytes) {
        byte[] hexChars = new byte[bytes.length * 2];
        for (int j = 0; j < bytes.length; j++) {
            int v = bytes[j] & 0xFF;
            hexChars[j * 2] = HEX_ARRAY[v >>> 4];
            hexChars[j * 2 + 1] = HEX_ARRAY[v & 0x0F];
        }
        return new String(hexChars, StandardCharsets.UTF_8);
    }

    public Serial(MainActivity m) throws IOException {
        mainActivity = m;

        // Find all available drivers from attached devices.
        UsbManager manager = (UsbManager) mainActivity.getSystemService(Context.USB_SERVICE);
        List<UsbSerialDriver> availableDrivers = UsbSerialProber.getDefaultProber().findAllDrivers(manager);
        if (availableDrivers.isEmpty()) {
            Log.i("Serial","No available drivers");
            return;
        }

        // Open a connection to the first available driver.
        UsbSerialDriver driver = availableDrivers.get(0);
        UsbDeviceConnection connection = manager.openDevice(driver.getDevice());
        if (connection == null) {
            Log.i("Serial","Null connecton");
            return;
        }

        port = driver.getPorts().get(0); // Most devices have just one port (port 0)
        port.open(connection);
        port.setParameters(19200, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE);

        SerialInputOutputManager usbIoManager = new SerialInputOutputManager(port, this);
        usbIoManager.start();
    }

    @Override
    public void onNewData(byte[] data) {
        for (int i = 0; i < data.length; i+= 4){
            readBuffer[0] = data[i];
            readBuffer[1] = data[i+1];
            readBuffer[2] = data[i+2];
            readBuffer[3] = data[i+3];

            float f = ByteBuffer.wrap(readBuffer).order(ByteOrder.LITTLE_ENDIAN).getFloat();

            if(f == SERIAL_CLICK){
                mainActivity.changeColor();
            }
            else{
                mainActivity.updateText("Response Time: " + f + "ms");

                if (f < AVG_UPPER_LIMIT && f > AVG_LOWER_LIMIT){
                    sum += f;
                    runs++;
                }

                if (runs % 50 == 0 && runs > 0){
                    mainActivity.updateAvgText("Avg in " + runs + " runs = " + String.format("%.4f", sum/runs));
                }
            }
        }
    }

    @Override
    public void onRunError(Exception e) {

    }

    //Not Working
    public void thread(){
        new Thread( new Runnable() { @Override public void run() {
            Process.setThreadPriority(mThreadPriority);
            int i = 0;
            byte[] buffer = new byte[4];

            while(true){
                Arrays.fill(buffer, (byte) 0);
                int len = 0;
                try {
                    len = port.read(buffer, READ_WAIT_MILLIS);
                } catch (IOException e) {
                    mainActivity.updateText("Error");
                    throw new RuntimeException(e);
                }

                float f = ByteBuffer.wrap(buffer).order(ByteOrder.LITTLE_ENDIAN).getFloat();

                mainActivity.updateText(i + " | " + len + ": " + f + " | " + bytesToHex(buffer));
                if(f != -1.0) {
                    mainActivity.updateText("Response Time: " + f + "ms");
                }

                i++;
            }
        } } ).start();
    }
}
