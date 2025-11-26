package com.lunasdk.multiplatformsample;

import static android.view.WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE;

import android.app.NativeActivity;
import android.view.WindowInsets;
import android.view.WindowInsetsController;

import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends NativeActivity {
    static {
        System.loadLibrary("MultiPlatformSample");
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);

        if (hasFocus) {
            hideSystemUi();
        }
    }

    private void hideSystemUi() {
        WindowInsetsController controller = getWindow().getDecorView().getWindowInsetsController();
        controller.hide(WindowInsets.Type.statusBars());
        controller.hide(WindowInsets.Type.navigationBars());
        controller.setSystemBarsBehavior(BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
        getWindow().setDecorFitsSystemWindows(false);
    }
}