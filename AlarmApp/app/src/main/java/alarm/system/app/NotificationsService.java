package alarm.system.app;

import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.os.Build;
import android.util.Log;

import androidx.core.app.ActivityCompat;
import androidx.core.app.NotificationCompat;
import androidx.core.app.NotificationManagerCompat;

import com.google.firebase.messaging.FirebaseMessagingService;
import com.google.firebase.messaging.RemoteMessage;

import java.time.LocalDateTime;

public class NotificationsService extends FirebaseMessagingService {

    private static final String TAG = "NotificationsService";

    private static final String NOTIFICATION_CHANNEL_ID = "fcm_default_channel";

    private static final String NOTIFICATION_BODY_KEY = "body";

    private static int notificationId = 0;

    @Override
    public void onMessageReceived(RemoteMessage message) {

        Log.d(TAG, "Message received");
        sendNotification(message.getData().get(NOTIFICATION_BODY_KEY));
        saveMessage(message.getData().get(NOTIFICATION_BODY_KEY));
    }

    @Override
    public void onNewToken(String token) {
        Log.d(TAG, "New token: " + token);
    }

    private void sendNotification(String message) {

        NotificationCompat.Builder builder = new NotificationCompat.Builder(this, NOTIFICATION_CHANNEL_ID)
                .setSmallIcon(com.google.android.material.R.drawable.notification_icon_background)
                .setContentTitle(getString(R.string.notification_title))
                .setContentText(message)
                .setPriority(NotificationCompat.PRIORITY_DEFAULT);

        if (ActivityCompat.checkSelfPermission(this, android.Manifest.permission.POST_NOTIFICATIONS) != PackageManager.PERMISSION_GRANTED) {
            return;
        }

        NotificationManagerCompat manager = NotificationManagerCompat.from(this);
        manager.notify(notificationId++, builder.build());
    }

    private void saveMessage(String message) {

        SharedPreferences sharedPreferences = getSharedPreferences(
                getString(R.string.notifications_history_file), MODE_PRIVATE
        );
        SharedPreferences.Editor editor = sharedPreferences.edit();

        editor.putString(Long.toString(System.currentTimeMillis()), message);
        editor.apply();
    }
}
