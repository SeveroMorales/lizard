/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * Purple is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
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

#ifndef PURPLE_SIGNALS_H
#define PURPLE_SIGNALS_H

#include <glib.h>
#include <glib-object.h>

/**
 * PurpleSignalMarshalFunc:
 * @cb: The #GCallback to call.
 * @args: The arguments to the function.
 * @data: Userdata to pass to @cb.
 * @return_val: (optional) (out): A return address for a return value.
 *
 * A generic function pointer type used to register signals.
 */
typedef void (*PurpleSignalMarshalFunc)(GCallback cb, va_list args,
									  void *data, void **return_val);

G_BEGIN_DECLS

/******************************************************************************
 * Signal API
 *****************************************************************************/

/**
 * PURPLE_SIGNAL_PRIORITY_DEFAULT:
 *
 * The priority of a signal connected using purple_signal_connect().
 *
 * See purple_signal_connect_priority()
 */
#define PURPLE_SIGNAL_PRIORITY_DEFAULT     0

/**
 * PURPLE_SIGNAL_PRIORITY_HIGHEST:
 *
 * The largest signal priority; signals with this priority will be called
 * <emphasis>last</emphasis>.  (This is highest as in numerical value, not as in
 * order of importance.)
 *
 * See purple_signal_connect_priority().
 */
#define PURPLE_SIGNAL_PRIORITY_HIGHEST  9999

/**
 * PURPLE_SIGNAL_PRIORITY_LOWEST:
 *
 * The smallest signal priority; signals with this priority will be called
 * <emphasis>first</emphasis>.  (This is lowest as in numerical value, not as in
 * order of importance.)
 *
 * See purple_signal_connect_priority().
 */
#define PURPLE_SIGNAL_PRIORITY_LOWEST  -9999

/**
 * purple_signal_register:
 * @instance:   The instance to register the signal for.
 * @signal:     The signal name.
 * @marshal: (scope notified):    The marshal function.
 * @ret_type:   The return type, or G_TYPE_NONE for no return type.
 * @num_values: The number of values to be passed to the callbacks.
 * @...:        The types of the parameters for the callbacks.
 *
 * Registers a signal in an instance.
 *
 * Returns: The signal ID local to that instance, or 0 if the signal
 *          couldn't be registered.
 */
gulong purple_signal_register(void *instance, const char *signal,
							PurpleSignalMarshalFunc marshal,
							GType ret_type, int num_values, ...);

/**
 * purple_signal_unregister:
 * @instance: The instance to unregister the signal for.
 * @signal:   The signal name.
 *
 * Unregisters a signal in an instance.
 */
void purple_signal_unregister(void *instance, const char *signal);

/**
 * purple_signals_unregister_by_instance:
 * @instance: The instance to unregister the signal for.
 *
 * Unregisters all signals in an instance.
 */
void purple_signals_unregister_by_instance(void *instance);

/**
 * purple_signal_connect_priority:
 * @instance: The instance to connect to.
 * @signal:   The name of the signal to connect.
 * @handle:   The handle of the receiver.
 * @func: (scope notified):    The callback function.
 * @data:     The data to pass to the callback function.
 * @priority: The priority with which the handler should be called. Signal
 *                 handlers are called in ascending numerical order of
 *                 @priority from #PURPLE_SIGNAL_PRIORITY_LOWEST to
 *                 #PURPLE_SIGNAL_PRIORITY_HIGHEST.
 *
 * Connects a signal handler to a signal for a particular object.
 *
 * Take care not to register a handler function twice. Purple will
 * not correct any mistakes for you in this area.
 *
 * See purple_signal_disconnect()
 *
 * Returns: The signal handler ID.
 */
gulong purple_signal_connect_priority(void *instance, const char *signal,
	void *handle, GCallback func, void *data, int priority);

