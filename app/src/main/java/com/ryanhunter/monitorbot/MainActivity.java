package com.ryanhunter.monitorbot;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.res.ColorStateList;
import android.content.res.Configuration;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.support.v7.app.AppCompatActivity;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.net.Uri;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageButton;
import android.widget.ProgressBar;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import org.videolan.libvlc.IVLCVout;
import org.videolan.libvlc.LibVLC;
import org.videolan.libvlc.Media;
import org.videolan.libvlc.MediaPlayer;

import java.lang.ref.WeakReference;
import java.util.ArrayList;

public class MainActivity extends AppCompatActivity implements IVLCVout.Callback {

    static boolean mBound;
    static Messenger mService = null;
    final Messenger mMessenger = new Messenger(new IncomingHandler());
    TextView text = null;
    ProgressBar robotBattery = null;
    private LibVLC libvlc;
    private int mVideoWidth;
    private int mVideoHeight;
    private SurfaceView mSurface;
    private SurfaceHolder mHolder;
    private MediaPlayer mMediaPlayer = null;
    private String mFilePath = "rtsp://192.168.7.2:5554/test.mpeg4";

    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            mService = new Messenger(service);
            mBound = true;
            try {
                Message msg = Message.obtain(null, ClientSock.REGISTER_CLIENT);
                msg.replyTo = mMessenger;
                mService.send(msg);
            } catch (RemoteException e) {
            }
        }


        @Override
        public void onServiceDisconnected(ComponentName className) {
            mBound = false;
            mService = null;
        }
    };

    public static void sendModeMessage(int mode)
    {
        if (mBound) {
            try {
                Message msg = Message.obtain(null, ClientSock.SET_MODE, mode, 0);
                mService.send(msg);
            } catch (RemoteException e) {e.printStackTrace();}
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        ImageButton forwards = (ImageButton) findViewById(R.id.forwards);
        ImageButton backwards = (ImageButton) findViewById(R.id.backwards);
        ImageButton left = (ImageButton) findViewById(R.id.left);
        ImageButton right = (ImageButton) findViewById(R.id.right);
        text = (TextView) findViewById(R.id.display_text);
        mSurface = (SurfaceView) findViewById(R.id.surface);
        robotBattery = (ProgressBar) findViewById((R.id.robotBattery));
        robotBattery.setProgressTintList(ColorStateList.valueOf(Color.GREEN));
        Spinner spinner = (Spinner) findViewById(R.id.selectMode);
        // Create an ArrayAdapter using the string array and a default spinner layout
        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this, R.array.modes_array, android.R.layout.simple_spinner_item);
        // Specify the layout to use when the list of choices appears
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        // Apply the adapter to the spinner
        spinner.setAdapter(adapter);
        spinner.setOnItemSelectedListener(new SpinnerActivity());

        Uri uri = Uri.parse("rtsp://192.168.7.2:5554/test.mpeg4");
        mHolder = mSurface.getHolder();
        //Uri uri = Uri.parse("http://bffmedia.com/bigbunny.mp4");


        forwards.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View View, MotionEvent motionEvent) {
                if (motionEvent.getAction() == MotionEvent.ACTION_DOWN) {
                    if (mBound) {
                        try {
                            Message msg = Message.obtain(null, ClientSock.DRIVING_COMMAND, 1, 0);
                            msg.replyTo = mMessenger;
                            mService.send(msg);
                        } catch (RemoteException e) {e.printStackTrace();}
                    }
                }
                if (motionEvent.getAction() == MotionEvent.ACTION_UP) {
                    if (mBound) {
                        try {
                            Message msg = Message.obtain(null, ClientSock.DRIVING_COMMAND, 5, 0);
                            msg.replyTo = mMessenger;
                            mService.send(msg);
                        } catch (RemoteException e) {e.printStackTrace();}
                    }
                }
                return false;
            }
        });

        backwards.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View View, MotionEvent motionEvent) {
                if (motionEvent.getAction() == MotionEvent.ACTION_DOWN) {
                    if (mBound) {
                        try {
                            Message msg = Message.obtain(null, ClientSock.DRIVING_COMMAND, 2, 0);
                            msg.replyTo = mMessenger;
                            mService.send(msg);
                        } catch (RemoteException e) {e.printStackTrace();}
                    }
                }
                if (motionEvent.getAction() == MotionEvent.ACTION_UP) {
                    if (mBound) {
                        try {
                            Message msg = Message.obtain(null, ClientSock.DRIVING_COMMAND, 5, 0);
                            msg.replyTo = mMessenger;
                            mService.send(msg);
                        } catch (RemoteException e) {e.printStackTrace();}
                    }
                }
                return false;
            }
        });

        left.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View View, MotionEvent motionEvent) {
                if (motionEvent.getAction() == MotionEvent.ACTION_DOWN) {
                    if (mBound) {
                        try {
                            Message msg = Message.obtain(null, ClientSock.DRIVING_COMMAND, 3, 0);
                            msg.replyTo = mMessenger;
                            mService.send(msg);
                        } catch (RemoteException e) {e.printStackTrace();}
                    }
                }
                if (motionEvent.getAction() == MotionEvent.ACTION_UP) {
                    if (mBound) {
                        try {
                            Message msg = Message.obtain(null, ClientSock.DRIVING_COMMAND, 5, 0);
                            msg.replyTo = mMessenger;
                            mService.send(msg);
                        } catch (RemoteException e) {e.printStackTrace();}
                    }
                }
                return false;
            }
        });

        right.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View View, MotionEvent motionEvent) {
                if (motionEvent.getAction() == MotionEvent.ACTION_DOWN) {
                    if (mBound) {
                        try {
                            Message msg = Message.obtain(null, ClientSock.DRIVING_COMMAND, 4, 0);
                            msg.replyTo = mMessenger;
                            mService.send(msg);
                        } catch (RemoteException e) {e.printStackTrace();}
                    }
                }
                if (motionEvent.getAction() == MotionEvent.ACTION_UP) {
                    if (mBound) {
                        try {
                            Message msg = Message.obtain(null, ClientSock.DRIVING_COMMAND, 5, 0);
                            msg.replyTo = mMessenger;
                            mService.send(msg);
                        } catch (RemoteException e) {e.printStackTrace();}
                    }
                }
                return false;
            }
        });

    }

    class IncomingHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {

            switch (msg.what) {
                case ClientSock.DISTANCE_VALUE:
                    text.setText(Integer.toString(msg.arg1));
                    break;
                case ClientSock.SOCKET_ERROR:
                    Toast.makeText(getApplicationContext(), "Socket Error", Toast.LENGTH_SHORT).show();
                    break;
                case ClientSock.BATTERY_LEVEL:
                    double percentage = ((msg.arg1 - 320.0) / 560.0);
                    robotBattery.setProgress((int) Math.round((percentage * 100)));
                    break;
                default:
                    super.handleMessage(msg);
                    break;
            }
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
        Intent intent = new Intent(this, ClientSock.class);
        startService(intent);
        bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
    }

    @Override
    protected void onStop() {
        super.onStop();
        // Unbind from the service
        if (mBound) {
            stopService(new Intent(this, ClientSock.class));
            unbindService(mConnection);
            mBound = false;
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mBound) {
            stopService(new Intent(this, ClientSock.class));
            unbindService(mConnection);
            mBound = false;
        }
        releasePlayer();
    }

    @Override
    protected void onPause() {
        super.onPause();

        if (mBound) {
            stopService(new Intent(this, ClientSock.class));
            unbindService(mConnection);
            mBound = false;
        }
        releasePlayer();
    }

    @Override
    protected void onResume() {
        super.onResume();
        Intent intent = new Intent(this, ClientSock.class);
        startService(intent);
        bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
        createPlayer(mFilePath);
    }

    private void setSize(int width, int height) {
        mVideoWidth = width;
        mVideoHeight = height;
        if (mVideoWidth * mVideoHeight <= 1)
            return;

        if (mHolder == null || mSurface == null)
            return;

        int w = getWindow().getDecorView().getWidth();
        int h = getWindow().getDecorView().getHeight();
        boolean isPortrait = getResources().getConfiguration().orientation == Configuration.ORIENTATION_PORTRAIT;
        if (w > h && isPortrait || w < h && !isPortrait) {
            int i = w;
            w = h;
            h = i;
        }

        float videoAR = (float) mVideoWidth / (float) mVideoHeight;
        float screenAR = (float) w / (float) h;

        if (screenAR < videoAR)
            h = (int) (w / videoAR);
        else
            w = (int) (h * videoAR);

        mHolder.setFixedSize(mVideoWidth, mVideoHeight);
        ViewGroup.LayoutParams lp = mSurface.getLayoutParams();
        lp.width = w;
        lp.height = h;
        mSurface.setLayoutParams(lp);
        mSurface.invalidate();
    }

    /**
     * Creates MediaPlayer and plays video
     *
     * @param media
     */
    private void createPlayer(String media) {
        releasePlayer();
        try {
            if (media.length() > 0) {
                Toast toast = Toast.makeText(this, media, Toast.LENGTH_LONG);
                toast.setGravity(Gravity.BOTTOM | Gravity.CENTER_HORIZONTAL, 0, 0);
                toast.show();
            }

            // Create LibVLC
            ArrayList<String> options = new ArrayList<String>();
            options.add("--aout=opensles");
            options.add("--audio-time-stretch"); // time stretching
            options.add("-vvv"); // verbosity
            libvlc = new LibVLC(this, options);
            mHolder.setKeepScreenOn(true);

            // Creating media player
            mMediaPlayer = new MediaPlayer(libvlc);
            mMediaPlayer.setEventListener(mPlayerListener);

            // Seting up video output
            final IVLCVout vout = mMediaPlayer.getVLCVout();
            vout.setVideoView(mSurface);
            vout.addCallback(this);
            vout.attachViews();

            Media m = new Media(libvlc, Uri.parse(media));
            mMediaPlayer.setMedia(m);
            mMediaPlayer.play();
        } catch (Exception e) {
            Toast.makeText(this, "Error in creating player!", Toast.LENGTH_LONG).show();
        }
    }

    private void releasePlayer() {
        if (libvlc == null)
            return;
        mMediaPlayer.stop();
        final IVLCVout vout = mMediaPlayer.getVLCVout();
        vout.removeCallback(this);
        vout.detachViews();
        mHolder = null;
        libvlc.release();
        libvlc = null;

        mVideoWidth = 0;
        mVideoHeight = 0;
    }

    private MediaPlayer.EventListener mPlayerListener = new MyPlayerListener(this);

    @Override
    public void onNewLayout(IVLCVout vout, int width, int height, int visibleWidth, int visibleHeight, int sarNum, int sarDen) {
        if (width * height == 0)
            return;

        // store video size
        mVideoWidth = width;
        mVideoHeight = height;
        setSize(mVideoWidth, mVideoHeight);
    }

    @Override
    public void onSurfacesCreated(IVLCVout vlcVout) {

    }

    @Override
    public void onSurfacesDestroyed(IVLCVout vlcVout) {

    }

    @Override
    public void onHardwareAccelerationError(IVLCVout vlcVout) {
        this.releasePlayer();
        Toast.makeText(this, "Error with hardware acceleration", Toast.LENGTH_LONG).show();
    }

    private static class MyPlayerListener implements MediaPlayer.EventListener {
        private WeakReference<MainActivity> mOwner;

        public MyPlayerListener(MainActivity owner) {
            mOwner = new WeakReference<MainActivity>(owner);
        }

        @Override
        public void onEvent(MediaPlayer.Event event) {
            MainActivity player = mOwner.get();

            switch (event.type) {
                case MediaPlayer.Event.EndReached:
                    player.releasePlayer();
                    break;
                case MediaPlayer.Event.Playing:
                case MediaPlayer.Event.Paused:
                case MediaPlayer.Event.Stopped:
                default:
                    break;
            }
        }
    }
}
