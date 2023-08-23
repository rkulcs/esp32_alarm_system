package alarm.system.server.api;

import com.google.firebase.messaging.FirebaseMessaging;
import com.google.firebase.messaging.FirebaseMessagingException;
import com.google.firebase.messaging.Message;
import com.google.firebase.messaging.Notification;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RestController;

@RestController
public class AlarmController {

    private final String ALARM_TOPIC = "alarm";

    private final FirebaseMessaging FIREBASE_MESSAGING;

    public AlarmController(FirebaseMessaging firebaseMessaging) {
        FIREBASE_MESSAGING = firebaseMessaging;
    }

    @PostMapping("/post-alarm")
    public ResponseEntity<String> postAlarm(@RequestBody String alarmMessage) throws FirebaseMessagingException {

        Notification notification = Notification.builder()
                .setTitle("Intruder detected")
                .build();

        Message message = Message.builder()
                .setTopic(ALARM_TOPIC)
                .setNotification(notification)
                .putData("body", alarmMessage)
                .build();
        String id = FIREBASE_MESSAGING.send(message);

        return ResponseEntity.status(HttpStatus.ACCEPTED).body(id);
    }
}
