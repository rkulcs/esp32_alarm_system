package alarm.system.server.config;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Component;

@Component
public class AuthURL {

    @Value("${google.identity.kit.api.url}")
    private String GOOGLE_AUTH_BASE_URL;

    @Value("${firebase.app.api.key}")
    private String FIREBASE_APP_API_KEY;

    private String value = null;

    public String get() {

        if (value == null)
            value = GOOGLE_AUTH_BASE_URL + FIREBASE_APP_API_KEY;

        return value;
    }
}
