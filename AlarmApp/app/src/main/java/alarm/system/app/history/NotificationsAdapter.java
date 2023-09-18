package alarm.system.app.history;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import java.util.List;

import alarm.system.app.R;

/**
 * RecyclerView implementation for displaying alarm notifications.
 */
public class NotificationsAdapter extends RecyclerView.Adapter<NotificationsAdapter.ViewHolder> {

    private List<String> notifications;

    public static class ViewHolder extends RecyclerView.ViewHolder {

        private final TextView textView;

        public ViewHolder(@NonNull View itemView) {

            super(itemView);

            textView = (TextView) itemView.findViewById(R.id.textView);
        }

        public TextView getTextView() {
            return textView;
        }
    }

    public NotificationsAdapter(List<String> notifications) {
        this.notifications = notifications;
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {

        View view = LayoutInflater.from(parent.getContext())
                .inflate(R.layout.list_item, parent, false);

        return new ViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
        holder.getTextView().setText(notifications.get(position));
    }

    @Override
    public int getItemCount() {
        return notifications.size();
    }
}
