package com.poe.pplayer;

import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class PoePlayer implements SurfaceHolder.Callback {
    private static final String TAG = "PoePlayer";

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("pplayer");
    }

    private SurfaceHolder mSurfaceHolder;
    private String mSource ;

    private OnPrepareListener onPrepareListener;
    private OnProgressListener onProgressListener;
    private OnErrorListener onErrorListener;

    public void setOnPrepareListener(OnPrepareListener onPrepareListener) {
        this.onPrepareListener = onPrepareListener;
    }

    public void setOnProgressListener(OnProgressListener onProgressListener) {
        this.onProgressListener = onProgressListener;
    }

    public void setOnErrorListener(OnErrorListener onErrorListener) {
        this.onErrorListener = onErrorListener;
    }

    /**
     * 设置播放路径
     * @param input
     */
    public void setDataSource(String input){
        mSource= input;
    }

    //准备播放.
    public void prepare(){
        native_prepare(mSource);
    }

    /**
     * mp4文件seek .
     * @param milliseconds
     */
    public void seek(long milliseconds){
        native_seek(milliseconds);
    }

    /**
     * 开始播放之前调用{@link #prepare}
     */
    public void start(){
        native_start();
    }


    //Ndk player need path and surfaceview .
    public void setSurfaceView(SurfaceView surfaceView){

        if(null != mSurfaceHolder){
            mSurfaceHolder.removeCallback(this);
        }

        this.mSurfaceHolder = surfaceView.getHolder();
        this.mSurfaceHolder.addCallback(this);
    }


    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        native_set_surface(mSurfaceHolder.getSurface());
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        mSurfaceHolder = holder;

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        native_pause();
    }



    public void close(){
        native_close();
    }


    private native void native_prepare(String url);
    private native void native_set_surface(Surface surface);
    private native void native_start();
    private native void native_seek(long ms);//seek毫秒.
    private native void native_pause();
    private native void native_close();


    /***回调方法***/
    /**
     * c层准备完毕.
     */
    void onPrepare(){

        if(null != onPrepareListener) onPrepareListener.onPrepare();
    }

    /**
     * 回调播放进度.
     * @param progress
     */
    void onProgress(int progress){
        if(null != onProgressListener) onProgressListener.onProgress(progress);
    }

    /**
     * 播放出错.
     * @param errorCode
     */
    void onError(int errorCode){

        if(null != onErrorListener) onErrorListener.onError(errorCode);
    }


    public interface OnPrepareListener{
        void onPrepare();
    }

    public interface OnProgressListener{
        void onProgress(int progress);
    }

    public interface OnErrorListener{
        void onError(int errorCode);
    }
}
