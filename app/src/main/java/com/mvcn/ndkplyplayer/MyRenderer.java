package com.mvcn.ndkplyplayer;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.Log;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MyRenderer implements GLSurfaceView.Renderer {
    public float mAngleX;
    public float mAngleY;
    public float scale;
    private Context mContext;
    public MyRenderer(Context pContext) {
        super();
        mContext = pContext;
    }

    //Called once to set up the view's OpenGL ES environment.
    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        String vetexShaderStr = LoadShaderStr(mContext, R.raw.vshader);
        String fragmentShaderStr = LoadShaderStr(mContext, R.raw.fshader);
        if(!nativeInitGLES20(vetexShaderStr, fragmentShaderStr))
            Log.e("NativeRenderer", "Fail to init gles!");
    }

    //Called for each redraw of the view.
    @Override
    public void onDrawFrame(GL10 gl) {
        nativeDrawGraphics(mAngleX,mAngleY, scale);
    }

    //Called if the geometry of the view changes, for example when the device's
    //screen orientation changes.
    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        nativeSurfaceChanged(width, height);
    }
    // Load OpenGL Shading Language (GLSL) contents from resource ID
    private String LoadShaderStr(Context context, int resId) {
        StringBuffer strBuf = new StringBuffer();
        try {
            InputStream inputStream = context.getResources().openRawResource(resId);
            // Setup Bufferedreader
            BufferedReader in = new BufferedReader(new InputStreamReader(inputStream));
            String read = in.readLine();
            while (read != null) {
                strBuf.append(read + "\n");
                read = in.readLine();
            }
            strBuf.deleteCharAt(strBuf.length() - 1);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return strBuf.toString();
    }

    /**
     * Native methods that is implemented by the native library,
     * which is packaged with this application.
     */
    private static native boolean nativeInitGLES20(String vertexShaderStr, String fragmentShaderStr);
    private static native void nativeDrawGraphics(float angleX, float angleY, float scale);
    private static native void nativeSurfaceChanged(int width, int height);


    static {
        System.loadLibrary("NativeRenderer");
    }
}
