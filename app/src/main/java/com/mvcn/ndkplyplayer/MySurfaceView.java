package com.mvcn.ndkplyplayer;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.view.MotionEvent;

public class MySurfaceView extends GLSurfaceView
{
    private MyRenderer mRenderer;
    private float mPreviousX;
    private float mPreviousY;
    private final float TOUCH_SCALE_FACTOR = 90.0f / 320;

    public static final int STATUS_INIT = 1;
    public static final int STATUS_ZOOM_OUT = 2;
    public static final int STATUS_ZOOM_IN = 3;
    public static final int STATUS_MOVE = 4;
    //
    private int currentStatus;      // Current state, options: STATUS_INIT、STATUS_ZOOM_OUT、STATUS_ZOOM_IN and STATUS_MOV
    private float movedDistanceX;   // Distance moved along X-axis
    private float movedDistanceY;   // Distance moved along Y-axis

    private float lastXMove = -1;   // X-value of last move
    private float lastYMove = -1;   // Y-alue of last move

    private float centerPointX;     // X-value of center point when two fingers moved
    private float centerPointY;     // Y-value of center point when two fingers moved

    private double lastFingerDis;   // Fingers distance record last
    private float initRatio = 1.0f;  // Initial zoom ratio
    private float scaledRatio;       // Ratio scaled by two fingers ratio
    private float totalRatio = 1.0f;


    public MySurfaceView(Context context)
    {
        super(context);
        this.setEGLContextClientVersion(2);
        mRenderer = new MyRenderer(context);
        this.setRenderer(mRenderer);
        this.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        mRenderer.scale = totalRatio;
    }
    /*
    public boolean onTouchEvent(final MotionEvent event) {

        float x = event.getX();
        float y = event.getY();
        switch (event.getAction()) {
            case MotionEvent.ACTION_MOVE:
                float dx = x - mPreviousX;
                float dy = y - mPreviousY;
                mRenderer.mAngleX += dx * TOUCH_SCALE_FACTOR;
                mRenderer.mAngleY += dy * TOUCH_SCALE_FACTOR;
                requestRender();
        }
        mPreviousX = x;
        mPreviousY = y;
        return true;
    }
   */

    @Override
    public boolean onTouchEvent(MotionEvent event)
    {
        switch (event.getActionMasked()){
            case MotionEvent.ACTION_POINTER_DOWN:
                if (event.getPointerCount() == 2) {
                    // 当有两个手指按在屏幕上时，计算两指之间的距离
                    lastFingerDis = distanceBetweenFingers(event);
                }
                break;
            case MotionEvent.ACTION_MOVE:
                if (event.getPointerCount() == 1)
                {
                    // Single finger moves
                    float xMove = event.getX();
                    float yMove = event.getY();
                    if (lastXMove == -1 && lastYMove == -1) {
                        lastXMove = xMove;
                        lastYMove = yMove;
                    }
                    currentStatus = STATUS_MOVE;
                    movedDistanceX = xMove - lastXMove;
                    movedDistanceY = yMove - lastYMove;
                    mRenderer.mAngleX += movedDistanceX * TOUCH_SCALE_FACTOR;
                    mRenderer.mAngleY += movedDistanceY * TOUCH_SCALE_FACTOR;
                    lastXMove = xMove;
                    lastYMove = yMove;
                }
                else if(event.getPointerCount() == 2)
                {
                    // Double fingers move
                    Log.i("MySurfaceView", "Two Fingers moved!");
                    centerPointBetweenFingers(event);
                    double fingerDis = distanceBetweenFingers(event);
                    if (fingerDis > lastFingerDis) {
                        currentStatus = STATUS_ZOOM_OUT;
                    } else {
                        currentStatus = STATUS_ZOOM_IN;
                    }
                    // 进行缩放倍数检查，最大只允许将图片放大10倍，最小可以缩小到初始化比例的0.1倍
                    if ((currentStatus == STATUS_ZOOM_OUT && totalRatio < 10 * initRatio)
                            || (currentStatus == STATUS_ZOOM_IN && totalRatio > 0.1 * initRatio)) {
                        scaledRatio = (float) (fingerDis / lastFingerDis);
                        totalRatio = totalRatio * scaledRatio;
                        if (totalRatio > 10 * initRatio) {
                            totalRatio = 10 * initRatio;
                        } else if (totalRatio < 0.1 * initRatio) {
                            totalRatio = 0.1f * initRatio;
                        }
                        mRenderer.scale = totalRatio;
                        lastFingerDis = fingerDis;
                    }
                }
            break;
            case MotionEvent.ACTION_POINTER_UP:
                if (event.getPointerCount() == 2) {
                    // 手指离开屏幕时将临时值还原
                    lastXMove = -1;
                    lastYMove = -1;
                }
                break;
            case MotionEvent.ACTION_UP:
                // 手指离开屏幕时将临时值还原
                lastXMove = -1;
                lastYMove = -1;
                break;
            default:
                break;
        }
        requestRender();
        return true;
    }

    /**
     * Calculate center point between two fingers
     * @param event
     */
    private void centerPointBetweenFingers(MotionEvent event) {
        float xPoint0 = event.getX(0);
        float yPoint0 = event.getY(0);
        float xPoint1 = event.getX(1);
        float yPoint1 = event.getY(1);
        centerPointX = (xPoint0 + xPoint1) / 2;
        centerPointY = (yPoint0 + yPoint1) / 2;
    }

    /**
     * Calculate distance between two fingers
     * @param event
     * @return The distance between two fingers
     */
    private double distanceBetweenFingers(MotionEvent event) {
        float disX = Math.abs(event.getX(0) - event.getX(1));
        float disY = Math.abs(event.getY(0) - event.getY(1));
        return Math.sqrt(disX * disX + disY * disY);
    }

}
