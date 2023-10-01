package alarm.system.app.history;

import static android.content.Context.MODE_PRIVATE;
import static android.media.CamcorderProfile.getAll;

import android.annotation.SuppressLint;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;

import androidx.annotation.RequiresApi;
import androidx.fragment.app.Fragment;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import java.time.Instant;
import java.time.ZoneId;
import java.time.format.DateTimeFormatter;
import java.time.format.FormatStyle;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Comparator;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.stream.Collectors;

import alarm.system.app.NotificationsService;
import alarm.system.app.R;
import alarm.system.app.util.IntentExtraNames;

/**
 * Fragment for displaying alarm notifications as a list.
 */
public class NotificationsHistoryFragment extends Fragment {

    private RecyclerView recyclerView;
    private NotificationsAdapter notificationsAdapter;
    private RecyclerView.LayoutManager layoutManager;

    private List<String> notifications;

    private BroadcastReceiver broadcastReceiver;

    private DateTimeFormatter formatter;

    private SharedPreferences sharedPreferences;

    @RequiresApi(api = Build.VERSION_CODES.O)
    @Override
    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        formatter = DateTimeFormatter.ofLocalizedDateTime(FormatStyle.MEDIUM);
        notifications = new ArrayList<>();

        sharedPreferences = getContext().getSharedPreferences(
                getString(R.string.notifications_history_file), MODE_PRIVATE
        );

        loadNotificationsHistory();
        initBroadcastReceiver();
    }

    @Override
    public void onStart() {

        super.onStart();

        LocalBroadcastManager.getInstance(getActivity()).registerReceiver(
                broadcastReceiver,
                new IntentFilter(NotificationsService.NOTIFICATION_SERVICE)
        );
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

    private void initBroadcastReceiver() {

        broadcastReceiver = new BroadcastReceiver() {
            @RequiresApi(api = Build.VERSION_CODES.O)
            @Override
            public void onReceive(Context context, Intent intent) {

                Bundle extras = intent.getExtras();
                String time = (String) extras.get(IntentExtraNames.TIME);
                String message = (String) extras.get(IntentExtraNames.MESSAGE);
                addNotificationToHistory(time, message);
            }
        };
    }

    @RequiresApi(api = Build.VERSION_CODES.O)
    private void loadNotificationsHistory() {

        List<String> notificationTimes = sharedPreferences.getAll()
                .keySet().stream().sorted(Comparator.reverseOrder())
                .collect(Collectors.toList());

        for (String time : notificationTimes)
        {
            String message = sharedPreferences.getString(time, "No message");
            notifications.add(formatNotificationHistoryEntry(time, message));
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.O)
    private String formatNotificationHistoryEntry(String time, String message) {

        String formattedTime = formatter.format(Instant.ofEpochMilli(
                Long.parseLong(time)).atZone(ZoneId.systemDefault()).toLocalDateTime()
        );

        return String.format("%s\n%s", formattedTime, message);
    }

    @RequiresApi(api = Build.VERSION_CODES.O)
    private void addNotificationToHistory(String time, String message) {

        sharedPreferences.edit().putString(time, message).apply();

        String newEntry = formatNotificationHistoryEntry(time, message);
        notifications.add(0, newEntry);
        recyclerView.getAdapter().notifyItemInserted(0);
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
