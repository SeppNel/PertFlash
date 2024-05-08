package com.seppnel.pertflash;

import android.graphics.Color;
import android.os.Bundle;
import android.widget.FrameLayout;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import java.io.IOException;

public class MainActivity extends AppCompatActivity {

    private FrameLayout container;
    private TextView bottomTextView;
    private TextView avgText;
    private int currentColor = Color.BLACK;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_main);
        bottomTextView = findViewById(R.id.bottomTextView);
        avgText = findViewById(R.id.avgText);

        container = findViewById(R.id.container);
    }

    @Override
    protected void onStart() {
        super.onStart();

        // Initialize Serial class
        Serial serial;
        try {
            serial = new Serial(this);
            //serial.thread();
        } catch (IOException e) {
            throw new RuntimeException(e);
        }

    }

    public void changeColor(){
        if (currentColor == Color.BLACK){
            container.setBackgroundColor(Color.WHITE);
            currentColor = Color.WHITE;
        }
        else {
            container.setBackgroundColor(Color.BLACK);
            currentColor = Color.BLACK;
        }
    }

    public void updateText(String newText) {
        bottomTextView.setText(newText);
    }

    public void updateAvgText(String newText) {
        avgText.setText(newText);
    }
}

