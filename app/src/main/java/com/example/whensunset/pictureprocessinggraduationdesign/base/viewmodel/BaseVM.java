package com.example.whensunset.pictureprocessinggraduationdesign.base.viewmodel;

import android.databinding.Observable;
import android.databinding.ObservableField;

import com.example.whensunset.pictureprocessinggraduationdesign.base.util.MyLog;
import com.example.whensunset.pictureprocessinggraduationdesign.base.ObservableAction;
import com.example.whensunset.pictureprocessinggraduationdesign.base.util.ObserverParamMap;
import com.example.whensunset.pictureprocessinggraduationdesign.base.uiaction.ClickUIAction;
import com.example.whensunset.pictureprocessinggraduationdesign.base.uiaction.UIActionManager;

import java.util.ArrayList;
import java.util.List;

import io.reactivex.Flowable;
import io.reactivex.functions.Function;

import static com.example.whensunset.pictureprocessinggraduationdesign.base.uiaction.UIActionManager.CLICK_ACTION;

/**
 * Created by whensunset on 2018/3/21.
 */

public abstract class BaseVM {
    public static final String TAG = "何时夕:BaseVM";

    protected final ObservableField<? super Object> mShowToastListener = new ObservableField<>();
    protected final List<ObservableField<? super Object>> mEventListenerList = new ArrayList<>();
    public UIActionManager mUIActionManager;

    public BaseVM(List<ObservableField<? super Object>> eventListenerList) {
        mEventListenerList.addAll(eventListenerList);
    }

    public BaseVM(int listenerSize) {
        for (int i = 0; i < listenerSize; i++) {
            mEventListenerList.add(new ObservableField<>());
        }
    }

    public BaseVM() {
    }

    protected void initDefaultUIActionManager() {
        mUIActionManager = new UIActionManager(this , CLICK_ACTION);
    }

    protected Flowable<Integer> getDefaultClickThrottleFlowable() {
        return mUIActionManager
                .<ClickUIAction>getDefaultThrottleFlowable(CLICK_ACTION)
                .map(ClickUIAction::getLastEventListenerPosition);

    }

    public void showToast(String message) {
        mShowToastListener.set(ObserverParamMap.setToastMessage(message));
    }

      public void checkEventListenerList(int eventListenerPosition) {
        if (eventListenerPosition >= mEventListenerList.size() || eventListenerPosition < 0) {
            throw new RuntimeException("数组越界，没有那么多listener");
        }
        MyLog.d(TAG, "checkEventListenerList", "状态:class:eventListenerPosition:", "", getRealClassName(), eventListenerPosition);
    }

    public List<ObservableField<? super Object>> getEventListenerList() {
        return mEventListenerList;
    }

    public ObservableField<? super Object> getShowToastListener() {
        return mShowToastListener;
    }

    public String getRealClassName() {
        return getClass().getSimpleName();
    }

    public static void initListener(ChildBaseVM childVM, ObservableAction observableAction, Integer... listenerPositions) {
        if (childVM == null) {
            throw new RuntimeException("被监听的childVM为null");
        }
        if (observableAction == null) {
            throw new RuntimeException("被监听的行为为null");
        }
        if (listenerPositions == null) {
            throw new RuntimeException("被传入的监听器的position列表为null");
        }
        // 监听 某个childVM的变化 以运行observableAction
        Flowable.fromArray(listenerPositions)
                .filter(position -> {
                    MyLog.d(TAG, "initListener", "状态:position:childVM.mClickListenerList.size():", "过滤监听器", position, childVM.mEventListenerList.size());
                    return !(position == null || position >= childVM.mEventListenerList.size());
                }).map((Function<Integer, ObservableField<? super Object>>) childVM.mEventListenerList::get)
                .subscribe(observableField -> observableField.addOnPropertyChangedCallback(new Observable.OnPropertyChangedCallback() {
                    @Override
                    public void onPropertyChanged(Observable observable, int i) {
                        observableAction.onPropertyChanged(observable, i);
                    }
                }));

    }

}
