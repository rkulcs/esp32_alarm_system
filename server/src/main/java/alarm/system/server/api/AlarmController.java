package alarm.system.server.api;

import alarm.system.server.auth.Authenticator;
import com.google.firebase.messaging.FirebaseMessaging;
import com.google.firebase.messaging.FirebaseMessagingException;
import com.google.firebase.messaging.Message;
import com.google.firebase.messaging.Notification;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RestController;

@RestController
public class AlarmController {

    private final String ALARM_TOPIC = "alarm";

    private final FirebaseMessaging FIREBASE_MESSAGING;

    @Autowired
    private Authenticator authenticator;

    public AlarmController(FirebaseMessaging firebaseMessaging) {
        FIREBASE_MESSAGING = firebaseMessaging;
    }

    @PostMapping("/login")
    public ResponseEntity<String> login(@RequestBody String loginDetails) {

        try {
            String token = authenticator.requestTokenFromGIT(loginDetails);
            return ResponseEntity.ok(token);
        } catch (RuntimeException e) {
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).body(e.getMessage());
        }
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
