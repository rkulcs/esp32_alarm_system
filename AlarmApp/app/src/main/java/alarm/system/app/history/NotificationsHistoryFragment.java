package alarm.system.app.history;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.fragment.app.Fragment;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import java.util.ArrayList;
import java.util.List;

import alarm.system.app.R;

/**
 * Fragment for displaying alarm notifications as a list.
 */
public class NotificationsHistoryFragment extends Fragment {

    private RecyclerView recyclerView;
    private NotificationsAdapter notificationsAdapter;
    private RecyclerView.LayoutManager layoutManager;

    private List<String> notifications;

    @Override
    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);

        // TODO: Load notifications
        notifications = new ArrayList<>();
        notifications.add("test");
    }

    public View onCreateView(LayoutInflater inflater, ViewGroup viewGroup,
                             Bundle savedInstanceState) {

        View view = inflater.inflate(R.layout.recycler_view_fragment, viewGroup, false);
        recyclerView = (RecyclerView) view.findViewById(R.id.recyclerView);

        layoutManager = new LinearLayoutManager(getActivity());
        recyclerView.setLayoutManager(layoutManager);

        notificationsAdapter = new NotificationsAdapter(notifications);
        recyclerView.setAdapter(notificationsAdapter);

        return view;
    }
}
