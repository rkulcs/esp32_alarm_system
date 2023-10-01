package alarm.system.app.history;

import static android.content.Context.MODE_PRIVATE;
import static android.media.CamcorderProfile.getAll;

import android.annotation.SuppressLint;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;

import androidx.annotation.RequiresApi;
import androidx.fragment.app.Fragment;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import java.time.Instant;
import java.time.ZoneId;
import java.time.format.DateTimeFormatter;
import java.time.format.FormatStyle;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;

import alarm.system.app.R;

/**
 * Fragment for displaying alarm notifications as a list.
 */
public class NotificationsHistoryFragment extends Fragment {

    private RecyclerView recyclerView;
    private NotificationsAdapter notificationsAdapter;
    private RecyclerView.LayoutManager layoutManager;

    private List<String> notifications;

    @RequiresApi(api = Build.VERSION_CODES.O)
    @Override
    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);

        notifications = new ArrayList<>();
        loadNotificationsHistory();
    }

    @RequiresApi(api = Build.VERSION_CODES.O)
    public View onCreateView(LayoutInflater inflater, ViewGroup viewGroup,
                             Bundle savedInstanceState) {

        View view = inflater.inflate(R.layout.recycler_view_fragment, viewGroup, false);
        recyclerView = (RecyclerView) view.findViewById(R.id.recyclerView);

        layoutManager = new LinearLayoutManager(getActivity());
        recyclerView.setLayoutManager(layoutManager);

        notificationsAdapter = new NotificationsAdapter(notifications);
        recyclerView.setAdapter(notificationsAdapter);

        Button clearButton = view.findViewById(R.id.clearButton);
        clearButton.setOnClickListener(v -> {
            clearNotificationsHistory();
        });

        return view;
    }

    @RequiresApi(api = Build.VERSION_CODES.O)
    private void loadNotificationsHistory() {

        SharedPreferences sharedPreferences = getContext().getSharedPreferences(
                getString(R.string.notifications_history_file), MODE_PRIVATE
        );

        Set<String> notificationTimes = sharedPreferences.getAll().keySet();
        DateTimeFormatter formatter = DateTimeFormatter.ofLocalizedDateTime(FormatStyle.MEDIUM);

        for (String time : notificationTimes)
        {
            String formattedTime = formatter.format(Instant.ofEpochMilli(
                    Long.parseLong(time)).atZone(ZoneId.systemDefault()).toLocalDateTime()
            );

            String message = sharedPreferences.getString(time, "No message");
            notifications.add(String.format("%s\n%s", formattedTime, message));
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.O)
    private void clearNotificationsHistory() {

        SharedPreferences sharedPreferences = getContext().getSharedPreferences(
                getString(R.string.notifications_history_file), MODE_PRIVATE
        );

        int numNotifications = sharedPreferences.getAll().size();

        sharedPreferences.edit().clear().apply();
        notifications.clear();
        recyclerView.getAdapter().notifyItemRangeRemoved(0, numNotifications);
    }
}