/**
 * purple_signal_connect:
 * @instance: The instance to connect to.
 * @signal:   The name of the signal to connect.
 * @handle:   The handle of the receiver.
 * @func: (scope notified):    The callback function.
 * @data:     The data to pass to the callback function.
 *
 * Connects a signal handler to a signal for a particular object.
 * (Its priority defaults to 0, aka #PURPLE_SIGNAL_PRIORITY_DEFAULT.)
 *
 * Take care not to register a handler function twice. Purple will
 * not correct any mistakes for you in this area.
 *
 * See purple_signal_disconnect()
 *
 * Returns: The signal handler ID.
 */
gulong purple_signal_connect(void *instance, const char *signal,
	void *handle, GCallback func, void *data);

/**
 * purple_signal_disconnect:
 * @instance: The instance to disconnect from.
 * @signal:   The name of the signal to disconnect.
 * @handle:   The handle of the receiver.
 * @func: (scope call):    The registered function to disconnect.
 *
 * Disconnects a signal handler from a signal on an object.
 *
 * See purple_signal_connect()
 */
void purple_signal_disconnect(void *instance, const char *signal,
							void *handle, GCallback func);

/**
 * purple_signals_disconnect_by_handle:
 * @handle: The receiver handle.
 *
 * Removes all callbacks associated with a receiver handle.
 */
void purple_signals_disconnect_by_handle(void *handle);

/**
 * purple_signal_emit:
 * @instance: The instance emitting the signal.
 * @signal:   The signal being emitted.
 * @...:      The arguments to pass to the callbacks.
 *
 * Emits a signal.
 *
 * See purple_signal_connect(), purple_signal_disconnect()
 */
void purple_signal_emit(void *instance, const char *signal, ...);

/**
 * purple_signal_emit_return_1:
 * @instance: The instance emitting the signal.
 * @signal:   The signal being emitted.
 * @...:      The arguments to pass to the callbacks.
 *
 * Emits a signal and returns the first non-NULL return value.
 *
 * Further signal handlers are NOT called after a handler returns
 * something other than NULL.
 *
 * Returns: The first non-NULL return value
 */
void *purple_signal_emit_return_1(void *instance, const char *signal, ...);

/**
 * purple_signals_init:
 *
 * Initializes the signals subsystem.
 */
void purple_signals_init(void);

/**
 * purple_signals_uninit:
 *
 * Uninitializes the signals subsystem.
 */
void purple_signals_uninit(void);

/**************************************************************************/
/* Marshal Functions                                                      */
/**************************************************************************/

/**
 * purple_marshal_VOID:
 * @cb: (scope call): Callback this marshaller is designed to invoke
 * @args: va_list of arguments to be passed to the closure
 * @data: (nullable): Data to be passed to the callback
 * @return_val: (nullable): Value to store the callback's return value
 *
 * A purple marshaller function for use with signals with no arguments.
 */
void purple_marshal_VOID(
		GCallback cb, va_list args, void *data, void **return_val);

/**
 * purple_marshal_VOID__POINTER:
 * @cb: (scope call): Callback this marshaller is designed to invoke
 * @args: va_list of arguments to be passed to the closure
 * @data: (nullable): Data to be passed to the callback
 * @return_val: (nullable): Value to store the callback's return value
 *
 * A purple marshaller function for use with signals with a single pointer
 * argument.
 */
void purple_marshal_VOID__POINTER(
		GCallback cb, va_list args, void *data, void **return_val);

/**
 * purple_marshal_VOID__POINTER_UINT:
 * @cb: (scope call): Callback this marshaller is designed to invoke
 * @args: va_list of arguments to be passed to the closure
 * @data: (nullable): Data to be passed to the callback
 * @return_val: (nullable): Value to store the callback's return value
 *
 * A purple marshaller function for use with signals with a pointer and
 * an unsigned integer argument.
 */
void purple_marshal_VOID__POINTER_UINT(
		GCallback cb, va_list args, void *data, void **return_val);

/**
 * purple_marshal_VOID__POINTER_INT_INT:
 * @cb: (scope call): Callback this marshaller is designed to invoke
 * @args: va_list of arguments to be passed to the closure
 * @data: (nullable): Data to be passed to the callback
 * @return_val: (nullable): Value to store the callback's return value
 *
 * A purple marshaller function for use with signals with a pointer and
 * two integer arguments.
 */
