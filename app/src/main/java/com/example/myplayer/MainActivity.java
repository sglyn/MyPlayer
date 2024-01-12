package com.example.myplayer;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import com.example.myplayer.databinding.ActivityMainBinding;

import java.io.File;
import java.io.FileInputStream;

public class MainActivity extends AppCompatActivity implements  SurfaceHolder.Callback {
    public static final String TAG = "FFMPEG";
    private ActivityMainBinding binding;
    Player player = new Player();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        requestPermision();

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        binding.vPlay.setOnClickListener(v -> {
            String path = "/sdcard/123.mp4";
            File file = new File(path);
            Log.e(TAG, path + " exists-> " + file.exists());
            player.setPath(path);
            player.prepared();


        });

        SurfaceView sufaceView = binding.surfaceView;
        sufaceView.getHolder().addCallback(this);


        player.setOnPreparedListener(new Player.OnPreparedListener() {
            @Override
            public void onPrepared() {
                Log.e(TAG, "MainActivity::onPrepared()");
                player.play();
            }
        });


        player.setOnErrorListener(new Player.OnErrorListener() {
            @Override
            public void onError(int code) {
                Log.e(TAG, "MainActivity::onError() code=  " + code);
            }
        });
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
        player.setSurface(holder.getSurface());
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {

    }


    private void requestPermision() {
        //常量，用于回调
        int MY_PERMISSION_APPLY = 1;
        //要使用的相机和存储权限
        String[] permissions = new String[]{
                Manifest.permission.READ_EXTERNAL_STORAGE,
                Manifest.permission.INTERNET,

                Manifest.permission.WRITE_EXTERNAL_STORAGE,
                Manifest.permission.MANAGE_EXTERNAL_STORAGE
        };
        //循环得到未同意权限
        for (int i = 0; i < permissions.length; i++) {
            if (ContextCompat.checkSelfPermission(this, permissions[i]) != PackageManager.PERMISSION_GRANTED) {
               Log.e(TAG,"申请 " +  permissions[i]);
                ActivityCompat.requestPermissions(this, permissions, MY_PERMISSION_APPLY);
            }
        }

    }



}