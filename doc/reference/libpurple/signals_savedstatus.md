Title: Saved Status Signals
Slug: saved-status-signals

## Saved Status Signals

### Signal List

* [savedstatus-changed](#savedstatus-changed)

### Signal Details

#### savedstatus-changed

```c
void user_function(PurpleSavedStatus *new,
                   PurpleSavedStatus *old,
                   gpointer user_data);
```

Emitted when a new saved status is activated.