void purple_marshal_VOID__POINTER_INT_INT(
		GCallback cb, va_list args, void *data, void **return_val);

/**
 * purple_marshal_VOID__POINTER_INT_POINTER:
 * @cb: (scope call): Callback this marshaller is designed to invoke
 * @args: va_list of arguments to be passed to the closure
 * @data: (nullable): Data to be passed to the callback
 * @return_val: (nullable): Value to store the callback's return value
 *
 * A purple marshaller function for use with signals with a pointer, an
 * integer, and then another pointer argument.
 */
void purple_marshal_VOID__POINTER_INT_POINTER(
		GCallback cb, va_list args, void *data, void **return_val);

/**
 * purple_marshal_VOID__POINTER_POINTER:
 * @cb: (scope call): Callback this marshaller is designed to invoke
 * @args: va_list of arguments to be passed to the closure
 * @data: (nullable): Data to be passed to the callback
 * @return_val: (nullable): Value to store the callback's return value
 *
 * A purple marshaller function for use with signals with two pointer
 * arguments.
 */
void purple_marshal_VOID__POINTER_POINTER(
		GCallback cb, va_list args, void *data, void **return_val);

/**
 * purple_marshal_VOID__POINTER_POINTER_UINT:
 * @cb: (scope call): Callback this marshaller is designed to invoke
 * @args: va_list of arguments to be passed to the closure
 * @data: (nullable): Data to be passed to the callback
 * @return_val: (nullable): Value to store the callback's return value
 *
 * A purple marshaller function for use with signals with two pointers
 * and an unsigned integer argument.
 */
void purple_marshal_VOID__POINTER_POINTER_UINT(
		GCallback cb, va_list args, void *data, void **return_val);

/**
 * purple_marshal_VOID__POINTER_POINTER_UINT_UINT:
 * @cb: (scope call): Callback this marshaller is designed to invoke
 * @args: va_list of arguments to be passed to the closure
 * @data: (nullable): Data to be passed to the callback
 * @return_val: (nullable): Value to store the callback's return value
 *
 * A purple marshaller function for use with signals with two pointers
 * and two unsigned integer arguments.
 */
void purple_marshal_VOID__POINTER_POINTER_UINT_UINT(
		GCallback cb, va_list args, void *data, void **return_val);

/**
 * purple_marshal_VOID__POINTER_UINT_UINT:
 * @cb: (scope call): Callback this marshaller is designed to invoke
 * @args: va_list of arguments to be passed to the closure
 * @data: (nullable): Data to be passed to the callback
 * @return_val: (nullable): Value to store the callback's return value
 *
 * A purple marshaller function for use with signals with a pointer and
 * two unsigned integer arguments.
 */
void purple_marshal_VOID__POINTER_UINT_UINT(
		GCallback cb, va_list args, void *data, void **return_val);

/**
 * purple_marshal_VOID__POINTER_POINTER_POINTER:
 * @cb: (scope call): Callback this marshaller is designed to invoke
 * @args: va_list of arguments to be passed to the closure
 * @data: (nullable): Data to be passed to the callback
 * @return_val: (nullable): Value to store the callback's return value
 *
 * A purple marshaller function for use with signals with three pointer
 * arguments.
 */
void purple_marshal_VOID__POINTER_POINTER_POINTER(
		GCallback cb, va_list args, void *data, void **return_val);

/**
 * purple_marshal_VOID__POINTER_POINTER_POINTER_POINTER:
 * @cb: (scope call): Callback this marshaller is designed to invoke
 * @args: va_list of arguments to be passed to the closure
 * @data: (nullable): Data to be passed to the callback
 * @return_val: (nullable): Value to store the callback's return value
 *
 * A purple marshaller function for use with signals with four pointer
 * arguments.
 */
void purple_marshal_VOID__POINTER_POINTER_POINTER_POINTER(
		GCallback cb, va_list args, void *data, void **return_val);

