package alarm.system.server.config;

import com.google.auth.oauth2.GoogleCredentials;
import com.google.firebase.FirebaseApp;
import com.google.firebase.FirebaseOptions;
import com.google.firebase.messaging.FirebaseMessaging;
import org.springframework.boot.context.properties.EnableConfigurationProperties;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

import java.io.IOException;
import java.io.InputStream;

@Configuration
@EnableConfigurationProperties(FirebaseProperties.class)
public class FirebaseConfig {

    private final FirebaseProperties PROPERTIES;

    public FirebaseConfig(FirebaseProperties properties) {
        PROPERTIES = properties;
    }

    @Bean
    public GoogleCredentials googleCredentials() {

        try (InputStream inputStream = PROPERTIES.getServiceAccount().getInputStream()) {
            return GoogleCredentials.fromStream(inputStream);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    @Bean
    public FirebaseApp firebaseApp(GoogleCredentials credentials) {

        return FirebaseApp.initializeApp(
                FirebaseOptions.builder().setCredentials(credentials).build()
        );
    }

    @Bean
    public FirebaseMessaging firebaseMessaging(FirebaseApp app) {
        return FirebaseMessaging.getInstance(app);
    }
}
