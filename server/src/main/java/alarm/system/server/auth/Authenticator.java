package alarm.system.server.auth;

import alarm.system.server.config.AuthURL;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Component;

import java.io.IOException;
import java.net.ProxySelector;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;

@Component
public class Authenticator {

    @Autowired
    private AuthURL authURL;

    // The expected index of the token in the array of strings after splitting the response
    private final int TOKEN_INDEX_IN_RESPONSE = 4;

    // The expected index of the token in a key-value pair after splitting it into two strings
    private final int TOKEN_INDEX_IN_KEY_VALUE_PAIR = 1;

    public String requestTokenFromGIT(String loginDetailsJSON) {

        String modifiedBody = loginDetailsJSON.replace("}",
                ", \"returnSecureToken\": true }");

        try {
            HttpRequest request = HttpRequest.newBuilder()
                    .uri(new URI(authURL.get()))
                    .headers("Content-Type", "application/json")
                    .POST(HttpRequest.BodyPublishers.ofString(modifiedBody))
                    .build();

            HttpResponse<String> response = HttpClient.newBuilder()
                    .proxy(ProxySelector.getDefault())
                    .build()
                    .send(request, java.net.http.HttpResponse.BodyHandlers.ofString());

            return extractTokenFromResponse(response);
        } catch (URISyntaxException e) {
            throw new RuntimeException("Invalid Google Identity Toolkit URL.");
        } catch (IOException | InterruptedException e) {
            throw new RuntimeException("Failed to log in.");
        }
    }

    private String extractTokenFromResponse(HttpResponse<String> response) {

        String[] responseComponents = response.body().split(",");
        String token = responseComponents[TOKEN_INDEX_IN_RESPONSE]
                .split(":")[TOKEN_INDEX_IN_KEY_VALUE_PAIR]
                .trim().replace("\"", "");

        return token;
    }
}