/**
 * purple_marshal_VOID__POINTER_POINTER_POINTER_POINTER_POINTER:
 * @cb: (scope call): Callback this marshaller is designed to invoke
 * @args: va_list of arguments to be passed to the closure
 * @data: (nullable): Data to be passed to the callback
 * @return_val: (nullable): Value to store the callback's return value
 *
 * A purple marshaller function for use with signals with five pointer
 * arguments.
 */
void purple_marshal_VOID__POINTER_POINTER_POINTER_POINTER_POINTER(
		GCallback cb, va_list args, void *data, void **return_val);

/**
 * purple_marshal_VOID__POINTER_POINTER_POINTER_UINT:
 * @cb: (scope call): Callback this marshaller is designed to invoke
 * @args: va_list of arguments to be passed to the closure
 * @data: (nullable): Data to be passed to the callback
 * @return_val: (nullable): Value to store the callback's return value
 *
 * A purple marshaller function for use with signals with three pointer
 * and one unsigned integer arguments.
 */
void purple_marshal_VOID__POINTER_POINTER_POINTER_UINT(
		GCallback cb, va_list args, void *data, void **return_val);

/**
 * purple_marshal_VOID__POINTER_POINTER_POINTER_POINTER_UINT:
 * @cb: (scope call): Callback this marshaller is designed to invoke
 * @args: va_list of arguments to be passed to the closure
 * @data: (nullable): Data to be passed to the callback
 * @return_val: (nullable): Value to store the callback's return value
 *
 * A purple marshaller function for use with signals with four pointer and
 * one unsigned integer arguments.
 */
void purple_marshal_VOID__POINTER_POINTER_POINTER_POINTER_UINT(
		GCallback cb, va_list args, void *data, void **return_val);

/**
 * purple_marshal_VOID__POINTER_POINTER_POINTER_UINT_UINT:
 * @cb: (scope call): Callback this marshaller is designed to invoke
 * @args: va_list of arguments to be passed to the closure
 * @data: (nullable): Data to be passed to the callback
 * @return_val: (nullable): Value to store the callback's return value
 *
 * A purple marshaller function for use with signals with three pointer
 * and two unsigned integer arguments.
 */
void purple_marshal_VOID__POINTER_POINTER_POINTER_UINT_UINT(
		GCallback cb, va_list args, void *data, void **return_val);

/**
 * purple_marshal_INT__POINTER_POINTER_POINTER:
 * @cb: (scope call): Callback this marshaller is designed to invoke
 * @args: va_list of arguments to be passed to the closure
 * @data: (nullable): Data to be passed to the callback
 * @return_val: (nullable): Value to store the callback's return value
 *
 * A purple marshaller function for use with signals with three pointer
 * arguments and returns an integer.
 */
void purple_marshal_INT__POINTER_POINTER_POINTER(
		GCallback cb, va_list args, void *data, void **return_val);

/**
 * purple_marshal_INT__POINTER_POINTER_POINTER_POINTER_POINTER:
 * @cb: (scope call): Callback this marshaller is designed to invoke
 * @args: va_list of arguments to be passed to the closure
 * @data: (nullable): Data to be passed to the callback
 * @return_val: (nullable): Value to store the callback's return value
 *
 * A purple marshaller function for use with signals with five pointer
 * arguments and returns an integer.
 */
void purple_marshal_INT__POINTER_POINTER_POINTER_POINTER_POINTER(
		GCallback cb, va_list args, void *data, void **return_val);

/**
 * purple_marshal_BOOLEAN__POINTER:
 * @cb: (scope call): Callback this marshaller is designed to invoke
 * @args: va_list of arguments to be passed to the closure
 * @data: (nullable): Data to be passed to the callback
 * @return_val: (nullable): Value to store the callback's return value
 *
 * A purple marshaller function for use with signals with a single pointer
 * argument and returns a boolean.
 */
void purple_marshal_BOOLEAN__POINTER(
		GCallback cb, va_list args, void *data, void **return_val);

