package com.ryanhunter.monitorbot;

import android.app.Service;
import android.content.Intent;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import java.net.*;
import java.io.*;
import java.util.ArrayList;


//Service to send driving commands and receive distance data
public class ClientSock extends Service {

    static final int REGISTER_CLIENT = 1;
    static final int DISTANCE_VALUE = 2;
    static final int SET_MODE = 3;
    static final int DRIVING_COMMAND = 4;
    static final int SOCKET_ERROR = 5;
    static final int BATTERY_LEVEL = 6;
    Socket sock = null;
    String data = "5.0.";
    String mode = "1";
    String drivingCommand = "5";
    int distance;

    ArrayList<Messenger> mClients = new ArrayList<Messenger>();
    final Messenger mMessenger = new Messenger(new IncomingHandler());
    boolean bound = false;

    public ClientSock() {
        try {
            SocketClient();
            Thread.sleep(500);
        } catch (InterruptedException e) {System.out.println("Problem with Constructor"); }
    }

    class IncomingHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case REGISTER_CLIENT:
                    mClients.add(msg.replyTo);
                    break;
                case SET_MODE:
                    mode = Integer.toString(msg.arg1);
                    data = drivingCommand + "." + mode + ".";
                    break;
                case DRIVING_COMMAND:
                    drivingCommand = Integer.toString(msg.arg1);
                    data = drivingCommand + "." + mode + ".";
                default:
                    super.handleMessage(msg);
            }
        }
    }

    private void sendDistanceMessage(int value) {
        try {
            mClients.get(0).send(Message.obtain(null, DISTANCE_VALUE, value, 0));
        } catch (Exception e) {}
    }

    private void sendBatteryMessage(int value) {
        try {
            mClients.get(0).send(Message.obtain(null, BATTERY_LEVEL, value, 0));
        } catch (Exception e) {}
    }



    @Override
    public void onCreate() {
        super.onCreate();
    }

    @Override
    public IBinder onBind(Intent intent) {
        //return mBinder;
        bound = true;
        return mMessenger.getBinder();

    }

    @Override
    public boolean onUnbind(Intent intent) {
        bound = false;
        stopSelf();
        return true;
    }

    @Override
    public void onRebind(Intent intent) {
        bound = true;
        SocketClient();
        super.onRebind(intent);
    }

    @Override
    public void onDestroy() {
        bound = false;
        super.onDestroy();
        stopSelf();
    }

    public void SocketClient()
    {
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    //drivingCommand.mode.
                    Socket sock = new Socket("192.168.7.2", 8420);
                    sock.setKeepAlive(true);
                    byte[] b = null;
                    byte[] buffer = new byte[1024];
                    DataOutputStream out = new DataOutputStream(sock.getOutputStream());
                    DataInputStream in = new DataInputStream(sock.getInputStream());
                    //connect to server, then wait half a second for it to initialize
                    Thread.sleep(500);
                    while (bound)
                    {
                        b = data.getBytes("UTF-8");
                        out.write(b);
                        out.flush();
                        in.read(buffer, 0, buffer.length);
                        String input = new String(buffer, "UTF-8");
                        //low battery, ultrasonic data
                        try
                        {
                            //low battery = input.parseBoolean; pull out low battery
                            int battery = Math.round(Integer.parseInt(input.substring(0, input.indexOf('.'))));
                            input = input.substring(input.indexOf('.') + 1, input.lastIndexOf('.') - 1);
                            distance = Math.round(Float.parseFloat(input));
                            sendDistanceMessage(distance);
                            sendBatteryMessage(battery);
                            input = "";
                        }
                        catch (Exception e) {
                            e.printStackTrace();
                            try{mClients.get(0).send(Message.obtain(null, SOCKET_ERROR, 1, 0));}
                            catch(Exception r) {}

                        }
                        Thread.sleep(100);
                    }
                }
                catch (UnknownHostException e) {
                    e.printStackTrace();
                    sock = null;
                    e.printStackTrace();
                    try{mClients.get(0).send(Message.obtain(null, SOCKET_ERROR, 1, 0));}
                    catch(Exception r) {}

                } catch (IOException e) {
                    sock = null;
                    e.printStackTrace();
                    try{mClients.get(0).send(Message.obtain(null, SOCKET_ERROR, 1, 0));}
                    catch(Exception r) {}

                } catch(InterruptedException e) {
                    sock = null;
                    e.printStackTrace();
                    try{mClients.get(0).send(Message.obtain(null, SOCKET_ERROR, 1, 0));}
                    catch(Exception r) {}

                } finally {
                    if (sock != null) {
                        try {
                            sock.close();
                            sock = null;
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    }
                }
            }
        }).start();
    }
}
