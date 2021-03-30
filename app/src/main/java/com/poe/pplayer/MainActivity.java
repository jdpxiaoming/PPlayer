package com.poe.pplayer;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.PermissionChecker;

import android.Manifest;
import android.content.Context;
import android.media.AudioManager;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.SeekBar;
import android.widget.TextView;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    private int requestPermissionCode = 10086;

    private String[] requestPermission = new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_EXTERNAL_STORAGE};
    private SurfaceView mSurfaceView;
    private SeekBar mSeekBar;
    private int mPorgress;
    private EditText mUrlEtv;
    private PoePlayer mPlayer;
    private String mUrl;//播放地址.

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
                WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON );
        setContentView(R.layout.activity_main);

        //初始化布局.
        mSurfaceView = findViewById(R.id.surface_view);
        Button play = findViewById(R.id.btn_play);
        mSeekBar = findViewById(R.id.seek_bar);
        mUrlEtv = findViewById(R.id.edt_url);

        mUrl="rtsp://106.75.254.198:5555/rtsp/c20ddaff-b2e5-4c10-80b7-938891409d35";//O3P
//        mUrl="http://106.75.254.198:5581/rtsp/68448ebb-d34b-4af7-9cf9-ffe650ced784.flv";//O3P
        mUrl = "http://106.75.254.198:5581/rtsp/4127aeff-4d1e-411b-9d21-23205f117e75.flv";//4x倍速播放.
        mUrlEtv.setText(mUrl);
        //监听进度变化.
        mSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        //DO: 2020/4/22 结束seek后进行seek操作.
        mPlayer.seek(25*1000);
    }
});

        play.setOnClickListener(new View.OnClickListener() {
@Override
public void onClick(View v) {
        playLocal();
        }
        });

        mPlayer = new PoePlayer();
        mPlayer.setSurfaceView(mSurfaceView);

        // Example of a call to a native method
        if(Build.VERSION.SDK_INT > Build.VERSION_CODES.M){
            if(PermissionChecker.checkSelfPermission(this,Manifest.permission.WRITE_EXTERNAL_STORAGE) != PermissionChecker.PERMISSION_GRANTED){
                requestPermissions(requestPermission,requestPermissionCode);
            }
        }
    }


    /**
     * 播放本地视频文件 /poe/input.mp4n   3205837018613102
     */
    private void playLocal() {
//        File input = new File(Environment.getExternalStorageDirectory(),"/poe/input.mp4");
//        File input = new File(getCacheDir(),"/input.mp4");
//        Log.i("poe","input file: "+input.getAbsolutePath());
//        if(input.exists()){
//            Log.i("poe","input 存在！");
//        }else{
//            Log.e("poe","input 不存存在！");
//        }
//        mPlayer.setDataSource(input.getAbsolutePath());
        mPlayer.setDataSource(mUrlEtv.getText().toString());
        mPlayer.setOnPrepareListener(new PoePlayer.OnPrepareListener() {
            @Override
            public void onPrepare() {
                Log.i("poe","onPrepare()# mplayer.start()!");
                mPlayer.start();
            }
        });
        mPlayer.prepare();
    }


    @Override
    protected void onDestroy() {
        super.onDestroy();
        if(null != mPlayer){
            mPlayer.close();
        }
    }
}
