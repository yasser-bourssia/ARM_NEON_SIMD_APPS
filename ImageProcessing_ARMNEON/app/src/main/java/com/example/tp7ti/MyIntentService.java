package com.example.tp7ti;

import android.app.IntentService;
import android.content.Intent;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.util.Log;

import java.nio.ByteBuffer;


public class MyIntentService extends IntentService {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    public native void nativeNiveauxGris(byte[] data, int nbPixels);
    public native void nativeLaplace(byte[] data, int Width, int Height, byte[] result);

    public MyIntentService() {
        super("MyIntentService");
    }

    @Override
    protected void onHandleIntent(Intent intent) {
        Log.v("DEBUG_TEST","IntentService TI");
        Bitmap bmp = intent.getParcelableExtra("IMAGE_SRC");
        String Action = intent.getStringExtra("ACTION");

        switch(Action)
        {
            case "GRIS":
                ColorToGrey(bmp);
                break;

            case "LAPLACE":
                Laplace(bmp);
                break;

            default:
        }

        Intent broadcast_intent = new Intent();
        broadcast_intent.setAction(MainActivity.mBroadcastFinTI);
        broadcast_intent.putExtra("IMAGE_TI", bmp);
        sendBroadcast(broadcast_intent);
    }

    private void ColorToGrey(Bitmap bmp)
    {
        int NbPixels = bmp.getWidth() * bmp.getHeight();

        ByteBuffer buffer = ByteBuffer.allocate(bmp.getByteCount());
        bmp.copyPixelsToBuffer(buffer);
        byte[] tmp32 = buffer.array();
        nativeNiveauxGris(tmp32, NbPixels);
        bmp.copyPixelsFromBuffer(ByteBuffer.wrap(tmp32));
    }

    private void Laplace(Bitmap bmp)
    {
        int Width = bmp.getWidth();
        int Height = bmp.getHeight();
        int NbPixels = Width * Height;

        ByteBuffer buffer = ByteBuffer.allocate(bmp.getByteCount());
        bmp.copyPixelsToBuffer(buffer);
        byte[] tmp32 = buffer.array();
        byte[] Laplace = new byte[tmp32.length];
        nativeLaplace(tmp32, Width, Height, Laplace);
        bmp.copyPixelsFromBuffer(ByteBuffer.wrap(Laplace));
    }
}
