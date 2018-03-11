package com.example.whensunset.pictureprocessinggraduationdesign.pictureProcessing;

import com.example.whensunset.pictureprocessinggraduationdesign.base.MyLog;
import com.example.whensunset.pictureprocessinggraduationdesign.impl.BaseMyConsumer;
import com.example.whensunset.pictureprocessinggraduationdesign.impl.LinkedMyConsumer;

import org.opencv.core.Mat;
import org.opencv.core.Rect;

/**
 * Created by whensunset on 2018/3/8.
 */

public class CutMyConsumer extends LinkedMyConsumer {
    public static final String TAG = "何时夕:CutMyConsumer";

    private Rect mRect;

    public CutMyConsumer(Rect rect) {
        super(null , null);
        mRect = rect;
    }

    @Override
    protected Mat onNewResultImpl(Mat oldResult) {
        MyLog.d(TAG, "onNewResultImpl", "oldResult:" , oldResult);

        if (oldResult == null) {
            throw new RuntimeException("被剪裁的Mat 不可为null");
        }

        if (mRect == null) {
            throw new RuntimeException("需要剪裁的区域Rect 不可为null");
        }

        Mat newResult = new Mat();
        cut(oldResult.nativeObj , newResult.nativeObj , mRect.x , mRect.y , mRect.width , mRect.height);
        return newResult;
    }

    @Override
    protected void onFailureImpl(Throwable t) {

    }

    @Override
    protected void onCancellationImpl() {

    }

    @Override
    public void copy(BaseMyConsumer baseMyConsumer) {
        MyLog.d(TAG, "copy", "beCopyConsumer:" , baseMyConsumer);

        if (baseMyConsumer == null) {
            MyLog.d(TAG, "copy", "传入的被拷贝的 consumer 为null");
            return;
        }

        if (!(baseMyConsumer instanceof CutMyConsumer)) {
            throw new RuntimeException("被拷贝的 consumer 需要和拷贝的 consumer 类型一致");
        }

        CutMyConsumer beCopyConsumer = (CutMyConsumer) baseMyConsumer;
        this.mRect.set(new double[]{beCopyConsumer.mRect.x , beCopyConsumer.mRect.y , beCopyConsumer.mRect.width , beCopyConsumer.mRect.height});;
    }

    private native void cut(long in_mat_addr , long out_mat_addr , int x , int y , int width , int height);

    @Override
    public Mat undo(Mat oldResult) {
        if (mPreviousConsumer == null) {
            MyLog.d(TAG, "undo", "已经是最第一个 consumer 了");
            return oldResult;
        }
        return mPreviousConsumer.onNewResult(oldResult);
    }

    public Rect getRect() {
        return mRect;
    }

    public void setRect(Rect rect) {
        mRect = rect;
    }


    @Override
    public String toString() {
        return "CutMyConsumer{" +
                "mRect=" + mRect +
                '}';
    }
}
