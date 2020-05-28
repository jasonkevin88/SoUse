package com.ndk.so.use;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

public class MainActivity extends AppCompatActivity {

  private Handler handler = new Handler(Looper.getMainLooper());

  private ThreadUtil threadUtil;

  private static int REQ_PERMISSION_CODE = 1001;

  private static final String[] PERMISSIONS = {Manifest.permission.READ_EXTERNAL_STORAGE,
      Manifest.permission.WRITE_EXTERNAL_STORAGE};

  private static String SD_CARD_PATH = Environment.getExternalStorageDirectory().getAbsolutePath();

  // Used to load the 'native-lib' library on application startup.
  static {
    System.loadLibrary("asTest");
  }

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);
    checkAndRequestPermissions();

    // Example of a call to a native method
    final TextView tv = findViewById(R.id.sample_text);
    tv.setText(stringFromJNI());
    tv.setOnClickListener(new View.OnClickListener() {
      @Override
      public void onClick(View v) {
        int result = doAdd(99, 66);
        tv.setText("result = " + result);
      }
    });

    threadUtil = new ThreadUtil();
    threadUtil.setJniEnv();
  }

  /**
   * A native method that is implemented by the 'native-lib' native library,
   * which is packaged with this application.
   */
  public native String stringFromJNI();

  public native int doAdd(int a, int b);

  public void onDiff(View view) {

    (new Thread(){
      @Override
      public void run() {
        String srcPath = SD_CARD_PATH + File.separator + "ZERO.rmvb";
        String partPath = SD_CARD_PATH + File.separator + "ZERO_%d.rmvb";
        final int result = FileUtil.diffFile(srcPath, partPath, 4);
        runOnUiThread(new Runnable() {
          @Override
          public void run() {
            if(result == 1) {
              showToast("拆分成功");
            }else {
              showToast("拆分失败");
            }
          }
        });
      }
    }).start();
  }

  public void onMerge(View view) {
    (new Thread(){
      @Override
      public void run() {
        String srcPath = SD_CARD_PATH + File.separator + "ZERO_merge.rmvb";
        String partPath = SD_CARD_PATH + File.separator + "ZERO_%d.rmvb";
        final int result = FileUtil.mergeFile(srcPath, partPath, 4);
        runOnUiThread(new Runnable() {
          @Override
          public void run() {
            if(result == 1) {
              showToast("合并成功");
            }else {
              showToast("合并失败");
            }
          }
        });
      }
    }).start();

  }

  public void onCreateThread(View view) {
    threadUtil.createThread();
  }

  public void onReleaseThread(View view) {
    threadUtil.releaseThread();
  }

  private void showToast(String text){
    Toast.makeText(MainActivity.this,text, Toast.LENGTH_SHORT).show();
  }

  /**
   * 权限检测以及申请
   */
  private void checkAndRequestPermissions() {
    // Manifest.permission.WRITE_EXTERNAL_STORAGE 和  Manifest.permission.READ_PHONE_STATE是必须权限，允许这两个权限才会显示广告。

    if (hasPermission(Manifest.permission.READ_EXTERNAL_STORAGE)
        && hasPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE)) {

    } else {
      ActivityCompat.requestPermissions(this, PERMISSIONS, REQ_PERMISSION_CODE);
    }

  }


  /**
   * 权限判断
   * @param permissionName
   * @return
   */
  private boolean hasPermission(String permissionName) {
    return ActivityCompat.checkSelfPermission(this, permissionName)
        == PackageManager.PERMISSION_GRANTED;
  }

  @Override
  public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {

    if (requestCode == REQ_PERMISSION_CODE) {
      checkAndRequestPermissions();
    }

    super.onRequestPermissionsResult(requestCode, permissions, grantResults);
  }

}
