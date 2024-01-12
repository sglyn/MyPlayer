package com.example.myplayer;

import android.view.Surface;

public class Player {
    static {
        System.loadLibrary("myplayer");
    }


    public void play() {
        nativePlay(handle);
    }

    private native void nativePlay(long handle);
    public void prepared(){
        nativePrepare(handle);
    }

    private native void  nativePrepare(long handle);

    interface OnPreparedListener {
        void onPrepared();
    }

    interface OnErrorListener {
        void onError(int code);
    }

    private OnPreparedListener onPreparedListener;
    private OnErrorListener onErrorListener;

    private long handle = 0L;

    private native long nativeInit();
    private native void nativeSetPath(long handle,String path);

    private native void nativeSetSurface(long handle,Surface surface);

    public  void setSurface(Surface surface){
        nativeSetSurface(handle,surface);
    }

    public Player(){
        handle = nativeInit();
    }


    private void onError(int code){
        if(onErrorListener != null){
            onErrorListener.onError(code);
        }
    }

    private void onPrepared(){
        if(onPreparedListener != null){
            onPreparedListener.onPrepared();
        }
    }

    public void setPath(String path) {
        nativeSetPath(handle,path);
    }

    public void setOnPreparedListener(OnPreparedListener onPreparedListener) {
        this.onPreparedListener = onPreparedListener;
    }

    public void setOnErrorListener(OnErrorListener onErrorListener) {
        this.onErrorListener = onErrorListener;
    }
}
