/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_QUEUED_OUTPUT_STREAM_H
#define PURPLE_QUEUED_OUTPUT_STREAM_H

#include <gio/gio.h>

G_BEGIN_DECLS

#define PURPLE_TYPE_QUEUED_OUTPUT_STREAM  purple_queued_output_stream_get_type()

/**
 * PurpleQueuedOutputStream:
 *
 * An implementation of #GFilterOutputStream which allows queuing data for
 * output. This allows data to be queued while other data is being output.
 * Therefore, data doesn't have to be manually stored while waiting for
 * stream operations to finish.
 *
 * To create a queued output stream, use [ctor@QueuedOutputStream.new].
 *
 * To queue data, use [method@QueuedOutputStream.push_bytes_async].
 *
 * If there's a fatal stream error, it's suggested to clear the remaining bytes
 * queued with [method@QueuedOutputStream.clear_queue] to avoid excessive
 * errors returned in [method@QueuedOutputStream.push_bytes_async]'s async
 * callback.
 *
 * Since: 3.0.0
 */
G_DECLARE_FINAL_TYPE(PurpleQueuedOutputStream, purple_queued_output_stream,
                     PURPLE, QUEUED_OUTPUT_STREAM, GFilterOutputStream)

/**
 * purple_queued_output_stream_new:
 * @base_stream: Base output stream to wrap with the queued stream
 *
 * Creates a new queued output stream for a base stream.
 *
 * Returns: (transfer full): The new stream.
 *
 * Since: 3.0.0
 */
PurpleQueuedOutputStream *purple_queued_output_stream_new(GOutputStream *base_stream);

/**
 * purple_queued_output_stream_push_bytes_async:
 * @stream: The instance.
 * @bytes: The bytes to queue.
 * @priority: IO priority of the request.
 * @cancellable: (nullable): A [class@Gio.Cancellable] or %NULL.
 * @callback: (scope async): Callback to call when the request is finished.
 * @data: (closure): Data to pass to @callback.
 *
 * Asynchronously queues and then writes data to @stream. Once the data has
 * been written, or an error occurs, @callback will be called.
 *
 * Be careful such that if there's a fatal stream error, all remaining queued
 * operations will likely return this error. Use
 * [method@Purple.QueuedOutputStream.clear_queue] to clear the queue on such
 * an error to only report it a single time.
 *
 * Since: 3.0.0
 */
void purple_queued_output_stream_push_bytes_async(PurpleQueuedOutputStream *stream, GBytes *bytes, int priority, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);

/**
 * purple_queued_output_stream_push_bytes_finish:
 * @stream: The instance.
 * @result: The [iface@Gio.AsyncResult] of this operation.
 * @error: Return address for a #GError, or %NULL.
 *
 * Finishes pushing bytes asynchronously.
 *
 * Returns: %TRUE on success, %FALSE if there was an error
 *
 * Since: 3.0.0
 */
gboolean purple_queued_output_stream_push_bytes_finish(PurpleQueuedOutputStream *stream, GAsyncResult *result, GError **error);

/**
 * purple_queued_output_stream_clear_queue:
 * @stream: The instance.
 *
 * Clears the queue of any pending bytes. However, any bytes that are in the
 * process of being sent will finish their operation.
 *
 * This function is useful for clearing the queue in case of an IO error. Call
 * this in the async callback in order to clear the queue and avoid having all
 * [method@Purple.QueuedOutputStream.push_bytes_async] calls on @stream return
 * errors if there's a fatal stream error.
 *
 * Since: 3.0.0
 */
void purple_queued_output_stream_clear_queue(PurpleQueuedOutputStream *stream);

G_END_DECLS

#endif /* PURPLE_QUEUED_OUTPUT_STREAM_H */
