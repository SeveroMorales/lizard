Title: File Transfer Signals
Slug: file-transfer-signals

## File Transfer Signals

### Signal List

* [file-recv-request](#file-recv-request)

### Signal Detail

#### file-recv-request

```c
void user_function(PurpleXfer *xfer, gpointer data);
```

Emitted before the user is prompted for an incoming file-transfer. Plugins can intercept the signal to auto-accept/auto-reject the requests. To auto-accept the file transfer, use purple_xfer_request_accepted(). To auto-reject, set the status of the xfer to PURPLE_XFER_STATUS_CANCEL_LOCAL.

**Parameters:**

**xfer**
: The file transfer.

**data**
: User data.