/**
 * purple_marshal_BOOLEAN__POINTER_POINTER:
 * @cb: (scope call): Callback this marshaller is designed to invoke
 * @args: va_list of arguments to be passed to the closure
 * @data: (nullable): Data to be passed to the callback
 * @return_val: (nullable): Value to store the callback's return value
 *
 * A purple marshaller function for use with signals with two pointer
 * arguments and returns a boolean.
 */
void purple_marshal_BOOLEAN__POINTER_POINTER(
		GCallback cb, va_list args, void *data, void **return_val);

/**
 * purple_marshal_BOOLEAN__POINTER_BOOLEAN:
 * @cb: (scope call): Callback this marshaller is designed to invoke
 * @args: va_list of arguments to be passed to the closure
 * @data: (nullable): Data to be passed to the callback
 * @return_val: (nullable): Value to store the callback's return value
 *
 * A purple marshaller function for use with signals with a pointer and
 * boolean argument and returns a boolean.
 */
void purple_marshal_BOOLEAN__POINTER_BOOLEAN(
		GCallback cb, va_list args, void *data, void **return_val);

/**
 * purple_marshal_BOOLEAN__POINTER_POINTER_POINTER:
 * @cb: (scope call): Callback this marshaller is designed to invoke
 * @args: va_list of arguments to be passed to the closure
 * @data: (nullable): Data to be passed to the callback
 * @return_val: (nullable): Value to store the callback's return value
 *
 * A purple marshaller function for use with signals with three pointer
 * arguments and returns a boolean.
 */
void purple_marshal_BOOLEAN__POINTER_POINTER_POINTER(
		GCallback cb, va_list args, void *data, void **return_val);

/**
 * purple_marshal_BOOLEAN__POINTER_POINTER_UINT:
 * @cb: (scope call): Callback this marshaller is designed to invoke
 * @args: va_list of arguments to be passed to the closure
 * @data: (nullable): Data to be passed to the callback
 * @return_val: (nullable): Value to store the callback's return value
 *
 * A purple marshaller function for use with signals with two pointer and
 * one unsigned integer arguments and returns a boolean.
 */
void purple_marshal_BOOLEAN__POINTER_POINTER_UINT(
		GCallback cb, va_list args, void *data, void **return_val);

/**
 * purple_marshal_BOOLEAN__POINTER_POINTER_POINTER_POINTER:
 * @cb: (scope call): Callback this marshaller is designed to invoke
 * @args: va_list of arguments to be passed to the closure
 * @data: (nullable): Data to be passed to the callback
 * @return_val: (nullable): Value to store the callback's return value
 *
 * A purple marshaller function for use with signals with four pointer
 * arguments and returns a boolean.
 */
void purple_marshal_BOOLEAN__POINTER_POINTER_POINTER_POINTER(
		GCallback cb, va_list args, void *data, void **return_val);

/**
 * purple_marshal_BOOLEAN__POINTER_POINTER_POINTER_POINTER_POINTER:
 * @cb: (scope call): Callback this marshaller is designed to invoke
 * @args: va_list of arguments to be passed to the closure
 * @data: (nullable): Data to be passed to the callback
 * @return_val: (nullable): Value to store the callback's return value
 *
 * A purple marshaller function for use with signals with five pointer
 * arguments and returns a boolean.
 */
void purple_marshal_BOOLEAN__POINTER_POINTER_POINTER_POINTER_POINTER(
		GCallback cb, va_list args, void *data, void **return_val);

/**
 * purple_marshal_BOOLEAN__POINTER_POINTER_POINTER_POINTER_POINTER_POINTER:
 * @cb: (scope call): Callback this marshaller is designed to invoke
 * @args: va_list of arguments to be passed to the closure
 * @data: (nullable): Data to be passed to the callback
 * @return_val: (nullable): Value to store the callback's return value
 *
 * A purple marshaller function for use with signals with six pointer
 * arguments and returns a boolean.
 */
void purple_marshal_BOOLEAN__POINTER_POINTER_POINTER_POINTER_POINTER_POINTER(
		GCallback cb, va_list args, void *data, void **return_val);

G_END_DECLS

#endif /* PURPLE_SIGNALS_H */
