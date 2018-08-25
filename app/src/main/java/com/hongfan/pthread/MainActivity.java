package com.hongfan.pthread;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    private PosixThread mPosixThread;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        initView();
        initData();
    }

    private void initView() {
        findViewById(R.id.bt_test).setOnClickListener(this);
    }

    private void initData() {
        mPosixThread = new PosixThread();
        mPosixThread.init();
    }

    @Override
    public void onClick(View v) {
        mPosixThread.pthread();
    }

    @Override
    protected void onDestroy() {
        mPosixThread.destroy();
        super.onDestroy();
    }
}
