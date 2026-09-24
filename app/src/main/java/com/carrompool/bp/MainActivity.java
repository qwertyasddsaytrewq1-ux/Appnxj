package com.carrompool.bp; // ← Schimbă cu package-ul tău

import android.app.Activity;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.PixelFormat;
import android.graphics.drawable.BitmapDrawable;
import android.net.Uri;
import android.os.Bundle;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import com.miniclip.carrom.R;

public class MainActivity extends AppCompatActivity {

    private static View floatingButtonView = null;
    private static Activity currentActivity = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        currentActivity = this;
        //setContentView(R.layout.activity_main);

        // Invocare din MainActivity
        floatingbtn(this);
    }

    /**
     * Creează și afișează butonul floating draggable cu logo Telegram.
     * Poate fi invocat din orice altă clasă: MainActivity.floatingbtn(activity);
     *
     * @param activity — Activity-ul curent (necesar pentru WindowManager)
     */
    public static void floatingbtn(Activity activity) {

        // Evită duplicate dacă metoda e apelată de mai multe ori
        if (floatingButtonView != null) return;

        WindowManager windowManager = (WindowManager) activity.getSystemService(Activity.WINDOW_SERVICE);

        // ── Layout params (TYPE_APPLICATION = fără permisiune overlay) ────────
        WindowManager.LayoutParams params = new WindowManager.LayoutParams(
                WindowManager.LayoutParams.WRAP_CONTENT,
                WindowManager.LayoutParams.WRAP_CONTENT,
                WindowManager.LayoutParams.TYPE_APPLICATION,   // ← no overlay perm needed
                WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE,
                PixelFormat.TRANSLUCENT
        );
        params.gravity = Gravity.TOP | Gravity.START;
        params.x = 50;
        params.y = 200;

        // ── Container principal ───────────────────────────────────────────────
        LinearLayout container = new LinearLayout(activity);
        container.setOrientation(LinearLayout.HORIZONTAL);
        container.setGravity(Gravity.CENTER_VERTICAL);
        container.setPadding(24, 16, 28, 16);
        container.setElevation(12f);

        // Fundal rotunjit albastru Telegram
        container.setBackground(createRoundedBackground());

        // ── Offline heart icon ───────────────────────────────────────────────
        ImageView icon = new ImageView(activity);
        icon.setLayoutParams(new LinearLayout.LayoutParams(72, 72));
        icon.setScaleType(ImageView.ScaleType.FIT_CENTER);

        try {
            Bitmap bitmap = BitmapFactory.decodeResource(
                    activity.getResources(),
                    R.drawable.offline_heart
            );
            if (bitmap != null) {
                icon.setImageBitmap(bitmap);
            } else {
                icon.setBackgroundColor(0xFFFFFFFF);
            }
        } catch (Exception e) {
            icon.setBackgroundColor(0xFFFFFFFF);
        }

        // ── Text buton ────────────────────────────────────────────────────────
        TextView text = new TextView(activity);
        LinearLayout.LayoutParams textParams = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
        );
        textParams.setMargins(16, 0, 0, 0);
        text.setLayoutParams(textParams);
        text.setText("GALVNIC ENGINE VIP SERVER");
        text.setTextColor(0xFFFFFFFF);
        text.setTextSize(14f);
        text.setTypeface(android.graphics.Typeface.DEFAULT_BOLD);

        container.addView(icon);
        container.addView(text);

        // ── Click → deschide link Telegram ───────────────────────────────────
        container.setOnClickListener(v -> {
            try {
                Intent intent = new Intent(Intent.ACTION_VIEW,
                        Uri.parse("https://t.me/GVM_TRUST"));
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                activity.startActivity(intent);
            } catch (Exception e) {
                e.printStackTrace();
            }
        });

        // ── Drag logic (mișcare buton) ────────────────────────────────────────
        container.setOnTouchListener(new View.OnTouchListener() {
            private int initialX, initialY;
            private float initialTouchX, initialTouchY;
            private long touchStartTime;
            private static final int CLICK_DURATION_MS = 200;
            private static final int DRAG_THRESHOLD_PX = 10;
            private boolean isDragging = false;

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                switch (event.getAction()) {
                    case MotionEvent.ACTION_DOWN:
                        initialX = params.x;
                        initialY = params.y;
                        initialTouchX = event.getRawX();
                        initialTouchY = event.getRawY();
                        touchStartTime = System.currentTimeMillis();
                        isDragging = false;
                        return true;

                    case MotionEvent.ACTION_MOVE:
                        float deltaX = event.getRawX() - initialTouchX;
                        float deltaY = event.getRawY() - initialTouchY;

                        if (!isDragging &&
                                (Math.abs(deltaX) > DRAG_THRESHOLD_PX ||
                                        Math.abs(deltaY) > DRAG_THRESHOLD_PX)) {
                            isDragging = true;
                        }

                        if (isDragging) {
                            params.x = initialX + (int) deltaX;
                            params.y = initialY + (int) deltaY;
                            windowManager.updateViewLayout(container, params);
                        }
                        return true;

                    case MotionEvent.ACTION_UP:
                        long clickDuration = System.currentTimeMillis() - touchStartTime;
                        if (!isDragging && clickDuration < CLICK_DURATION_MS) {
                            v.performClick(); // declanșează onClick
                        }
                        return true;
                }
                return false;
            }
        });

        // ── Adaugă butonul în fereastră ───────────────────────────────────────
        windowManager.addView(container, params);
        floatingButtonView = container;
    }



    /**
     * Enables/disables Android's secure-window flag so screenshots and
     * screen recordings cannot capture this Activity while enabled.
     */
    public static void setHideRecording(boolean enabled) {
        final Activity activity = currentActivity;
        if (activity == null) return;

        activity.runOnUiThread(() -> {
            try {
                if (enabled) {
                    activity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_SECURE);
                } else {
                    activity.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_SECURE);
                }
            } catch (Throwable ignored) {}
        });
    }

    /**
     * Elimină butonul floating (apelează în onDestroy dacă vrei cleanup).
     */
    public static void removeFloatingBtn(Activity activity) {
        if (floatingButtonView != null) {
            WindowManager wm = (WindowManager) activity.getSystemService(Activity.WINDOW_SERVICE);
            try {
                wm.removeView(floatingButtonView);
            } catch (Exception ignored) {}
            floatingButtonView = null;
        }
    }

    @Override
    protected void onDestroy() {
        currentActivity = null;
        super.onDestroy();
        removeFloatingBtn(this);
    }

    // ── Helper: fundal rotunjit albastru Telegram ─────────────────────────────
    private static android.graphics.drawable.Drawable createRoundedBackground() {
        android.graphics.drawable.GradientDrawable shape = new android.graphics.drawable.GradientDrawable();
        shape.setShape(android.graphics.drawable.GradientDrawable.RECTANGLE);
        shape.setCornerRadius(60f);
        shape.setColor(0xFFB00000); // login red
        return shape;
    }
}
