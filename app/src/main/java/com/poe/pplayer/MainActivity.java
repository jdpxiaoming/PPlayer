package com.poe.pplayer;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.PermissionChecker;

import android.Manifest;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
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

    private PoePlayer mPlayer;
    private String mUrl = "rtmp://192.168.1.3:1935/oflaDemo/guardians2.mp4";
    /**
     * 播放本地视频文件 /poe/input.mp4n   3205837018613102
     */
    private void playLocal() {
//        File input = new File(Environment.getExternalStorageDirectory(),"/poe/input.mp4");
        File input = new File(getCacheDir(),"/input.mp4");
        Log.i("poe","input file: "+input.getAbsolutePath());
        if(input.exists()){
            Log.i("poe","input 存在！");
        }else{
            Log.e("poe","input 不存存在！");
        }

        mPlayer.setDataSource(input.getAbsolutePath());
        mPlayer.setOnPrepareListener(new PoePlayer.OnPrepareListener() {
            @Override
            public void onPrepare() {
                Log.i("poe","onPrepare()# mplayer.start()!");
                mPlayer.start();
            }
        });
        mPlayer.prepare();
    }

}
