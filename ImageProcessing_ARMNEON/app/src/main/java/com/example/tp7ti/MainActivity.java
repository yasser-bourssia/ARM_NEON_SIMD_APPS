package com.example.tp7ti;

import android.Manifest;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.net.Uri;
import android.os.Environment;
import android.os.Parcel;
import android.provider.MediaStore;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.List;

public class MainActivity extends AppCompatActivity {

    private final int requestCode_Directory = 1;
    private final int requestCode_Permissions = 2;

    private final String[] allPermissions = {Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_EXTERNAL_STORAGE};

    private Uri uriOrigin = null;
    private Bitmap mBitmap = null;

    private ImageView imageView = null;
    private TextView textView_Filename = null;

    private Intent intent_TI = null;

    // Action correspondant à la fin d'un calcul de TI
    public static final String mBroadcastFinTI = "com.example.franck.broadcast.finTI";

    // Intent pour filtrage
    private IntentFilter intentFilter = null;
    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String tmp = intent.getAction();
            if( intent.getAction() == mBroadcastFinTI )
            {
                mBitmap = intent.getParcelableExtra("IMAGE_TI");
                imageView.setImageBitmap(mBitmap);
            }
        }
    };

    private void checkPermissions() {
        boolean[] Failures = new boolean[allPermissions.length];
        int NbFailures = 0;

        for (int i = 0; i < allPermissions.length; i++) {
            String permission = allPermissions[i];
            if (ActivityCompat.checkSelfPermission(this, permission) == PackageManager.PERMISSION_GRANTED) {
                Failures[i] = false;
            } else {
                Failures[i] = true;
                NbFailures++;
            }
        }

        if (NbFailures == 0)
            return;

        String[] Requests = new String[NbFailures];
        for (int i = 0, j = 0; i < allPermissions.length; i++) {
            if (Failures[i])
                Requests[j++] = allPermissions[i];
        }

        ActivityCompat.requestPermissions(this, Requests, requestCode_Permissions);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        if (requestCode == requestCode_Permissions) {
            for (int result : grantResults)
                if (result != PackageManager.PERMISSION_GRANTED)
                    finish();
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        imageView = (ImageView)findViewById(R.id.imageView);
        textView_Filename = (TextView)findViewById(R.id.textView_Directory);

        intent_TI = new Intent(this, MyIntentService.class);

        intentFilter = new IntentFilter();
        intentFilter.addAction(mBroadcastFinTI);
        registerReceiver(mReceiver, intentFilter);
        checkPermissions();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        switch (requestCode) {
            case requestCode_Directory:
                if (resultCode == RESULT_OK) {          // Une image a été sélectionnée
                    Uri tmpUri = data.getData();
                    try {
                        uriOrigin = tmpUri;                                                             // On en récupère le lien
                        mBitmap = MediaStore.Images.Media.getBitmap(getContentResolver(), uriOrigin);   // On charge l'image pour l'afficher dans son contrôle
                        mBitmap = mBitmap.copy(mBitmap.getConfig(), true);
                        imageView.setImageBitmap(mBitmap);
                        // On récupère le nom du fichier pour le TextView
                        Cursor cursor = getContentResolver().query(tmpUri, new String[] {MediaStore.MediaColumns.DISPLAY_NAME}, null, null, null);
                        if(cursor!=null) {
                            cursor.moveToFirst();
                            textView_Filename.setText(cursor.getString(0));
                        }
                        else
                        {
                            textView_Filename.setText("Erreur !");
                        }

                    } catch (Exception e)
                    {
                    }
                }
                break;

            default:
                break;
        }

        super.onActivityResult(requestCode, resultCode, data);
    }

    public void onClick_Load(View v) {
        Intent fileIntent = new Intent(Intent.ACTION_GET_CONTENT);
        fileIntent.setType("*/*");
        startActivityForResult(fileIntent, requestCode_Directory);
    }

    public void onClick_ReLoad(View v)
    {
        if(uriOrigin != null)
        {
            try {
                mBitmap = MediaStore.Images.Media.getBitmap(getContentResolver(), uriOrigin);// BitmapFactory.decodeFile(tmpUri.getPath().toString());
                mBitmap = mBitmap.copy(mBitmap.getConfig(), true);
                imageView.setImageBitmap(mBitmap);
            }catch (Exception e) { }
        }
    }

    public void onClick_Gris(View v)
    {
        intent_TI.putExtra("ACTION", "GRIS");
        intent_TI.putExtra("IMAGE_SRC", mBitmap);
        startService(intent_TI);
    }

    public void onClick_Laplace(View v)
    {
        intent_TI.putExtra("ACTION", "LAPLACE");
        intent_TI.putExtra("IMAGE_SRC", mBitmap);
        startService(intent_TI);
    }
}


