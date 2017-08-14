package com.ryanhunter.monitorbot;

import android.app.Activity;
import android.view.View;
import android.widget.AdapterView;

public class SpinnerActivity extends Activity implements AdapterView.OnItemSelectedListener {

    public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
        // An item was selected. You can retrieve the selected item using
        MainActivity.sendModeMessage((int) id);
    }

    public void onNothingSelected(AdapterView<?> parent) {
        // Another interface callback
    }
}