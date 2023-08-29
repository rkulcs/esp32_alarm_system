package alarm.system.app;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.app.NotificationCompat;
import androidx.core.app.NotificationManagerCompat;

import android.content.pm.PackageManager;
import android.os.Bundle;
import android.util.Log;
import android.widget.Toast;

import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.firebase.messaging.FirebaseMessaging;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "Main";

    private static final String ALARM_TOPIC = "alarm";

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        subscribeToAlarmTopic();
    }

    private void subscribeToAlarmTopic() {

        FirebaseMessaging.getInstance().subscribeToTopic(ALARM_TOPIC)
                .addOnCompleteListener(task -> {
                    if (task.isSuccessful())
                        Log.d(TAG, getString(R.string.successful_subscription));
                    else
                        Log.e(TAG, getString(R.string.unsuccessful_subscription));
                });
    }
}